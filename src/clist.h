#ifndef __COL_LIST_H__
#define __COL_LIST_H__

#if !defined(__COL_LIB_INSIDE__) && !defined(__COL_LIST_C_FILE__)
#error "Only <collib.h> can be included directly."
#endif

#define __COL_H_FILE__
#include "core.h"
#undef __COL_H_FILE__
#include <stdbool.h>

/*
 * Opaque type, representing linked list.
 */
typedef struct CList Clist;

/*
 * Creates a new empty 'Clist'.
 */
Clist *clist_new(void);

/*
 * Removes the value from the front of
 * the list and returns the pointer to that
 * value.
 *
 * In case user passes in NULL, function
 * also returns NULL back.
 */
cptr clist_pop_front(Clist *);

/*
 * Removes the value from the back of
 * the list and returns the pointer to that
 * value.
 *
 * In case user passes in NULL, function
 * also returns NULL back.
 */
cptr clist_pop_back(Clist *);

/*
 * Inserts the value at the front of the list.
 *
 * In case user passes in NULL, function
 * also returns NULL back.
 */
Clist *clist_push_front(Clist *, cconstptr);

/*
 * Inserts the value at the back of the list.
 *
 * In case user passes in NULL, function
 * also returns NULL back.
 */
Clist *clist_push_back(Clist *, cconstptr);

/*
 * Returns the length of the list.
 * Returns -1 if the list is NULL.
 */
int clist_len(const Clist *);

/*
 * Returns the pointer to the value at the
 * front of the list.
 *
 * Note: Freeing value via this pointer
 * directly is undefined behaviour.
 * However data can be treated as mutable.
 *
 * If you only want to remove (free) the
 * value from the list, use pop functions
 * instead.
 */
cptr clist_front(const Clist *);

/*
 * Returns the pointer to the value at the
 * back of the list.
 *
 * Note: Freeing value via this pointer
 * directly is undefined behaviour.
 * However data can be treated as mutable.
 *
 * If you only want to remove (free) the
 * value from the list, use pop functions
 * instead.
 */
cptr clist_back(const Clist *);

/*
 * Checks if the list contains DATA using CMP
 * function and pop's the found value from the list
 * returning the pointer to it if it is found in the list.
 * Otherwise it returns NULL.
 *
 * In case user passes in NULL for any of the function
 * parameters function returns NULL.
 * Except if only the DATA is null and the list stores NULL
 * pointers then it won't return NULL.
 */
cptr clist_pop(Clist *, cconstptr data, CCompareKeyFn cmp);

/*
 * Checks if the list contains DATA using CMP
 * function.
 * Returns 'true' if the DATA is found.
 * Returns 'false' if list doesn't contain the value.
 *
 * In case user passes in NULL for any of the function
 * parameters function returns NULL.
 * Except if only the DATA is null and the list stores NULL
 * pointers then it won't return NULL.
 */
bool clist_contains(const Clist *, cconstptr data, CCompareKeyFn cmp);

/*
 * Checks if the list contains DATA using CMP
 * function.
 * Returns the pointer to the value if it is found.
 * Returns NULL otherwise.
 *
 * In case user passes in NULL for any of the function
 * parameters function returns NULL.
 * Except if only the DATA is null and the list stores NULL
 * pointers then it won't return NULL.
 */
cptr clist_find(const Clist *, cconstptr data, CCompareKeyFn cmp);

/*
 * Returns true if the list is empty,
 * returns false if the list is not empty
 * or the Clist is NULL.
 */
bool clist_is_empty(const Clist *);

/*
 * Free's the entire list and all the
 * values inside.
 */
void clist_free(Clist *);

/*
 * Opaque type representing CListCursor abstraction.
 */
typedef struct Cursor CListCursor;

/*
 * Creates a new cursor.
 * Returns NULL if the pointer to the list is NULL.
 */
CListCursor *clist_cursor(Clist *);

/*
 * Moves CListCursor to the next element
 * in the list.
 */
void cursor_move_next(CListCursor *);

/*
 * Moves CListCursor to the previous element
 * in the list.
 */
void cursor_move_next(CListCursor *);

/*
 * Returns pointer to the node that is
 * currently pointed at.
 */
cptr cursor_current(CListCursor *);

/*
 * Peeks at the next value in the list
 * returning pointer to it.
 * Returns NULL if the next value doesn't exist,
 * or the CListCursor is NULL.
 */
cptr cursor_peek_next(CListCursor *);

/*
 * Peeks at the previous value in the list
 * returning pointer to it.
 * Returns NULL if the next value doesn't exist,
 * or the CListCursor is NULL.
 */
cptr cursor_peek_prev(CListCursor *);

/*
 * Splits the underlying list before the cursor
 * into two lists.
 *
 * Returns pointer to the new list or returns
 * NULL if pointer to the cursor is NULL or the
 * new list allocation failed (out of memory).
 */
Clist *cursor_split_before(CListCursor *);

/*
 * Splits the underlying list after the cursor
 * into two lists.
 *
 * Returns pointer to the new list or returns
 * NULL if pointer to the cursor is NULL or the
 * new list allocation failed (out of memory).
 */
Clist *cursor_split_after(CListCursor *);

/*
 * Combines the underlying list with the OTHER_LIST
 * before the node the cursor is pointing at.
 */
void cursor_splice_before(CListCursor *, Clist **other_list);

/*
 * Combines the underlying list with the OTHER_LIST
 * after the node the cursor is pointing at.
 */
void cursor_splice_after(CListCursor *, Clist **other_list);

/*
 * Free's the cursor, it doesn't free the
 * underlying CList.
 */
void cursor_free(CListCursor *);

#endif
