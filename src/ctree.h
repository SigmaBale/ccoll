#ifndef CLTREE_H
#define CLTREE_H

#if !defined(_COLLIB_INSIDE_) && !defined(_CTREE_SOURCE_FILE_) && !defined(_CTREE_TEST_FILE_)
#error "Only <collib.h> can be included directly."
#endif

#include "core.h"
#include <stdbool.h>

typedef struct _CTree CTree;

/*
 * 'CTree' constructor.
 * Returns pointer to the 'CTree'.
 * Requires 'CCompareKeyFn', a comparison function
 * for sorting the keys (fn pointer).
 * Optional argument 'CFreeKeyFn', a function pointer
 * if provided will run each time we remove key
 * from the 'CTree' with the removed key as its
 * argument.
 * If NULL *USER* is required to free the memory
 * not the CTree.
 */
CTree *ctree_new(CCompareKeyFn, CFreeKeyFn);

/*
 * Inserts the 'cptr' (key) into the 'CTree'.
 * ('CTree' accepts duplicate key values)
 * If the value is inserted it returns true,
 * otherwise false. If the 'CTree' or 'cptr'
 * are NULL then it also returns false.
 */
bool ctree_insert(CTree *, cptr);

/*
 * Removes the 'cptr' from the 'CTree'.
 *
 */
bool ctree_remove(CTree *, cptr);

uint ctree_size(CTree*);

size_t ctree_size_bytes(CTree*);

#endif
