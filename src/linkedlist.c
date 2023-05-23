/*
 * This is my personal implementation of Linked List
 * in C.
 *
 * Linked list is a linear collection of data
 * elements. It is not to be confused with
 * arrays, list elements are not sequential
 * when it comes to memory layout, instead they
 * are referenced by each other using pointers
 * (memory references).
 *
 * For example, finding some value inside an array
 * is equivallent to finding the index of the value
 * multiplied by the size of the element itself
 * and that product added to the base pointer.
 * Base pointer being the start of the allocation
 * for that sequence (array).
 *
 * When it comes to the lists, you can't acess the
 * element at the 'middle' of the list or near the 'end',
 * matter of a fact you can't access any arbitrary node in
 * the list before actually traversing the list from the
 * start or the end in the case of doubly linked list.
 * Then you could lets say cache the memory location of
 * the node in the list and access it later without again
 * traversing the list.
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
 * doubly linked list is O(1), while removing at the inserting/removing
 * anywhere except the start/end is O(n) (Peek time) + O(1).
 * Peek time complexity is O(n).
 * Space complexity is O(n).
 */

#include <assert.h>
/*
 * memc.h currently is only used for memswap
 * so you can use your own implementation
 * to avoid linking
 */
#include <memc.h>
#include <stdlib.h>

/*
 * 'Node' is a representative of a value that is stored
 * inside the linked list, it is a core structure that
 * forms the linked list.
 *
 * Contains pointer to the 'data', pointers to the 'next'
 * and 'prev' (previous) Node in the list.
 */
typedef struct Node {
    void*        data;
    struct Node* next;
    struct Node* prev;
} Node;

/*
 * 'LinkedList' is a entry point to the linked list.
 * It holds pointers to the 'head' (start) and the
 * 'tail' (end) of the list.
 *
 * 'len' is a value representing total amount of
 * nodes in the LinkedList.
 * It provides us with some usefull helper functions
 * and usefull API for the user.
 * It also is used in functions defined on the Cursor
 * struct (read more about Cursor above its definition),
 * that gives us some neat features such as splicing
 * and spliting the linked list.
 */
typedef struct LinkedList {
    Node*        head;
    Node*        tail;
    unsigned int len;
} LinkedList;

/* Private ----------------------------------------------------------------------*/

Node* node_new(void*);
void* node_free(Node*);

/*-------------------------------------------------------------------------------*/

/* Public API -------------------------------------------------------------------*/

LinkedList* linkedlist_new(void);
void*       linkedlist_pop_front(LinkedList*);
void*       linkedlist_pop_back(LinkedList*);
LinkedList* linkedlist_push_front(LinkedList*, void*);
LinkedList* linkedlist_push_back(LinkedList*, void*);
int         linkedlist_len(LinkedList*);
void*       linkedlist_front(LinkedList*);
void*       linkedlist_back(LinkedList*);
void*       linkedlist_pop(LinkedList*, void*, int (*)(void*, void*));
int         linkedlist_contains(LinkedList*, void* data, int (*)(void*, void*));
void*       linkedlist_find(LinkedList*, void* data, int (*)(void*, void*));
int         linkedlist_is_empty(LinkedList*);
void        linkedlist_free(LinkedList*);

/*-------------------------------------------------------------------------------*/

/*
 * LinkedList constructor, returns the pointer
 * to the underlying structure allocated on the heap.
 */
LinkedList*
linkedlist_new(void)
{
    LinkedList* linkedlist = (LinkedList*) malloc(sizeof(LinkedList));
    if(linkedlist != NULL) {
        linkedlist->head = NULL;
        linkedlist->tail = NULL;
        linkedlist->len  = 0;
    }
    return linkedlist;
}

/*
 * Removes the value from the front (head) of the list,
 * returning the pointer to the value that was stored in the list.
 */
void*
linkedlist_pop_front(LinkedList* linkedlist)
{
    if(linkedlist != NULL) {
        if(linkedlist->head != NULL) {
            Node* old_head   = linkedlist->head;
            linkedlist->head = old_head->next;

            if(linkedlist->head != NULL) {
                linkedlist->head->next = NULL;
            } else {
                linkedlist->tail = NULL;
            }

            linkedlist->len--;
            return node_free(old_head);
        }
    }
    return NULL;
}

/*
 * Removes the value from the back (tail) of the list,
 * returning the pointer to the value that was stored in the list.
 */
void*
linkedlist_pop_back(LinkedList* linkedlist)
{
    if(linkedlist != NULL) {
        if(linkedlist->tail != NULL) {
            Node* old_tail   = linkedlist->tail;
            linkedlist->tail = old_tail->prev;

            if(linkedlist->tail != NULL) {
                linkedlist->tail->next = NULL;
            } else {
                linkedlist->head = NULL;
            }

            linkedlist->len--;
            return node_free(linkedlist->head);
        }
    }
    return NULL;
}

/*
 * Removes the given value/data in the list if the value/data
 * is found by comparing the list values to the value to be removed,
 * using the provided comparison function.
 * Returns NULL if the value was not found, or any of the parameter
 * pointers is NULL.
 */
void*
linkedlist_pop(LinkedList* linkedlist, void* data, int (*cmp)(void*, void*))
{
    if(linkedlist != NULL && data != NULL && cmp != NULL) {
        Node* current;
        for(current = linkedlist->head; current != NULL; current = current->next) {
            if(cmp(current->data, data) == 0) {
                if(current->next != NULL) {
                    current->next->prev = current->prev;
                }
                if(current->prev != NULL) {
                    current->prev->next = current->next;
                }

                linkedlist->len--;
                return node_free(current);
            }
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
LinkedList*
linkedlist_push_front(LinkedList* linkedlist, void* data)
{
    if(linkedlist != NULL) {
        Node* new_head = node_new(data);

        if(new_head == NULL) {
            return NULL;
        }

        if(linkedlist->head != NULL) {
            linkedlist->head->prev = new_head;
            new_head->next         = linkedlist->head;
        } else {
            linkedlist->tail = new_head;
        }

        linkedlist->len++;
        linkedlist->head = new_head;
    }
    return linkedlist;
}

/*
 * Pushes/Inserts the value in the back (tail) of the list.
 * Returns pointer to the list if the value is inserted.
 * Returns NULL if the pointer to the list is NULL or if the
 * value couldn't be allocated.
 * In case of inability to allocate the value inside the list,
 * the list will stay in its previous state before insertion.
 */
LinkedList*
linkedlist_push_back(LinkedList* linkedlist, void* data)
{
    if(linkedlist != NULL) {
        Node* new_tail = node_new(data);

        if(new_tail == NULL) {
            return NULL;
        }

        if(linkedlist->tail != NULL) {
            linkedlist->tail->next = new_tail;
            new_tail->prev         = linkedlist->tail;
        } else {
            linkedlist->head = new_tail;
        }

        linkedlist->len++;
        linkedlist->tail = new_tail;
    }
    return linkedlist;
}

/*
 * Returns the len of the list.
 * If the pointer to the list is NULL,
 * it returns -1.
 */
int
linkedlist_len(LinkedList* linkedlist)
{
    if(linkedlist != NULL) {
        return linkedlist->len;
    }
    return -1;
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
void*
linkedlist_front(LinkedList* linkedlist)
{
    if(linkedlist != NULL) {
        if(linkedlist->head != NULL) {
            return linkedlist->head->data;
        } else {
            return NULL;
        }
    }
    return linkedlist;
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
void*
linkedlist_back(LinkedList* linkedlist)
{
    if(linkedlist != NULL) {
        if(linkedlist->tail != NULL) {
            return linkedlist->tail->data;
        } else {
            return NULL;
        }
    }
    return linkedlist;
}

/*
 * Returns either 1 if the value is found in
 * the list or 0 if its not, or if any of the
 * parameter pointers is NULL.
 */
int
linkedlist_contains(LinkedList* linkedlist, void* data, int (*cmp)(void*, void*))
{
    return linkedlist_find(linkedlist, data, cmp) != NULL;
}

/*
 * Returns pointer to the value in the list if its found,
 * if not it returns NULL, also returns NULL if any of the
 * parmeter pointers of the function are NULL.
 *
 * Note:
 * Value is free to be mutated, but
 * be aware of UB especially if you free
 * the value while it is still 'stored'
 * in the list.
 */
void*
linkedlist_find(LinkedList* linkedlist, void* data, int (*cmp)(void*, void*))
{
    if(linkedlist != NULL) {
        Node* current;
        for(current = linkedlist->head; current != NULL; current = current->next) {
            if(cmp(current->data, data) == 0) {
                return current->data;
            }
        }
        return NULL;
    }
    return linkedlist;
}

/*
 * Returns 0 if the list is not empty,
 * 1 if the list is empty and -1 if the
 * pointer to the list is NULL.
 */
int
linkedlist_is_empty(LinkedList* linkedlist)
{
    if(linkedlist != NULL) {
        return linkedlist->len == 0;
    }
    return -1;
}

/*
 * Frees the entire list, including all
 * the values stored inside it.
 */
void
linkedlist_free(LinkedList* linkedlist)
{
    void* data;
    while((data = linkedlist_pop_back(linkedlist)) != NULL) {
        free(data);
    }
    assert(linkedlist->len == 0);
    linkedlist->head = NULL;
    linkedlist->tail = NULL;
    linkedlist->len  = 0;
    free(linkedlist);
}

/*
 * Node constructor (private function).
 * Returns NULL only if the allocation fails.
 */
Node*
node_new(void* data)
{
    assert(data != NULL);
    Node* node = (Node*) malloc(sizeof(Node));
    if(node != NULL) {
        node->next = NULL;
        node->prev = NULL;
        node->data = data;
    }
    return node;
}

/*
 * Node destructor (private function).
 * Frees the node returning the pointer
 * that was stored in it.
 */
void*
node_free(Node* node)
{
    assert(node != NULL);
    void* temp = node->data;
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
 * to, and the index aka current position in the linked
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
typedef struct Cursor {
    Node*       node;
    LinkedList* linkedlist;
    long int    index;
} Cursor;

/* PUBLIC API-------------------------------------------------------*/

Cursor*     linkedlist_cursor(LinkedList*);
void        cursor_move_next(Cursor*);
void        cursor_move_prev(Cursor*);
void*       cursor_current(Cursor*);
void*       cursor_peek_next(Cursor*);
void*       cursor_peek_prev(Cursor*);
LinkedList* cursor_split_before(Cursor*);
LinkedList* cursor_split_after(Cursor*);
void        cursor_splice_before(Cursor*, LinkedList**);
void        cursor_splice_after(Cursor*, LinkedList**);
void        cursor_free(Cursor*);

/*-----------------------------------------------------------------*/

/*
 * Cursor constructor.
 * Returns pointer to the underlying structure on the heap.
 * Returns NULL if pointer to the list is NULL.
 */
Cursor*
linkedlist_cursor(LinkedList* linkedlist)
{
    if(linkedlist != NULL) {
        Cursor* cursor = (Cursor*) malloc(sizeof(Cursor));
        if(cursor != NULL) {
            cursor->linkedlist = linkedlist;
            cursor->node       = NULL;
            cursor->index      = -1;
        }
        return cursor;
    }
    return NULL;
}

/*
 * Moves Cursor to the next element
 * in the list.
 * Does nothing if pointer to the Cursor
 * is NULL.
 */
void
cursor_move_next(Cursor* cursor)
{
    if(cursor != NULL) {
        if(cursor->node != NULL) {
            cursor->node = cursor->node->next;
            if(cursor->node != NULL) {
                cursor->index++;
            } else {
                cursor->node  = cursor->linkedlist->head;
                cursor->index = 0;
            }
        } else if(!linkedlist_is_empty(cursor->linkedlist)) {
            cursor->node  = cursor->linkedlist->head;
            cursor->index = 0;
        } else {
            return;
        }
    }
}

/*
 * Moves Cursor to the previous element
 * in the list.
 * Does nothing if pointer to the Cursor
 * is NULL.
 */
void
cursor_move_prev(Cursor* cursor)
{
    if(cursor != NULL) {
        if(cursor->node != NULL) {
            cursor->node = cursor->node->prev;

            if(cursor->node != NULL) {
                cursor->index--;
            } else {
                cursor->node  = cursor->linkedlist->tail;
                cursor->index = cursor->linkedlist->len - 1;
            }
        } else if(!linkedlist_is_empty(cursor->linkedlist)) {
            cursor->node  = cursor->linkedlist->tail;
            cursor->index = cursor->linkedlist->len - 1;
        } else {
            return;
        }
    }
}

/*
 * Returns pointer to the currently
 * pointed node by the Cursor.
 */
void*
cursor_current(Cursor* cursor)
{
    return (cursor) ? cursor->node : NULL;
}

/*
 * Peeks at the next value and returns it.
 * Returns NULL if the value is NULL or
 * pointer to the Cursor is NULL.
 */
void*
cursor_peek_next(Cursor* cursor)
{
    if(cursor != NULL) {
        Node *current, *next;
        if((current = cursor->node) != NULL) {
            if((next = current->next) != NULL) {
                return next->data;
            }
        }
    }
    return NULL;
}

/*
 * Peeks at the prev value and returns it.
 * Returns NULL if the value is NULL or
 * pointer to the Cursor is NULL.
 */
void*
cursor_peek_prev(Cursor* cursor)
{
    if(cursor != NULL) {
        Node *current, *prev;
        if((current = cursor->node) != NULL) {
            if((prev = current->next) != NULL) {
                return prev->data;
            }
        }
    }
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
LinkedList*
cursor_split_before(Cursor* cursor)
{
    if(cursor != NULL) {
        Node *      current, *prev;
        LinkedList* new_list = linkedlist_new();

        if(new_list == NULL) {
            return NULL;
        }

        if((current = cursor->node) != NULL) {
            if((prev = cursor->node->prev) != NULL) {
                Node* new_tail = prev;
                Node* new_head = cursor->linkedlist->head;
                int   new_len  = cursor->index;

                cursor->linkedlist->head = current;

                new_tail->next = NULL;
                current->prev  = NULL;

                new_list->head = new_head;
                new_list->tail = new_tail;
                new_list->len  = new_len;

                return new_list;
            } else {
                return new_list;
            }
        } else {
            return memswap(cursor->linkedlist, new_list);
        }
    }
    return NULL;
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
LinkedList*
cursor_split_after(Cursor* cursor)
{
    if(cursor != NULL) {
        Node *      current, *next;
        LinkedList* new_list = linkedlist_new();

        if(new_list == NULL) {
            return NULL;
        }

        if((current = cursor->node) != NULL) {
            if((next = cursor->node->next) != NULL) {
                Node* new_head = next;
                Node* new_tail = cursor->linkedlist->tail;
                int   new_len  = cursor->linkedlist->len - cursor->index - 1;

                cursor->linkedlist->tail = current;

                new_head->prev = NULL;
                current->next  = NULL;

                new_list->head = new_head;
                new_list->tail = new_tail;
                new_list->len  = new_len;

                return new_list;
            } else {
                return new_list;
            }
        } else {
            return memswap(cursor->linkedlist, new_list);
        }
    }
    return NULL;
}

/*
 * Combines the underlying list with the other one
 * before the node the cursor is pointing to.
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
cursor_splice_before(Cursor* cursor, LinkedList** other_list)
{
    if(cursor != NULL && other_list != NULL && *other_list != NULL) {
        Node*        current;
        unsigned int swapped = 0;
        if((current = cursor->node) != NULL) {
            Node* other_head = (*other_list)->head;
            Node* other_tail = (*other_list)->tail;
            Node* prev       = current->prev;

            if(prev != NULL) {
                prev->next       = other_head;
                other_head->prev = prev;
            } else {
                cursor->linkedlist->head = other_head;
            }
            other_tail->next = current;
            current->prev    = other_tail;
            cursor->index    += (*other_list)->len;
        } else if(cursor->linkedlist->tail != NULL) {
            Node* other_tail = (*other_list)->tail;
            Node* other_head = (*other_list)->head;

            other_head->prev               = cursor->linkedlist->tail;
            cursor->linkedlist->tail->next = other_head;
            cursor->linkedlist->tail       = other_tail;
        } else {
            memswap(cursor->linkedlist, *other_list);
            swapped = 1;
        }
        if(!swapped) {
            cursor->linkedlist->len += (*other_list)->len;
        }
        (*other_list)->len  = 0;
        (*other_list)->tail = NULL;
        (*other_list)->head = NULL;
    } else {
        return;
    }

    free(*other_list);
    *other_list = NULL;
}

/*
 * Combines the underlying list with the other one
 * after the node the cursor is pointing to.
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
cursor_splice_after(Cursor* cursor, LinkedList** other_list)
{
    if(cursor != NULL && other_list != NULL && *other_list != NULL) {
        Node*        current;
        unsigned int swapped;

        if((current = cursor->node) != NULL) {
            Node* other_head = (*other_list)->head;
            Node* other_tail = (*other_list)->tail;
            Node* next       = current->next;

            if(next != NULL) {
                next->prev       = other_tail;
                other_tail->next = next;
            } else {
                cursor->linkedlist->head = other_head;
            }
            other_head->prev = current;
            current->next    = other_head;
        } else if(cursor->linkedlist->head != NULL) {
            Node* other_tail = (*other_list)->tail;
            Node* other_head = (*other_list)->head;

            other_tail->next               = cursor->linkedlist->head;
            cursor->linkedlist->head->prev = other_tail;
            cursor->linkedlist->head       = other_head;
        } else {
            memswap(cursor->linkedlist, *other_list);
            return;
        }
        if(!swapped) {
            cursor->linkedlist->len += (*other_list)->len;
        }
        (*other_list)->len  = 0;
        (*other_list)->tail = NULL;
        (*other_list)->head = NULL;
    } else {
        return;
    }

    free(*other_list);
    *other_list = NULL;
}

/*
 * Cursor destructor, it doesn't free any
 * of the underlying structures it only frees
 * itself.
 */
void
cursor_free(Cursor* cursor)
{
    if(cursor != NULL) {
        cursor->node       = NULL;
        cursor->linkedlist = NULL;
        cursor->index      = 0;
        free(cursor);
    }
}
