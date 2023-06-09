#define __COL_SRC_FILE__
#include "citer.h"
#undef __COL_SRC_FILE__
#include <stdio.h>

_c_iter_vals
_c_iter_vals_new(cptr_t start, cptr_t end)
{
    return (_c_iter_vals) {
        .start = start,
        .end   = end,
    };
}

_c_iter_vals
_c_iter_vals_default(void)
{
    return (_c_iter_vals) {
        .start = NULL,
        .end   = NULL,
    };
}

_c_iter
_c_iter_new(cptr_t start, cptr_t end)
{
    return (_c_iter) {
        .vals = {
            .start = start,
            .end = end,
        },
    };
}

_c_iter
_c_iter_default(void)
{
    return (_c_iter) {
        .vals = _c_iter_vals_default(),
    };
}
