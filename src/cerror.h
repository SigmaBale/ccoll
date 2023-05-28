#ifndef __COL_ERROR_H__
#define __COL_ERROR_H__

#if !defined(__COL_C_FILE__) && !defined(__COL_LIB_INSIDE__)
#error "Only <collib.h> can be included directly"
#endif

#include <stdio.h>

#ifndef COL_ERROR_OUT_OF_MEMORY
#define COL_ERROR_OUT_OF_MEMORY(module) fprintf(stderr, "%s: out of memory\n", module)
#endif

#ifndef COL_ERROR_INVALID_KEY
#define COL_ERROR_INVALID_KEY(module, key) \
    fprintf(stderr, "%s: invalid key -> '%p' (pointer)\n", module, key)
#endif

#ifndef COL_ERROR_INVALID_COMPARISON_FUNCTION
#define COL_ERROR_INVALID_COMPARISON_FUNCTION(module) \
    fprintf(stderr, "%s: comparison fn is NULL\n", module)
#endif

#endif
