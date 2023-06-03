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

typedef struct _CVec CVec;

CVec *cvec_new(size_t t_size);
CVec *cvec_from(cptr src, size_t len);
CVec *cvec_with_capacity(size_t t_size, size_t capacity);
cuint cvec_push(CVec *vec, cptr value);
cptr cvec_get(CVec *vec, cuint idx);
cptr cvec_pop(CVec *vec);
int cvec_len(CVec *vec);
void cvec_free(CVec *vec);
int cvec_capacity(CVec *vec);

#endif
