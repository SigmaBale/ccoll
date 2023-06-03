/*
 * Note:
 * This is not the fastest implementation of
 * self balanced binary tree nor does it strive to be,
 * this is recursive approach to the avl tree data structure.
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
 * must start all the way from the root because tree might rebalance.
 */

#define __COL_TREE_C_FILE__
#include "ctree.h"
#undef __COL_TREE_C_FILE__
#define __COL_C_FILE__
#include "cerror.h"
#undef __COL_C_FILE__

#include <assert.h>
#include <memc.h>
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

typedef struct {
    cuint removed  : 1;
    cuint inserted : 1;
    cuint replaced : 1;
} cflags;

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
typedef struct _cnode {
    cptr  key;
    cptr  value;
    cuint height;
    int   balance;

    struct _cnode* right;
    struct _cnode* left;
} cnode;

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
struct _ctree {
    cnode*  root;
    cflags* flags;
    cuint   size;

    CCompareKeyFn compare_key_fn;
    CFreeKeyFn    free_key_fn;
    CFreeValueFn  free_value_fn;
};

/*
 * Flags constructor
 */
static cflags*
flags_new(void)
{
    cflags* flags = memc_malloc(cflags);
    if(flags != NULL) {
        flags->removed  = 0;
        flags->inserted = 0;
        flags->replaced = 0;
    }
    return flags;
}

/*
 * Flags destructor
 */
static void
flags_free(cflags* flags)
{
    flags->inserted = 0;
    flags->removed  = 0;
    flags->replaced = 0;
    free(flags);
}

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
          CFreeValueFn  free_value_fn)
{
    ctree* tree = memc_malloc(ctree);
    if(tree != NULL) {
        if((tree->compare_key_fn = compare_key_fn) == NULL
           || (tree->flags = flags_new()) == NULL)
        {
            if(tree->compare_key_fn == NULL) {
                COL_ERROR_INVALID_COMPARISON_FUNCTION("ctree");
                free(tree);
            } else
                COL_ERROR_OUT_OF_MEMORY("ctree");

            return NULL;
        }
        tree->free_key_fn   = free_key_fn;
        tree->free_value_fn = free_value_fn;
        tree->root          = NULL;
        tree->size          = 0;
    }

    if(tree == NULL)
        COL_ERROR_OUT_OF_MEMORY("ctree");

    return tree;
}

/*
 * CTreeNode constructor
 */
static cnode*
ctreenode_new(cptr key, cptr value)
{
    cnode* node;
    if((node = memc_malloc(cnode)) == NULL || (node->key = key) == NULL) {
        if(node == NULL) {
            COL_ERROR_OUT_OF_MEMORY("ctree");
        } else {
            COL_ERROR_INVALID_KEY("ctree", key);
            free(node);
        }
        return NULL;
    }
    node->value   = value;
    node->height  = 0;
    node->balance = 0;
    node->right   = NULL;
    node->left    = NULL;

    return node;
}

/*
 * CTreeNode destructor
 */
static void
ctreenode_free(cnode* node)
{
    node->height  = 0;
    node->balance = 0;
    node->right   = NULL;
    node->left    = NULL;
    node->key     = NULL;
    node->value   = NULL;
    free(node);
}

/*
 * Helper function for CTree destructor.
 * Recursively frees all the nodes
 */
static void
ctreenode_freeall(ctree* tree, cnode* node)
{
    if(node != NULL) {
        ctreenode_freeall(tree, node->left);
        ctreenode_freeall(tree, node->right);

        if(tree->free_key_fn)
            tree->free_key_fn(node->key);

        if(tree->free_value_fn)
            tree->free_value_fn(node->value);

        tree->size--;
        ctreenode_free(node);
    }
}

/*
 * CTree 'destructor', cleans up all the nodes
 * and itself.
 * Note: If the user didn't pass his own
 * 'free_key_fn' on tree creation then it is
 * the users responsibility to clean up the
 * inserted keys.
 * */
void
ctree_free(ctree* tree)
{
    assert(tree != NULL);
    if(tree != NULL) {
        ctreenode_freeall(tree, tree->root);
        flags_free(tree->flags);
        tree->compare_key_fn = NULL;
        tree->free_key_fn    = NULL;
        tree->free_value_fn  = NULL;
        assert(tree->size == 0);
        free(tree);
    }
}

/*
 * Updates the CTreeNode height and balance factor
 */
static void
ctreenode_update(cnode* node)
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
static cnode*
ctreenode_rotate_right(cnode* node)
{
    cnode* new_root = node->left;
    node->left      = new_root->right;
    new_root->right = node;
    ctreenode_update(node);
    ctreenode_update(new_root);
    return new_root;
}

/*
 * Rotation function, performs left rotation, after rotation
 * it updates the pivot and root node (height and balance factor)
 */
static cnode*
ctreenode_rotate_left(cnode* node)
{
    cnode* new_root = node->right;
    node->right     = new_root->left;
    new_root->left  = node;
    ctreenode_update(node);
    ctreenode_update(new_root);
    return new_root;
}

/*
 * Rotation function, performs multiple rotations by first rotating the right
 * child of the root by applying the right rotation on it, then rotates the root
 * and pivot (new right child) by applying left rotation
 */
static cnode*
ctreenode_rotate_right_left(cnode* node)
{
    node->right = ctreenode_rotate_right(node->right);
    return ctreenode_rotate_left(node);
}

/*
 * Rotation function, performs multiple rotations by first rotating the left
 * child of the root by applying the left rotation on it, then rotates the root
 * and pivot (new left child) by applying right rotation
 */
static cnode*
ctreenode_rotate_left_right(cnode* node)
{
    node->left = ctreenode_rotate_left(node->left);
    return ctreenode_rotate_right(node);
}

/*
 * Rebalancing function, initiates rotations only
 * if the node balance factor is not part of the
 * {-1, 0, 1} set.
 */
static cnode*
ctreenode_rebalance(cnode* node)
{
    if(node->balance < -1) {
        // AVL tree invariant must not be broken
        assert(node->balance == -2);
        return (node->left->balance <= 0) ? ctreenode_rotate_right(node)
                                          : ctreenode_rotate_left_right(node);
    } else if(node->balance > 1) {
        // AVL tree invariant must not be broken
        assert(node->balance == 2);
        return (node->right->balance >= 0) ? ctreenode_rotate_left(node)
                                           : ctreenode_rotate_right_left(node);
    }
    return node;
}

/*
 * Finds the max key value in the left subtree
 * of the passed in 'CTreeNode'
 */
static cnode*
ctreenode_find_right(cnode* node)
{
    while(node->left != NULL)
        node = node->left;

    return node;
}

/*
 * Finds the min key value in the right subtree
 * of the passed in 'CTreeNode'
 */
static cnode*
ctreenode_find_left(cnode* node)
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
static cnode*
ctreenode_insert(ctree* tree, cnode* node, cptr key, cptr value, bool replace)
{
    int cmp;
    if(node == NULL && (node = ctreenode_new(key, value)) != NULL) {
        tree->size++;
        tree->flags->inserted = 1;
    } else if((cmp = tree->compare_key_fn(node->key, key)) > 0) {
        node->left = ctreenode_insert(tree, node->left, key, value, replace);
    } else if(cmp < 0) {
        node->right = ctreenode_insert(tree, node->right, key, value, replace);
    } else {
        if(tree->free_value_fn)
            tree->free_value_fn(node->value);

        node->value = value;

        if(replace) {
            tree->flags->replaced = 1;

            if(tree->free_key_fn)
                tree->free_key_fn(node->key);

            node->key = key;
        }
    }

    if(tree->flags->inserted) {
        ctreenode_update(node);
        return ctreenode_rebalance(node);
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
static cnode*
ctreenode_remove(ctree* tree, cnode* node, cptr key)
{
    if(node == NULL)
        return node;

    int cmp = tree->compare_key_fn(node->key, key);

    if(cmp > 0) {
        node->left = ctreenode_remove(tree, node->left, key);
    } else if(cmp < 0) {
        node->right = ctreenode_remove(tree, node->right, key);
    } else if(cmp == 0) {
        cnode* temp;

        if(node->right == NULL || node->left == NULL) {
            temp = (node->right != NULL) ? node->right : node->left;

            // If we freed the value in some recursion call
            // before this one, then don't try to free again!
            if(!tree->flags->removed) {
                (tree->free_key_fn) ? tree->free_key_fn(node->key) : 0;
                (tree->free_value_fn) ? tree->free_value_fn(node->value) : 0;
                tree->flags->removed = 1;
            }

            ctreenode_free(node);

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
            tree->flags->removed = 1;

            if(node->left->height > node->right->height) {
                temp        = ctreenode_find_left(node->left);
                node->key   = temp->key;
                node->value = temp->value;
                node->left  = ctreenode_remove(tree, node->left, node->key);
            } else {
                temp        = ctreenode_find_right(node->right);
                node->key   = temp->key;
                node->value = temp->value;
                node->right = ctreenode_remove(tree, node->right, node->key);
            }
        }
    }

    // During traceback only update if we removed the node
    if(tree->flags->removed) {
        ctreenode_update(node);
        return ctreenode_rebalance(node);
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
ctree_insert(ctree* tree, cptr key, cptr value)
{
    return_val_if_fail(tree != NULL, false);

    tree->root = ctreenode_insert(tree, tree->root, key, value, false);

    if(tree->flags->inserted) {
        tree->flags->inserted = 0;
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
ctree_replace(ctree* tree, cptr key, cptr value)
{
    return_val_if_fail(tree != NULL, false);

    tree->root = ctreenode_insert(tree, tree->root, key, value, true);

    if(tree->flags->replaced) {
        tree->flags->replaced = 0;
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
ctree_remove(ctree* tree, cptr key)
{
    return_val_if_fail((tree != NULL && key != NULL), false);

    tree->root = ctreenode_remove(tree, tree->root, key);

    if(tree->flags->removed) {
        tree->flags->removed = 0;
        return true;
    } else
        return false;
}

/*
 * Internal function, tries to find the key in tree.
 * Returns NULL if key was not found or the pointer
 * to the key if it was found.
 */
cptr
ctreenode_find(ctree* tree, cptr key)
{
    return_val_if_fail(tree != NULL, NULL);

    int    cmp;
    cnode* current = tree->root;

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
cptr
ctree_entry(ctree* tree, cptr key)
{
    return_val_if_fail(tree != NULL, NULL);

    cnode* entry;

    if((entry = ctreenode_find(tree, key)) != NULL)
        return entry->value;

    return NULL;
}

/*
 * Returns pointer to the key of the key/value pair.
 * Returns NULL if key is not inside the tree.
 */
cptr
ctree_key(ctree* tree, cptr key)
{
    return_val_if_fail(tree != NULL, NULL);

    cnode* entry;

    if((entry = ctreenode_find(tree, key)) != NULL)
        return entry->key;

    return NULL;
}

/*
 * Returns the total node count in the CTree.
 */
cuint
ctree_size(ctree* tree)
{
    return tree->size;
}

/*
 * Returns the total size of the CTree
 * (size of all nodes) in bytes.
 */
size_t
ctree_size_bytes(ctree* tree)
{
    return tree->size * sizeof(cnode);
}

// Debug
void
print_node(cnode* node)
{
    if(node != NULL) {
        print_node(node->left);
        print_node(node->right);
    }
}

// Debug
void
print_tree(ctree* tree)
{
    print_node(tree->root);
}
