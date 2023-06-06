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

typedef unsigned char *bptr_t;

cvec cvec_new(size_t t_size, CFreeValueFn free_val_fn);

cvec cvec_from(cptr_t src, size_t len, CFreeValueFn free_val_fn);

cvec cvec_with_capacity(size_t t_size, size_t capacity, CFreeValueFn free_val_fn);

uint cvec_push(cvec *vec, cptr_t value);

cptr_t cvec_pop(cvec *vec);

void cvec_clear(cvec *vec);

void cvec_clear_with_cap(cvec *vec);

cconstptr_t cvec_get(cvec *vec, uint index);

cptr_t cvec_get_mut(cvec *vec, uint index);

int cvec_len(cvec *vec);

void cvec_free(cvec *vec);

int cvec_capacity(cvec *vec);

uint cvec_insert(cvec *vec, cptr_t element, uint index);

cptr_t cvec_remove(cvec *vec, uint index);

#endif
