#ifndef __COL_LIST_H__
#define __COL_LIST_H__

#if !defined(__COL_LIB_INSIDE__) && !defined(__COL_LIST_C_FILE__) &&           \
    !defined(__COL_TEST__)
#error "Only <collib.h> can be included directly."
#endif

#define __COL_H_FILE__
#include "ccore.h"
#undef __COL_H_FILE__
#include <stdbool.h>

/*
 * Opaque type 'clist', representing doubly linked list.
 */
typedef struct _clist clist;

/*
 * Creates a new empty 'clist'.
 *
 * FREE_FN is a function pointer that is called everytime on each
 * stored value in the list when you call 'clist_free' on it.
 * This will free all the values stored inside as described
 * in the FREE_FN.
 *
 * If user does not provide the FREE_FN (inputs NULL),
 * then the user is responsible for freeing the values.
 */
clist *clist_new(CFreeValueFn FREE_FN);

/*
 * Removes the value from the front of the LIST and returns it
 * to the callee.
 *
 * In case user passes in NULL or the LIST is empty, function
 * returns NULL.
 */
cptr_t clist_pop_front(clist *LIST);

/*
 * Removes the VALUE from the back of the LIST and returns it
 * to the callee.
 *
 * In case user passes in NULL or the LIST is empty, function
 * returns NULL.
 */
cptr_t clist_pop_back(clist *LIST);

/*
 * Inserts the value at the front of the LIST.
 *
 * In case user passes in NULL or allocation fails, function
 * returns NULL.
 */
bool clist_push_front(clist *LIST, cconstptr_t VALUE);

/*
 * Inserts the VALUE at the back of the LIST.
 *
 * In case user passes in NULL or allocation fails, function
 * returns NULL.
 */
bool clist_push_back(clist *LIST, cconstptr_t VALUE);

/*
 * Returns the length of the LIST.
 * Returns '-1' if passed in LIST is NULL.
 */
int clist_len(const clist *LIST);

/*
 * Returns the pointer to the value at the front of the LIST.
 *
 * Warning: Freeing value via this pointer directly is
 * UNDEFINED BEHAVIOUR.
 *
 * This function is used for 'peeking' at the value. Mutating
 * the value will not cause undefined behaviour but it's not
 * reccomended.
 *
 * If you only want to remove (free) the value from the list,
 * use pop functions instead.
 */
cconstptr_t clist_front(const clist *LIST);

/*
 * Returns the pointer to the value at the back of the LIST.
 *
 * Warning: Freeing value via this pointer directly is
 * UNDEFINED BEHAVIOUR.
 *
 * This function is used for 'peeking' at the value. Mutating
 * the value will not cause undefined behaviour but it's not
 * reccomended.
 *
 * If you only want to remove (free) the value from the list,
 * use pop functions instead.
 */
cconstptr_t clist_back(const clist *LIST);

/*
 * Tries to pop the value off the LIST returning the pointer of
 * the value to the callee.
 * CMP function pointer is used in finding the DATA inside the LIST.
 *
 * In case user passes in NULL for any of the function parameters or
 * value is not found, function returns NULL.
 */
cptr_t clist_pop(clist *LIST, cconstptr_t DATA, CCompareKeyFn CMP);

/*
 * Checks if the LIST contains DATA using CMP function.
 * Returns 'TRUE' if the DATA is found.
 * Returns 'FALSE' if LIST doesn't contain DATA.
 *
 * In case user passes in NULL for any of the function parameters or
 * value is not found, function returns NULL.
 */
bool clist_contains(const clist *LIST, cconstptr_t DATA, CCompareKeyFn CMP);

/*
 * Checks if the list contains DATA using CMP function.
 * Returns 'const' pointer to the value if it's found.
 * This function should be used only for immutable operations
 * on the returned value (read only).
 * For mutating the value use 'clist_find_mut'.
 *
 * Warning: Freeing the value that gets returned is UNDEFINED
 * BEHAVIOUR.
 *
 * In case user passes in NULL for any of the function parameters or
 * value is not found, function returns NULL.
 */
cconstptr_t clist_find(const clist *LIST, cconstptr_t DATA, CCompareKeyFn CMP);

/*
 * Checks if the list contains DATA using CMP function.
 * Returns pointer to the value if it's found.
 * This function can be used in the context where the returned value
 * is or is expected to get mutated.
 *
 * Warning: Freeing the value that gets returned is UNDEFINED
 * BEHAVIOUR.
 *
 * In case user passes in NULL for any of the function parameters or
 * value is not found, function returns NULL.
 */
cptr_t clist_find_mut(const clist *LIST, cconstptr_t DATA, CCompareKeyFn CMP);

/*
 * Returns TRUE if the LIST is empty.
 * Returns FALSE if the LIST is not empty or the LIST is NULL.
 */
bool clist_is_empty(const clist *LIST);

/*
 * Free's the entire LIST and the values pushed into it if the
 * 'CFreeValueFn' was provided when constructing the LIST.
 */
void clist_free(clist *LIST);

/*
 * Opaque type representing Cursor abstraction.
 */
typedef struct _ccursor ccursor;

/*
 * Creates a new 'ccursor'.
 * Returns NULL if LIST is NULL or allocation failed.
 */
ccursor *clist_cursor(clist *LIST);

/*
 * Moves CURSOR to the next element in the list.
 */
void cursor_move_next(ccursor *CURSOR);

/*
 * Moves CURSOR to the previous element in the list.
 */
void cursor_move_prev(ccursor *CURSOR);

/*
 * Returns value that the CURSOR is pointing at.
 * If CURSOR is NULL function returns NULL.
 */
cptr_t cursor_current(ccursor *CURSOR);

/*
 * Peeks at the next value after the CURSOR.
 * If CURSOR is NULL function returns NULL.
 */
cptr_t cursor_peek_next(ccursor *CURSOR);

/*
 * Peeks at the prev value before the CURSOR.
 * If CURSOR is NULL function returns NULL.
 */
cptr_t cursor_peek_prev(ccursor *CURSOR);

/*
 * Splits the underlying list before the CURSOR into two lists.
 * Returns the list before the CURSOR. Returns NULL if the
 * CURSOR is NULL or the allocation failed.
 */
clist *cursor_split_before(ccursor *CURSOR);

/*
 * Splits the underlying list after the CURSOR into two lists.
 * Returns the list after the CURSOR. Returns NULL if the
 * CURSOR is NULL or the allocation failed.
 */
clist *cursor_split_after(ccursor *CURSOR);

/*
 * Combines the underlying list with the OTHER_LIST 'before' CURSOR.
 * OTHER_LIST after combining with CURSORs list is freed.
 * Function takes pointer to the pointer of the OTHER_LIST, that way
 * after function returns the pointer to the OTHER_LIST is nulled.
 */
void cursor_splice_before(ccursor *CURSOR, clist **OTHER_LIST);

/*
 * Combines the underlying list with the OTHER_LIST 'after' CURSOR.
 * OTHER_LIST after combining with CURSORs list is freed.
 * Function takes pointer to the pointer of the OTHER_LIST, that way
 * after function returns the pointer to the OTHER_LIST is nulled.
 */
void cursor_splice_after(ccursor *CURSOR, clist **OTHER_LIST);

/*
 * 'ccursor' destructor, only the CURSOR gets freed, underlying
 * list is untouched.
 */
void cursor_free(ccursor *CURSOR);

#endif
