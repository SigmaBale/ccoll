/*
 * Linked list is a linear collection of data
 * elements. It is not to be confused with
 * arrays, list elements are not sequential
 * in memory, instead they are 'linked' together
 * only by their references to each other (pointers).
 *
 * For example, finding some value inside an array
 * is equivalent to finding the index of the value
 * in the array multiplied by the size of the element itself
 * and that product added to the base pointer.
 * Base pointer being the start of the allocation
 * for that sequence (array).
 *
 * When it comes to linked list, you can't acess the
 * element at the 'middle' of the list or near the 'end'
 * by offset, you can't access any arbitrary node in
 * the list before actually traversing the list from the
 * start or the end in the case of doubly linked list.
 *
 * Think of it as a chain of nodes each pointing to the next
 * node or NULL (if the node is the tail of the list) and
 * to its previous neighbour or NULL (if the node is
 * the head of the list).
 *
 * Doubly linked list:
 * NULL <- (A) <-> (B) <-> (C) <-> (D) <-> (E) -> NULL
 *
 * Time complexity for inserting/removing at the start/end of the
 * doubly linked list is O(1), while inserting/removing
 * anywhere except the start/end is O(n) (Peek time) + O(1).
 * Peek time complexity is O(n).
 * Space complexity is O(n).
 */

#define __COL_LIST_C_FILE__
#include "clist.h"
#undef __COL_LIST_C_FILE__
#include <assert.h>
#include <memc.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * 'Node' is a representative of a value that is stored
 * inside the linked list, it is a core structure that
 * forms the linked list.
 *
 * Contains pointer to the 'data', pointers to the 'next'
 * and 'prev' (previous) Node in the list.
 */
typedef struct _clist_node {
    cptr                data;
    struct _clist_node* next;
    struct _clist_node* prev;
} clist_node;

/*
 * 'CList' is a entry point to the linked list.
 * It holds pointers to the 'head' (start) and the
 * 'tail' (end) of the list.
 *
 * 'len' is a value representing total amount of
 * nodes in the CList.
 * It provides us with some usefull helper functions
 * and usefull API for the user.
 * It also is used in functions defined on the CListCursor
 * struct (read more about Cursor above its definition),
 * that gives us some neat features such as splicing
 * and spliting the linked list.
 */
struct _clist {
    clist_node*  head;
    clist_node*  tail;
    cuint        len;
    CFreeValueFn free_val_fn;
};

static clist_node* node_new(cconstptr);
static cptr        node_free(clist_node*);

/*
 * CList constructor, returns the pointer
 * to the underlying structure allocated on the heap.
 */
clist*
clist_new(CFreeValueFn free_val_fn)
{
    clist* list = memc_malloc(clist);
    if(list != NULL) {
        list->head        = NULL;
        list->tail        = NULL;
        list->len         = 0;
        list->free_val_fn = free_val_fn;
    }
    return list;
}

/*
 * Removes the value from the front (head) of the list,
 * returning the pointer to the value that was stored in the list.
 */
cptr
clist_pop_front(clist* clist)
{
    return_val_if_fail(clist != NULL && clist->head != NULL, NULL);

    clist_node* old_head = clist->head;
    clist->head          = old_head->next;

    if(clist->head != NULL)
        clist->head->prev = NULL;
    else
        clist->tail = NULL;

    clist->len--;

    return node_free(old_head);
}

/*
 * Removes the value from the back (tail) of the list,
 * returning the pointer to the value that was stored in the list.
 */
cptr
clist_pop_back(clist* clist)
{
    return_val_if_fail(clist != NULL && clist->tail != NULL, NULL);

    clist_node* old_tail = clist->tail;
    clist->tail          = old_tail->prev;

    if(clist->tail != NULL)
        clist->tail->next = NULL;
    else
        clist->head = NULL;

    clist->len--;

    return node_free(old_tail);
}

/*
 * Removes the given value/data in the list if the value/data
 * is found by comparing the list values to the value to be removed,
 * using the provided comparison function.
 * Returns NULL if the value was not found, or any of the parameter
 * pointers is NULL.
 */
cptr
clist_pop(clist* clist, cconstptr data, CCompareKeyFn cmp)
{
    return_val_if_fail(clist != NULL && data != NULL && cmp != NULL, NULL);

    clist_node* current;
    cuint       idx = 0;
    for(current = clist->head; current != NULL; current = current->next) {
        if(cmp(current->data, data) == 0) {
            if(idx == 0)
                clist->head = current->next;
            else if(idx == clist->len - 1)
                clist->tail = current->prev;

            if(current->next != NULL)
                current->next->prev = current->prev;
            if(current->prev != NULL)
                current->prev->next = current->next;

            clist->len--;
            return node_free(current);
        }
        idx++;
    }

    return NULL;
}

/*
 * Pushes/Inserts the value in the front (head) of the list.
 * Returns pointer to the list if the value is inserted.
 * Returns NULL if the pointer to the list is NULL or if the
 * value couldn't be allocated.
 * In case of inability to allocate the value inside the list,
 * the list will stay in its previous state before insertion.
 */
bool
clist_push_front(clist* clist, cconstptr data)
{
    return_val_if_fail(clist != NULL && data != NULL, false);

    clist_node* new_head = node_new(data);

    if(new_head == NULL)
        return false;

    if(clist->head != NULL) {
        clist->head->prev = new_head;
        new_head->next    = clist->head;
    } else
        clist->tail = new_head;

    clist->head = new_head;
    clist->len++;

    return true;
}

/*
 * Pushes/Inserts the value in the back (tail) of the list.
 * Returns pointer to the list if the value is inserted.
 * Returns NULL if the pointer to the list is NULL or if the
 * value couldn't be allocated.
 * In case of inability to allocate the value inside the list,
 * the list will stay in its previous state before insertion.
 */
bool
clist_push_back(clist* clist, cconstptr data)
{
    return_val_if_fail(clist != NULL && data != NULL, false);

    clist_node* new_tail = node_new(data);

    if(new_tail == NULL)
        return false;

    if(clist->tail != NULL) {
        clist->tail->next = new_tail;
        new_tail->prev    = clist->tail;
    } else
        clist->head = new_tail;

    clist->tail = new_tail;
    clist->len++;

    return true;
}

/*
 * Returns the len of the list.
 * If the pointer to the list is NULL,
 * it returns -1.
 */
int
clist_len(const clist* clist)
{
    return_val_if_fail(clist != NULL, -1);

    return clist->len;
}

/*
 * Returns pointer to the data stored in
 * the front (head) of the list.
 * Returns NULL in case pointer to the list
 * is NULL.
 *
 * Note:
 * Value is free to be mutated, but
 * be aware of UB especially if you free
 * the value while it is still 'stored'
 * in the list.
 */
cconstptr
clist_front(const clist* clist)
{
    return_val_if_fail(clist != NULL, NULL);

    if(clist->head != NULL)
        return clist->head->data;
    else
        return NULL;
}

/*
 * Returns pointer to the data stored in
 * the back (tail) of the list.
 * Returns NULL in case pointer to the list
 * is NULL.
 *
 * Note:
 * Value is free to be mutated, but
 * be aware of UB especially if you free
 * the value while it is still 'stored'
 * in the list.
 */
cconstptr
clist_back(const clist* clist)
{
    return_val_if_fail(clist != NULL, NULL);

    if(clist->tail != NULL)
        return clist->tail->data;
    else
        return NULL;
}

/*
 * Returns either true if the value is found in
 * the list or false if its not, or if any of the
 * parameter pointers is NULL.
 */
bool
clist_contains(const clist* clist, cconstptr data, CCompareKeyFn cmp)
{
    return_val_if_fail(clist != NULL && data != NULL && cmp != NULL, NULL);

    return (bool) (clist_find(clist, data, cmp) != NULL);
}

/*
 * Wrapper around clist_find, exactly the same functionality
 * but it should be used in the context in which you expect
 * to mutate the value or the value might get mutated.
 *
 * Freeing the value is 'Undefined Behaviour'.
 */
cptr
clist_find_mut(const clist* clist, cconstptr data, CCompareKeyFn cmp)
{
    return (cptr) clist_find(clist, data, cmp);
}

/*
 * Returns const pointer to the value in the list if its found,
 * if not it returns NULL, also returns NULL if any of the
 * parmeters of the function are NULL.
 *
 * Returns 'const' ptr because it expects the value to not be mutated.
 * In case you try to free the value it is 'Undefined Behaviour'.
 *
 * If you need mutation or expect the value to get mutated use
 * 'clist_find_mut' instead.
 */
cconstptr
clist_find(const clist* clist, cconstptr data, CCompareKeyFn cmp)
{
    return_val_if_fail(clist != NULL && cmp != NULL && data != NULL, NULL);

    clist_node* current;
    for(current = clist->head; current != NULL; current = current->next)
        if(cmp(current->data, data) == 0)
            return current->data;

    return NULL;
}

/*
 * Returns TRUE if the CLIST is empty.
 * Returns FALSE if CLIST is NULL or
 * if the list is not empty.
 */
bool
clist_is_empty(const clist* clist)
{
    return_val_if_fail(clist != NULL, false);

    return (bool) (clist->len == 0);
}

/*
 * Frees the entire list, it also frees
 * all the values stored inside if the
 * 'CFreeValueFn' was provided.
 */
void
clist_free(clist* clist)
{
    void* val;
    while((val = clist_pop_back(clist)) != NULL)
        if(clist->free_val_fn)
            clist->free_val_fn(val);

    assert(clist->len == 0);
    assert(clist->head == NULL);
    assert(clist->tail == NULL);

    free(clist);
}

/*
 * 'clist_node' constructor (private function).
 * Returns NULL only if the allocation fails.
 */
clist_node*
node_new(cconstptr data)
{
    assert(data != NULL);
    clist_node* node = memc_malloc(clist_node);
    if(node != NULL) {
        node->next = NULL;
        node->prev = NULL;
        node->data = (cptr) data;
    }
    return node;
}

/*
 * Node destructor (private function).
 * Frees the node returning the pointer
 * to the data stored in it.
 */
cptr
node_free(clist_node* node)
{
    assert(node != NULL);
    cptr temp  = node->data;
    node->next = NULL;
    node->prev = NULL;
    node->data = NULL;
    free(node);
    return temp;
}

/*
 * Cursor is abstraction that allows us
 * to do a bit more complex manipulation and
 * interaction with other linked lists.
 *
 * It stores the linked list from which it is
 * created, current node that the cursor is 'pointing'
 * at and the index aka current position in the linked
 * list.
 *
 * We say cursor is moving to the next element when its
 * index is increasing (we are traversing the list from front to back),
 * and we also say cursor is moving to the previous element
 * when its index is decreasing (we are traversing the list
 * form back to the front).
 *
 * Term 'before cursor' is meant in a sense the values or the
 * positions before the current node the cursor points to.
 * Term 'after cursor' is understood as all the values or the
 * positions after the current node the cursor points to.
 *
 * -------------------------------------------------------------------
 * For example given a list:
 *
 *        front            back
 * NULL <- (C) <-> (D) <-> (E) -> NULL
 *          0       1       2
 * -------------------------------------------------------------------
 *
 * -------------------------------------------------------------------
 * If the Cursor is pointing to the D then:
 *
 * |<----BEFORE--->| |<----AFTER---->|
 * |               | |               |
 * |      front   cursor   back      |
 * NULL <- (C) <-> (D) <-> (E) -> NULL
 *          0       1       2
 * -------------------------------------------------------------------
 */
struct _ccursor {
    clist_node* node;
    clist*      clist;
    long int    index;
};

/* PUBLIC API-------------------------------------------------------*/

/*-----------------------------------------------------------------*/

/*
 * 'ccursor' constructor.
 * Returns NULL if the CLIST is NULL or
 * the allocation failed.
 */
ccursor*
clist_cursor(clist* clist)
{
    return_val_if_fail(clist != NULL, NULL);

    ccursor* cursor = (ccursor*) malloc(sizeof(ccursor));
    if(cursor != NULL) {
        cursor->clist = clist;
        cursor->node  = NULL;
        cursor->index = -1;
    }
    return cursor;
}

/*
 * Moves CListCursor to the next element
 * in the list.
 * Does nothing if pointer to the CListCursor
 * is NULL.
 */
void
cursor_move_next(ccursor* cursor)
{
    if(cursor != NULL) {
        clist_node* current;
        if((current = cursor->node) != NULL) {

            cursor->node = current->next;

            if(cursor->node != NULL)
                cursor->index++;
            else
                cursor->index = -1; // Index is None or in between the head and tail

        } else if(cursor->clist->len != 0) {
            cursor->node  = cursor->clist->head;
            cursor->index = 0;
        } else {
            return;
        }
    }
}

/*
 * Moves CURSOR to the previous element
 * in the list.
 * Does nothing if pointer to the CListCursor
 * is NULL.
 */
void
cursor_move_prev(ccursor* cursor)
{
    if(cursor != NULL) {
        clist_node* current;
        if((current = cursor->node) != NULL) {
            cursor->node = current->prev;

            if(cursor->node != NULL)
                cursor->index--;
            else
                cursor->index = -1; // Index is None or in between the head and tail

        } else if(cursor->clist->len != 0) {
            cursor->node  = cursor->clist->tail;
            cursor->index = cursor->clist->len - 1;
        } else {
            return;
        }
    }
}

/*
 * Returns the value 'under' the CURSOR.
 * If CURSOR is NULL or CURSOR is pointing
 * at NULL node, then it returns NULL.
 */
cptr
cursor_current(ccursor* cursor)
{
    return_val_if_fail(cursor != NULL, NULL);

    return (cursor->node) ? cursor->node->data : NULL;
}

/*
 * Peeks at the next value and returns it.
 * Returns NULL if the value is NULL or CURSOR is NULL.
 */
cptr
cursor_peek_next(ccursor* cursor)
{
    return_val_if_fail(cursor != NULL, NULL);

    clist_node* return_val;
    return_val = (cursor->node) ? cursor->node->next : cursor->clist->head;

    return (return_val) ? return_val->data : NULL;
}

/*
 * Peeks at the prev value and returns it.
 * Returns NULL if the value is NULL or CURSOR is NULL.
 */
cptr
cursor_peek_prev(ccursor* cursor)
{
    return_val_if_fail(cursor != NULL, NULL);

    clist_node* return_val;
    return_val = (cursor->node) ? cursor->node->prev : cursor->clist->tail;

    return (return_val) ? return_val->data : NULL;
}

/*
 * Splits the underlying list before the cursor
 * into two lists.
 * Updates the new index for the cursor.
 *
 * Returns pointer to the new list or returns
 * NULL if CURSOR is NULL or the new list allocation failed.
 *
 * New list will inherit the 'CFreeValueFn' if
 * it was provided to the original list.
 */
clist*
cursor_split_before(ccursor* cursor)
{
    return_val_if_fail(cursor != NULL, NULL);

    clist_node* current;
    clist_node* prev;
    clist*      new_list = clist_new(cursor->clist->free_val_fn);
    clist*      temp;

    if(new_list == NULL)
        return NULL;

    if((current = cursor->node) != NULL) {
        if((prev = cursor->node->prev) != NULL) {
            clist* list = cursor->clist;

            clist_node* old_head = list->head;
            int         old_idx  = cursor->index;
            int         old_len  = list->len;

            list->head    = current;
            list->len     = list->len - old_idx;
            cursor->index = 0;

            prev->next    = NULL;
            current->prev = NULL;

            new_list->head = old_head;
            new_list->tail = prev;
            new_list->len  = old_len - list->len;

            return new_list;
        } else
            return new_list;
    } else {
        temp = memc_replace(cursor->clist, new_list, sizeof(clist));
        free(new_list);
        return temp;
    }
}

/*
 * Splits the underlying list after the cursor
 * into two lists.
 * Updates the new index for the cursor.
 *
 * Returns pointer to the new list or returns
 * NULL if CURSOR is NULL or the new list allocation failed.
 *
 * New list will inherit the 'CFreeValueFn' if
 * it was provided to the original list.
 */
clist*
cursor_split_after(ccursor* cursor)
{
    return_val_if_fail(cursor != NULL, NULL);

    clist_node* current;
    clist_node* next;
    clist*      new_list = clist_new(cursor->clist->free_val_fn);
    clist*      temp;

    if(new_list == NULL)
        return NULL;

    if((current = cursor->node) != NULL) {
        if((next = cursor->node->next) != NULL) {
            clist* list = cursor->clist;

            clist_node* old_tail = list->tail;
            int         old_len  = list->len;

            list->tail = current;
            list->len  = cursor->index + 1;

            current->next = NULL;
            next->prev    = NULL;

            new_list->head = next;
            new_list->tail = old_tail;
            new_list->len  = old_len - list->len;

            return new_list;
        } else {
            return new_list;
        }
    } else {
        temp = memc_replace(cursor->clist, new_list, sizeof(clist));
        free(new_list);
        return temp;
    }
}

/*
 * ------------------------------------------------------------
 * ------------------------------------------------------------
 * Combines the underlying list with the other one
 * before the node the cursor is pointing at.
 *
 * If the 'CFreeValueFn' functions do not match (it is fine
 * if they are both NULL aka not provided) then the splicing
 * won't happen.
 *
 * Splicing the list with itself is 'Undefined Behaviour'!
 * ------------------------------------------------------------
 * ------------------------------------------------------------
 *
 * Example:
 *
 * Underlying list:
 *
 *        front   cursor   back
 * NULL <- (A) <-> (B) <-> (C) -> NULL
 *
 *
 * Other list:
 *
 * NULL <- (D) <-> (E) <-> (F) -> NULL
 *
 * ------------------------------------------------------------
 * ------------------------------------------------------------
 *
 *
 *        front                           cursor   back
 * NULL <- (A) <-> (D) <-> (E) <-> (F) <-> (B) <-> (C) -> NULL
 *                  |               |
 *                  |<-----new----->|
 *
 * ------------------------------------------------------------
 * ------------------------------------------------------------
 */
void
cursor_splice_before(ccursor* cursor, clist** other_list)
{
    if(cursor != NULL && other_list != NULL && *other_list != NULL
       && cursor->clist->free_val_fn == (*other_list)->free_val_fn)
    {
        clist_node*  current;
        unsigned int swapped = 0;
        if((current = cursor->node) != NULL) {
            clist_node* other_head = (*other_list)->head;
            clist_node* other_tail = (*other_list)->tail;
            clist_node* prev       = current->prev;

            if(prev != NULL) {
                prev->next       = other_head;
                other_head->prev = prev;
            } else
                cursor->clist->head = other_head;

            other_tail->next = current;
            current->prev    = other_tail;
            cursor->index    += (*other_list)->len;
        } else if(cursor->clist->tail != NULL) {
            clist_node* other_tail = (*other_list)->tail;
            clist_node* other_head = (*other_list)->head;

            other_head->prev          = cursor->clist->tail;
            cursor->clist->tail->next = other_head;
            cursor->clist->tail       = other_tail;
        } else {
            memc_swap(cursor->clist, *other_list);
            swapped = 1;
        }

        if(!swapped)
            cursor->clist->len += (*other_list)->len;

        (*other_list)->len  = 0;
        (*other_list)->tail = NULL;
        (*other_list)->head = NULL;
    } else
        return;

    free(*other_list);
    *other_list = NULL;
}

/*
 * ------------------------------------------------------------
 * ------------------------------------------------------------
 * Combines the underlying list with the other one
 * after the node the cursor is pointing at.
 *
 * If the 'CFreeValueFn' functions do not match (it is fine
 * if they are both NULL aka not provided) then the splicing
 * won't happen.
 *
 * Splicing the list with itself is 'Undefined Behaviour'!
 * ------------------------------------------------------------
 * ------------------------------------------------------------
 *
 * Example:
 *
 * Underlying list:
 *
 *        front   cursor   back
 * NULL <- (A) <-> (B) <-> (C) -> NULL
 *
 *
 * Other list:
 *
 * NULL <- (D) <-> (E) <-> (F) -> NULL
 *
 * ------------------------------------------------------------
 * ------------------------------------------------------------
 *
 *
 *        front   cursor                           back
 * NULL <- (A) <-> (B) <-> (D) <-> (E) <-> (F) <-> (C) -> NULL
 *                          |               |
 *                          |<-----new----->|
 *
 *-------------------------------------------------------------
 *-------------------------------------------------------------
 */
void
cursor_splice_after(ccursor* cursor, clist** other_list)
{
    if(cursor != NULL && other_list != NULL && *other_list != NULL
       && cursor->clist->free_val_fn == (*other_list)->free_val_fn)
    {
        clist_node*  current;
        unsigned int swapped;

        if((current = cursor->node) != NULL) {
            clist_node* other_head = (*other_list)->head;
            clist_node* other_tail = (*other_list)->tail;
            clist_node* next       = current->next;

            if(next != NULL) {
                next->prev       = other_tail;
                other_tail->next = next;
            } else
                cursor->clist->head = other_head;

            other_head->prev = current;
            current->next    = other_head;
        } else if(cursor->clist->head != NULL) {
            clist_node* other_tail = (*other_list)->tail;
            clist_node* other_head = (*other_list)->head;

            other_tail->next          = cursor->clist->head;
            cursor->clist->head->prev = other_tail;
            cursor->clist->head       = other_head;
        } else {
            memc_swap(cursor->clist, *other_list);
            swapped = 1;
        }

        if(!swapped)
            cursor->clist->len += (*other_list)->len;

        (*other_list)->len  = 0;
        (*other_list)->tail = NULL;
        (*other_list)->head = NULL;
    } else
        return;

    free(*other_list);
    *other_list = NULL;
}

/*
 * 'ccursor' destructor, it doesn't free any
 * of the underlying structures it only frees
 * itself.
 */
void
cursor_free(ccursor* cursor)
{
    if(cursor != NULL) {
        cursor->node  = NULL;
        cursor->clist = NULL;
        cursor->index = 0;
        free(cursor);
    }
}
