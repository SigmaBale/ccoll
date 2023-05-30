#define __TEST__
#include "../../../src/cvec.h"
#include "../include/test.h"
#undef __TEST__

void
vec_push_test(void)
{
    START_TEST("vec_push_test");

    CVec* vec = cvec_new(sizeof(int));

    cvec_push(vec, &(int) { 5 });
    ASSERT_EQ(cvec_len(vec), 1, "Len is invalid");

    cvec_push(vec, &(int) { 5 });
    ASSERT_EQ(cvec_len(vec), 2, "Len is invalid");

    cvec_push(vec, &(int) { 10 });
    cvec_push(vec, &(int) { 20 });
    cvec_push(vec, &(int) { 30 });
    cvec_push(vec, &(int) { 15 });
    cvec_push(vec, &(int) { 17 });
    cvec_push(vec, &(int) { 10000 });
    ASSERT(cvec_len(vec) == 8, "Len is invalid");

    cvec_free(vec);

    END_TEST;
}
