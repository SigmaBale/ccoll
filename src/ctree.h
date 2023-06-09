#ifndef __COL_TREE_H__
#define __COL_TREE_H__

#if !defined(__COL_LIB_INSIDE__) && !defined(__COL_TREE_C_FILE__) &&           \
    !defined(__COL_TEST__)
#error "Only <collib.h> can be included directly."
#endif

#define __COL_H_FILE__
#include "ccore.h"
#undef __COL_H_FILE__

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

typedef struct ctree ctree;

typedef struct ctree_node ctree_node;

typedef struct ctree_iterator ctree_iterator;

typedef struct ctree_iter ctree_iter;


/*
 * 'CTree' constructor.
 *
 * Returns pointer to the 'CTree'.
 * Requires 'CCompareKeyFn', a comparison function
 * for sorting the keys (fn pointer).
 *
 * Optional argument 'CFreeKeyFn', a function pointer
 * if provided will run each time we remove key
 * from the 'CTree' with the removed key as its
 * argument.
 * If it is NULL *USER* is required to free the memory
 * not the CTree.
 *
 * 'CFreeValueFn' is the same concept as free key fn,
 * if user provides it then all the values will be
 * freed either when the value gets updated or key removed
 * together with the value.
 * If it is NULL then the user must free the memory.
 */
ctree *ctree_new(CCompareKeyFn, CFreeKeyFn, CFreeValueFn, CClone, CClone);

/*
 * Free's up the tree and additionally all
 * key value pairs inside of it only and only if the user
 * provided 'CFreeKeyFn' and 'CFreeValueFn' when he created
 * the tree using 'ctree_new'.
 *
 * If 'CFreeKeyFn' wasn't provided key won't be freed.
 * If 'CFreeValueFn' wasn't provided value won't be freed.
 *
 * Either way the tree will get freed.
 */
void ctree_free(ctree *tree);

/*
 * Inserts key-value pair in the tree.
 *
 * Returns false if the tree is NULL or
 * key is NULL, additionally returns false
 * if the key already existed in the tree
 * and only its value got updated.
 *
 * Returns true if the key was inserted.
 */
bool ctree_insert(ctree *tree, cptr_t key, cptr_t value);

/*
 * Removes the 'cptr_t' from the 'CTree'.
 * Returns true if key was removed.
 * Returns false if key was not found.
 *
 * Key and/or value get freed if their
 * free functions were provided upong
 * creation of the tree.
 *
 * Otherwise user is required to free
 * the key and/or value.
 */
bool ctree_remove(ctree *tree, cptr_t key, bool return_ele);

/*
 * Returns current size of the tree.
 * Size being total amount of keys.
 */
uint ctree_size(ctree *tree);

/*
 * Returns size of the tree in bytes.
 * Size being total amount of keys * size of node.
 */
size_t ctree_size_bytes(ctree *tree);

/*
 * Returns pointer to the value of the key/value pair.
 * Returns NULL if key is not inside the tree.
 */
cptr_t ctree_entry(ctree *tree, cptr_t key);

/*
 * Returns pointer to the key of the key/value pair.
 * Returns NULL if key is not inside the tree.
 */
cptr_t ctree_key(ctree *tree, cptr_t key);

#endif
