#ifndef __COL_VEC_H__
#define __COL_VEC_H__

#if !defined(__COL_LIB_INISDE__) && !defined(__COL_VEC_C_FILE__) &&            \
    !defined(__COL_TEST__)
#error "Only <collib.h> can be included directly."
#endif

#define __COL_H_FILE__
#include "ccore.h"
#undef __COL_H_FILE__
#include <stddef.h>
#include <stdlib.h>

typedef struct _cvec cvec;

typedef struct _cvec_iterator cvec_iterator;

typedef struct _cvec_iterref cvec_iterref;

typedef struct _cvec_itermut cvec_itermut;

typedef unsigned char *bptr_t;

cvec *cvec_new(size_t t_size, CFreeValueFn free_val_fn);

cvec *cvec_from(cconstptr_t array, size_t len, size_t element_size,
                CFreeValueFn free_val_fn);

cvec *cvec_with_capacity(size_t t_size, size_t capacity,
                         CFreeValueFn free_val_fn);

uint cvec_push(cvec *vec, cconstptr_t value);

cptr_t cvec_pop(cvec *vec);

void cvec_clear(cvec *vec);

void cvec_clear_with_cap(cvec *vec);

cptr_t cvec_get(const cvec *vec, uint idx);

uint cvec_get_into(const cvec *vec, uint idx, cptr_t out);

cconstptr_t cvec_get_ref(const cvec *vec, uint idx);

cptr_t cvec_get_mut(cvec *vec, uint idx);

void cvec_set(cvec *vec, uint idx, cconstptr_t value);

int cvec_len(const cvec *vec);

void cvec_drop(cvec **vecp, bool drop_buf);

int cvec_capacity(const cvec *vec);

uint cvec_insert(cvec *vec, cconstptr_t element, uint idx);

cptr_t cvec_remove(cvec *vec, uint idx);

cvec_iterator *cvec_into_iter(cvec **vecp);

cptr_t cvec_iterator_next(cvec_iterator *iterator);

cptr_t cvec_iterator_next_back(cvec_iterator *iterator);

void cvec_iterator_drop(cvec_iterator **iteratorp);

cvec_iterref *cvec_ref_iter(cvec *vec);

cconstptr_t cvec_iterref_next(cvec_iterref *iterator);

cconstptr_t cvec_iterref_next_back(cvec_iterref *iterator);

void cvec_iterref_drop(cvec_iterref *iterator);

cvec_itermut *cvec_mut_iterator(cvec *vec);

cptr_t cvec_itermut_next(cvec_itermut *iterator);

cptr_t cvec_itermut_next_back(cvec_itermut *iterator);

void cvec_itermut_drop(cvec_itermut *iterator);

#endif
