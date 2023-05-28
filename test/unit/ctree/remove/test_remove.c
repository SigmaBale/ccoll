#define __COL_TREE_C_FILE__
#include "../../../src/ctree.h"
#undef __COL_TREE_C_FILE__
#define __COL_TREE_T_FILE__
#include "../../include/test.h"
#undef __COL_TREE_T_FILE__
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void ctree_test_remove_nodup(void);
void ctree_test_remove_dup(void);

void free_key(cptr key);

int
main()
{
    PRINT_TEST_FILE("ctree_test_remove")
    PRINT_TEST("ctree_test_remove_nodup")
    PRINT_SUCCESS
    PRINT_TEST("ctree_test_remove_dup")
    PRINT_SUCCESS
}

void
ctree_test_remove_nodup(void)
{
    CTree* tree = ctree_new((CCompareKeyFn) strcmp, (CFreeKeyFn) free_key);
    assert(tree != NULL);

    char* name = (char*) malloc(sizeof("Mark"));
    ctree_insert(tree, name);
    assert(ctree_size(tree) == 1);

    name = (char*) malloc(sizeof("Tom"));
    ctree_insert(tree, name);
    assert(ctree_size(tree) == 2);

    name = (char*) malloc(sizeof("Vader"));
    ctree_insert(tree, name);
    assert(ctree_size(tree) == 3);

    name = (char*) malloc(sizeof("Platinum"));
    ctree_insert(tree, name);
    assert(ctree_size(tree) == 4);

    name = (char*) malloc(sizeof("Diavolo"));
    ctree_insert(tree, name);
    assert(ctree_size(tree) == 5);

    name = (char*) malloc(sizeof("Giorno"));
    ctree_insert(tree, name);
    assert(ctree_size(tree) == 6);

    name = (char*) malloc(sizeof("Dio"));
    ctree_insert(tree, name);
    assert(ctree_size(tree) == 7);

    name = (char*) malloc(sizeof("Vader"));
    ctree_remove(tree, name);
    assert(ctree_size(tree) == 6);

    name = (char*) malloc(sizeof("Dio"));
    ctree_remove(tree, name);
    assert(ctree_size(tree) == 5);

    name = (char*) malloc(sizeof("Mark"));
    ctree_remove(tree, name);
    assert(ctree_size(tree) == 4);

    name = (char*) malloc(sizeof("Diavolo"));
    ctree_remove(tree, name);
    assert(ctree_size(tree) == 3);

    name = (char*) malloc(sizeof("Giorno"));
    ctree_remove(tree, name);
    assert(ctree_size(tree) == 2);

    name = (char*) malloc(sizeof("Platinum"));
    ctree_remove(tree, name);
    assert(ctree_size(tree) == 1);

    name = (char*) malloc(sizeof("Tom"));
    ctree_remove(tree, name);
    assert(ctree_size(tree) == 0);

    ctree_free(tree);
    return;
}

void
ctree_test_remove_dup(void)
{
    CTree* tree = ctree_new((CCompareKeyFn) strcmp, (CFreeKeyFn) free_key);
    assert(tree != NULL);

    char* name = (char*) malloc(sizeof("Mark"));
    ctree_insert(tree, name);
    assert(ctree_size(tree) == 1);

    name = (char*) malloc(sizeof("Mark"));
    ctree_insert(tree, name);
    assert(ctree_size(tree) == 2);

    name = (char*) malloc(sizeof("Platinum"));
    ctree_insert(tree, name);
    assert(ctree_size(tree) == 3);

    name = (char*) malloc(sizeof("Platinum"));
    ctree_insert(tree, name);
    assert(ctree_size(tree) == 4);

    name = (char*) malloc(sizeof("Diavolo"));
    ctree_insert(tree, name);
    assert(ctree_size(tree) == 5);

    name = (char*) malloc(sizeof("Giorno"));
    ctree_insert(tree, name);
    assert(ctree_size(tree) == 6);

    name = (char*) malloc(sizeof("Diavolo"));
    ctree_insert(tree, name);
    assert(ctree_size(tree) == 7);

    name = (char*) malloc(sizeof("Mark"));
    ctree_insert(tree, name);
    assert(ctree_size(tree) == 8);

    name = (char*) malloc(sizeof("Dio"));
    ctree_insert(tree, name);
    assert(ctree_size(tree) == 9);

    name = (char*) malloc(sizeof("Diavolo"));
    ctree_insert(tree, name);
    assert(ctree_size(tree) == 10);

    name = (char*) malloc(sizeof("Mark"));
    ctree_remove(tree, name);
    assert(ctree_size(tree) == 9);

    name = (char*) malloc(sizeof("Dio"));
    ctree_remove(tree, name);
    assert(ctree_size(tree) == 8);

    name = (char*) malloc(sizeof("Mark"));
    ctree_remove(tree, name);
    assert(ctree_size(tree) == 7);

    name = (char*) malloc(sizeof("Diavolo"));
    ctree_remove(tree, name);
    assert(ctree_size(tree) == 6);

    name = (char*) malloc(sizeof("Diavolo"));
    ctree_remove(tree, name);
    assert(ctree_size(tree) == 5);

    name = (char*) malloc(sizeof("Platinumk"));
    ctree_remove(tree, name);
    assert(ctree_size(tree) == 4);

    name = (char*) malloc(sizeof("Mark"));
    ctree_remove(tree, name);
    assert(ctree_size(tree) == 3);

    name = (char*) malloc(sizeof("Platinum"));
    ctree_remove(tree, name);
    assert(ctree_size(tree) == 2);

    name = (char*) malloc(sizeof("Giorno"));
    ctree_remove(tree, name);
    assert(ctree_size(tree) == 1);

    name = (char*) malloc(sizeof("Diavolo"));
    ctree_remove(tree, name);
    assert(ctree_size(tree) == 0);


    ctree_free(tree);
    return;
}

void
free_key(cptr key)
{
    free(key);
}
