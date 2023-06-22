#include <limits.h>
#include <memc.h>
#include <memory.h>
#include <stdbool.h>

#define __COL_VEC_C_FILE__
#include "cvec.h"
#undef __COL_VEC_C_FILE__

#define __COL_C_FILE__
#include "cerror.h"
#undef __COL_C_FILE__

/*
 * '_cvec' is wrapper around a memory allocation, containing some
 * additional information about allocation size (capacity), number of
 * elements of 'element_size' in the vec aka 'len' and the optional
 * 'clear_val_fn' which is a function pointer dictating how the value
 * in the vec should be 'cleared' upon its removal.
 * For difference between clearing and freeing the value please check the
 * 'ccore.h' header file under 'CClearValueFn' type.
 *
 * All of this is called a 'vec'/'vector' and it is a dynamically allocated
 * array that manages its size/capacity internally.
 * Vec can hold any type but only one type per single Vec.
 */
struct _cvec {
    cptr_t        buffer;
    size_t        capacity;
    size_t        len;
    const size_t  element_size;
    CClearValueFn clear_val_fn;
    uint          ref_count; // Useless for now, will implement...
};

/*
 * 'cvec' constructor.
 * Providing 'element_size' of > 0 is mandatory for constructing the vec.
 * 'clear_val_fn' is optional, if user does not provide this function then
 * it's the user's responsibility of clearing the contents of the value,
 * especially if the value is a struct containing internal parts that are
 * malloc'ed.
 */
cvec*
cvec_new(size_t element_size, CClearValueFn clear_val_fn)
{

    return_val_if_fail(element_size > 0, NULL);
    return_val_if_fail(element_size < UINT_MAX, NULL);

    cvec init = {
        .buffer       = NULL,
        .capacity     = 0,
        .len          = 0,
        .element_size = element_size,
        .clear_val_fn = clear_val_fn,
    };

    cvec* vec = memc_malloc(cvec);

#ifndef COL_MEMORY_CONSTRAINED
    if(__builtin_expect(vec == NULL, 0)) {
#else
    if(vec == NULL) {
#endif
        COL_ALLOC_ERROR;
        return NULL;
    }

    memcpy(vec, &init, sizeof(cvec));

    return vec;
}

/*
 * Internal function that gets the 'element' inside the 'vec' at index 'idx'.
 * This is used after all the error checking is done to fetch the value.
 * This function should be inlined during compilation.
 */
static inline cptr_t
_cvec_index(cvec* vec, uint idx)
{
    return vec->buffer + (idx * (vec->element_size));
}

/*
 * Internal function that 'maybe' expands the 'cvec' buffer.
 * If 'capacity' is same size as 'len' then the buffer expands.
 * If buffer is expanded successfully function returns 0.
 * If new buffer size would overflow 'UINT_MAX' then the buffer is not expanded,
 * err msg gets printed to stderr and function fails returning 1.
 * If buffer size is valid but allocation fails, err msg gets printed to stderr
 * and function returns 1.
 */
static uint
_cvec_maybe_expand(cvec* vec)
{
    size_t capacity = vec->capacity;

    if(capacity == vec->len) {
        size_t new_cap;
        size_t elsize = vec->element_size;

        new_cap = (capacity == 0) ? 1 : capacity * 2;

#ifndef COL_MEMORY_CONSTRAINED
        if(__builtin_expect(new_cap * elsize > (uint) UINT_MAX, 0)) {
#else
        if(new_cap * elsize > (uint) INT_MAX) {
#endif
            COL_CAPACITY_EXCEEDED_ERROR;
            return 1;
        }

        bptr_t new_buf = realloc(vec->buffer, elsize * new_cap);

#ifndef COL_MEMORY_CONSTRAINED
        if(__builtin_expect(new_buf == NULL, 0)) {
#else
        if(new_buf == NULL) {
#endif
            COL_ALLOC_ERROR;
            return 1;
        }

        vec->buffer   = new_buf;
        vec->capacity = new_cap;
    }

    return 0;
}

/*
 * Clears the 'cvec' resseting it's values to default.
 * It does not touch the 'capacity' and the underlying 'buffer'.
 */
void
cvec_clear(cvec* vec)
{
    if(vec != NULL && vec->buffer != NULL) {

        if(vec->clear_val_fn) {
            uint size = vec->len;
            while(size--) {
                vec->clear_val_fn(cvec_get_mut(vec, size));
            }
        }

        vec->len = 0;
    }
}

/*
 * Clears the enitre 'cvec' including the 'buffer'.
 * All the values are reset to default values same as when 'cvec' is
 * constructed.
 */
void
cvec_clear_with_cap(cvec* vec)
{
    if(vec != NULL && vec->buffer != NULL) {

        if(vec->clear_val_fn) {
            uint size = vec->len;
            while(size--) {
                vec->clear_val_fn(cvec_get_mut(vec, size));
            }
        }

        free(vec->buffer);

        vec->len      = 0;
        vec->capacity = 0;
        vec->buffer   = NULL;
    }
}

/*
 * 'cvec' constructor but with custom capacity, returns 'cvec' with the
 * 'capacity' provided. If 'element_size' is 0 function returns NULL, if
 * allocation fails err msg gets printed to stderr and NULL is returned.
 */
cvec*
cvec_with_capacity(size_t        element_size,
                   size_t        capacity,
                   CClearValueFn clear_val_fn)
{
    return_val_if_fail(element_size != 0, NULL);

    cvec* vec = cvec_new(element_size, clear_val_fn);

#ifndef COL_MEMORY_CONSTRAINED
    if(__builtin_expect(vec == NULL, 0)) {
#else
    if(vec == NULL) {
#endif
        return NULL;
    }

    void* new_buf = realloc(vec->buffer, capacity * element_size);

#ifndef COL_MEMORY_CONSTRAINED
    if(__builtin_expect(new_buf == NULL, 0)) {
#else
    if(_cvec_maybe_expand(vec) != 0) {
#endif
        COL_ALLOC_ERROR;
        free(vec);
        return NULL;
    }

    vec->buffer   = new_buf;
    vec->capacity = capacity;

    return vec;
}

/*
 * Constructs 'cvec' from the 'array'.
 * Copies 'len' elements of size 'element_size' from 'array' into the cvec
 * 'buffer'. If allocation fails, err msg is printed to stderr and function
 * returns NULL, if 'array' is NULL or 'len' is 0 then NULL is returned.
 */
cvec*
cvec_from(cconstptr_t   array,
          size_t        len,
          size_t        element_size,
          CClearValueFn clear_val_fn)
{
    return_val_if_fail(array != NULL && len != 0, NULL);

    cvec* vec = cvec_with_capacity(element_size, len, clear_val_fn);

#ifndef COL_MEMORY_CONSTRAINED
    if(__builtin_expect(vec == NULL, 0)) {
#else
    if(vec == NULL) {
#endif
        return NULL;
    }

    memmove(vec->buffer, array, len);
    vec->len = len;

    return vec;
}

/*
 * Returns the copied element of 'vec' at index 'idx'.
 * Use this if you require read/write to the element and expect the value to
 * outlive the 'cvec' it was retrieved from or you just need the duplicate that
 * does not refer to the 'cvec' in any way. If 'idx' is out of bounds then err
 * msg is printed to stderr and NULL is returned. If 'vec' is NULL function
 * returns NULL.
 */
cptr_t
cvec_get(const cvec* vec, uint idx)
{
    return_val_if_fail(vec != NULL, NULL);

    if(idx >= vec->len) {
        COL_INDEX_OUT_OF_BOUNDS_ERROR;
        return NULL;
    }

    size_t elsize = vec->element_size;

    cptr_t ret_val = malloc(elsize);
#ifndef COL_MEMORY_CONSTRAINED
    if(__builtin_expect(ret_val == NULL, 0)) {
#else
    if(ret_val == NULL) {
#endif
        COL_ALLOC_ERROR;
        return NULL;
    }

    memcpy(ret_val, cvec_get_ref(vec, idx), elsize);
    return ret_val;
}

/*
 * Copies the element of 'vec' at index 'idx' into 'out'.
 * If element gets copied successfully function returns 0.
 * This function has a precondition that 'out' and element at 'idx' must not
 * overlap or alias each other otherwise it is Undefined Behaviour. If 'idx' is
 * out of bounds, err gets printed to stderr and function returns 1. If 'out' is
 * not the same size as 'element_size' then segfault will teach you a lesson or
 * two.
 */
uint
cvec_get_into(const cvec* vec, uint idx, cptr_t out)
{
    return_val_if_fail(vec != NULL, 1);

    if(idx >= vec->len) {
        COL_INDEX_OUT_OF_BOUNDS_ERROR;
        return 1;
    }

    memcpy(out, cvec_get_ref(vec, idx), vec->element_size);

    return 0;
}

/*
 * Returns 'const' pointer to the element of 'vec' at index 'idx'.
 * Use this to read values/elements from 'cvec', do not use this
 * if you require mutation of the value or the shallow copy that will outlive
 * the 'cvec'. If 'idx' is out of bounds, err gets printed to stderr and NULL is
 * returned.
 */
cconstptr_t
cvec_get_ref(const cvec* vec, uint idx)
{
    return_val_if_fail(vec != NULL, NULL);

    if(idx >= vec->len) {
        COL_INDEX_OUT_OF_BOUNDS_ERROR;
        return NULL;
    }

    return (cconstptr_t) vec->buffer + (idx * (vec->element_size));
}

/*
 * Returns pointer to the element of 'vec' at index 'idx'.
 * This allows mutation of the returned element but is still bound to the
 * lifetime of 'cvec'. Freeing the returned 'element' is Undefined Behaviour.
 * Use this to read/write to the element but you are not allowed to free it.
 * If 'idx' is out of bounds, err gets printed to stderr and NULL is returned.
 */
cptr_t
cvec_get_mut(cvec* vec, uint idx)
{
    return_val_if_fail(vec != NULL, NULL);

    if(idx >= vec->len) {
        COL_INDEX_OUT_OF_BOUNDS_ERROR;
        return NULL;
    }

    return _cvec_index(vec, idx);
}

/*
 * Sets the value of the 'vec' 'element' at index 'idx' by shallow copying the
 * 'element'. If 'idx' is out of bounds, err gets printed to stderr and function
 * returns without performing the shallow copy.
 */
void
cvec_set(cvec* vec, uint idx, cconstptr_t element)
{
    if(vec != NULL) {

        if(idx >= vec->len) {
            COL_INDEX_OUT_OF_BOUNDS_ERROR;
            return;
        }

        memcpy(cvec_get_mut(vec, idx), element, vec->element_size);
    }
}

/*
 * Pushes aka appends the 'element' to the 'vec'.
 * If function is successfull 0 is returned.
 * If 'vec' is NULL or 'element' is NULL function fails and returns 1.
 * If 'vec' needs to expand and allocation fails function returns 1.
 */
uint
cvec_push(cvec* vec, cconstptr_t element)
{
    return_val_if_fail(vec != NULL && element != NULL, 1);

    // If len would exceed the capacity, expand!
#ifndef COL_MEMORY_CONSTRAINED
    if(__builtin_expect(_cvec_maybe_expand(vec) != 0, 0)) {
#else
    if(_cvec_maybe_expand(vec) != 0) {
#endif
        return 1;
    }

    cptr_t hole = _cvec_index(vec, vec->len);
    memmove(hole, element, vec->element_size);

    vec->len++;

    return 0;
}

/*
 * Pops/removes the value from the end of the 'vec' returning shallow copy of
 * the element. If allocation for shallow copy fails, err gets printed to stderr
 * and function returns NULL.
 */
cptr_t
cvec_pop(cvec* vec)
{
    return_val_if_fail(vec != NULL && vec->len != 0, NULL);

    size_t ele_size = vec->element_size;
    void*  val      = _cvec_index(vec, vec->len - 1);
    void*  ret_val  = malloc(ele_size);

#ifndef COL_MEMORY_CONSTRAINED
    if(__builtin_expect(ret_val == NULL, 0)) {
#else
    if(ret_val == NULL) {
#endif
        COL_ALLOC_ERROR;
        return NULL;
    }

    memcpy(ret_val, val, ele_size);

    vec->len--;

    return ret_val;
}

/*
 * Getter function, returns length of 'vec'.
 * If 'vec' is NULL function returns -1.
 */
int
cvec_len(const cvec* vec)
{
    return_val_if_fail(vec != NULL, -1);
    return vec->len;
}

/*
 * Getter function, returns current 'capacity' of 'vec'.
 * If 'vec' is NULL function returns -1.
 */
int
cvec_capacity(const cvec* vec)
{
    return_val_if_fail(vec != NULL, -1);
    return vec->capacity;
}

/*
 * Inserts 'element' into the 'vec' at index 'idx'.
 * If buffer needs to expand and fails function returns 1.
 * If 'idx' is out of bounds, err msg is printed to stderr and function
 * returns 1. If 'idx' is equal to the 'vec' 'len' then the push is performed
 * (check 'cvec_push').
 */
uint
cvec_insert(cvec* vec, cconstptr_t element, uint idx)
{
    return_val_if_fail(vec != NULL && element != NULL, 1);

    uint len = vec->len;

    if(idx > len) {
        COL_INDEX_OUT_OF_BOUNDS_ERROR;
        return 1;
    } else if(idx != len) {
#ifndef COL_MEMORY_CONSTRAINED
        if(__builtin_expect(_cvec_maybe_expand(vec) != 0, 0)) {
#else
        if(_cvec_maybe_expand(vec) != 0) {
#endif
            return 1;
        }

        cptr_t hole     = cvec_get_mut(vec, idx);
        size_t ele_size = vec->element_size;

        memmove(hole + ele_size, hole, (len - idx) * ele_size);

        memcpy(hole, element, ele_size);

        return 0;
    } else {
        return cvec_push(vec, element);
    }
}

/*
 * Removes the 'element' from 'vec' at index 'idx' and returns its shallow copy.
 * If 'idx' is out of bounds, err msg is printed to stderr and function returns
 * NULL. If allocation for shallow copy fails, err msg is printed to stderr and
 * function returns NULL.
 */
cptr_t
cvec_remove(cvec* vec, uint idx)
{
    return_val_if_fail(vec != NULL, NULL);

    if(idx >= vec->len) {
        COL_INDEX_OUT_OF_BOUNDS_ERROR;
        return NULL;
    }

    size_t ele_size = vec->element_size;
    cptr_t hole     = cvec_get_mut(vec, idx);
    cptr_t ret_val  = malloc(sizeof(ele_size));

#ifndef COL_MEMORY_CONSTRAINED
    if(__builtin_expect(ret_val == NULL, 0)) {
#else
    if(ret_val == NULL) {
#endif
        COL_ALLOC_ERROR;
        return NULL;
    }

    memcpy(ret_val, hole, ele_size);

    memmove(hole, hole + vec->element_size, (--vec->len - idx) * ele_size);

    return ret_val;
}

/*
 * 'Drops' the 'cvec', clearing all the fields and nulling the
 * dereferenced pointer (vecp).
 * Additionally it free's the underlying buffer if the 'drop_buf' is TRUE.
 */
void
cvec_drop(cvec** vecp, bool drop_buf)
{
    if(vecp != NULL) {
        cvec* vec = *vecp;
        if(vec != NULL) {
            cptr_t temp       = vec->buffer;
            vec->len          = 0;
            vec->capacity     = 0;
            vec->clear_val_fn = NULL;
            vec->buffer       = NULL;
            if(drop_buf) {
                free(temp);
            }
            temp = *vecp;
            free(temp);
            *vecp = NULL;
        }
    }
}

//********************************************************************************//
//                                  ITERATORS //
//********************************************************************************//

/*
 * 'vec_iter_vals' contains the 'start' and 'end' of iterator.
 */
typedef struct _vec_iter_vals {
    cptr_t start;
    cptr_t end;
    size_t element_size;
} vec_iter_vals;

/*
 * Internal only function for constructing 'vec_iter_vals'.
 * Start and end are derived from the passed in vec.
 * Checks for vec pointer and vec length are done before this function gets
 * called.
 */
vec_iter_vals
_cvec_iter_vals_new(cvec* vec)
{
    vec_iter_vals vals = {
        .start        = cvec_get_mut(vec, 0),
        .end          = cvec_get_mut(vec, vec->len - 1),
        .element_size = vec->element_size,
    };

    return vals;
}

/*
 * 'cvec_iterator' is consuming iterator, after constructing one from the
 * 'cvec', the vec used for constructing it must not be used anymore,
 * additionally underlying 'cvec' will get 'dropped' and should not be used from
 * that point on (its pointer also gets nulled).
 */
struct _cvec_iterator {
    size_t        len;
    cptr_t        buffer;
    CClearValueFn clear_val_fn;
    vec_iter_vals iter_vals;
};

/*
 * 'cvec_iterator' constructor, it consumes the 'vec' used for constructing this
 * iterator.
 * 'Consumed' vec after this function returns is not valid anymore and is
 * dropped. Dereferenced pointer ('vecp' that was passed in as argument) is also
 * nulled. After this iterator gets freed it also frees the underlying 'buffer'
 * and calls 'clear_val_fn' on each of the elements (if 'clear_val_fn is not
 * NULL). This is why this iterator is 'consuming' the 'cvec'.
 */
cvec_iterator*
cvec_into_iter(cvec** vecp)
{
    cvec* vec;
    return_val_if_fail(vecp != NULL && (vec = *vecp) != NULL && vec->len != 0,
                       NULL);
    cvec_iterator* iterator = memc_malloc(cvec_iterator);

#ifndef COL_MEMORY_CONSTRAINED
    if(__builtin_expect(iterator == NULL, 0)) {
#else
    if(iterator == NULL) {
#endif
        COL_ALLOC_ERROR;
        return NULL;
    }

    vec_iter_vals vals = {
        .start = cvec_get_mut(vec, 0),
        .end   = cvec_get_mut(vec, vec->len - 1),
    };

    iterator->buffer    = vec->buffer;
    iterator->len       = vec->len;
    iterator->iter_vals = vals;

    cvec_drop(vecp, false);

    return iterator;
}

/*
 * Initiates iteration of 'cvec_iterator'.
 * Each subsequent call to 'cvec_iterator_next' yields either an element
 * or NULL if the iterator reached the end.
 * Returned element is pointer to the copied value of the element in 'cvec'.
 * Meaning the value will remain valid even after 'cvec_iterator' gets freed.
 *
 * If allocation for the copied value fails, NULL is returned.
 * This iterator should not be used if any of the other iterators is constructed
 * for the same 'cvec'.
 */
cptr_t
cvec_iterator_next(cvec_iterator* iterator)
{
    return_val_if_fail(iterator != NULL, NULL);
    vec_iter_vals vals = iterator->iter_vals;

    if(vals.start == vals.end) {
        return NULL;
    } else {
        cptr_t old = malloc(vals.element_size);
#ifndef COL_MEMORY_CONSTRAINED
        if(__builtin_expect(old == NULL, 0)) {
#else
        if(old == NULL) {
#endif
            COL_ALLOC_ERROR;
            return NULL;
        }

        memcpy(old, vals.start, vals.element_size);
        vals.start += vals.element_size;
        return old;
    }
}

/*
 * Same as 'cvec_iterator_next' except the iteration starts from the back of
 * the 'cvec'.
 */
cptr_t
cvec_iterator_next_back(cvec_iterator* iterator)
{
    return_val_if_fail(iterator != NULL, NULL);
    vec_iter_vals vals = iterator->iter_vals;

    if(vals.start == vals.end) {
        return NULL;
    } else {
        cptr_t old = malloc(vals.element_size);

#ifndef COL_MEMORY_CONSTRAINED
        if(__builtin_expect(old == NULL, 0)) {
#else
        if(old == NULL) {
#endif
            COL_ALLOC_ERROR;
            return NULL;
        }

        memcpy(old, vals.end, vals.element_size);
        vals.end -= vals.element_size;
        return old;
    }
}

/*
 * Wrapper around 'free', does the NULL check on 'iterator' and drops/frees it.
 * Additionally the underlying buffer is also dropped and the passed in
 * dereference of 'iteratorp' is nulled. If 'CClearValueFn' was provided to the
 * consumed 'cvec' then this function iterates over each element applying that
 * function to each element.
 *
 * Warning:
 * If the 'CClearValueFn' was provided and the type you stored
 * in 'cvec' is a struct or some object containing members/field and one of
 * those fields is malloc'ed and the provided function frees that allocation,
 * then the values that were yielded by this iterator are no longer valid. This
 * is because this iterator returns a shallow copy of each element and does not
 * clone the entire element recursively.
 */
void
cvec_iterator_drop(cvec_iterator** iteratorp)
{
    cvec_iterator* iterator = NULL;
    if(iteratorp != NULL && (iterator = *iteratorp) != NULL) {
        if(iterator->clear_val_fn) {
            size_t        len  = iterator->len;
            cptr_t        bufp = iterator->buffer;
            CClearValueFn fn   = iterator->clear_val_fn;
            while(len--) {
                fn(bufp++);
            }
        }
        cptr_t temp = iterator;
        free(iterator->buffer);
        *iteratorp = NULL;
        free(temp);
    }
}

/*
 * 'cvec_iterref' is iterator over 'const' elements of 'cvec'.
 * It is non-consuming and handles only const pointers of the elements it
 * is iterating over.
 * After this iterator is exhausted it keeps on returning NULL and can
 * be safely freed without touching the underlying buffer of 'cvec'.
 */
struct _cvec_iterref {
    vec_iter_vals iter_vals;
};

/*
 * 'cvec_iterref' constructor.
 * If allocation fails err msg is printed to stderr and NULL is returned,
 * if passed in pointer is NULL or 'cvec' len is 0 it also returns NULL.
 */
cvec_iterref*
cvec_ref_iter(cvec* vec)
{
    return_val_if_fail(vec != NULL && vec->len != 0, NULL);
    cvec_iterref* iterator = memc_malloc(cvec_iterref);

#ifndef COL_MEMORY_CONSTRAINED
    if(__builtin_expect(iterator == NULL, 0)) {
#else
    if(iterator == NULL) {
#endif
        COL_ALLOC_ERROR;
        return NULL;
    }

    iterator->iter_vals = _cvec_iter_vals_new(vec);
    return iterator;
}

/*
 * Initiates iteration of 'cvec_iterref' iterator.
 * Each subsequent call to 'cvec_iterref_next' yields either and element
 * or NULL if the iterator reached the end.
 * Returned element is 'const' pointer to the value of the element in 'cvec'.
 * The value is not supposed to be mutable and will become invalid if underlying
 * buffer or 'cvec' get freed.
 * This iterator should not be used if any of the other iterators is constructed
 * for the same 'cvec'.
 */
cconstptr_t
cvec_iterref_next(cvec_iterref* iterator)
{
    return_val_if_fail(iterator != NULL, NULL);

    vec_iter_vals vals = iterator->iter_vals;

    if(vals.start == vals.end) {
        return NULL;
    } else {
        cptr_t old = vals.start;
        vals.start += vals.element_size;
        return (cconstptr_t) old;
    }
}

/*
 * Same as 'cvec_iterref_next' except iteration starts from the end of the
 * 'cvec'. Returns NULL if the start and end are same addresses. For example you
 * can start iterating from the front of 'cvec' and later (if you haven't
 * reached end) start iterating from the back, until the ends reach each other.
 */
cconstptr_t
cvec_iterref_next_back(cvec_iterref* iterator)
{
    return_val_if_fail(iterator != NULL, NULL);

    vec_iter_vals vals = iterator->iter_vals;

    if(vals.start == vals.end) {
        return NULL;
    } else {
        cptr_t old = vals.end;
        vals.end   -= vals.element_size;
        return (cconstptr_t) old;
    }
}

/*
 * Wrapper around 'free', does the NULL check on 'iterator' and drops/frees it.
 */
void
cvec_iterref_drop(cvec_iterref* iterator)
{
    if(iterator != NULL) {
        free(iterator);
    }
}

/*
 * 'cvec_itermut' is iterator over pointers to elements of 'cvec'.
 * It is reccomended to use 'cvec_itermut' if you need to iterate over
 * the 'cvec' and mutate the values.
 * Mutating the 'cvec' and 'cvec_itermut' elements at the same time is Undefined
 * Behaviour.
 */
struct _cvec_itermut {
    vec_iter_vals iter_vals;
};

/*
 * 'cvec_itermut' constructor,
 * If allocation fails err msg is printed to stderr and NULL is returned,
 * if passed in pointer is NULL or 'cvec' len is 0 it also returns NULL.
 */
cvec_itermut*
cvec_mut_iterator(cvec* vec)
{
    return_val_if_fail(vec != NULL && vec->len != 0, NULL);
    cvec_itermut* iterator = memc_malloc(cvec_itermut);

#ifndef COL_MEMORY_CONSTRAINED
    if(__builtin_expect(iterator == NULL, 0)) {
#else
    if(iterator == NULL) {
#endif
        COL_ALLOC_ERROR;
        return NULL;
    }

    iterator->iter_vals = _cvec_iter_vals_new(vec);
    return iterator;
}

/*
 * Initiates iteration of 'cvec_itermut' iterator.
 * Each subsequent call to 'cvec_itermut_next' yields either and element
 * or NULL if the iterator reached the end.
 * Returned element is pointer to the value of the element in 'cvec'.
 * The value is free to being mutated, additionally value will become invalid if
 * underlying buffer or 'cvec' get freed. This iterator should not be used if
 * any of the other iterators is constructed for the same 'cvec'.
 */
cptr_t
cvec_itermut_next(cvec_itermut* iterator)
{
    return_val_if_fail(iterator != NULL, NULL);

    vec_iter_vals vals = iterator->iter_vals;

    if(vals.start == vals.end) {
        return NULL;
    } else {
        cptr_t old = vals.start;
        vals.start += vals.element_size;
        return old;
    }
}

/*
 * Iterates over elements starting from the back.
 * Each subsequent call returns element or if iterator reached end aka start and
 * end addresses are same. Everything is same as 'cvec_itermut_next' only the
 * direction of iteration is changed.
 */
cptr_t
cvec_itermut_next_back(cvec_itermut* iterator)
{
    return_val_if_fail(iterator != NULL, NULL);

    vec_iter_vals vals = iterator->iter_vals;

    if(vals.start == vals.end) {
        return NULL;
    } else {
        cptr_t old = vals.end;
        vals.end   -= vals.element_size;
        return old;
    }
}

/*
 * Wrapper around 'free', does the NULL check on 'iterator' and drops/frees it.
 */
void
cvec_itermut_drop(cvec_itermut* iterator)
{
    if(iterator != NULL) {
        free(iterator);
    }
}
