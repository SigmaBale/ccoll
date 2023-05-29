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
typedef struct CListNode {
    cptr              data;
    struct CListNode* next;
    struct CListNode* prev;
} CListNode;

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
struct CList {
    CListNode*   head;
    CListNode*   tail;
    unsigned int len;
};

static CListNode* node_new(cconstptr);
static cptr       node_free(CListNode*);

/*
 * CList constructor, returns the pointer
 * to the underlying structure allocated on the heap.
 */
Clist*
clist_new(void)
{
    Clist* clist = memc_malloc(Clist);
    if(clist != NULL) {
        clist->head = NULL;
        clist->tail = NULL;
        clist->len  = 0;
    }
    return clist;
}

/*
 * Removes the value from the front (head) of the list,
 * returning the pointer to the value that was stored in the list.
 */
cptr
clist_pop_front(Clist* clist)
{
    return_val_if_fail(clist != NULL && clist->head != NULL, NULL);

    CListNode* old_head = clist->head;
    clist->head         = old_head->next;

    if(clist->head != NULL) {
        clist->head->next = NULL;
    } else {
        clist->tail = NULL;
    }

    clist->len--;

    return node_free(old_head);
}

/*
 * Removes the value from the back (tail) of the list,
 * returning the pointer to the value that was stored in the list.
 */
cptr
clist_pop_back(Clist* clist)
{
    return_val_if_fail(clist != NULL && clist->tail != NULL, NULL);

    CListNode* old_tail = clist->tail;
    clist->tail         = old_tail->prev;

    if(clist->tail != NULL)
        clist->tail->next = NULL;
    else
        clist->head = NULL;

    clist->len--;

    return node_free(clist->head);
}

/*
 * Removes the given value/data in the list if the value/data
 * is found by comparing the list values to the value to be removed,
 * using the provided comparison function.
 * Returns NULL if the value was not found, or any of the parameter
 * pointers is NULL.
 */
cptr
clist_pop(Clist* clist, cconstptr data, CCompareKeyFn cmp)
{
    return_val_if_fail(clist != NULL && cmp != NULL, NULL);

    CListNode* current;
    for(current = clist->head; current != NULL; current = current->next) {
        if(cmp(current->data, data) == 0) {
            if(current->next != NULL)
                current->next->prev = current->prev;
            if(current->prev != NULL)
                current->prev->next = current->next;

            clist->len--;
            return node_free(current);
        }
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
Clist*
clist_push_front(Clist* clist, cconstptr data)
{
    return_val_if_fail(clist != NULL, NULL);

    CListNode* new_head = node_new(data);

    if(new_head == NULL)
        return NULL;

    if(clist->head != NULL) {
        clist->head->prev = new_head;
        new_head->next    = clist->head;
    } else
        clist->tail = new_head;

    clist->len++;
    clist->head = new_head;

    return clist;
}

/*
 * Pushes/Inserts the value in the back (tail) of the list.
 * Returns pointer to the list if the value is inserted.
 * Returns NULL if the pointer to the list is NULL or if the
 * value couldn't be allocated.
 * In case of inability to allocate the value inside the list,
 * the list will stay in its previous state before insertion.
 */
Clist*
clist_push_back(Clist* clist, cconstptr data)
{
    return_val_if_fail(clist != NULL, NULL);

    CListNode* new_tail = node_new(data);

    if(new_tail == NULL)
        return NULL;

    if(clist->tail != NULL) {
        clist->tail->next = new_tail;
        new_tail->prev    = clist->tail;
    } else
        clist->head = new_tail;

    clist->len++;
    clist->tail = new_tail;

    return clist;
}

/*
 * Returns the len of the list.
 * If the pointer to the list is NULL,
 * it returns -1.
 */
int
clist_len(const Clist* clist)
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
cptr
clist_front(const Clist* clist)
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
cptr
clist_back(const Clist* clist)
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
clist_contains(const Clist* clist, cconstptr data, CCompareKeyFn cmp)
{
    return_val_if_fail(clist != NULL && cmp != NULL, NULL);

    return (bool) (clist_find(clist, data, cmp) != NULL);
}

/*
 * Returns pointer to the value in the list if its found,
 * if not it returns NULL, also returns NULL if any of the
 * parmeter pointers of the function are NULL.
 *
 * If only DATA is NULL the function won't return NULL.
 *
 * Note:
 * Value is free to be mutated, but you free the
 * value while it is still 'stored' in the list
 * it is undefined behaviour.
 */
cptr
clist_find(const Clist* clist, cconstptr data, CCompareKeyFn cmp)
{
    return_val_if_fail(clist != NULL && cmp != NULL, NULL);

    CListNode* current;
    for(current = clist->head; current != NULL; current = current->next)
        if(cmp(current->data, data) == 0)
            return current->data;

    return NULL;
}

/*
 * Returns true if the list is empty.
 * Returns false if clist is NULL or
 * if the list is not empty.
 */
bool
clist_is_empty(const Clist* clist)
{
    return_val_if_fail(clist != NULL, false);

    return (bool) (clist->len == 0);
}

/*
 * Frees the entire list, including all
 * the values stored inside.
 */
void
clist_free(Clist* clist)
{
    cptr data;

    while((data = clist_pop_back(clist)) != NULL)
        free(data);

    assert(clist->len == 0);
    clist->head = NULL;
    clist->tail = NULL;
    free(clist);
}

/*
 * Node constructor (private function).
 * Returns NULL only if the allocation fails.
 */
CListNode*
node_new(cconstptr data)
{
    assert(data != NULL);
    CListNode* node = memc_malloc(CListNode);
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
 * that was stored in it.
 */
cptr
node_free(CListNode* node)
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
 * to and the index aka current position in the linked
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
struct Cursor {
    CListNode* node;
    Clist*     clist;
    long int   index;
};

/* PUBLIC API-------------------------------------------------------*/

/*-----------------------------------------------------------------*/

/*
 * Cursor constructor.
 * Returns NULL if pointer to the list is NULL.
 */
CListCursor*
clist_cursor(Clist* clist)
{
    return_val_if_fail(clist != NULL, NULL);

    CListCursor* cursor = (CListCursor*) malloc(sizeof(CListCursor));
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
cursor_move_next(CListCursor* cursor)
{
    if(cursor != NULL) {
        if(cursor->node != NULL) {
            cursor->node = cursor->node->next;
            if(cursor->node != NULL) {
                cursor->index++;
            } else {
                cursor->node  = cursor->clist->head;
                cursor->index = 0;
            }
        } else if(!clist_is_empty(cursor->clist)) {
            cursor->node  = cursor->clist->head;
            cursor->index = 0;
        } else {
            return;
        }
    }
}

/*
 * Moves CListCursor to the previous element
 * in the list.
 * Does nothing if pointer to the CListCursor
 * is NULL.
 */
void
cursor_move_prev(CListCursor* cursor)
{
    if(cursor != NULL) {
        if(cursor->node != NULL) {
            cursor->node = cursor->node->prev;

            if(cursor->node != NULL) {
                cursor->index--;
            } else {
                cursor->node  = cursor->clist->tail;
                cursor->index = cursor->clist->len - 1;
            }
        } else if(!clist_is_empty(cursor->clist)) {
            cursor->node  = cursor->clist->tail;
            cursor->index = cursor->clist->len - 1;
        } else {
            return;
        }
    }
}

/*
 * Returns pointer to the currently
 * pointed node by the CListCursor.
 */
cptr
cursor_current(CListCursor* cursor)
{
    return (cursor) ? cursor->node : NULL;
}

/*
 * Peeks at the next value and returns it.
 * Returns NULL if the value is NULL or
 * pointer to the CListCursor is NULL.
 */
cptr
cursor_peek_next(CListCursor* cursor)
{
    return_val_if_fail(cursor != NULL, NULL);

    CListNode *current, *next;
    if((current = cursor->node) != NULL)
        if((next = current->next) != NULL)
            return next->data;

    return NULL;
}

/*
 * Peeks at the prev value and returns it.
 * Returns NULL if the value is NULL or
 * pointer to the CListCursor is NULL.
 */
cptr
cursor_peek_prev(CListCursor* cursor)
{
    return_val_if_fail(cursor != NULL, NULL);

    CListNode *current, *prev;
    if((current = cursor->node) != NULL)
        if((prev = current->next) != NULL)
            return prev->data;

    return NULL;
}

/*
 * Splits the underlying list before the cursor
 * into two lists.
 * Updates the new index for the cursor.
 *
 * Returns pointer to the new list or returns
 * NULL if pointer to the cursor is NULL or the
 * new list allocation failed.
 */
Clist*
cursor_split_before(CListCursor* cursor)
{
    return_val_if_fail(cursor != NULL, NULL);

    CListNode *current, *prev;
    Clist *    new_list = clist_new(), *temp;

    if(new_list == NULL)
        return NULL;

    if((current = cursor->node) != NULL) {
        if((prev = cursor->node->prev) != NULL) {
            CListNode* new_tail = prev;
            CListNode* new_head = cursor->clist->head;
            int        new_len  = cursor->index;

            cursor->clist->head = current;

            new_tail->next = NULL;
            current->prev  = NULL;

            new_list->head = new_head;
            new_list->tail = new_tail;
            new_list->len  = new_len;

            return new_list;
        } else
            return new_list;
    } else {
        temp = memc_replace(cursor->clist, new_list, sizeof(Clist));
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
 * NULL if pointer to the cursor is NULL or the
 * new list allocation failed.
 */
Clist*
cursor_split_after(CListCursor* cursor)
{
    return_val_if_fail(cursor != NULL, NULL);

    CListNode *current, *next;
    Clist *    new_list = clist_new(), *temp;

    if(new_list == NULL)
        return NULL;

    if((current = cursor->node) != NULL) {
        if((next = cursor->node->next) != NULL) {
            CListNode* new_head = next;
            CListNode* new_tail = cursor->clist->tail;
            int        new_len  = cursor->clist->len - cursor->index - 1;

            cursor->clist->tail = current;

            new_head->prev = NULL;
            current->next  = NULL;

            new_list->head = new_head;
            new_list->tail = new_tail;
            new_list->len  = new_len;

            return new_list;
        } else
            return new_list;
    } else {
        temp = memc_replace(cursor->clist, new_list, sizeof(Clist));
        free(new_list);
        return temp;
    }
}

/*
 * Combines the underlying list with the other one
 * before the node the cursor is pointing at.
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
 */
void
cursor_splice_before(CListCursor* cursor, Clist** other_list)
{
    if(cursor != NULL && other_list != NULL && *other_list != NULL) {
        CListNode*   current;
        unsigned int swapped = 0;
        if((current = cursor->node) != NULL) {
            CListNode* other_head = (*other_list)->head;
            CListNode* other_tail = (*other_list)->tail;
            CListNode* prev       = current->prev;

            if(prev != NULL) {
                prev->next       = other_head;
                other_head->prev = prev;
            } else
                cursor->clist->head = other_head;

            other_tail->next = current;
            current->prev    = other_tail;
            cursor->index    += (*other_list)->len;
        } else if(cursor->clist->tail != NULL) {
            CListNode* other_tail = (*other_list)->tail;
            CListNode* other_head = (*other_list)->head;

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
 * Combines the underlying list with the other one
 * after the node the cursor is pointing at.
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
 *-------------------------------------------------------------
 */
void
cursor_splice_after(CListCursor* cursor, Clist** other_list)
{
    if(cursor != NULL && other_list != NULL && *other_list != NULL) {
        CListNode*   current;
        unsigned int swapped;

        if((current = cursor->node) != NULL) {
            CListNode* other_head = (*other_list)->head;
            CListNode* other_tail = (*other_list)->tail;
            CListNode* next       = current->next;

            if(next != NULL) {
                next->prev       = other_tail;
                other_tail->next = next;
            } else
                cursor->clist->head = other_head;

            other_head->prev = current;
            current->next    = other_head;
        } else if(cursor->clist->head != NULL) {
            CListNode* other_tail = (*other_list)->tail;
            CListNode* other_head = (*other_list)->head;

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
 * CListCursor destructor, it doesn't free any
 * of the underlying structures it only frees
 * itself.
 */
void
cursor_free(CListCursor* cursor)
{
    if(cursor != NULL) {
        cursor->node  = NULL;
        cursor->clist = NULL;
        cursor->index = 0;
        free(cursor);
    }
}
