#ifndef __COL_CORE_H__
#define __COL_CORE_H__

#if !defined(__COL_H_FILE__) && !defined(__COL_LIB_INSIDE__)
#error "Only <collib.h> can be included directly."
#endif

#define return_val_if_fail(expr, val)                                          \
  if (!(expr))                                                                 \
  return (val)

typedef unsigned char *bptr_t;

typedef unsigned int cuint_t;

typedef const void *cconstptr_t;

typedef void *cptr_t;

/*
 * 'CCompareKeyFn' is a function used for comparing the elements/keys when
 * inserting/searching/removing it from the data structure.
 */
typedef int (*CCompareKeyFn)(cconstptr_t a_key, cconstptr_t b_key);

/*
 * 'CFreeKeyFn' is a function used for freeing the malloc'ed key that is
 * stored inside a data structure.
 * If not provided in the data structure that requires it in order to free
 * the key, then freeing the key is user's responsibility.
 */
typedef void (*CFreeKeyFn)(cptr_t key);

/*
 * 'CFreeValueFn' is a function used for freeing the malloc'ed value that is
 * stored inside a data structure.
 * If not provided in the data structure that requires it in order to free
 * the value, then freeing the value is user's responsibility.
 */
typedef void (*CFreeValueFn)(cptr_t value);

/*
 * 'CClearValue' is a function used for 'clearing' the value inside
 * the data structure.
 * This differs from 'freeing' the value, clearing would define
 * freeing all the values that the original value contains or just
 * resetting the original value.
 * The original value should not be free'd if malloc'ed, doing so
 * will cause Undefined Behaviour.
 */
typedef void (*CClearValueFn)(cptr_t value);

#endif
