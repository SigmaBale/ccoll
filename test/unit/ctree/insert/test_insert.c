#define __COL_TREE_C_FILE__
#include "../../../../src/ctree.h"
#undef __COL_TREE_C_FILE__
#define __COL_TREE_T_FILE__
#include "../../include/test.h"
#undef __COL_TREE_T_FILE__
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void ctree_test_insert_nodup(void);
void ctree_test_insert_dup(void);

void free_key(cptr);

int
main()
{
    PRINT_TEST_FILE("avltre_test_insert")
    PRINT_TEST("ctree_test_insert_nodup")
    ctree_test_insert_nodup();
    PRINT_SUCCESS

    PRINT_TEST("ctree_test_insert_dup")
    ctree_test_insert_dup();
    PRINT_SUCCESS

    return 0;
}

void
ctree_test_insert_nodup(void)
{
    CTree* tree = ctree_new((CCompareKeyFn) strcmp, (CFreeKeyFn) free_key);
    assert(tree != NULL);

    char* name = (char*) malloc(sizeof("Mark"));
    strcpy(name, "Mark");
    ctree_insert(tree, name);
    assert(ctree_size(tree) == 1);

    name = (char*) malloc(sizeof("Tony"));
    strcpy(name, "Tony");
    ctree_insert(tree, name);
    assert(ctree_size(tree) == 2);

    name = (char*) malloc(sizeof("Apollo"));
    strcpy(name, "Apollo");
    ctree_insert(tree, name);
    assert(ctree_size(tree) == 3);

    name = (char*) malloc(sizeof("Bale"));
    strcpy(name, "Bale");
    ctree_insert(tree, name);
    assert(ctree_size(tree) == 4);

    name = (char*) malloc(sizeof("Diavolo"));
    strcpy(name, "Diavolo");
    ctree_insert(tree, name);
    assert(ctree_size(tree) == 5);

    name = (char*) malloc(sizeof("Eren"));
    strcpy(name, "Eren");
    ctree_insert(tree, name);
    assert(ctree_size(tree) == 6);

    name = (char*) malloc(sizeof("Platinum"));
    strcpy(name, "Platinum");
    ctree_insert(tree, name);
    assert(ctree_size(tree) == 7);

    name = (char*) malloc(sizeof("Rusty"));
    strcpy(name, "Rusty");
    ctree_insert(tree, name);
    assert(ctree_size(tree) == 8);

    ctree_free(tree);
    return;
}

void
ctree_test_insert_dup(void)
{
    CTree* tree = ctree_new((CCompareKeyFn) strcmp, (CFreeKeyFn) free_key);
    assert(tree != NULL);

    char* name = (char*) malloc(sizeof("Mark"));
    strcpy(name, "Mark");
    ctree_insert(tree, name);
    assert(ctree_size(tree) == 1);

    name = (char*) malloc(sizeof("Tony"));
    strcpy(name, "Tony");
    ctree_insert(tree, name);
    assert(ctree_size(tree) == 2);

    name = (char*) malloc(sizeof("Mark"));
    strcpy(name, "Mark");
    ctree_insert(tree, name);
    assert(ctree_size(tree) == 3);

    name = (char*) malloc(sizeof("Tony"));
    strcpy(name, "Tony");
    ctree_insert(tree, name);
    assert(ctree_size(tree) == 4);

    name = (char*) malloc(sizeof("Mark"));
    strcpy(name, "Mark");
    ctree_insert(tree, name);
    assert(ctree_size(tree) == 5);

    name = (char*) malloc(sizeof("Mark"));
    strcpy(name, "Mark");
    ctree_insert(tree, name);
    assert(ctree_size(tree) == 6);

    name = (char*) malloc(sizeof("Tony"));
    strcpy(name, "Tony");
    ctree_insert(tree, name);
    assert(ctree_size(tree) == 7);

    name = (char*) malloc(sizeof("Platinum"));
    strcpy(name, "Platinum");
    ctree_insert(tree, name);
    assert(ctree_size(tree) == 8);

    name = (char*) malloc(sizeof("Platinum"));
    strcpy(name, "Platinum");
    ctree_insert(tree, name);
    assert(ctree_size(tree) == 9);

    name = (char*) malloc(sizeof("Eren"));
    strcpy(name, "Eren");
    ctree_insert(tree, name);
    assert(ctree_size(tree) == 10);

    ctree_free(tree);
    return;
}

void
free_key(cptr key)
{
    free(key);
}
