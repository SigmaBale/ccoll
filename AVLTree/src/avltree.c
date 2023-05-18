#include <assert.h>
#include <stdlib.h>
#include <string.h>

/*
 * AVL tree (self-balancing binary search tree), it ensures
 * that the height of the two child subtrees of any node differ
 * by at most one.
 * In case of AVL tree invariant violation, tree rebalancing is done
 * using left, right, left-right and right-left rotations.
 * AVLTree will always have O(log(n)) operational time complexity.
 *
 * Comparisons are done based on the comparison function provided upon
 * constructing the AVLTree (see fn avltree_new).
 *
 * There is also additional size field that contains the current
 * node count contained in the AVLTree.
 */
struct AVLTree {
    struct AVLNode* root;
    unsigned int    size;
    int (*cmp)(void*, void*);
    struct Flags* flags;
};

/*
 * Group of flags that are used for state checking inside
 * nested recursion calls.
 *
 * Flag 'removed' is used in order to prevent unnecessary parameter
 * updating, calls to avlnode_rebalance function and avoid freeing memory
 * in case we are removing a value but that value is not found in
 * the AVL tree or that value was inserted multiple times and when getting
 * removed its count was only decremented in which case the
 * removed flag would not be set because the value still remains in the tree.
 *
 * Flag 'inserted' is used when we insert value (value to be inserted
 * was not found in the tree and is unique), if the flag is set then when
 * tracebacking we will update all nodes parameters and rebalance the AVL Tree.
 * If we insert the key (value) that is already in the tree, 'inserted' will not
 * be set instead only nodes count param will be incremented.
 */
struct Flags {
    unsigned int removed  : 1;
    unsigned int inserted : 1;
};

/*
 * Core private structure that represents the Node (value)
 * stored inside the AVLTree.
 *
 * It must contain valid non-NULL value in order to even be constructed,
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
struct AVLNode {
    // TODO: Make AVLNode contain void* pointer instead
    // This will require for user to provide size of the
    // underlying type the pointer is pointing at
    char*           value;
    unsigned int    count;
    unsigned int    height;
    // TODO: Remove balance factor we only need height
    int             balance;
    struct AVLTree* tree;
    struct AVLNode* right;
    struct AVLNode* left;
};

// TODO: Add search function
/* Private Interface ---------------------------------------------------------*/
struct Flags* flags_new(void);
void          flags_free(struct Flags*);

int avltree_check_for_null(struct AVLTree*, const char*);

// TODO: All function declarations and their definitions
// need to take void* pointer instead, also fix the insert function
// to memcpy the value instead of strdup (duh...)
struct AVLNode* avlnode_new(struct AVLTree*, const char*);
struct AVLNode* avlnode_insert(struct AVLTree*, struct AVLNode*, const char*);
struct AVLNode* avlnode_remove(struct AVLTree*, struct AVLNode*, const char*);
void            avlnode_free(struct AVLTree*, struct AVLNode*);
void            avlnode_freeall(struct AVLTree*, struct AVLNode*);
void            avlnode_decrement_count(struct AVLTree*, struct AVLNode*);
void            avlnode_increment_count(struct AVLTree*, struct AVLNode*);

// Debug functions
void print_tree(struct AVLTree*);
void print_node(struct AVLNode*);

// Param update and rebalancing
void            avlnode_update(struct AVLNode*);
struct AVLNode* avlnode_rotate_right(struct AVLNode*);
struct AVLNode* avlnode_rotate_left(struct AVLNode*);
struct AVLNode* avlnode_rotate_left_right(struct AVLNode*);
struct AVLNode* avlnode_rotate_right_left(struct AVLNode*);
struct AVLNode* avlnode_rebalance(struct AVLNode*);
struct AVLNode* avlnode_find_left(struct AVLNode*);
struct AVLNode* avlnode_find_right(struct AVLNode*);

//------------------------------------------------------------------------------

/* Flags constructor */
struct Flags*
flags_new(void)
{
    struct Flags* flags = (struct Flags*) malloc(sizeof(struct Flags));
    if(flags != NULL) {
        flags->removed  = 0;
        flags->inserted = 0;
    }
    return flags;
}

/* Flags destructor */
void
flags_free(struct Flags* flags)
{
    flags->inserted = 0;
    flags->removed  = 0;
    free(flags);
}

/* AVLTree constructor */
struct AVLTree*
avltree_new(int (*cmp)(void*, void*))
{
    struct AVLTree* avltree = (struct AVLTree*) malloc(sizeof(struct AVLTree));
    if(avltree != NULL) {
        if((avltree->cmp = cmp) == NULL || (avltree->flags = flags_new()) == NULL) {
            free(avltree);
            return NULL;
        }
        avltree->root = NULL;
        avltree->size = 0;
    }
    return avltree;
}

/*
 * AVL Tree 'destructor', frees up all the nodes in the
 * tree before the tree itself
 * */
void
avltree_free(struct AVLTree* tree)
{
    assert(tree != NULL);
    if(tree != NULL) {
        avlnode_freeall(tree, tree->root);
        flags_free(tree->flags);
        tree->cmp = NULL;
        assert(tree->size == 0);
        free(tree);
    }
}

/* Helper function for AVL Tree 'destructor'
 * recursively frees all the nodes */
void
avlnode_freeall(struct AVLTree* tree, struct AVLNode* node)
{
    if(node != NULL) {
        avlnode_freeall(tree, node->left);
        avlnode_freeall(tree, node->right);
        avlnode_free(tree, node);
    }
}

/* AVLTree recursive BST insertion function
 * Returns -1 if avltree_check failed 0 otherwise
 * (check avltree_check_for_null) */
int
avltree_insert(struct AVLTree* tree, const char* value)
{
    if(avltree_check_for_null(tree, value) == 0) {
        tree->root            = avlnode_insert(tree, tree->root, value);
        tree->flags->inserted = 0;
        return 0;
    }
    return -1;
}

/* Check if tree/value is valid before inserting */
int
avltree_check_for_null(struct AVLTree* tree, const char* value)
{
    if(tree == NULL) {
        return -1;
    } else if(value == NULL) {
        return 1;
    } else {
        return 0;
    }
}

/* Getter function, returns the total node count
 * int the AVLTree */
int
avltree_size(struct AVLTree* tree)
{
    return tree->size;
}

/* Getter function, returns the total size
 * of the AVLTree (*size of all nodes) in bytes */
size_t
avltree_size_bytes(struct AVLTree* tree)
{
    return tree->size * sizeof(struct AVLNode);
}

/* Internal recursive BST insertion function
 * + updates the node parameters
 * + recursive AVLTree rebalancing */
struct AVLNode*
avlnode_insert(struct AVLTree* tree, struct AVLNode* node, const char* value)
{
    int cmp;
    if(node == NULL) {
        node = avlnode_new(tree, value);
    } else if((cmp = tree->cmp(node->value, (char*) value)) > 0) {
        node->left = avlnode_insert(tree, node->left, value);
    } else if(cmp < 0) {
        node->right = avlnode_insert(tree, node->right, value);
    } else {
        avlnode_increment_count(tree, node);
    }

    if(tree->flags->inserted) {
        avlnode_update(node);
        return avlnode_rebalance(node);
    }

    return node;
}

/* Removes the given value from the tree, returns
 * 1 upon finding and removing value
 * 0 if no value was found
 * -1 if tree or value are NULL*/
int
avltree_remove(struct AVLTree* tree, const char* value)
{
    if(avltree_check_for_null(tree, value) == 0) {
        tree->root = avlnode_remove(tree, tree->root, value);
        if(tree->flags->removed) {
            tree->flags->removed = 0;
            return 1;
        } else {
            return 0;
        }
    }
    return -1;
}

/*
 * Internal recursive avl tree remove function.
 * Three possible cases when removing the node are:
 *  1. Node contains left or right subtree
 *  2. Node is leaf
 *  3. Node contains both left and right subtree
 *
 * Because our tree keeps count of each node (can
 * contain nodes with duplicate key/value) if the node
 * being removed has a count greater than 1 only then our
 * removal cases are relevant.
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
 * either the max value in the left subtree or min value
 * from the right subtree and copy it into our node that
 * should get replaced, and then call the avlnode_remove
 * again on the node we just copied the value from.
 */
struct AVLNode*
avlnode_remove(struct AVLTree* tree, struct AVLNode* node, const char* value)
{
    if(node == NULL) {
        return node;
    }

    int cmp = tree->cmp(node->value, (char*) value);

    if(cmp > 0) {
        node->left = avlnode_remove(tree, node->left, value);
    } else if(cmp < 0) {
        node->right = avlnode_remove(tree, node->right, value);
    } else if(cmp == 0) {
        struct AVLNode* temp;

        if(node->right == NULL || node->left == NULL) {
            temp = (node->right != NULL) ? node->right : node->left;
            avlnode_decrement_count(tree, node);

            if(tree->flags->removed) {
                if(temp == NULL) {
                    return NULL;
                } else {
                    node = temp;
                }
            }
        } else {
            if(node->count > 1) {
                avlnode_decrement_count(tree, node);
            } else if(node->left->height > node->right->height) {
                temp        = avlnode_find_left(node->left);
                node->value = strdup(temp->value);
                node->left  = avlnode_remove(tree, node->left, node->value);
            } else {
                temp        = avlnode_find_right(node->right);
                node->value = strdup(temp->value);
                node->right = avlnode_remove(tree, node->right, node->value);
            }
        }
    }
    // During traceback only update if we removed the node
    if(tree->flags->removed) {
        avlnode_update(node);
        return avlnode_rebalance(node);
    } else {
        return node;
    }
}

struct AVLNode*
avlnode_find_right(struct AVLNode* node)
{
    while(node->left != NULL) {
        node = node->left;
    }
    return node;
}

struct AVLNode*
avlnode_find_left(struct AVLNode* node)
{
    while(node->right != NULL) {
        node = node->right;
    }
    return node;
}

/* AVLNode constructor */
struct AVLNode*
avlnode_new(struct AVLTree* tree, const char* value)
{
    struct AVLNode* node = (struct AVLNode*) malloc(sizeof(struct AVLNode));
    if(node != NULL) {
        if((node->value = strdup(value)) == NULL) {
            free(node);
            return NULL;
        }
        node->count   = 1;
        node->height  = 0;
        node->balance = 0;
        node->tree    = tree;
        node->right   = NULL;
        node->left    = NULL;
        tree->size++;
        tree->flags->inserted = 1;
    }
    return node;
}

// Flag 'removed' will only be set if the count reaches 0
void
avlnode_decrement_count(struct AVLTree* tree, struct AVLNode* node)
{
    tree->size--;
    if(--node->count == 0) {
        avlnode_free(tree, node);
    }
}

// Flag 'inserted' will not be set
void
avlnode_increment_count(struct AVLTree* tree, struct AVLNode* node)
{
    node->count++;
    tree->size++;
}

/* AVLNode destructor, frees up the malloc'd value and the node */
void
avlnode_free(struct AVLTree* tree, struct AVLNode* node)
{
    node->height  = 0;
    node->balance = 0;
    tree->size    -= node->count;
    node->count   = 0;
    node->tree    = NULL;
    node->right   = NULL;
    node->left    = NULL;
    free(node->value);
    node->value = NULL;
    free(node);
    tree->flags->removed = 1;
}

/* Updates the AVLNode height and balance factor */
void
avlnode_update(struct AVLNode* node)
{
    int left      = (node->left != NULL) ? (int) node->left->height : -1;
    int right     = (node->right != NULL) ? (int) node->right->height : -1;
    node->height  = ((left > right) ? left : right) + 1;
    node->balance = right - left;
}

/* Rebalancing function, initiates rotations (AVLTree rebalancing)
 * if the node balance factor is not part of the {-1, 0, 1} set */
struct AVLNode*
avlnode_rebalance(struct AVLNode* node)
{
    if(node->balance < -1) {
        // AVL invariant must not be broken
        assert(node->balance == -2);
        return (node->left->balance <= 0) ? avlnode_rotate_right(node)
                                          : avlnode_rotate_left_right(node);
    } else if(node->balance > 1) {
        // AVL invariant must not be broken
        assert(node->balance == 2);
        return (node->right->balance >= 0) ? avlnode_rotate_left(node)
                                           : avlnode_rotate_right_left(node);
    }
    return node;
}

/* Rotation function, performs right rotation, after rotation
 * it updates the pivot and root node (height and balance factor) */
struct AVLNode*
avlnode_rotate_right(struct AVLNode* node)
{
    struct AVLNode* new_root = node->left;
    node->left               = new_root->right;
    new_root->right          = node;
    avlnode_update(node);
    avlnode_update(new_root);
    return new_root;
}

/* Rotation function, performs left rotation, after rotation
 * it updates the pivot and root node (height and balance factor) */
struct AVLNode*
avlnode_rotate_left(struct AVLNode* node)
{
    struct AVLNode* new_root = node->right;
    node->right              = new_root->left;
    new_root->left           = node;
    avlnode_update(node);
    avlnode_update(new_root);
    return new_root;
}

/* Rotation function, performs multiple rotations by first rotating the right
 * child of the root by applying the right rotation on it, then rotates the root
 * and pivot (new right child) by applying left rotation */
struct AVLNode*
avlnode_rotate_right_left(struct AVLNode* node)
{
    node->right = avlnode_rotate_right(node->right);
    return avlnode_rotate_left(node);
}

/* Rotation function, performs multiple rotations by first rotating the left
 * child of the root by applying the left rotation on it, then rotates the root
 * and pivot (new left child) by applying right rotation */
struct AVLNode*
avlnode_rotate_left_right(struct AVLNode* node)
{
    node->left = avlnode_rotate_left(node->left);
    return avlnode_rotate_right(node);
}

// Debug section--------------------------------------------------------------
void
print_tree(struct AVLTree* tree)
{
    print_node(tree->root);
}

void
print_node(struct AVLNode* node)
{
    if(node != NULL) {
        print_node(node->left);
        print_node(node->right);
    }
}
