#include <assert.h>
#define __COL_TEST__
#include "../../../src/ctree.h"
#include <stdio.h>
#include <stdlib.h>
#include <stest.h>

// ****************************************************************//
//                           TEST STRUCTS
// ****************************************************************//

typedef struct _Athlete {
    double      height;
    int         muscle_mass;
    const char* name;
    const char* surname;
} athlete;

athlete*
athlete_new(const char* name, const char* surname, double height, int muscle_mass)
{
    // we assume surname and name are static and can't be freed.
    athlete* ath = (athlete*) malloc(sizeof(athlete));
    if(ath != NULL) {
        ath->name        = name;
        ath->surname     = surname;
        ath->height      = height;
        ath->muscle_mass = muscle_mass;
    }
    return ath;
}

int
athlete_cmp(const athlete* left, const athlete* right)
{
    return left->muscle_mass - right->muscle_mass;
}

typedef struct _Rank {
    unsigned int local;
    unsigned int global;
} rank;

rank*
rank_new(unsigned int global, unsigned int local)
{
    rank* r = (rank*) malloc(sizeof(rank));

    if(r != NULL) {
        r->global = global;
        r->local  = local;
    }

    return r;
}

// ****************************************************************//
//                           MAIN RUNNER
// ****************************************************************//

TEST(ctree_create_test);
TEST(ctree_insert_test);
TEST(ctree_remove_test);
TEST(ctree_entry_test);

int
main(void)
{

    ssuite* suite = ssuite_new("ctree");
    ssuite_add_test(suite, ctree_create_test);
    ssuite_add_test(suite, ctree_insert_test);
    ssuite_add_test(suite, ctree_remove_test);
    ssuite_add_test(suite, ctree_entry_test);

    srunner* runner = srunner_new();
    srunner_add_suite(runner, suite);
    srunner_run(runner);
    srunner_free(runner);

    return 0;
}

// ****************************************************************//
//                          TEST DEFINITIONS
// ****************************************************************//
TEST(ctree_create_test)
{
    ctree* tree = ctree_new((CCompareKeyFn) strcmp, NULL, NULL);

    ASSERT(tree != NULL);

    // No elements
    ASSERT_EQ(ctree_size_bytes(tree), 0);
    ASSERT_EQ(ctree_size(tree), 0);

    ctree_free(tree);
}

TEST(ctree_insert_test)
{
    ctree* tree = ctree_new((CCompareKeyFn) athlete_cmp, free, free);
    ASSERT_EQ(ctree_size_bytes(tree), 0);
    ASSERT_EQ(ctree_size(tree), 0);

    athlete* ath = athlete_new("Bozo", "Licious", 1.69, 110);
    ASSERT(ath != NULL);
    rank* ath_rank = rank_new(10523, 1);
    ASSERT(ath_rank != NULL);

    ctree_insert(tree, ath, ath_rank);
    ASSERT_EQ(ctree_size(tree), 1);
    ASSERT(ctree_size_bytes(tree) != 0);

    // Updating the rank
    ath_rank = rank_new(1523, 5);

    ctree_insert(tree, ath, ath_rank);
    // Size should stay the same
    // we just updated the value of the key
    ASSERT_EQ(ctree_size(tree), 1);
    ASSERT(ctree_size_bytes(tree) != 0);

    ath = athlete_new("Dio", "Brando", 1.90, 120);
    ASSERT(ath != NULL);

    ath_rank = rank_new(1, 1);

    ctree_insert(tree, ath, ath_rank);
    ASSERT_EQ(ctree_size(tree), 2);

    ath = athlete_new("Dio", "Brando2", 1.90, 130);
    ASSERT(ath != NULL);

    ath_rank = rank_new(2, 1);
    ctree_insert(tree, ath, ath_rank);
    ASSERT_EQ(ctree_size(tree), 3);

    ath = athlete_new("Dio", "Brando3", 1.84, 135);
    ASSERT(ath != NULL);

    ath_rank = rank_new(3, 2);
    ctree_insert(tree, ath, ath_rank);
    ASSERT_EQ(ctree_size(tree), 4);
    ASSERT_NEQ(ctree_size_bytes(tree), 0);

    ctree_free(tree);
}

TEST(ctree_remove_test)
{
    // First we insert doing the same thing
    ctree* tree = ctree_new((CCompareKeyFn) athlete_cmp, free, free);
    ASSERT_EQ(ctree_size_bytes(tree), 0);
    ASSERT_EQ(ctree_size(tree), 0);

    athlete* ath1 = athlete_new("Bozo", "Licious", 1.69, 110);
    ASSERT(ath1 != NULL);

    rank* ath_rank = rank_new(10523, 1);
    ASSERT(ath_rank != NULL);

    ASSERT_EQ(ctree_insert(tree, ath1, ath_rank), true);
    ASSERT_EQ(ctree_size(tree), 1);
    ASSERT(ctree_size_bytes(tree) != 0);

    ath_rank = rank_new(1523, 5);
    ASSERT_EQ(ctree_insert(tree, ath1, ath_rank), false);
    ASSERT_EQ(ctree_size(tree), 1);
    ASSERT(ctree_size_bytes(tree) != 0);

    athlete* ath2 = athlete_new("Dio", "Brando", 1.90, 120);
    ath_rank      = rank_new(1, 1);
    ASSERT_EQ(ctree_insert(tree, ath2, ath_rank), true);
    ASSERT_EQ(ctree_size(tree), 2);

    athlete* ath3 = athlete_new("Dio", "Brando2", 1.90, 130);
    ath_rank      = rank_new(2, 1);
    ASSERT_EQ(ctree_insert(tree, ath3, ath_rank), true);
    ASSERT_EQ(ctree_size(tree), 3);

    athlete* ath4 = athlete_new("Dio", "Brando3", 1.84, 135);
    ath_rank      = rank_new(3, 2);
    ASSERT_EQ(ctree_insert(tree, ath4, ath_rank), true);
    ASSERT_EQ(ctree_size(tree), 4);
    ASSERT_NEQ(ctree_size_bytes(tree), 0);

    // Now we remove
    ASSERT(ctree_remove(tree, ath1));
    ASSERT(ctree_size(tree) == 3);
    ASSERT(ctree_remove(tree, ath3));
    ASSERT(ctree_size(tree) == 2);
    ASSERT(ctree_remove(tree, ath2));
    ASSERT(ctree_size(tree) == 1);
    ASSERT(ctree_remove(tree, ath4));
    ASSERT(ctree_size(tree) == 0);
    ASSERT(ctree_size_bytes(tree) == 0);

    ctree_free(tree);
}

TEST(ctree_entry_test)
{
    ctree* tree = ctree_new((CCompareKeyFn) athlete_cmp, free, free);
    ASSERT_EQ(ctree_size_bytes(tree), 0);
    ASSERT_EQ(ctree_size(tree), 0);

    athlete* ath1 = athlete_new("Bozo", "Licious", 1.69, 110);
    ASSERT(ath1 != NULL);

    rank* ath_rank = rank_new(10523, 1);
    ASSERT(ath_rank != NULL);

    ASSERT_EQ(ctree_insert(tree, ath1, ath_rank), true);
    ASSERT_EQ(ctree_size(tree), 1);
    ASSERT(ctree_size_bytes(tree) != 0);

    ath_rank = rank_new(1523, 5);
    ASSERT_EQ(ctree_insert(tree, ath1, ath_rank), false);
    ASSERT_EQ(ctree_size(tree), 1);
    ASSERT(ctree_size_bytes(tree) != 0);

    athlete* ath2 = athlete_new("Dio", "Brando", 1.90, 120);
    ath_rank      = rank_new(1, 1);
    ASSERT_EQ(ctree_insert(tree, ath2, ath_rank), true);
    ASSERT_EQ(ctree_size(tree), 2);

    athlete* ath3 = athlete_new("Dio", "Brando2", 1.90, 130);
    ath_rank      = rank_new(2, 1);
    ASSERT_EQ(ctree_insert(tree, ath3, ath_rank), true);
    ASSERT_EQ(ctree_size(tree), 3);

    athlete* ath4 = athlete_new("Dio", "Brando3", 1.84, 135);
    ath_rank      = rank_new(3, 2);
    ASSERT_EQ(ctree_insert(tree, ath4, ath_rank), true);
    ASSERT_EQ(ctree_size(tree), 4);
    ASSERT_NEQ(ctree_size_bytes(tree), 0);

    // New uninserted athlete
    athlete* ath5 = athlete_new("Dio", "Brando3", 1.64, 137);

    // Check if we receive valid stuff back
    ASSERT(athlete_cmp(ctree_key(tree, ath1), ath1) == 0);
    ASSERT(athlete_cmp(ctree_key(tree, ath2), ath2) == 0);
    ASSERT(athlete_cmp(ctree_key(tree, ath3), ath3) == 0);
    ASSERT(athlete_cmp(ctree_key(tree, ath4), ath4) == 0);
    ASSERT(athlete_cmp(ctree_key(tree, ath2), ath3) != 0);
    assert(ctree_key(tree, ath5) == NULL);

    // Let's not forget to free our athlete!
    free(ath5);
    ctree_free(tree);
}
