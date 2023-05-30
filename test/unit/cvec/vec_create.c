#define __TEST__
#include "../../../src/cvec.h"
#include "../include/test.h"
#undef __TEST__

void
vec_create_test(void)
{
    START_TEST("vec_create_test");

    CVec* new = cvec_new(sizeof(int));
    ASSERT_NEQ(new, NULL, "new vec is NULL");

    CVec* new_with_cap = cvec_with_capacity(sizeof(char*), 10);
    ASSERT_EQ(cvec_capacity(new_with_cap),
              10,
              "new_with_cap does not have capacity of 10");

    double arr[] = { 53.5, 10.5342, 100.15 };
    size_t len   = sizeof(arr) / sizeof(double);

    CVec* new_from = cvec_from(arr, len);
    ASSERT(cvec_len(new_from) == len, NULL);

    cvec_free(new);
    cvec_free(new_with_cap);
    cvec_free(new_from);

    END_TEST;
}
