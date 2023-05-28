#ifndef __COL_TREE_H__
#define __COL_TREE_H__

#if !defined(__COL_LIB_INSIDE__) && !defined(__COL_TREE_C_FILE__)
#error "Only <collib.h> can be included directly."
#endif

#define __COL_H_FILE__
#include "core.h"
#undef __COL_H_FILE__
#include <stdbool.h>
#include <stddef.h>

typedef struct _CTree CTree;

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
CTree *ctree_new(CCompareKeyFn, CFreeKeyFn, CFreeValueFn);

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
void ctree_free(CTree *tree);

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
bool ctree_insert(CTree *tree, cptr key, cptr value);

/*
 * Removes the 'cptr' from the 'CTree'.
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
bool ctree_remove(CTree *tree, cptr key);

/*
 * Returns current size of the tree.
 * Size being total amount of keys.
 */
cuint ctree_size(CTree *tree);

/*
 * Returns size of the tree in bytes.
 * Size being total amount of keys * size of node.
 */
size_t ctree_size_bytes(CTree *tree);

/*
 * Returns pointer to the value of the given key.
 * Returns NULL if key is not inside the tree.
 */
cptr ctree_entry(CTree *tree, cptr key);

#endif
