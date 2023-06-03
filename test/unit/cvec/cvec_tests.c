#define __COL_TEST__
#include "../../../src/cvec.h"
#include <stest.h>

TEST(cvec_create_test);
TEST(cvec_pop_test);
TEST(cvec_push_test);

int
main(void)
{
    ssuite* suite = ssuite_new("cvec");
    ssuite_add_test(suite, cvec_create_test);
    ssuite_add_test(suite, cvec_pop_test);
    ssuite_add_test(suite, cvec_push_test);

    srunner* runner = srunner_new();
    srunner_add_suite(runner, suite);
    srunner_run(runner);
    srunner_free(runner);

    return 0;
}

TEST(cvec_create_test)
{
    CVec* new = cvec_new(sizeof(int));
    ASSERT_NEQ(new, NULL);

    CVec* new_with_cap = cvec_with_capacity(sizeof(char*), 10);
    ASSERT_EQ(cvec_capacity(new_with_cap), 10);

    double arr[] = { 53.5, 10.5342, 100.15 };
    size_t len   = sizeof(arr) / sizeof(double);

    CVec* new_from = cvec_from(arr, len);
    ASSERT(cvec_len(new_from) == (int) len);

    cvec_free(new);
    cvec_free(new_with_cap);
    cvec_free(new_from);
}

TEST(cvec_pop_test)
{
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

    ASSERT_EQ(cvec_len(vec), 6);

    struct TestStruct* value;
    // pop the value
    ASSERT_NEQ((value = cvec_pop(vec)), NULL);
    // and check all of its fields
    ASSERT_EQ(value->dfield, 67.68);
    ASSERT_EQ(value->ifield, 69);
    ASSERT(value->pfield == NULL);

    // and repeat
    value = cvec_pop(vec);
    ASSERT_EQ(value->dfield, 0.115);
    ASSERT_EQ(value->ifield, 9998);
    ASSERT(value->pfield == NULL);

    value = cvec_pop(vec);
    ASSERT_EQ(value->dfield, 5552.5);
    ASSERT_EQ(value->ifield, -252);
    ASSERT(value->pfield == NULL);

    ASSERT(cvec_len(vec) == 3);

    value = cvec_pop(vec);
    ASSERT_EQ(value->dfield, 10.523455);
    ASSERT_EQ(value->ifield, 155);
    ASSERT(value->pfield == NULL);

    value = cvec_pop(vec);
    ASSERT_EQ(value->dfield, 2.5);
    ASSERT_EQ(value->ifield, 1);
    ASSERT(value->pfield == NULL);

    ASSERT(cvec_len(vec) == 1);

    ASSERT_NEQ(value = cvec_pop(vec), NULL);
    ASSERT_EQ(value->dfield, 0);
    ASSERT_EQ(value->ifield, 5);
    ASSERT(value->pfield == NULL);

    ASSERT(cvec_len(vec) == 0);

    cvec_free(vec);
}

TEST(cvec_push_test)
{
    CVec* vec = cvec_new(sizeof(int));
    ASSERT(vec != NULL);

    cvec_push(vec, &(int) { 5 });
    ASSERT_EQ(cvec_len(vec), 1);

    cvec_push(vec, &(int) { 5 });
    ASSERT_EQ(cvec_len(vec), 2);

    cvec_push(vec, &(int) { 10 });
    cvec_push(vec, &(int) { 20 });
    cvec_push(vec, &(int) { 30 });
    cvec_push(vec, &(int) { 15 });
    cvec_push(vec, &(int) { 17 });
    cvec_push(vec, &(int) { 10000 });
    ASSERT(cvec_len(vec) == 8);

    cvec_free(vec);
}
