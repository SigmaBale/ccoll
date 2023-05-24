#include "../include/avltree.h"
#include "avltree_test.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// TODO: Fix the tests after fixing the API
void avltree_test_remove_nodup(void);
void avltree_test_remove_dup(void);

int
main()
{
    PRINT_TEST_FILE("[avltree_test_remove]")
    PRINT_TEST("avltree_test_remove_nodup")
    PRINT_SUCCESS
    PRINT_TEST("avltree_test_remove_dup")
    PRINT_SUCCESS
}

void
avltree_test_remove_nodup(void)
{
    Avltree_char tree = avltree_new((int (*)(void*, void*)) strcmp);
    assert(tree != NULL);

    avltree_insert(tree, "Mark");
    assert(avltree_size(tree) == 1);
    avltree_insert(tree, "Tom");
    assert(avltree_size(tree) == 2);
    avltree_insert(tree, "Vader");
    assert(avltree_size(tree) == 3);
    avltree_insert(tree, "Platinum");
    assert(avltree_size(tree) == 4);
    avltree_insert(tree, "Diavolo");
    assert(avltree_size(tree) == 5);
    avltree_insert(tree, "Giorno");
    assert(avltree_size(tree) == 6);
    avltree_insert(tree, "Dio");
    assert(avltree_size(tree) == 7);

    avltree_remove(tree, "Vader");
    assert(avltree_size(tree) == 6);
    avltree_remove(tree, "Dio");
    assert(avltree_size(tree) == 5);
    avltree_remove(tree, "Mark");
    assert(avltree_size(tree) == 4);
    avltree_remove(tree, "Diavolo");
    assert(avltree_size(tree) == 3);
    avltree_remove(tree, "Giorno");
    assert(avltree_size(tree) == 2);
    avltree_remove(tree, "Platinum");
    assert(avltree_size(tree) == 1);
    avltree_remove(tree, "Tom");
    assert(avltree_size(tree) == 0);

    avltree_free(tree);
    return;
}

void
avltree_test_remove_dup(void)
{
    Avltree_char tree = avltree_new((int (*)(void*, void*)) strcmp);
    assert(tree != NULL);

    avltree_insert(tree, "Mark");
    assert(avltree_size(tree) == 1);
    avltree_insert(tree, "Mark");
    assert(avltree_size(tree) == 2);
    avltree_insert(tree, "Platinum");
    assert(avltree_size(tree) == 3);
    avltree_insert(tree, "Platinum");
    assert(avltree_size(tree) == 4);
    avltree_insert(tree, "Diavolo");
    assert(avltree_size(tree) == 5);
    avltree_insert(tree, "Giorno");
    assert(avltree_size(tree) == 6);
    avltree_insert(tree, "Diavolo");
    assert(avltree_size(tree) == 7);
    avltree_insert(tree, "Mark");
    assert(avltree_size(tree) == 8);
    avltree_insert(tree, "Dio");
    assert(avltree_size(tree) == 9);
    avltree_insert(tree, "Diavolo");
    assert(avltree_size(tree) == 10);

    avltree_remove(tree, "Mark");
    assert(avltree_size(tree) == 9);
    avltree_remove(tree, "Dio");
    assert(avltree_size(tree) == 8);
    avltree_remove(tree, "Mark");
    assert(avltree_size(tree) == 7);
    avltree_remove(tree, "Diavolo");
    assert(avltree_size(tree) == 6);
    avltree_remove(tree, "Diavolo");
    assert(avltree_size(tree) == 5);
    avltree_remove(tree, "Platinum");
    assert(avltree_size(tree) == 4);
    avltree_remove(tree, "Mark");
    assert(avltree_size(tree) == 3);
    avltree_remove(tree, "Platinum");
    assert(avltree_size(tree) == 2);
    avltree_remove(tree, "Giorno");
    assert(avltree_size(tree) == 1);
    avltree_remove(tree, "Diavolo");
    assert(avltree_size(tree) == 0);

    avltree_free(tree);
    return;
}
