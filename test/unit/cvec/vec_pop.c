#define __TEST__
#include "../../../src/cvec.h"
#include "../include/test.h"
#undef __TEST__

void
vec_pop_test(void)
{
    START_TEST("vec_pop_test");

    struct TestStruct {
        double dfield;
        int    ifield;
        void*  pfield;
    };

    CVec* vec = cvec_new(sizeof(struct TestStruct));

    cvec_push(vec, &(struct TestStruct) { 0, 5, NULL });
    cvec_push(vec, &(struct TestStruct) { 2.5, 1, NULL });
    cvec_push(vec, &(struct TestStruct) { 10.523455, 155, NULL });
    cvec_push(vec, &(struct TestStruct) { 5552.5, -252, NULL });
    cvec_push(vec, &(struct TestStruct) { 0.115, 9998, NULL });
    cvec_push(vec, &(struct TestStruct) { 67.68, 69, NULL });

    ASSERT_EQ(cvec_len(vec), 6, "Length incorrect");

    ASSERT_NEQ(cvec_pop(vec), NULL, "Got NULL expected non-null");

    cvec_pop(vec);
    cvec_pop(vec);

    ASSERT(cvec_len(vec) == 3, "Invalid len");

    cvec_pop(vec);
    cvec_pop(vec);

    ASSERT(cvec_len(vec) == 1, "Invalid len");

    ASSERT_NEQ(cvec_pop(vec), NULL, "Got NULL expected non-null");

    ASSERT(cvec_len(vec) == 0, "Invalid len");

    cvec_free(vec);

    END_TEST;
}
