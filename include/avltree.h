#include <stddef.h>

#ifndef AVLTREE_H
#define AVLTREE_H

/* AVLTree holding char* values */
typedef struct AVLTree *Avltree_char;

/* 'Avltree_char' constructor, takes comparison CMP function
 * that will be used for sorting inserted elements */
Avltree_char avltree_new(int (*cmp)(void *, void *));

/* 'Avltree_char' destructor, frees the complete TREE */
void avltree_free(Avltree_char tree);

/* 
 * Inserts VALUE into the TREE.
 * Returns 1 if the VALUE got inserted,
 * 0 if
 */
int avltree_insert(Avltree_char tree, const char *value);

/* Removes VALUE from the TREE */
int avltree_remove(Avltree_char tree, const char *value);

/* Returns total amount of nodes (values) in the TREE */
int avltree_size(Avltree_char tree);

/* Returns size of all nodes (values) in the TREE in bytes */
size_t avltree_size_bytes(Avltree_char tree);

#endif
