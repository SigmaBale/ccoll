#ifndef __COL_CORE_H__
#define __COL_CORE_H__

#if !defined(__COL_H_FILE__) && !defined(__COL_LIB_INSIDE__)
#error "Only <collib.h> can be included directly."
#endif

#define return_val_if_fail(expr, val)                                          \
  if (!(expr))                                                                 \
  return (val)

typedef unsigned char* bptr;

typedef unsigned int cuint;

typedef const void *cconstptr;

typedef void *cptr;

/*
 * 'CCompareKeyFn' is alias for function pointer that is
 * used to compare keys inserted into data
 * structures that rely on comparison functions
 * for sorting the elements (returns int).
 */
typedef int (*CCompareKeyFn)(cconstptr a_key, cconstptr b_key);

/*
 * 'CFreeKeyFn' is alias for function pointer that
 * is user provided and applied each time key
 * gets removed from the data structure.
 * Meaning if you want to free the value (key) when it
 * gets removed from the data structure you should
 * provide the CFreeKeyFree.
 */
typedef void (*CFreeKeyFn)(cptr key);

typedef void (*CFreeValueFn)(cptr value);

#endif
