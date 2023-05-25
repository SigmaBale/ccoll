/*
 * Note:
 * This is not the fastest implementation of
 * self balanced binary tree nor does it strive to be,
 * this is recursive approach to the avl tree data structure.
 * However, it is possible that the implementation might
 * be rewritten with iterative approach.
 */

#define __CTREE_SOURCE_FILE__
#include "ctree.h"
#undef __CTREE_SOURCE_FILE__

#include <memc.h>
// TODO: Debug macro for turning on/off
// the assertions and debug functions
#include <assert.h>
#include <stdbool.h>

/*
 * Group of flags that are used for state checking inside
 * nested recursion calls.
 *
 * Flag 'removed' gets set whenever we actually 'free' the node
 * actually removing it form the tree, this way we can also
 * optimize when to update the tree.
 * This is mainly usefull if our tree allows duplicate key values
 * which it does by default.
 *
 * Flag 'inserted' gets set whenever new key gets inserted.
 * This way we can optimize when to update nodes and
 * rebalance the tree during recursion traceback.
 * If our tree allows duplicate key values then the flag will only be
 * set if the key inserted is unique.
 * If the key already exists then only the node count that holds that
 * key gets increased while 'inserted' flag stays off.
 *
 * Flag 'node_copied' gets set whenever we are removing the key
 * from the tree and the node holding it happens to have both
 * subtrees and additionally in case of tree that allows duplicate values
 * (which CTree does by default) where node count is 1 before removing.
 * It gets set because we will copy over the key and node count (in
 * case of allowing duplicate key values) of the node either from
 * left subtree or right subtree, so the key value stored in the node
 * we just copied from doesn't get removed (in case user specified
 * key_free_fn).
 */

typedef struct {
    cuint removed     : 1;
    cuint inserted    : 1;
    cuint node_copied : 1;
    cuint node_found  : 1;
} CFlags;

/*
 * Core private structure that represents the Node (key)
 * stored inside the CTree.
 *
 * It must contain valid non-NULL key in order to even be constructed,
 * it also contains the node parameters, and those would be its
 * balance factor and relative height.
 *
 * Node parameters (height and balance factor) are used in tree rebalancing,
 * for example if balance factor [ BF > 1 or BF < -1 ] then the avl tree
 * invariant is violated.
 *
 * If so then the tree rebalances itself until the balance factor
 * of all nodes is one of the values of the given set {-1, 0, 1}
 */
typedef struct CTreeNode {
    cptr              key;
    cuint             count;
    cuint             height;
    int               balance;
    struct CTreeNode* right;
    struct CTreeNode* left;
} CTreeNode;

/*
 * CTree is AVL tree (self-balancing binary search tree).
 * Comparisons are done based on the comparison function provided upon
 * constructing the CTree ('CCompareKeyFn').
 *
 * Free key function is used on keys each time they are removed from the tree.
 * ('CFreeKeyFn')
 * If no key removal function is provided then the tree will not free
 * the underlying memory of the key passed in when it gets removed
 * from the tree.
 * Meaning the user is responsible for freeing that memory in that
 * specific case.
 */
struct _CTree {
    CTreeNode*    root;
    CFlags*       flags;
    cuint         size;
    CCompareKeyFn compare_key_fn;
    CFreeKeyFn    free_key_fn;
};

/*
 * Private declarations
 */
CFlags* flags_new(void);
void    flags_free(CFlags*);

CTreeNode* ctreenode_new(cptr);
CTreeNode* ctreenode_insert(CTree*, CTreeNode*, cptr);
CTreeNode* ctreenode_remove(CTree*, CTreeNode*, cptr);
void       ctreenode_free(CTreeNode*);
void       ctreenode_freeall(CTree*, CTreeNode*);

void       ctreenode_update(CTreeNode*);
CTreeNode* ctreenode_rotate_right(CTreeNode*);
CTreeNode* ctreenode_rotate_left(CTreeNode*);
CTreeNode* ctreenode_rotate_left_right(CTreeNode*);
CTreeNode* ctreenode_rotate_right_left(CTreeNode*);
CTreeNode* ctreenode_rebalance(CTreeNode*);
CTreeNode* ctreenode_find_left(CTreeNode*);
CTreeNode* ctreenode_find_right(CTreeNode*);

// Debug functions
void print_tree(CTree*);
void print_node(CTreeNode*);

/*
 * Flags constructor
 */
CFlags*
flags_new(void)
{
    CFlags* flags = memc_malloc(CFlags);
    if(flags != NULL) {
        flags->removed     = 0;
        flags->inserted    = 0;
        flags->node_copied = 0;
        flags->node_found  = 0;
    }
    return flags;
}

/*
 * Flags destructor
 */
void
flags_free(CFlags* flags)
{
    flags->inserted    = 0;
    flags->removed     = 0;
    flags->node_copied = 0;
    flags->node_found  = 0;
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
CTree*
ctree_new(CCompareKeyFn compare_key_fn, CFreeKeyFn free_key_fn)
{
    CTree* avltree = memc_malloc(CTree);
    if(avltree != NULL) {
        if((avltree->compare_key_fn = compare_key_fn) == NULL || (avltree->flags = flags_new()) == NULL) {
            free(avltree);
            return NULL;
        }
        avltree->free_key_fn = free_key_fn;
        avltree->root        = NULL;
        avltree->size        = 0;
    }
    return avltree;
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
ctree_free(CTree* tree)
{
    assert(tree != NULL);
    if(tree != NULL) {
        ctreenode_freeall(tree, tree->root);
        flags_free(tree->flags);
        tree->compare_key_fn = NULL;
        tree->free_key_fn    = NULL;
        assert(tree->size == 0);
        free(tree);
    }
}

/*
 * Helper function for CTree destructor.
 * Recursively frees all the nodes
 */
void
ctreenode_freeall(CTree* tree, CTreeNode* node)
{
    if(node != NULL) {
        ctreenode_freeall(tree, node->left);
        ctreenode_freeall(tree, node->right);

        if(tree->free_key_fn)
            tree->free_key_fn(node->key);

        tree->size -= node->count;
        ctreenode_free(node);
    }
}

/*
 * CTree recursive BST insertion function
 * Returns false if no insertion occured,
 * otherwise true.
 */
bool
ctree_insert(CTree* tree, cptr key)
{
    return_val_if_fail(tree != NULL, false);

    tree->root = ctreenode_insert(tree, tree->root, key);

    tree->flags->inserted = 0;

    return true;
}

/*
 * Getter, returns the total node count in the CTree.
 */
cuint
ctree_size(CTree* tree)
{
    return tree->size;
}

/*
 * Getter, returns the total size of the CTree
 * (*size of all nodes) in bytes.
 */
size_t
ctree_size_bytes(CTree* tree)
{
    return tree->size * sizeof(CTreeNode);
}

/*
 * Internal recursive BST insertion function.
 * It updates the node parameters and does the
 * recursive tree rebalancing.
 */
CTreeNode*
ctreenode_insert(CTree* tree, CTreeNode* node, cptr key)
{
    int cmp;
    if(node == NULL) {
        node = ctreenode_new(key);
        tree->size++;
        tree->flags->inserted = 1;
    } else if((cmp = tree->compare_key_fn(node->key, key)) > 0) {
        node->left = ctreenode_insert(tree, node->left, key);
    } else if(cmp < 0) {
        node->right = ctreenode_insert(tree, node->right, key);
    } else {
        tree->size++;
        node->count++;
    }

    if(tree->flags->inserted) {
        ctreenode_update(node);
        return ctreenode_rebalance(node);
    }

    return node;
}

/*
 * Removes the given key from the tree.
 * Returns true upon finding and 'removing' the key.
 * Also returns true if the value was found but only
 * the count was decreased instead of the node getting removed.
 * False if no key was found or (tree | key) is NULL.
 */
bool
ctree_remove(CTree* tree, cptr key)
{
    return_val_if_fail((tree != NULL && key != NULL), false);

    tree->root = ctreenode_remove(tree, tree->root, key);

    if(tree->flags->node_found | tree->flags->node_found) {
        tree->flags->removed    = 0;
        tree->flags->node_found = 0;
        return true;
    } else
        return false;
}

/*
 * Internal recursive ctree remove function.
 * Three possible cases when removing the node are:
 *  1. Node contains left or right subtree
 *  2. Node is leaf
 *  3. Node contains both left and right subtree
 *
 * Because our tree keeps count of each node (can
 * contain nodes with duplicate keys) if the node
 * being removed has a count of 1 only then our
 * removal cases are relevant and apply.
 * If the nodes count is above 1 we just decrement the count
 * and the tree size then we return without updating nodes.
 *
 * -- IF THE NODE COUNT IS 1 -------------------------------
 * In the first case where node contains either left
 * or right subtree, we check if left subtree is null if
 * true we return the left subtree if false we return right
 * subtree, before returning we free/remove the node.
 *
 * In the second case where node is leaf, we free/remove
 * the node and return NULL.
 *
 * In the case where node contains both subtrees, we find
 * either the max key value in the left subtree or min key
 * value from the right subtree and copy it into our node that
 * should get replaced, and then call the ctreenode_remove
 * again on the node we just copied the key from.
 * Note the flag 'node_copied' will be set avoiding
 * freeing the copied key value (if free_key_fn was provided).
 *
 * But the key in the original node that copied over the
 * other key will be freed (if free_key_fn was provided).
 */
CTreeNode*
ctreenode_remove(CTree* tree, CTreeNode* node, cptr key)
{
    if(node == NULL)
        return node;

    int cmp = tree->compare_key_fn(node->key, key);

    if(cmp > 0) {
        node->left = ctreenode_remove(tree, node->left, key);
    } else if(cmp < 0) {
        node->right = ctreenode_remove(tree, node->right, key);
    } else if(cmp == 0) {
        CTreeNode* temp;

        if((--node->count == 0) | tree->flags->node_copied) {

            // We will free the node, count is 0
            if(node->right == NULL || node->left == NULL) {
                temp = (node->right != NULL) ? node->right : node->left;

                if(!tree->flags->node_copied && tree->free_key_fn)
                    tree->free_key_fn(node->key);

                ctreenode_free(node);
                tree->size--;
                tree->flags->removed = 1;

                if(temp == NULL)
                    return NULL;
                else
                    node = temp;

            } else {
                // Chose from which subtree to remove
                // the node, depending on the height difference
                if(node->left->height > node->right->height) {
                    temp = ctreenode_find_left(node->left);

                    if(tree->free_key_fn)
                        tree->free_key_fn(node->key);

                    node->key                = temp->key;
                    node->count              = temp->count;
                    tree->flags->node_copied = 1;
                    node->left               = ctreenode_remove(tree, node->left, node->key);
                    tree->flags->node_copied = 0;
                } else {
                    temp = ctreenode_find_right(node->right);

                    if(tree->free_key_fn)
                        tree->free_key_fn(node->key);

                    node->key                = temp->key;
                    node->count              = temp->count;
                    tree->flags->node_copied = 1;
                    node->right              = ctreenode_remove(tree, node->right, node->key);
                    tree->flags->node_copied = 0;
                }
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
 * Finds the max key value in the left subtree
 * of the passed in 'CTreeNode'
 */
CTreeNode*
ctreenode_find_right(CTreeNode* node)
{
    while(node->left != NULL)
        node = node->left;

    return node;
}

/*
 * Finds the min key value in the right subtree
 * of the passed in 'CTreeNode'
 */
CTreeNode*
ctreenode_find_left(CTreeNode* node)
{
    while(node->right != NULL)
        node = node->right;

    return node;
}

/*
 * CTreeNode constructor
 */
CTreeNode*
ctreenode_new(cptr key)
{
    CTreeNode* node = memc_malloc(CTreeNode);
    if(node != NULL) {
        if((node->key = key) == NULL) {
            free(node);
            return NULL;
        }
        node->count   = 1;
        node->height  = 0;
        node->balance = 0;
        node->right   = NULL;
        node->left    = NULL;
    }
    return node;
}

/*
 * CTreeNode destructor
 */
void
ctreenode_free(CTreeNode* node)
{
    node->height  = 0;
    node->balance = 0;
    node->count   = 0;
    node->right   = NULL;
    node->left    = NULL;
    node->key     = NULL;
    free(node);
}

/*
 * Updates the CTreeNode height and balance factor
 */
void
ctreenode_update(CTreeNode* node)
{
    int left      = (node->left != NULL) ? (int) node->left->height : -1;
    int right     = (node->right != NULL) ? (int) node->right->height : -1;
    node->height  = ((left > right) ? left : right) + 1;
    node->balance = right - left;
}

/*
 * Rebalancing function, initiates rotations only
 * if the node balance factor is not part of the
 * {-1, 0, 1} set.
 */
CTreeNode*
ctreenode_rebalance(CTreeNode* node)
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
 * Rotation function, performs right rotation, after rotation
 * it updates the pivot and root node (height and balance factor)
 */
CTreeNode*
ctreenode_rotate_right(CTreeNode* node)
{
    CTreeNode* new_root = node->left;
    node->left          = new_root->right;
    new_root->right     = node;
    ctreenode_update(node);
    ctreenode_update(new_root);
    return new_root;
}

/*
 * Rotation function, performs left rotation, after rotation
 * it updates the pivot and root node (height and balance factor)
 */
CTreeNode*
ctreenode_rotate_left(CTreeNode* node)
{
    CTreeNode* new_root = node->right;
    node->right         = new_root->left;
    new_root->left      = node;
    ctreenode_update(node);
    ctreenode_update(new_root);
    return new_root;
}

/*
 * Rotation function, performs multiple rotations by first rotating the right
 * child of the root by applying the right rotation on it, then rotates the root
 * and pivot (new right child) by applying left rotation
 */
CTreeNode*
ctreenode_rotate_right_left(CTreeNode* node)
{
    node->right = ctreenode_rotate_right(node->right);
    return ctreenode_rotate_left(node);
}

/*
 * Rotation function, performs multiple rotations by first rotating the left
 * child of the root by applying the left rotation on it, then rotates the root
 * and pivot (new left child) by applying right rotation
 */
CTreeNode*
ctreenode_rotate_left_right(CTreeNode* node)
{
    node->left = ctreenode_rotate_left(node->left);
    return ctreenode_rotate_right(node);
}

// Debug section--------------------------------------------------------------
void
print_tree(CTree* tree)
{
    print_node(tree->root);
}

void
print_node(CTreeNode* node)
{
    if(node != NULL) {
        print_node(node->left);
        print_node(node->right);
    }
}
