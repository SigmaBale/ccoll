/*
 * Note:
 * This is not the fastest implementation of self balanced binary tree.
 * This is recursive approach to the avl tree data structure.
 * However, it is possible that the implementation might
 * be rewritten with iterative approach.
 *
 * About thread safety...
 * Having mix and match of which functions are thread safe and which
 * ones are not would be the ideal solution.
 * Only read operations can be thread safe or
 * operations that increment/decrement a simple counter
 * or set/unset a flag using atomic increment/decrement.
 * These operations should be the only ones that can be safely
 * called from any thread.
 *
 * Operations that affect tree balance won't benefit from
 * the multithreading anyways, if the balance changes when
 * insertion '1' is done inserting then the conccurent insertion '2'
 * must start all the way from the root because tree might have been rebalanced.
 */

#define __COL_TREE_C_FILE__
#include "ctree.h"
#undef __COL_TREE_C_FILE__
#define __COL_C_FILE__
#include "cerror.h"
#undef __COL_C_FILE__
#define __COL_SRC_FILE__
#include "citer.h"
#undef __COL_SRC_FILE__

#include <assert.h>
#include <memc.h>
#include <memory.h>
#include <stdatomic.h>
#include <stdbool.h>

/*
 * Group of flags that are used for state checking inside
 * nested recursion calls.
 *
 * Flag 'removed' is set each time the key (and the value) get
 * removed from the tree, meaning they actually have to be inside
 * the tree to get removed.
 * If the key is not found in the tree, then the flag is not set,
 * and we don't need to update the tree during recursion traceback.
 *
 * Flag 'inserted' gets set whenever new key gets inserted.
 * This way we can optimize when to update nodes and
 * rebalance the tree during recursion traceback.
 * If the key is not unique and is found in the tree,
 * then the value will get updated and key reference but
 * the count and balance won't change therefore 'inserted'
 * will not be set.
 *
 * Flag 'replaced' gets set whenever the key gets replaced.
 * Detecting the key getting replaced can be only done by
 * setting this flag inside the ctreenode_insert function
 * inside nested recursion.
 */

typedef u_char byte;

#ifndef COL_BYTE
#define COL_BYTE (byte) 0
#endif

typedef enum {
    REMOVED  = 1 << 0,
    INSERTED = 1 << 1,
    REPLACED = 1 << 2,
} _opflags;

/*
 * Core private structure that represents the Node (key)
 * stored inside the CTree.
 *
 * It must contain valid non-NULL key in order to even be constructed,
 * Value is allowed to be NULL, it is the same reason the library
 * provides ctree_key_entry function for direct access to the value.
 *
 * Node parameters (height and balance factor) are used in tree rebalancing,
 * for example if balance factor [ BF > 1 or BF < -1 ] then the avl tree
 * invariant is violated.
 * If so then the tree rebalances itself until the balance factor
 * of all nodes is one of the values of the given set {-1, 0, 1}
 */
struct ctree_node {
    cptr_t key;
    cptr_t value;
    uint   height;
    int    balance;

    struct ctree_node* right;
    struct ctree_node* left;
    struct ctree_node* parent;
};

/*
 * CTree is AVL tree (self-balancing binary search tree).
 * Comparisons are done based on the comparison function provided upon
 * constructing the CTree ('CCompareKeyFn').
 * Comparison function must be provided in order to construct the tree.
 *
 * Free key function is used on keys each time they are removed from the tree.
 * ('CFreeKeyFn').
 * If no key removal function is provided then the tree will not free
 * the underlying memory of the key passed in when it gets removed
 * from the tree.
 * Meaning the user is responsible for freeing that memory in that
 * specific case.
 *
 * Same goes for the free value function, when the value gets
 * replaced or key gets removed, the value won't be freed unless
 * user provided 'CFreeValueFn'.
 */
struct ctree {
    ctree_node* root;

    CCompareKeyFn compare_key_fn;
    CFreeKeyFn    free_key_fn;
    CFreeValueFn  free_value_fn;
    CClone        clone_key_fn;
    CClone        clone_value_fn;

    atomic_uint size;
    byte        flags;
};

/*
 * CTree constructor
 * 'compare_key_fn' is required.
 * 'free_key_fn' is optional, if user provides free_key_fn
 * then the key that gets removed gets also freed as specified
 * in the implementation of free_key_fn, if it is NULL then the
 * user is responsible for freeing the key memory.
 */
ctree*
ctree_new(CCompareKeyFn compare_key_fn,
          CFreeKeyFn    free_key_fn,
          CFreeValueFn  free_value_fn,
          CClone        clone_key_fn,
          CClone        clone_value_fn)
{
    ctree* tree = memc_malloc(ctree);

#ifndef COL_MEMORY_CONSTRAINED
    if(__builtin_expect(tree == NULL, 0)) {
#else
    if(tree != NULL) {
#endif
        COL_ALLOC_ERROR;
        return NULL;
    }

    if((tree->compare_key_fn = compare_key_fn) == NULL) {
        COL_INVALID_CMPFN_ERROR;
        free(tree);
        return NULL;
    }

    tree->clone_key_fn   = clone_key_fn;
    tree->clone_value_fn = clone_value_fn;
    tree->free_key_fn    = free_key_fn;
    tree->free_value_fn  = free_value_fn;
    tree->root           = NULL;
    tree->size           = 0;
    tree->flags          = COL_BYTE;

    return tree;
}

/*
 * CTreeNode constructor
 */
static ctree_node*
_ctreenode_new(cptr_t key, cptr_t value, ctree_node* parent)
{
    ctree_node* node;

#ifndef COL_MEMORY_CONSTRAINED
    if(__builtin_expect((node = memc_malloc(ctree_node)) == NULL, 0) || (node->key = key) == NULL) {
#else
    if((node = memc_malloc(cnode)) == NULL || (node->key = key) == NULL) {
#endif
#ifndef COL_MEMORY_CONSTRAINED
        if(__builtin_expect(node == NULL, 0)) {
#else
        if(node == NULL) {
#endif
            COL_ALLOC_ERROR;
        } else {
            COL_INVALID_KEY_ERROR;
            free(node);
        }
        return NULL;
    }

    node->value   = value;
    node->height  = 0;
    node->balance = 0;
    node->right   = NULL;
    node->left    = NULL;
    node->parent  = parent;

    return node;
}

/*
 * CTreeNode destructor
 */
void
ctree_node_drop(ctree_node** nodep)
{
    ctree_node* node;
    if(nodep != NULL && (node = *nodep) != NULL) {
        node->height  = 0;
        node->balance = 0;
        node->right   = NULL;
        node->left    = NULL;
        node->key     = NULL;
        node->value   = NULL;
        node->parent  = NULL;
        *nodep        = NULL;
        free(node);
    }
}

/*
 * Helper function for CTree destructor.
 * Recursively frees all the nodes
 */
static void
_ctreenode_freeall(ctree* tree, ctree_node* node)
{
    if(node != NULL) {
        _ctreenode_freeall(tree, node->left);
        _ctreenode_freeall(tree, node->right);

        if(tree->free_key_fn)
            tree->free_key_fn(node->key);

        if(tree->free_value_fn)
            tree->free_value_fn(node->value);

        tree->size--;
        ctree_node_drop(&node);
    }
}

/*
 * Updates the CTreeNode height and balance factor
 */
static void
_ctreenode_update(ctree_node* node)
{
    int left      = (node->left != NULL) ? (int) node->left->height : -1;
    int right     = (node->right != NULL) ? (int) node->right->height : -1;
    node->height  = ((left > right) ? left : right) + 1;
    node->balance = right - left;
}

/*
 * Rotation function, performs right rotation, after rotation
 * it updates the pivot and root node (height and balance factor)
 */
static ctree_node*
_ctreenode_rotate_right(ctree_node* node)
{
    ctree_node* new_root = node->left;
    node->left           = new_root->right;
    new_root->right      = node;
    _ctreenode_update(node);
    _ctreenode_update(new_root);
    return new_root;
}

/*
 * Rotation function, performs left rotation, after rotation
 * it updates the pivot and root node (height and balance factor)
 */
static ctree_node*
_ctreenode_rotate_left(ctree_node* node)
{
    ctree_node* new_root = node->right;
    node->right          = new_root->left;
    new_root->left       = node;
    _ctreenode_update(node);
    _ctreenode_update(new_root);
    return new_root;
}

/*
 * Rotation function, performs multiple rotations by first rotating the right
 * child of the root by applying the right rotation on it, then rotates the root
 * and pivot (new right child) by applying left rotation
 */
static ctree_node*
_ctreenode_rotate_right_left(ctree_node* node)
{
    node->right = _ctreenode_rotate_right(node->right);
    return _ctreenode_rotate_left(node);
}

/*
 * Rotation function, performs multiple rotations by first rotating the left
 * child of the root by applying the left rotation on it, then rotates the root
 * and pivot (new left child) by applying right rotation
 */
static ctree_node*
_ctreenode_rotate_left_right(ctree_node* node)
{
    node->left = _ctreenode_rotate_left(node->left);
    return _ctreenode_rotate_right(node);
}

/*
 * Rebalancing function, initiates rotations only
 * if the node balance factor is not part of the
 * {-1, 0, 1} set.
 */
static ctree_node*
_ctreenode_rebalance(ctree_node* node)
{
    if(node->balance < -1) {
        // AVL tree invariant must not be broken
        assert(node->balance == -2);
        return (node->left->balance <= 0) ? _ctreenode_rotate_right(node)
                                          : _ctreenode_rotate_left_right(node);
    } else if(node->balance > 1) {
        // AVL tree invariant must not be broken
        assert(node->balance == 2);
        return (node->right->balance >= 0) ? _ctreenode_rotate_left(node)
                                           : _ctreenode_rotate_right_left(node);
    }
    return node;
}

/*
 * Finds the max key value in the left subtree
 * of the passed in 'CTreeNode'
 */
static ctree_node*
_ctreenode_find_right(ctree_node* node)
{
    while(node->left != NULL)
        node = node->left;

    return node;
}

/*
 * Finds the min key value in the right subtree
 * of the passed in 'CTreeNode'
 */
static ctree_node*
_ctreenode_find_left(ctree_node* node)
{
    while(node->right != NULL)
        node = node->right;

    return node;
}

/*
 * Internal recursive BST insertion function.
 * It updates the node parameters and does the
 * recursive tree rebalancing.
 */
static ctree_node*
_ctreenode_insert(ctree*      tree,
                  ctree_node* parent,
                  ctree_node* node,
                  cptr_t      key,
                  cptr_t      value,
                  bool        replace)
{
    int cmp;

#ifndef COL_MEMORY_CONSTRAINED
    if(node == NULL && __builtin_expect((node = _ctreenode_new(key, value, parent)) != NULL, 1)) {
#else
    if(node == NULL && (node = ctreenode_new(key, value)) != NULL) {
#endif
        tree->size++;
        tree->flags |= INSERTED;
    } else if((cmp = tree->compare_key_fn(node->key, key)) > 0) {
        node->left = _ctreenode_insert(tree, node, node->left, key, value, replace);
    } else if(cmp < 0) {
        node->right = _ctreenode_insert(tree, node, node->right, key, value, replace);
    } else {

        if(tree->free_value_fn)
            tree->free_value_fn(node->value);

        node->value = value;

        if(replace) {
            tree->flags |= REPLACED;

            if(tree->free_key_fn)
                tree->free_key_fn(node->key);

            node->key = key;
        }
    }

    if(tree->flags & INSERTED) {
        _ctreenode_update(node);
        return _ctreenode_rebalance(node);
    }

    return node;
}

/*
 * Recursively searches for the key, if the key is found
 * remove it, check if the 'CFreeKeyFn' is present, if
 * it is call the function on the key, freeing it.
 * If the 'CFreeValueFn' is present call it on the value,
 * freeing it.
 * If either one or both of the functions are not present,
 * then the user is responsible for freeing the value and/or key.
 */
static ctree_node*
_ctreenode_remove(ctree* tree, ctree_node* node, cptr_t key, bool return_ele)
{
    if(node == NULL)
        return NULL;

    int cmp = tree->compare_key_fn(node->key, key);

    if(cmp > 0) {
        node->left = _ctreenode_remove(tree, node->left, key, return_ele);
    } else if(cmp < 0) {
        node->right = _ctreenode_remove(tree, node->right, key, return_ele);
    } else {
        ctree_node* temp;

        if(node->right == NULL || node->left == NULL) {
            temp = (node->right != NULL) ? node->right : node->left;

            // If we freed the value in some recursion call
            // before this one, then don't try to free again!
            if(!(tree->flags & REMOVED)) {
                (tree->free_key_fn) ? tree->free_key_fn(node->key) : 0;
                (tree->free_value_fn) ? tree->free_value_fn(node->value) : 0;
                tree->flags |= REMOVED;
            }

            ctree_node_drop(&node);

            tree->size--;

            if(temp == NULL)
                return NULL;
            else
                return temp;

        } else {
            if(tree->free_key_fn)
                tree->free_key_fn(node->key);

            if(tree->free_value_fn)
                tree->free_value_fn(node->value);

            // Set the flag so we can check in the upper case
            // if the flag is set, so we don't free the value
            // that will get assigned to this current node
            tree->flags |= REMOVED;

            if(node->left->height > node->right->height) {
                temp        = _ctreenode_find_left(node->left);
                node->key   = temp->key;
                node->value = temp->value;
                node->left  = _ctreenode_remove(tree, node->left, node->key, return_ele);
            } else {
                temp        = _ctreenode_find_right(node->right);
                node->key   = temp->key;
                node->value = temp->value;
                node->right = _ctreenode_remove(tree, node->right, node->key, return_ele);
            }
        }
    }

    // During traceback only update if we removed the node
    if(tree->flags & REMOVED) {
        _ctreenode_update(node);
        return _ctreenode_rebalance(node);
    }

    return node;
}

/*
 * CTree recursive BST insertion function
 * Returns false if no insertion occured,
 * meaning either tree is NULL or the same key
 * was already in the tree, and only the value got
 * updated, but the key was not inserted.
 *
 * together with the value, returning true.
 * If the key was not found then the key is inserted
 */
bool
ctree_insert(ctree* tree, cptr_t key, cptr_t value)
{
    return_val_if_fail(tree != NULL, false);

    tree->root = _ctreenode_insert(tree, NULL, tree->root, key, value, false);

    if(tree->flags & INSERTED) {
        tree->flags &= ~(COL_BYTE | INSERTED);
        return true;
    }

    return false;
}

/*
 * Replaces the key in the tree with the provided key.
 * Returns false if no insertion occured,
 * meaning either tree is NULL or the key was not found.
 * If the key was found then the value gets updated and
 * old one gets freed if the 'CFreeValueFn' is present.
 * Old key gets replaced with the new key and freed if the
 * 'CFreeKeyFn' is present.
 */
bool
ctree_replace(ctree* tree, cptr_t key, cptr_t value)
{
    return_val_if_fail(tree != NULL, false);

    tree->root = _ctreenode_insert(tree, NULL, tree->root, key, value, true);

    if(tree->flags & REPLACED) {
        tree->flags &= ~(COL_BYTE | REPLACED);
        return true;
    }

    return false;
}

/*
 * Removes the given key from the tree.
 * If the key was found and removed it returns true.
 * If no matching key was found and no removal occured
 * returns false.
 * Additionally if tree or key are invalid (NULL) it
 * returns false.
 */
bool
ctree_remove(ctree* tree, cptr_t key, bool return_ele)
{
    return_val_if_fail((tree != NULL && key != NULL), false);

    tree->root = _ctreenode_remove(tree, tree->root, key, return_ele);

    if(tree->flags & REMOVED) {
        tree->flags &= ~(COL_BYTE | REMOVED);
        return true;
    } else
        return false;
}

/*
 * Internal function, tries to find the key in tree.
 * Returns NULL if key was not found or the pointer
 * to the key if it was found.
 */
cptr_t
_ctreenode_find(ctree* tree, cptr_t key)
{
    return_val_if_fail(tree != NULL, NULL);

    int         cmp;
    ctree_node* current = tree->root;

    while(current != NULL) {
        if((cmp = tree->compare_key_fn(current->key, key)) == 0) {
            return current;
        } else if(cmp > 0)
            current = current->left;
        else
            current = current->right;
    }

    return NULL;
}

/*
 * Returns pointer to the value of the key/value pair.
 * Returns NULL if key is not inside the tree.
 */
cptr_t
ctree_entry(ctree* tree, cptr_t key)
{
    return_val_if_fail(tree != NULL, NULL);

    ctree_node* entry;

    if((entry = _ctreenode_find(tree, key)) != NULL)
        return entry->value;

    return NULL;
}

/*
 * Returns pointer to the key of the key/value pair.
 * Returns NULL if key is not inside the tree.
 */
cptr_t
ctree_key(ctree* tree, cptr_t key)
{
    return_val_if_fail(tree != NULL, NULL);

    ctree_node* entry;

    if((entry = _ctreenode_find(tree, key)) != NULL)
        return entry->key;

    return NULL;
}

/*
 * Returns the total node count in the CTree.
 */
uint
ctree_size(ctree* tree)
{
    return tree->size;
}

/*
 * Returns the total size of the CTree (size of all nodes) in bytes.
 * This represents only the size of nodes not the actual allocation size of
 * key and value pairs (if they are malloc'ed).
 */
size_t
ctree_size_bytes(ctree* tree)
{
    return tree->size * sizeof(ctree_node);
}

/*
 * Drops the 'ctree' freeing the wrapper, but the nodes are not freed.
 * Pointer to the wrapper also gets nulled.
 */
void
ctree_drop(ctree** treep, bool free_tree)
{
    ctree* tree;
    if(treep != NULL && (tree = *treep) != NULL) {
        tree->root  = NULL;
        tree->size  = 0;
        tree->flags = COL_BYTE;
        *treep      = NULL;
        if(free_tree)
            free(tree);
    }
}

/*
 * Finds the smallest node given the root 'node'.
 */
static ctree_node*
_ctree_min(const ctree_node* node)
{
    return_val_if_fail(node != NULL, NULL);

    while(node->left != NULL)
        node = node->left;

    return (ctree_node*) node;
}

/*
 * Finds the largest node given the root 'node'.
 */
static ctree_node*
_ctree_max(const ctree_node* node)
{
    return_val_if_fail(node != NULL, NULL);

    while(node->right != NULL)
        node = node->right;

    return (ctree_node*) node;
}

//
//
//
//
/****************************************************************************/
/*                        BI-DIRECTIONAL ITERATORS                          */
/****************************************************************************/

/*
 * Private enum used in simple algorithm for traversing the tree.
 *
 * 'UP' is set each time if starting from 'start' (front) when 'node' is part
 * of the right subtree and the next successor is its 'parent' node.
 * 'UP' is also set each time when starting from 'end' (back) when 'node' is
 * part of the left subtree and the next successor is its 'parent' node.
 * This prevents cycles.
 *
 * 'RESET' is set in every other case, for example if we are iterating from
 * start (front) and the node was part of the left subtree and its successor
 * is its parent.
 */
typedef enum _ctree_iter_state {
    UP,
    RESET,
} _ctree_iter_state;

/*
 * Check function definition below for info.
 */
static ctree_node*
_ctree_node_next(const ctree_node* node, _ctree_iter_state* state, CCompareKeyFn key_cmp_fn);

/*
 * Check function definition below for info.
 */
static ctree_node*
_ctree_node_prev(const ctree_node* node, _ctree_iter_state* state, CCompareKeyFn key_cmp_fn);

/*
 * This private struct holds iteration state for both the 'start' and 'end'.
 * Meaning that calling 'ctree_iter_next' and 'ctree_iter_next_back' won't mix
 * their states together instead they each have their own state.
 */
typedef struct {
    _ctree_iter_state start;
    _ctree_iter_state end;
} _ctree_iter_states;

/*
 * Default initializator of '_ctree_iter_states' struct.
 * Both states are set to 'RESET' for obvious reasons.
 */
_ctree_iter_states
_ctree_iter_states_default(void)
{
    return (_ctree_iter_states) {
        .start = RESET,
        .end   = RESET,
    };
}

/*
 * Bi-directional Non-consuming iterator.
 * Nodes having 'parent' member is what allows this iterator to have O(1) space
 * complexity and O(1) time complexity when fetching next node.
 *
 * Note:
 * 'CCompareKeyFn' is also part of the algorithm for traversing our iterator,
 * it is used in case where we compare our nodes key with the parents right node key,
 * to determine if our node is right child of a parent node (in case we are traversing from
 * start) so that the 'UP' state can be set (or not).
 */
struct ctree_iter {
    _c_iter            iter;
    ulong              size;
    CCompareKeyFn      key_cmp_fn;
    _ctree_iter_states states;
};

/*
 * 'ctree_iter' constructor, the returned iterator is stack allocated as are all of
 * its members.
 *
 * It is fine to stack allocate it because its lifetime does not depend on any
 * underlying node.
 * This iterator is usefull for reading the node key/value pairs and/or mutating them.
 *
 * Warning:
 * Freeing the returned node is Undefined Behaviour.
 */
ctree_iter
ctree_iter_new(ctree* tree)
{
    return (ctree_iter) {
        .states     = _ctree_iter_states_default(),
        .size       = tree->size,
        .key_cmp_fn = tree->compare_key_fn,
        .iter       = _c_iter_new(_ctree_min(tree->root), _ctree_max(tree->root)),
    };
}

/*
 * Returns the next node from the front of the iterator in order.
 * Returns NULL if 'iter' is NULL or if iterator is exhausted.
 */
ctree_node*
ctree_iter_next(ctree_iter* iter)
{
    return_val_if_fail(iter != NULL, NULL);

    ctree_node* retval
        = _ctree_node_next(iter->iter.vals.start, &iter->states.start, iter->key_cmp_fn);

    if(retval != NULL)
        iter->size--;

    return retval;
}

/*
 * Returns the next node from the back of the iterator in order (reverse).
 * Returns NULL if 'iter' is NULL or if iterator is exhausted.
 */
ctree_node*
ctree_iter_next_back(ctree_iter* iter)
{
    return_val_if_fail(iter != NULL, NULL);

    ctree_node* retval = _ctree_node_prev(iter->iter.vals.end, &iter->states.end, iter->key_cmp_fn);

    if(retval != NULL)
        iter->size--;

    return retval;
}

/*
 * Finds the next node in order.
 *
 * Uses the mentioned algorithm described above under the state enum.
 * Ignoring the algorithm and explaining it roughly, it returns minimum value
 * of the right subtree, if right child node is NULL then it returns parent node
 * (there are edge cases which algorithm take care of).
 */
ctree_node*
_ctree_node_next(const ctree_node* node, _ctree_iter_state* state, CCompareKeyFn key_cmp_fn)
{
    return_val_if_fail(node != NULL, NULL);

    if(node->right == NULL || *state == UP) {

        ctree_node* parent;

        if((parent = node->parent) && parent->right
           && key_cmp_fn(parent->right->key, node->key) == 0)
        {
            *state = UP;
        } else {
            *state = RESET;
        }

        return node->parent;

    } else {
        *state = RESET;
        return _ctree_min(node->right);
    }
}

/*
 * Finds the previous node in order.
 *
 * Exactly same as the '_ctree_node_next' except opposite side.
 */
ctree_node*
_ctree_node_prev(const ctree_node* node, _ctree_iter_state* state, CCompareKeyFn key_cmp_fn)
{
    return_val_if_fail(node != NULL, NULL);

    if(node->left == NULL || *state == UP) {

        ctree_node* parent;

        if((parent = node->parent) && parent->left && key_cmp_fn(parent->left->key, node->key) == 0)
        {
            *state = UP;
        } else {
            *state = RESET;
        }

        return node->parent;

    } else {
        *state = RESET;
        return _ctree_max(node->left);
    }
}

/*
 * 'Drains' the 'size' of nodes from the front of the 'iterator'.
 * This basically advances the iterator 'size' amount forward.
 * If 'size' is bigger than the 'iterator' size, error message is printed to stderr
 * and function returns without trying to drain the iterator.
 */
void
ctree_iter_drain_front(ctree_iter* iterator, ulong size)
{
    if(iterator == NULL)
        return;

    if(size > iterator->size) {
        COL_SIZE_OUT_OF_BOUNDS;
        return;
    }

    while(size--) {
        iterator->iter.vals.start = _ctree_node_next(iterator->iter.vals.start,
                                                     &iterator->states.start,
                                                     iterator->key_cmp_fn);
        iterator->size--;
    }
}

/*
 * 'Drains' the 'size' of nodes from the back of the 'iterator'.
 * This basically advances the iterator 'size' amount forward.
 * If 'size' is bigger than the 'iterator' size, error message is printed to stderr
 * and function returns without trying to drain the iterator.
 */
void
ctree_iter_drain_back(ctree_iter* iterator, ulong amount)
{
    if(iterator == NULL)
        return;

    if(amount > iterator->size) {
        COL_SIZE_OUT_OF_BOUNDS;
        return;
    }

    while(amount--) {
        iterator->iter.vals.end = _ctree_node_prev(iterator->iter.vals.end,
                                                   &iterator->states.end,
                                                   iterator->key_cmp_fn);
        iterator->size--;
    }
}

/*
 * Bi-directional Consuming Iterator.
 *
 * Constructing this Iterator consumes the 'ctree' meaning it drops the
 * 'ctree' wrapper in order to construct this Iterator.
 *
 * All the function pointers are transfered and the Iterator becomes the
 * 'owner' of the underlying nodes ('ctree_iterator' outlives all the nodes it holds).
 *
 * Each call to the 'ctree_iterator_next' or 'ctree_iterator_next_back' returns next
 * node in order or previous.
 * Returned node is 'cloned' meaning it first gets shallow copied then the iterator
 * checks if the user supplied the 'CClone' functions when constructing the tree and
 * it applies the 'clone_key_fn' and 'clone_val_fn' if present (not NULL).
 * This then either returns a shallow copy of the node, a shallow copy + cloned key or
 * a shallow copy + cloned key + cloned value, returning the complete clone of the node.
 * Of course user is responsible for soundness of the cloning functions since he provides
 * them.
 *
 * Old node that the returned node gets cloned from is dropped and the functions
 * 'free_key_fn' and 'free_val_fn' are applied if present (not NULL).
 * Both the 'free_key_fn' and 'free_value_fn' are provided when constructing the 'ctree'.
 *
 * This iterator along with the non-consuming one can be iterated from any direction
 * at the same time, iterator will make sure to stop when both ends meet.
 */
struct ctree_iterator {
    ulong              size;
    _c_iter            _iter;
    _ctree_iter_states states;

    CClone        clone_key_fn;
    CClone        clone_val_fn;
    CFreeKeyFn    free_key_fn;
    CFreeValueFn  free_val_fn;
    CCompareKeyFn cmp_key_fn;
};

/*
 * 'ctree_iterator_new' constructor.
 * This function consumes the 'ctree', somehow modifying or reading
 * from the 'ctree' is Undefined Behaviour.
 *
 * As explained above the double pointer to 'ctree' is provided in order to
 * drop the 'ctree' and null the underlying pointer to it.
 */
ctree_iterator*
ctree_iterator_new(ctree** treep)
{
    ctree* tree;
    return_val_if_fail(treep != NULL && (tree = *treep) != NULL, NULL);

    ctree_iterator* iterator = memc_malloc(ctree_iterator);

#ifndef COL_MEMORY_CONSTRAINED
    if(__builtin_expect(iterator == NULL, 0)) {
#else
    if(iterator == NULL) {
#endif
        COL_ALLOC_ERROR;
        return NULL;
    }

    ctree_node* root = tree->root;

    iterator->size         = tree->size;
    iterator->states       = _ctree_iter_states_default();
    iterator->_iter        = _c_iter_new(_ctree_min(root), _ctree_max(root));
    iterator->clone_key_fn = tree->clone_key_fn;
    iterator->clone_val_fn = tree->clone_value_fn;
    iterator->free_key_fn  = tree->free_key_fn;
    iterator->free_val_fn  = tree->free_value_fn;
    iterator->cmp_key_fn   = tree->compare_key_fn;

    ctree_drop(treep, true);

    return iterator;
}

/*
 * Clones the 'node' by first doing shallow copy and then it applies 'clone_key_fn'
 * and/or 'clone_val_fn' if not NULL.
 * 'node' gets cloned into 'out'.
 */
void
_ctree_node_clone(ctree_node* node, CClone clone_key_fn, CClone clone_val_fn, ctree_node* out)
{
    memcpy(out, node, sizeof(ctree_node));

    if(clone_key_fn)
        out->key = clone_key_fn(node->key);
    if(clone_val_fn)
        out->value = clone_val_fn(node->value);
}

/*
 * Internal helper function that clones the current node in the direction
 * iterator is being traversed and also performs the free functions on the
 * original node.
 */
ctree_node*
_ctree_iterator_clone_current_and_free(ctree_iterator* iterator, cptr_t current)
{
    ctree_node* temp   = current;
    ctree_node* retval = memc_malloc(ctree_node);

#ifndef COL_MEMORY_CONSTRAINED
    if(__builtin_expect(retval == NULL, 0)) {
#else
    if(retval == NULL) {
#endif
        COL_ALLOC_ERROR;
        return NULL;
    }

    _ctree_node_clone(temp, iterator->clone_key_fn, iterator->clone_val_fn, retval);

    if(iterator->free_key_fn)
        iterator->free_key_fn(temp->key);
    if(iterator->free_val_fn)
        iterator->free_val_fn(temp->value);

    return retval;
}

/*
 * Traverses the iterator from the 'start' (front) fetching the next node.
 * This performs the deep copy of node and the dropping of the original one.
 * Returns the node or NULL if iterator is exhausted or if 'iterator' is NULL.
 */
ctree_node*
ctree_iterator_next(ctree_iterator* iterator)
{
    return_val_if_fail(iterator != NULL && iterator->_iter.vals.start != NULL, NULL);

    ctree_node* retval
        = _ctree_iterator_clone_current_and_free(iterator, iterator->_iter.vals.start);

    ctree_node* temp = iterator->_iter.vals.start;

    if(temp == iterator->_iter.vals.end) {
        iterator->_iter.vals.start = NULL;
        iterator->_iter.vals.end   = NULL;
    } else
        iterator->_iter.vals.start
            = _ctree_node_next(temp, &iterator->states.start, iterator->cmp_key_fn);

    ctree_node_drop(&temp);

    return retval;
}

/*
 * Traverses the iterator from the 'end' (back) fetching the previous node.
 * This performs the deep copy of node and the dropping of the original one.
 * Returns the node or NULL if iterator is exhausted or if 'iterator' is NULL.
 */
ctree_node*
ctree_iterator_next_back(ctree_iterator* iterator)
{
    return_val_if_fail(iterator != NULL && iterator->_iter.vals.end != NULL, NULL);

    ctree_node* retval = _ctree_iterator_clone_current_and_free(iterator, iterator->_iter.vals.end);
    ctree_node* temp   = iterator->_iter.vals.end;

    if(temp == iterator->_iter.vals.start) {
        iterator->_iter.vals.end   = NULL;
        iterator->_iter.vals.start = NULL;
    } else
        iterator->_iter.vals.end
            = _ctree_node_prev(temp, &iterator->states.end, iterator->cmp_key_fn);

    ctree_node_drop(&temp);

    return retval;
}

void
ctree_iterator_drain_front(ctree_iterator* iterator, ulong amount)
{
    if(iterator == NULL)
        return;

    if(amount > iterator->size) {
        COL_SIZE_OUT_OF_BOUNDS;
        return;
    }

    while(amount--) {
        ctree_node* current = iterator->_iter.vals.start;

        iterator->_iter.vals.start
            = _ctree_node_next(current, &iterator->states.start, iterator->cmp_key_fn);

        if(iterator->free_key_fn)
            iterator->free_key_fn(current->key);

        if(iterator->free_val_fn)
            iterator->free_val_fn(current->value);

        ctree_node_drop(&current);
    }

    iterator->size -= amount;
}

/*
 * 'Drains' from the back the 'iterator' for 'amount' of nodes.
 * All the drained nodes are also dropped.
 * Does nothing if 'iterator' is NULL or if the amount is greater than iterator size,
 * additionally it prints the error msg to stderr.
 */
void
ctree_iterator_drain_back(ctree_iterator* iterator, ulong amount)
{
    if(iterator == NULL)
        return;

    if(amount > iterator->size) {
        COL_SIZE_OUT_OF_BOUNDS;
        return;
    }

    while(amount--) {
        ctree_node* current = iterator->_iter.vals.end;

        iterator->_iter.vals.end
            = _ctree_node_next(current, &iterator->states.end, iterator->cmp_key_fn);

        if(iterator->free_key_fn)
            iterator->free_key_fn(current->key);

        if(iterator->free_val_fn)
            iterator->free_val_fn(current->value);

        ctree_node_drop(&current);
    }

    iterator->size -= amount;
}

/*
 * Returns the amount of remaining elements in iterator that are yet to be traversed.
 */
uint
ctree_iterator_size(ctree_iterator* iterator)
{
    return (iterator) ? iterator->size : 0;
}

/*
 * Iterator destructor, drops the iterator and nulls out the pointer that points to it.
 * If the iterator is not exhausted all of the remaining nodes are also dropped.
 */
void
ctree_iterator_drop(ctree_iterator** iteratorp)
{
    ctree_iterator* iterator;
    if(iteratorp != NULL && (iterator = *iteratorp) != NULL) {
        ctree_iterator_drain_front(iterator, iterator->size);
        // Sanity check
        assert(iterator->size == 0);
        iterator->size         = 0;
        iterator->states       = _ctree_iter_states_default();
        iterator->_iter        = _c_iter_default();
        iterator->free_key_fn  = NULL;
        iterator->free_val_fn  = NULL;
        iterator->clone_key_fn = NULL;
        iterator->clone_val_fn = NULL;
        iterator->cmp_key_fn   = NULL;
        *iteratorp             = NULL;
        free(iterator);
    }
}

/*
 * Returns the key stored inside the 'node'.
 * If 'node' is NULL function returns NULL.
 */
cptr_t
ctree_node_key(ctree_node* node)
{
    return_val_if_fail(node != NULL, NULL);
    return node->key;
}

/*
 * Returns the value stored inside the 'node'.
 * If 'node' is NULL function returns NULL.
 */
cptr_t
ctree_node_value(ctree_node* node)
{
    return_val_if_fail(node != NULL, NULL);
    return node->value;
}
