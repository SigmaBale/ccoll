#include <assert.h>
#include <limits.h>
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

struct _cvec {
    cptr_t       buffer;
    size_t       capacity;
    size_t       len;
    const size_t element_size;
    CFreeValueFn free_val_fn;
};

cvec
cvec_new(size_t element_size, CFreeValueFn free_val_fn)
{
    cvec vec = {
        .buffer       = calloc(0, element_size),
        .capacity     = 0,
        .len          = 0,
        .element_size = element_size,
        .free_val_fn  = free_val_fn,
    };

    return vec;
}

void
cvec_clear(cvec* vec)
{
    if(vec != NULL && vec->buffer != NULL) {

        if(vec->free_val_fn) {
            uint size = vec->len;
            while(size--)
                vec->free_val_fn(cvec_get_mut(vec, size));
        }

        vec->len = 0;
    }
}

void
cvec_clear_with_cap(cvec* vec)
{
    if(vec != NULL && vec->buffer != NULL) {

        if(vec->free_val_fn) {
            uint size = vec->len;
            while(size--)
                vec->free_val_fn(cvec_get_mut(vec, size));
        }

        free(vec->buffer);

        vec->len      = 0;
        vec->capacity = 0;
    }
}

void
cvec_free(cvec* vec)
{
    if(vec != NULL) {

        if(vec->buffer != NULL) {

            if(vec->free_val_fn) {
                uint size = vec->len;
                while(size--)
                    vec->free_val_fn(cvec_get_mut(vec, size));
            }

            free(vec->buffer);
        }

        free(vec);
    }
}

uint
_cvec_maybe_expand(cvec* vec)
{
    size_t capacity = vec->capacity;

    if(capacity == vec->len) {
        size_t new_cap;

        new_cap = (capacity == 0) ? 1 : capacity * 2;

        if(new_cap > (uint) INT_MAX) {
            COL_CAPACITY_EXCEEDED_ERROR;
            return 1;
        }

        bptr_t new_buf = realloc(vec->buffer, vec->element_size * new_cap);

        if(new_buf == NULL) {
            COL_ALLOC_ERROR;
            return 1;
        }

        vec->buffer   = new_buf;
        vec->capacity = new_cap;
    }
    return 0;
}

cvec
cvec_with_capacity(size_t element_size, size_t capacity, CFreeValueFn free_val_fn)
{
    cvec vec = cvec_new(element_size, free_val_fn);

    if(_cvec_maybe_expand(&vec) != 0) {
        COL_ERROR("failed to expand the capacity");
    }

    return vec;
}

cvec
cvec_from(cconstptr_t array, size_t len, CFreeValueFn free_val_fn)
{

    size_t element_size;

    element_size = (array != NULL) ? sizeof(array[0]) : 0;

    cvec vec = cvec_with_capacity(element_size, len, free_val_fn);

    if(len != 0 && vec.buffer != NULL) {
        // Sanity  check
        assert(array != NULL);
        memmove(vec.buffer, array, len);
        vec.len = len;
    }

    return vec;
}

cptr_t
cvec_get(const cvec* vec, uint idx)
{
    if(vec != NULL) {
        if(idx >= vec->len) {
            COL_INDEX_OUT_OF_BOUNDS_ERROR;
            return NULL;
        }

        cptr_t ret_val;
        memcpy(ret_val, cvec_get_ref(vec, idx), vec->element_size);

        return ret_val;
    }
    return NULL;
}

uint
cvec_get_into(const cvec* vec, uint idx, cptr_t out)
{
    if(vec != NULL) {
        if(idx >= vec->len) {
            COL_INDEX_OUT_OF_BOUNDS_ERROR;
            return 1;
        }

        memcpy(out, cvec_get_ref(vec, idx), vec->element_size);

        return 0;
    }
    return 1;
}

cconstptr_t
cvec_get_ref(const cvec* vec, uint idx)
{
    return (cconstptr_t) &vec->buffer[idx * vec->element_size];
}

cptr_t
cvec_get_mut(cvec* vec, uint idx)
{
    return &vec->buffer[idx * vec->element_size];
}

void
cvec_set(cvec* vec, uint idx, cconstptr_t value)
{
    if(vec != NULL) {

        if(idx >= vec->len) {
            COL_INDEX_OUT_OF_BOUNDS_ERROR;
            return;
        }

        memcpy(cvec_get_mut(vec, idx), value, vec->element_size);
    }
}

uint
cvec_push(cvec* vec, cconstptr_t value)
{
    // TODO: check __Non_null__ attribute and is user responsible for
    // Undefined Behaviour of passing in NULL?
    return_val_if_fail(vec != NULL && value != NULL, 1);

    // If len would exceed the capacity, expand!
    if(_cvec_maybe_expand(vec) != 0)
        return 1;

    cptr_t hole = cvec_get_mut(vec, vec->len);
    memmove(hole, value, vec->element_size);

    vec->len++;

    return 0;
}

cptr_t
cvec_pop(cvec* vec)
{
    return_val_if_fail(vec != NULL && vec->len != 0, NULL);

    size_t ele_size = vec->element_size;
    void*  val      = cvec_get_mut(vec, --vec->len);
    void*  ret_val  = malloc(sizeof(ele_size));

    memcpy(ret_val, val, ele_size);

    return ret_val;
}

int
cvec_len(const cvec* vec)
{
    return_val_if_fail(vec != NULL, -1);
    return vec->len;
}

uint
cvec_insert(cvec* vec, cconstptr_t element, uint index)
{
    return_val_if_fail(vec != NULL && element != NULL, 1);

    if(index > vec->len) {
        COL_INDEX_OUT_OF_BOUNDS_ERROR;
        return 1;
    }

    uint capacity = vec->capacity;
    uint len      = vec->len;

    if(_cvec_maybe_expand(vec) != 0)
        return 1;

    cptr_t hole     = cvec_get_mut(vec, index);
    size_t ele_size = vec->element_size;

    memmove(hole + ele_size, hole, len - index);

    memcpy(hole, element, ele_size);

    return 0;
}

cptr_t
cvec_remove(cvec* vec, uint index)
{
    return_val_if_fail(vec != NULL, NULL);

    if(index >= vec->len) {
        COL_INDEX_OUT_OF_BOUNDS_ERROR;
        return NULL;
    }

    size_t ele_size = vec->element_size;

    cptr_t hole    = cvec_get_mut(vec, index);
    cptr_t ret_val = malloc(sizeof(ele_size));

    memcpy(ret_val, hole, ele_size);

    memmove(hole, hole + vec->element_size, --vec->len - index);

    return ret_val;
}
