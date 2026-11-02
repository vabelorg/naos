/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * plan.c: which tests a round runs, and in what order. The rules are checked here, apart from the
 * output; the names of --naos-list and the message when no test matches are left to behavior/.
 * These tests restart the random order that this executable shares with the harness; under
 * --naos-shuffle that changes the order of the tests after them, the same way for the same seed.
 */

#include <naos/naos.h>

#include <stddef.h>
#include <string.h>

#include <naos/exit.h>

#include "filter.h"
#include "plan.h"
#include "shuffle.h"

/* Most tests of a round: every test of the suites below. */
#define TEST_COUNT 8

/* Suites of different sizes, so that the order of the tests needs room for the largest. */
static naos_test_t g_a_tests[] = { { "t1", NULL, NULL, NULL, 0 }, { "t2", NULL, NULL, NULL, 0 }, { "t3", NULL, NULL, NULL, 0 } };
static naos_test_t g_b_tests[] = { { "t1", NULL, NULL, NULL, 0 } };
static naos_test_t g_c_tests[] = { { "t1", NULL, NULL, NULL, 0 }, { "t2", NULL, NULL, NULL, 0 }, { "t3", NULL, NULL, NULL, 0 }, { "t4", NULL, NULL, NULL, 0 } };

static naos_suite_t g_suites[] =
{
    { "a", g_a_tests, 3, 3, NULL, NULL },
    { "b", g_b_tests, 1, 1, NULL, NULL },
    { "c", g_c_tests, 4, 4, NULL, NULL }
};

#define SUITE_COUNT (sizeof(g_suites) / sizeof(g_suites[0]))

/* The tests of one round, in the order the plan gave them. */
typedef struct round_tests
{
    const naos_test_t *tests[TEST_COUNT];
    const naos_suite_t *suites[TEST_COUNT];
    size_t count;
} round_tests_t;

/* Makes and prepares the plan of g_suites for filter (NULL for every test) and shuffle. */
static void make_plan(naos_plan_t *plan, const char *filter, int shuffle)
{
    naos_options_t options;

    memset(&options, 0, sizeof(options));
    options.program = "plan";
    options.filter = filter;
    options.shuffle = shuffle;

    NAOS_REQUIRE_INT_EQ(NAOS_EXIT_PASS, naos_plan_make(plan, &options, g_suites, SUITE_COUNT));
    NAOS_REQUIRE_TRUE(naos_plan_prepare(plan));
}

/* Runs a whole round of plan with seed, keeping what it gives in round. */
static void take_round(naos_plan_t *plan, unsigned long seed, round_tests_t *round)
{
    const naos_suite_t *suite;
    const naos_test_t *test;

    round->count = 0;
    naos_plan_start_round(plan, seed);
    while (naos_plan_next(plan, &suite, &test) != 0)
    {
        NAOS_REQUIRE_UINT_LT(round->count, TEST_COUNT);
        round->suites[round->count] = suite;
        round->tests[round->count] = test;
        round->count++;
    }
}

/* Whether the two rounds gave the same tests in the same order. */
static int same_round(const round_tests_t *a, const round_tests_t *b)
{
    size_t i;

    if (a->count != b->count)
    {
        return 0;
    }

    for (i = 0; i < a->count; i++)
    {
        if (a->tests[i] != b->tests[i])
        {
            return 0;
        }
    }

    return 1;
}

/* The seeds the rules are checked over: 1 to 50, and the largest. */
static unsigned long seed_at(size_t index)
{
    return (index < 50) ? (unsigned long)(index + 1) : NAOS_SHUFFLE_SEED_MAX;
}

#define SEED_COUNT 51

NAOS_TEST(registration_order_without_shuffle)
{
    static const naos_test_t *const expected[TEST_COUNT] =
    {
        &g_a_tests[0], &g_a_tests[1], &g_a_tests[2], &g_b_tests[0],
        &g_c_tests[0], &g_c_tests[1], &g_c_tests[2], &g_c_tests[3]
    };
    naos_plan_t plan;
    round_tests_t round;

    make_plan(&plan, NULL, 0);
    take_round(&plan, 1, &round);

    NAOS_CHECK_UINT_EQ(TEST_COUNT, plan.selected);
    NAOS_CHECK_MEM_EQ(expected, sizeof(expected), round.tests, round.count * sizeof(round.tests[0]));
    naos_plan_release(&plan);
}

NAOS_TEST(filter_selects_in_registration_order)
{
    static const naos_test_t *const expected[] =
    {
        &g_a_tests[0], &g_a_tests[2], &g_c_tests[0], &g_c_tests[1], &g_c_tests[3]
    };
    naos_plan_t plan;
    round_tests_t round;

    make_plan(&plan, "a.*:c.*-a.t2:c.t3", 0);
    take_round(&plan, 1, &round);

    NAOS_CHECK_UINT_EQ(sizeof(expected) / sizeof(expected[0]), plan.selected);
    NAOS_CHECK_MEM_EQ(expected, sizeof(expected), round.tests, round.count * sizeof(round.tests[0]));
    naos_plan_release(&plan);
}

/* Shuffled, a round still gives every selected test once, and no other. */
NAOS_TEST(each_selected_once_per_round)
{
    const char *const filter = "*.t1:c.*-c.t2";
    naos_plan_t plan;
    round_tests_t round;
    size_t i;
    size_t j;
    size_t s;

    make_plan(&plan, filter, 1);
    for (s = 0; s < SEED_COUNT; s++)
    {
        take_round(&plan, seed_at(s), &round);
        NAOS_NOTE("seed %lu", seed_at(s));
        NAOS_CHECK_UINT_EQ(plan.selected, round.count);

        for (i = 0; i < round.count; i++)
        {
            NAOS_NOTE("seed %lu, %s.%s", seed_at(s), round.suites[i]->name, round.tests[i]->name);
            NAOS_CHECK_TRUE(naos_filter_matches(filter, round.suites[i]->name, round.tests[i]->name));
            for (j = i + 1; j < round.count; j++)
            {
                NAOS_CHECK_TRUE(round.tests[i] != round.tests[j]);
            }
        }
    }

    naos_plan_release(&plan);
}

/* Shuffled, the tests of a suite still come together: a suite left is never come back to. */
NAOS_TEST(grouped_by_suite)
{
    naos_plan_t plan;
    round_tests_t round;
    int left[SUITE_COUNT];
    size_t i;
    size_t s;

    make_plan(&plan, NULL, 1);
    for (s = 0; s < SEED_COUNT; s++)
    {
        take_round(&plan, seed_at(s), &round);
        memset(left, 0, sizeof(left));
        for (i = 1; i < round.count; i++)
        {
            if (round.suites[i] != round.suites[i - 1])
            {
                left[round.suites[i - 1] - g_suites] = 1;
                NAOS_NOTE("seed %lu, back to %s", seed_at(s), round.suites[i]->name);
                NAOS_CHECK_INT_EQ(0, left[round.suites[i] - g_suites]);
            }
        }
    }

    naos_plan_release(&plan);
}

/*
 * Every test takes part in the shuffle, selected or not, so a filter keeps the order the seed gives
 * the tests it selects: the filtered round is the whole round with the other tests left out.
 */
NAOS_TEST(filter_keeps_shuffled_order)
{
    static const char *const filters[] = { "a.*:c.t2", "*-c.*", "b.t1:c.t4:a.t1" };
    naos_plan_t whole;
    naos_plan_t filtered;
    round_tests_t all;
    round_tests_t some;
    round_tests_t kept;
    size_t f;
    size_t s;
    size_t i;

    make_plan(&whole, NULL, 1);
    for (f = 0; f < sizeof(filters) / sizeof(filters[0]); f++)
    {
        make_plan(&filtered, filters[f], 1);
        for (s = 0; s < SEED_COUNT; s++)
        {
            take_round(&whole, seed_at(s), &all);
            take_round(&filtered, seed_at(s), &some);

            kept.count = 0;
            for (i = 0; i < all.count; i++)
            {
                if (naos_filter_matches(filters[f], all.suites[i]->name, all.tests[i]->name) != 0)
                {
                    kept.tests[kept.count++] = all.tests[i];
                }
            }

            NAOS_NOTE("filter %s, seed %lu", filters[f], seed_at(s));
            NAOS_CHECK_TRUE(same_round(&kept, &some));
        }

        naos_plan_release(&filtered);
    }

    naos_plan_release(&whole);
}

/* Starting a round again with its seed gives its order again, also after a round cut short. */
NAOS_TEST(restart_repeats_order)
{
    const naos_suite_t *suite;
    const naos_test_t *test;
    naos_plan_t plan;
    round_tests_t first;
    round_tests_t again;

    make_plan(&plan, NULL, 1);
    take_round(&plan, 7, &first);

    /* A round left after two tests, as --naos-fail-fast leaves it, then one of another seed. */
    naos_plan_start_round(&plan, 7);
    NAOS_CHECK_TRUE(naos_plan_next(&plan, &suite, &test));
    NAOS_CHECK_TRUE(naos_plan_next(&plan, &suite, &test));
    take_round(&plan, 8, &again);

    take_round(&plan, 7, &again);
    NAOS_CHECK_TRUE(same_round(&first, &again));
    naos_plan_release(&plan);
}

/*
 * A seed gives the same order from one version to the next, so that a seed noted down keeps
 * reproducing its order. The order is written out: drawing the random numbers in another order,
 * such as shuffling every suite when the round starts, keeps the rules above but shows here.
 */
NAOS_TEST(seed_gives_fixed_order)
{
    static const naos_test_t *const expected[TEST_COUNT] =
    {
        &g_b_tests[0], &g_a_tests[1], &g_a_tests[0], &g_a_tests[2],
        &g_c_tests[1], &g_c_tests[2], &g_c_tests[3], &g_c_tests[0]
    };
    naos_plan_t plan;
    round_tests_t round;

    make_plan(&plan, NULL, 1);
    take_round(&plan, 1, &round);

    NAOS_CHECK_MEM_EQ(expected, sizeof(expected), round.tests, round.count * sizeof(round.tests[0]));
    naos_plan_release(&plan);
}

NAOS_TEST(no_suites)
{
    const naos_suite_t *suite;
    const naos_test_t *test;
    naos_options_t options;
    naos_plan_t plan;

    memset(&options, 0, sizeof(options));
    options.program = "plan";

    NAOS_REQUIRE_INT_EQ(NAOS_EXIT_PASS, naos_plan_make(&plan, &options, NULL, 0));
    NAOS_CHECK_UINT_EQ(0, plan.selected);
    NAOS_REQUIRE_TRUE(naos_plan_prepare(&plan));
    naos_plan_start_round(&plan, 1);
    NAOS_CHECK_FALSE(naos_plan_next(&plan, &suite, &test));
    naos_plan_release(&plan);
}

NAOS_SUITE(plan,
    NAOS_ADD_TEST(registration_order_without_shuffle),
    NAOS_ADD_TEST(filter_selects_in_registration_order),
    NAOS_ADD_TEST(each_selected_once_per_round),
    NAOS_ADD_TEST(grouped_by_suite),
    NAOS_ADD_TEST(filter_keeps_shuffled_order),
    NAOS_ADD_TEST(restart_repeats_order),
    NAOS_ADD_TEST(seed_gives_fixed_order),
    NAOS_ADD_TEST(no_suites)
)
