#ifndef __COL_ERROR_H__
#define __COL_ERROR_H__

#if !defined(__COL_C_FILE__) && !defined(__COL_LIB_INSIDE__)
#error "Only <collib.h> can be included directly"
#endif

#include <stdio.h>
#include <stdlib.h>

#ifndef COL_ALLOC_ERROR
#define COL_ALLOC_ERROR                                                        \
  fprintf(stderr, "%s:%d: allocation failed\n", __FILE_NAME__, __LINE__)
#endif

#ifndef COL_ERROR
#define COL_ERROR(desc)                                                        \
  fprintf(stderr, "%s:%d: %s\n", __FILE_NAME__, __LINE__, desc)
#endif

#ifndef COL_CAPACITY_EXCEEDED_ERROR
#define COL_CAPACITY_EXCEEDED_ERROR                                            \
  fprintf(stderr, "%s:%d: capacity exceeded, allocation too large\n",          \
          __FILE_NAME__, __LINE__)
#endif

#ifndef COL_INDEX_OUT_OF_BOUNDS_ERROR
#define COL_INDEX_OUT_OF_BOUNDS_ERROR                                          \
  fprintf(stderr, "%s:%d: index out of bounds\n", __FILE_NAME__, __LINE__)
#endif

#endif
