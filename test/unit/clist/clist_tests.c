#define __COL_TEST__
#include "../../../src/clist.h"
#include <stdio.h>
#include <stest.h>

TEST(clist_new_test);
TEST(clist_push_test);
TEST(clist_pop_test);
TEST(clist_getters_test);
TEST(cursor_new_test);
TEST(cursor_movement_test);
TEST(cursor_split_test);
TEST(cursor_splice_test);

int
main(void)
{
    ssuite* suite_clist  = ssuite_new("clist-core");
    ssuite* suite_cursor = ssuite_new("clist-cursor");

    ssuite_add_test(suite_clist, clist_new_test);
    ssuite_add_test(suite_clist, clist_push_test);
    ssuite_add_test(suite_clist, clist_pop_test);
    ssuite_add_test(suite_clist, clist_getters_test);

    ssuite_add_test(suite_cursor, cursor_new_test);
    ssuite_add_test(suite_cursor, cursor_movement_test);
    ssuite_add_test(suite_cursor, cursor_split_test);
    ssuite_add_test(suite_cursor, cursor_splice_test);

    srunner* runner = srunner_new();
    srunner_add_suite(runner, suite_clist);
    srunner_add_suite(runner, suite_cursor);

    srunner_run(runner);

    srunner_free(runner);
    return 0;
}

void
dummy_free(void* val)
{
    if(val != NULL) {
        ;
    }
    return;
}

int
intcmp(const int* x, const int* y)
{
    return *x - *y;
}

TEST(clist_new_test)
{
    clist* list = clist_new(NULL);

    ASSERT(list != NULL);

    clist_free(list);
}

TEST(clist_push_test)
{
    clist* list = clist_new(NULL);
    ASSERT(list != NULL);

    clist_push_front(list, &(int) { 5 });
    ASSERT_EQ(clist_len(list), 1);
    clist_push_front(list, &(int) { 7 });
    ASSERT_EQ(clist_len(list), 2);
    clist_push_front(list, &(int) { 1 });
    ASSERT_EQ(clist_len(list), 3);
    clist_push_front(list, &(int) { 10 });
    ASSERT_EQ(clist_len(list), 4);
    clist_push_front(list, &(int) { 107 });
    ASSERT_EQ(clist_len(list), 5);
    clist_push_front(list, &(int) { 68 });
    ASSERT_EQ(clist_len(list), 6);

    clist_push_back(list, &(int) { -1 });
    ASSERT_EQ(clist_len(list), 7);
    clist_push_back(list, &(int) { 234325 });
    ASSERT_EQ(clist_len(list), 8);
    clist_push_back(list, &(int) { 2526 });
    ASSERT_EQ(clist_len(list), 9);
    clist_push_back(list, &(int) { 63458 });
    ASSERT_EQ(clist_len(list), 10);
    clist_push_back(list, &(int) { 863 });
    ASSERT_EQ(clist_len(list), 11);
    clist_push_back(list, &(int) { 4256 });
    ASSERT_EQ(clist_len(list), 12);

    clist_free(list);
}

TEST(clist_pop_test)
{
    clist* list = clist_new(NULL);
    ASSERT(list != NULL);

    clist_push_front(list, &(int) { 5 });
    ASSERT_EQ(clist_len(list), 1);
    clist_push_front(list, &(int) { 7 });
    ASSERT_EQ(clist_len(list), 2);
    clist_push_front(list, &(int) { 1 });
    ASSERT_EQ(clist_len(list), 3);
    clist_push_front(list, &(int) { 10 });
    ASSERT_EQ(clist_len(list), 4);
    clist_push_front(list, &(int) { 107 });
    ASSERT_EQ(clist_len(list), 5);
    clist_push_front(list, &(int) { 68 });
    ASSERT_EQ(clist_len(list), 6);

    clist_push_back(list, &(int) { -1 });
    ASSERT_EQ(clist_len(list), 7);
    clist_push_back(list, &(int) { 234325 });
    ASSERT_EQ(clist_len(list), 8);
    clist_push_back(list, &(int) { 2526 });
    ASSERT_EQ(clist_len(list), 9);
    clist_push_back(list, &(int) { 63458 });
    ASSERT_EQ(clist_len(list), 10);
    clist_push_back(list, &(int) { 863 });
    ASSERT_EQ(clist_len(list), 11);
    clist_push_back(list, &(int) { 4256 });
    ASSERT_EQ(clist_len(list), 12);

    ASSERT(*((int*) clist_pop_front(list)) == 68);
    ASSERT_EQ(clist_len(list), 11);
    ASSERT(*((int*) clist_pop_front(list)) == 107);
    ASSERT_EQ(clist_len(list), 10);
    ASSERT(*((int*) clist_pop_front(list)) == 10);
    ASSERT_EQ(clist_len(list), 9);
    ASSERT(*((int*) clist_pop_front(list)) == 1);
    ASSERT_EQ(clist_len(list), 8);
    ASSERT(*((int*) clist_pop_front(list)) == 7);
    ASSERT_EQ(clist_len(list), 7);
    ASSERT(*((int*) clist_pop_back(list)) == 4256);
    ASSERT_EQ(clist_len(list), 6);
    ASSERT(*((int*) clist_pop_back(list)) == 863);
    ASSERT_EQ(clist_len(list), 5);
    ASSERT(*((int*) clist_pop_back(list)) == 63458);
    ASSERT_EQ(clist_len(list), 4);
    ASSERT(*((int*) clist_pop_back(list)) == 2526);
    ASSERT_EQ(clist_len(list), 3);
    ASSERT(*((int*) clist_pop_back(list)) == 234325);
    ASSERT_EQ(clist_len(list), 2);
    ASSERT(*((int*) clist_pop_front(list)) == 5);
    ASSERT_EQ(clist_len(list), 1);
    ASSERT(*((int*) clist_pop_front(list)) == -1);
    ASSERT_EQ(clist_len(list), 0);

    clist_free(list);
}

TEST(clist_getters_test)
{
    clist* list = clist_new(NULL);
    ASSERT(list != NULL);

    clist_push_front(list, &(int) { 5 });
    ASSERT_EQ(clist_len(list), 1);
    clist_push_front(list, &(int) { 7 });
    ASSERT_EQ(clist_len(list), 2);
    clist_push_front(list, &(int) { 1 });
    ASSERT_EQ(clist_len(list), 3);
    clist_push_front(list, &(int) { 10 });
    ASSERT_EQ(clist_len(list), 4);
    clist_push_front(list, &(int) { 107 });
    ASSERT_EQ(clist_len(list), 5);
    clist_push_front(list, &(int) { 68 });
    ASSERT_EQ(clist_len(list), 6);

    clist_push_back(list, &(int) { -1 });
    ASSERT_EQ(clist_len(list), 7);
    clist_push_back(list, &(int) { 234325 });
    ASSERT_EQ(clist_len(list), 8);
    clist_push_back(list, &(int) { 2526 });
    ASSERT_EQ(clist_len(list), 9);
    clist_push_back(list, &(int) { 63458 });
    ASSERT_EQ(clist_len(list), 10);
    clist_push_back(list, &(int) { 863 });
    ASSERT_EQ(clist_len(list), 11);
    clist_push_back(list, &(int) { 4256 });
    ASSERT_EQ(clist_len(list), 12);

    ASSERT_EQ(*((int*) clist_front(list)), 68);
    ASSERT_EQ(*((int*) clist_back(list)), 4256);

    ASSERT_EQ(*((int*) clist_pop_front(list)), 68);
    ASSERT_EQ(clist_len(list), 11);

    ASSERT_EQ(*((int*) clist_front(list)), 107);
    ASSERT_EQ(*((int*) clist_back(list)), 4256);

    ASSERT_EQ(*((int*) clist_pop_back(list)), 4256);
    ASSERT_EQ(clist_len(list), 10);

    ASSERT_EQ(*((int*) clist_front(list)), 107);
    ASSERT_EQ(*((int*) clist_back(list)), 863);

    ASSERT(clist_contains(list, &(int) { 63458 }, (CCompareKeyFn) intcmp) == true);
    ASSERT(clist_contains(list, &(int) { 4256 }, (CCompareKeyFn) intcmp) == false);

    ASSERT(*((int*) clist_find(list, &(int) { 2526 }, (CCompareKeyFn) intcmp)) == 2526);
    ASSERT(*((int*) clist_find(list, &(int) { 863 }, (CCompareKeyFn) intcmp)) == 863);

    ASSERT_NEQ(clist_is_empty(list), true);

    ASSERT_EQ(*((int*) clist_pop(list, &(int) { 863 }, (CCompareKeyFn) intcmp)), 863);
    ASSERT_EQ(clist_len(list), 9);

    int len = clist_len(list);

    while(len--)
        clist_pop_back(list);

    ASSERT(clist_is_empty(list) == true);

    clist_free(list);
}

TEST(cursor_new_test)
{
    clist* list = clist_new(NULL);
    ASSERT(list != NULL);

    ccursor* cursor = clist_cursor(list);
    ASSERT(cursor != NULL);

    // Freeing list before cursor and using
    // cursor after is Undefined Behaviour.
    cursor_free(cursor);
    clist_free(list);
}

TEST(cursor_movement_test)
{
    clist* list = clist_new(NULL);
    ASSERT(list != NULL);

    ccursor* cursor = clist_cursor(list);
    ASSERT(cursor != NULL);

    ASSERT(cursor_current(cursor) == NULL);
    ASSERT(cursor_peek_next(cursor) == NULL);
    ASSERT(cursor_peek_prev(cursor) == NULL);

    clist_push_front(list, "Waow");
    ASSERT(cursor_current(cursor) == NULL);
    ASSERT_STR_EQ((const char*) cursor_peek_next(cursor), "Waow");
    ASSERT_STR_EQ((const char*) cursor_peek_prev(cursor), "Waow");

    clist_push_front(list, "Meow");
    ASSERT(cursor_current(cursor) == NULL);
    ASSERT_STR_EQ((const char*) cursor_peek_next(cursor), "Meow");
    ASSERT_STR_EQ((const char*) cursor_peek_prev(cursor), "Waow");

    clist_push_front(list, "Okay");
    ASSERT(cursor_current(cursor) == NULL);
    ASSERT_STR_EQ((const char*) cursor_peek_next(cursor), "Okay");
    ASSERT_STR_EQ((const char*) cursor_peek_prev(cursor), "Waow");

    cursor_move_next(cursor);
    ASSERT_STR_EQ((const char*) cursor_current(cursor), "Okay");
    ASSERT_STR_EQ((const char*) cursor_peek_next(cursor), "Meow");
    ASSERT(cursor_peek_prev(cursor) == NULL);

    cursor_move_next(cursor);
    ASSERT_STR_EQ((const char*) cursor_current(cursor), "Meow");
    ASSERT_STR_EQ((const char*) cursor_peek_next(cursor), "Waow");
    ASSERT_STR_EQ((const char*) cursor_peek_prev(cursor), "Okay");

    cursor_move_next(cursor);
    ASSERT_STR_EQ((const char*) cursor_current(cursor), "Waow");
    ASSERT(cursor_peek_next(cursor) == NULL);
    ASSERT_STR_EQ((const char*) cursor_peek_prev(cursor), "Meow");

    cursor_move_prev(cursor);
    ASSERT_STR_EQ((const char*) cursor_current(cursor), "Meow");
    ASSERT_STR_EQ((const char*) cursor_peek_next(cursor), "Waow");
    ASSERT_STR_EQ((const char*) cursor_peek_prev(cursor), "Okay");

    cursor_move_prev(cursor);
    ASSERT_STR_EQ((const char*) cursor_current(cursor), "Okay");
    ASSERT_STR_EQ((const char*) cursor_peek_next(cursor), "Meow");
    ASSERT(cursor_peek_prev(cursor) == NULL);

    cursor_free(cursor);
    clist_free(list);
}

TEST(cursor_split_test)
{
    clist* list = clist_new(NULL);
    ASSERT(list != NULL);

    ccursor* cursor = clist_cursor(list);
    ASSERT(cursor != NULL);

    ASSERT(cursor_current(cursor) == NULL);
    ASSERT(cursor_peek_next(cursor) == NULL);
    ASSERT(cursor_peek_prev(cursor) == NULL);

    clist_push_front(list, "Waow");

    clist_push_front(list, "Meow");

    clist_push_front(list, "Okay");

    clist_push_front(list, "Not");

    clist_push_front(list, "Or");

    clist_push_front(list, "Lame");

    ASSERT(clist_len(list) == 6);

    clist* new_list;
    ASSERT_NEQ((new_list = cursor_split_after(cursor)), NULL);

    ASSERT(clist_len(new_list) == 6);

    ASSERT_STR_EQ((const char*) clist_find(new_list, "Waow", (CCompareKeyFn) strcmp), "Waow");
    ASSERT_STR_EQ((const char*) clist_find(new_list, "Or", (CCompareKeyFn) strcmp), "Or");
    ASSERT_STR_EQ((const char*) clist_find(new_list, "Not", (CCompareKeyFn) strcmp), "Not");
    ASSERT_STR_EQ((const char*) clist_find(new_list, "Meow", (CCompareKeyFn) strcmp), "Meow");
    ASSERT_STR_EQ((const char*) clist_find(new_list, "Okay", (CCompareKeyFn) strcmp), "Okay");
    ASSERT_STR_EQ((const char*) clist_find(new_list, "Lame", (CCompareKeyFn) strcmp), "Lame");

    ccursor* new_cursor = clist_cursor(new_list);
    ASSERT(cursor_current(new_cursor) == NULL);
    ASSERT_STR_EQ((const char*) cursor_peek_next(new_cursor), "Lame");
    ASSERT_STR_EQ((const char*) cursor_peek_prev(new_cursor), "Waow");

    cursor_move_next(new_cursor);
    ASSERT_STR_EQ((const char*) cursor_current(new_cursor), "Lame");
    ASSERT_STR_EQ((const char*) cursor_peek_next(new_cursor), "Or");
    ASSERT(cursor_peek_prev(new_cursor) == NULL);

    cursor_move_next(new_cursor);
    ASSERT_STR_EQ((const char*) cursor_current(new_cursor), "Or");
    ASSERT_STR_EQ((const char*) cursor_peek_next(new_cursor), "Not");
    ASSERT_STR_EQ((const char*) cursor_peek_prev(new_cursor), "Lame");

    cursor_move_next(new_cursor);
    ASSERT_STR_EQ((const char*) cursor_current(new_cursor), "Not");
    ASSERT_STR_EQ((const char*) cursor_peek_next(new_cursor), "Okay");
    ASSERT_STR_EQ((const char*) cursor_peek_prev(new_cursor), "Or");

    cursor_move_next(new_cursor);
    ASSERT_STR_EQ((const char*) cursor_current(new_cursor), "Okay");
    ASSERT_STR_EQ((const char*) cursor_peek_next(new_cursor), "Meow");
    ASSERT_STR_EQ((const char*) cursor_peek_prev(new_cursor), "Not");

    cursor_move_next(new_cursor);
    ASSERT_STR_EQ((const char*) cursor_current(new_cursor), "Meow");
    ASSERT_STR_EQ((const char*) cursor_peek_next(new_cursor), "Waow");
    ASSERT_STR_EQ((const char*) cursor_peek_prev(new_cursor), "Okay");

    cursor_move_next(new_cursor);
    ASSERT_STR_EQ((const char*) cursor_current(new_cursor), "Waow");
    ASSERT(cursor_peek_next(new_cursor) == NULL);
    ASSERT_STR_EQ((const char*) cursor_peek_prev(new_cursor), "Meow");

    cursor_move_prev(new_cursor);
    ASSERT_STR_EQ((const char*) cursor_current(new_cursor), "Meow");
    ASSERT_STR_EQ((const char*) cursor_peek_next(new_cursor), "Waow");
    ASSERT_STR_EQ((const char*) cursor_peek_prev(new_cursor), "Okay");

    ASSERT(clist_len(list) == 0);
    ASSERT_EQ((const char*) clist_find(list, "Waow", (CCompareKeyFn) strcmp), NULL);
    ASSERT_EQ((const char*) clist_find(list, "Or", (CCompareKeyFn) strcmp), NULL);
    ASSERT_EQ((const char*) clist_find(list, "Not", (CCompareKeyFn) strcmp), NULL);
    ASSERT_EQ((const char*) clist_find(list, "Meow", (CCompareKeyFn) strcmp), NULL);
    ASSERT_EQ((const char*) clist_find(list, "Okay", (CCompareKeyFn) strcmp), NULL);
    ASSERT_EQ((const char*) clist_find(list, "Lame", (CCompareKeyFn) strcmp), NULL);

    ASSERT(cursor_current(cursor) == NULL);
    ASSERT(cursor_peek_next(cursor) == NULL);
    ASSERT(cursor_peek_prev(cursor) == NULL);

    clist* even_newer_list;
    ASSERT((even_newer_list = cursor_split_before(new_cursor)) != NULL);

    ccursor* even_newer_cursor = clist_cursor(even_newer_list);
    ASSERT(cursor_current(even_newer_cursor) == NULL);
    ASSERT_STR_EQ((const char*) cursor_peek_next(even_newer_cursor), "Lame");
    ASSERT_STR_EQ((const char*) cursor_peek_prev(even_newer_cursor), "Okay");
    ASSERT_EQ(clist_len(even_newer_list), 4);

    cursor_free(even_newer_cursor);
    cursor_free(new_cursor);
    cursor_free(cursor);
    clist_free(even_newer_list);
    clist_free(new_list);
    clist_free(list);
}

TEST(cursor_splice_test)
{
    clist* list = clist_new(dummy_free);
    ASSERT(list != NULL);

    ccursor* cursor = clist_cursor(list);
    ASSERT(cursor != NULL);

    ASSERT(cursor_current(cursor) == NULL);
    ASSERT(cursor_peek_next(cursor) == NULL);
    ASSERT(cursor_peek_prev(cursor) == NULL);

    clist_push_front(list, "Waow");

    clist_push_front(list, "Meow");

    clist_push_front(list, "Okay");

    clist_push_front(list, "Not");

    clist_push_front(list, "Or");

    clist_push_front(list, "Lame");

    ASSERT(clist_len(list) == 6);

    clist* new_list = clist_new(dummy_free);
    ASSERT(new_list != NULL);

    clist_push_back(new_list, "One");
    clist_push_back(new_list, "Two");
    clist_push_back(new_list, "Three");
    clist_push_back(new_list, "Four");

    ASSERT_EQ(clist_len(new_list), 4);

    ccursor* new_cursor = clist_cursor(new_list);
    ASSERT(new_cursor != NULL);

    ASSERT_STR_EQ((const char*) cursor_peek_next(new_cursor), "One");
    ASSERT_STR_EQ((const char*) cursor_peek_prev(new_cursor), "Four");

    cursor_move_next(cursor);
    cursor_move_next(cursor);

    ASSERT_STR_EQ((const char*) cursor_current(cursor), "Or");

    // Free the cursor of the new_list before the new list
    // gets freed during splicing!
    cursor_free(new_cursor);

    cursor_splice_before(cursor, &new_list);
    ASSERT(new_list == NULL);

    ASSERT_STR_EQ((const char*) cursor_peek_next(cursor), "Not");
    ASSERT_STR_EQ((const char*) cursor_peek_prev(cursor), "Four");
    ASSERT(clist_len(list) == 10);

    clist* empty_list = clist_new(dummy_free);
    ASSERT(empty_list != NULL);
    ccursor* empty_cursor = clist_cursor(empty_list);
    ASSERT(empty_cursor != NULL);

    ASSERT(clist_len(empty_list) == 0);

    cursor_free(cursor);
    cursor_splice_after(empty_cursor, &list);
    ASSERT(clist_len(empty_list) == 10);
    ASSERT(list == NULL);
    ASSERT_STR_EQ((const char*) cursor_peek_next(empty_cursor), "Lame");
    ASSERT_STR_EQ((const char*) cursor_peek_prev(empty_cursor), "Waow");

    cursor_free(empty_cursor);
    clist_free(empty_list);
    // Pain and suffering
}
