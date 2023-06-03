#include <assert.h>
#include <memc.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#define __COL_VEC_C_FILE__
#include "cvec.h"
#undef __COL_VEC_C_FILE__
#define __COL_C_FILE__
#include "cerror.h"
#undef __COL_C_FILE__

struct _CVec {
    bptr   allocation;
    size_t t_size;
    size_t capacity;
    size_t len;
};

cptr cvec_index(CVec* vec, cuint idx);

CVec*
cvec_new(size_t t_size)
{
    return_val_if_fail(t_size != 0, NULL);

    CVec* vec;

    if((vec = memc_malloc(CVec)) != NULL) {
        vec->allocation = NULL;
        vec->t_size     = t_size;
        vec->capacity   = 0;
        vec->len        = 0;
    }

    return vec;
}

void
cvec_free(CVec* vec)
{
    if(vec != NULL) {
        vec->capacity = 0;
        vec->t_size   = 0;
        vec->len      = 0;
        free(vec->allocation);
        free(vec);
    }
}

cuint
cvec_expand(CVec* vec, size_t len)
{
    bptr new = (bptr) realloc(vec->allocation, vec->t_size * len);

    if(new == NULL)
        return 1;

    vec->allocation = new;
    vec->capacity   = len;

    return 0;
}

CVec*
cvec_with_capacity(size_t t_size, size_t capacity)
{
    return_val_if_fail(t_size != 0, NULL);

    CVec* vec;

    if((vec = memc_malloc(CVec)) != NULL) {
        if(capacity == 0) {
            vec->allocation = NULL;
        } else if((vec->allocation = (bptr) malloc(capacity * t_size)) == NULL) {
            free(vec);
            return NULL;
        }
        vec->capacity = capacity;
        vec->t_size   = t_size;
        vec->len      = 0;
    }

    return vec;
}

CVec*
cvec_from(cptr array, size_t len)
{
    size_t t_size = sizeof(array);

    return_val_if_fail(array != NULL && t_size != 0, NULL);

    CVec* vec;

    if((vec = cvec_with_capacity(sizeof(array), len)) != NULL) {

        assert(vec->capacity == len);

        if(len == 0)
            return vec;

        memcpy(vec->allocation, array, len);
        vec->len = len;
    }

    return vec;
}

cuint
cvec_push(CVec* vec, cptr value)
{
    // TODO: introduce proper error checking
    // by rewritting cerror.h
    if(vec == NULL)
        return 1;

    if(value == NULL)
        return 2;

    if(vec->capacity < vec->len + 1)
        if(cvec_expand(vec, vec->capacity + 1) != 0)
            return 3;

    bptr new_index = &vec->allocation[vec->len * vec->t_size];
    memcpy(new_index, value, vec->t_size);

    vec->len++;

    return 0;
}

/*
 * Same as cvec_index except it is
 * a public function in the header.
 */
cptr
cvec_get(CVec* vec, cuint idx)
{
    return cvec_index(vec, idx);
}

/*
 * Internal function for retrieving elements
 * from given index.
 */
cptr
cvec_index(CVec* vec, cuint idx)
{
    return_val_if_fail(vec != NULL && idx < vec->len, NULL);

    return (cptr) &vec->allocation[idx * vec->t_size];
}

cptr
cvec_pop(CVec* vec)
{
    return_val_if_fail(vec != NULL, NULL);

    if(vec->len == 0)
        return NULL;

    cptr value = cvec_index(vec, vec->len - 1);

    vec->len--;

    return value;
}

int
cvec_len(CVec* vec)
{
    return_val_if_fail(vec != NULL, -1);
    return vec->len;
}

/* Debug */
int
cvec_capacity(CVec* vec)
{
    return_val_if_fail(vec != NULL, -1);
    return vec->capacity;
}
