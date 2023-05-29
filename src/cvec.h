#ifndef __COL_VEC_H__
#define __COL_VEC_H__

#if !defined (__COL_LIB_INISDE__) && !defined (__COL_VEC_C_FILE__)
#error "Only <collib.h> can be included directly."
#endif

#define __COL_H_FILE__
#include "ccore.h"
#undef __COL_H_FILE__

typedef struct _CVec CVec;

#endif
