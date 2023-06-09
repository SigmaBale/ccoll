#ifndef __COL_ITER_H__
#define __COL_ITER_H__

#if !defined(__COL_SRC_FILE__) && !defined(__COL_H_FILE__)
#error "Only <collib.h> can be included directly."
#endif

#define __COL_H_FILE__
#include "ccore.h"
#undef __COL_H_FILE__

typedef struct {
  cptr_t start;
  cptr_t end;
} _c_iter_vals;

typedef struct _c_iter {
  _c_iter_vals vals;
} _c_iter;

_c_iter_vals _c_iter_vals_new(cptr_t start, cptr_t end);

_c_iter_vals _c_iter_vals_default(void);

_c_iter _c_iter_new(cptr_t start, cptr_t end);

_c_iter _c_iter_default(void);

#endif
