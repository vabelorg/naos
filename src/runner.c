/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * The whole run of the selected tests (runner.h): the rounds of --naos-repeat, the order of
 * --naos-shuffle, --naos-fail-fast, the lines around the rounds and the exit code.
 */

#include "runner.h"

#include <stdint.h>
#include <stdlib.h>

#include <naos/exit.h>

#include "filter.h"
#include "order.h"
#include "report.h"
#include "test.h"

typedef struct naos_tally
{
    size_t passed;
    size_t failed;
    size_t skipped;
} naos_tally_t;

/* Order of the suites, and of the tests of one suite at a time; malloc as for the builder. */
typedef struct naos_orders
{
    size_t *suites;
    size_t *tests;
} naos_orders_t;

/* a + b, or SIZE_MAX when the sum does not fit. */
static size_t add_saturated(size_t a, size_t b)
{
    return (a > (SIZE_MAX - b)) ? SIZE_MAX : (a + b);
}

/* a * b, or SIZE_MAX when the product does not fit. */
static size_t multiply_saturated(size_t a, size_t b)
{
    return ((b != 0) && (a > (SIZE_MAX / b))) ? SIZE_MAX : (a * b);
}

/*
 * Runs the selected tests once, adding their results to tally. --naos-shuffle
 * shuffles the suites, then the tests of each suite, so that the output stays grouped by suite.
 * Every test takes part in the shuffle, selected or not, so that a seed gives the selected tests the
 * same order whatever the filter. With --naos-fail-fast, the round ends after the first failing test,
 * its teardown included.
 */
static void run_round(const naos_options_t *options, const naos_suite_t *suites, size_t count, const naos_orders_t *orders, naos_tally_t *tally)
{
    const naos_suite_t *suite;
    const naos_test_t *test;
    size_t i;
    size_t j;

    naos_order_make(orders->suites, count, options->shuffle);
    for (i = 0; i < count; i++)
    {
        suite = &suites[orders->suites[i]];
        naos_order_make(orders->tests, suite->count, options->shuffle);

        for (j = 0; j < suite->count; j++)
        {
            test = &suite->tests[orders->tests[j]];
            if (naos_filter_matches(suite, test, options->filter) == 0)
            {
                continue;
            }

            switch (naos_test_run(suite, test))
            {
            case NAOS_OUTCOME_FAIL:
                tally->failed++;
                if (options->fail_fast != 0)
                {
                    return;
                }
                break;
            case NAOS_OUTCOME_SKIP:
                tally->skipped++;
                break;
            case NAOS_OUTCOME_PASS:
                tally->passed++;
                break;
            }
        }
    }
}

static void tally_add(naos_tally_t *total, const naos_tally_t *tally)
{
    total->passed += tally->passed;
    total->failed += tally->failed;
    total->skipped += tally->skipped;
}

/* Exit code of the tests tallied: FAIL if any failed, SKIP if all were skipped, PASS otherwise. */
static int tally_result(const naos_tally_t *tally)
{
    if (tally->failed > 0)
    {
        return NAOS_EXIT_FAIL;
    }
    else if ((tally->skipped > 0) && (tally->passed == 0))
    {
        return NAOS_EXIT_SKIP;
    }

    return NAOS_EXIT_PASS;
}

/* Allocates the orders for suites[0, count). Returns 0, with nothing allocated, when the memory runs out. */
static int alloc_orders(naos_orders_t *orders, const naos_suite_t *suites, size_t count)
{
    size_t most;
    size_t i;

    most = 1;
    for (i = 0; i < count; i++)
    {
        most = (suites[i].count > most) ? suites[i].count : most;
    }

    orders->suites = (size_t *)malloc(((count > 0) ? count : 1) * sizeof(size_t));
    orders->tests = (size_t *)malloc(most * sizeof(size_t));
    if ((orders->suites == NULL) || (orders->tests == NULL))
    {
        free(orders->suites);
        free(orders->tests);

        return 0;
    }

    return 1;
}

static void free_orders(naos_orders_t *orders)
{
    free(orders->suites);
    free(orders->tests);
}

/* Seed of the first round: the one given, or with --naos-shuffle and none given, one from the time. */
static unsigned long choose_seed(const naos_options_t *options)
{
    return (options->shuffle != 0) ? naos_order_pick_seed(options->random_seed) : options->random_seed;
}

/* Prints what comes before a round: the ITERATION and RANDOM SEED lines. */
static void begin_round(const naos_options_t *options, long round, unsigned long seed)
{
    if (options->repeat > 1)
    {
        naos_report_iteration(round, options->repeat);
    }

    if (options->shuffle != 0)
    {
        /* --naos-shuffle --naos-random-seed <seed> runs the same order again. */
        naos_report_random_seed(seed);
        naos_order_start(seed);
    }
}

/*
 * Prints what follows a round: the FAIL FAST, SUMMARY and ITERATION FAILED lines. Returns 1 when
 * --naos-fail-fast stops the run after this round, and 0 otherwise.
 */
static int end_round(const naos_options_t *options, long round, unsigned long seed, size_t selected, const naos_tally_t *tally)
{
    size_t not_run;
    int stopped;

    stopped = 0;
    if ((options->fail_fast != 0) && (tally->failed > 0))
    {
        /* The rest of this round, then every later round; --naos-repeat may make it more than size_t holds. */
        stopped = 1;
        not_run = add_saturated(selected - (tally->passed + tally->failed + tally->skipped), multiply_saturated(selected, (size_t)(options->repeat - round)));
        if (not_run > 0)
        {
            naos_report_fail_fast(not_run);
        }
    }

    if (selected > 1)
    {
        naos_report_summary(tally->passed, tally->failed, tally->skipped);
    }

    if ((options->repeat > 1) && (tally->failed > 0))
    {
        naos_report_iteration_failed(round, options->repeat, options->shuffle, seed);
    }

    naos_report_flush();

    return stopped;
}

/* Prints the REPEAT line after the rounds; next_round is one past the last round run. */
static void report_repeat(const naos_options_t *options, long failed_rounds, long next_round)
{
    if (options->repeat <= 1)
    {
        return;
    }

    naos_report_repeat(failed_rounds, options->repeat, options->repeat - next_round + 1);
    naos_report_flush();
}

/*
 * Runs the selected tests options->repeat times; the seed of --naos-shuffle goes up by 1 each
 * time, from NAOS_ORDER_SEED_MAX back to 1. --naos-fail-fast stops them all at the first failing
 * test. Returns the result of all the runs together.
 */
static int run_rounds(const naos_options_t *options, const naos_suite_t *suites, size_t count, size_t selected)
{
    naos_orders_t orders;
    naos_tally_t total = { 0, 0, 0 };
    unsigned long seed;
    long failed_rounds;
    long round;
    int stopped;

    if (alloc_orders(&orders, suites, count) == 0)
    {
        naos_report_error("out of memory while ordering the tests");

        return NAOS_EXIT_FAIL;
    }

    seed = choose_seed(options);
    failed_rounds = 0;
    stopped = 0;
    for (round = 1; (round <= options->repeat) && (stopped == 0); round++)
    {
        naos_tally_t tally = { 0, 0, 0 };

        begin_round(options, round, seed);
        run_round(options, suites, count, &orders, &tally);
        stopped = end_round(options, round, seed, selected, &tally);

        failed_rounds += (tally.failed > 0) ? 1 : 0;
        tally_add(&total, &tally);
        seed = naos_order_next_seed(seed);
    }

    report_repeat(options, failed_rounds, round);
    free_orders(&orders);

    return tally_result(&total);
}

/* Number of tests the filter selects. */
static size_t count_selected(const naos_options_t *options, const naos_suite_t *suites, size_t count)
{
    size_t selected;
    size_t i;
    size_t j;

    selected = 0;
    for (i = 0; i < count; i++)
    {
        for (j = 0; j < suites[i].count; j++)
        {
            selected += (size_t)naos_filter_matches(&suites[i], &suites[i].tests[j], options->filter);
        }
    }

    return selected;
}

/*
 * Prints the selected tests for --naos-list: always in the order of registration, one name per
 * line, for the tools reading it.
 */
static void list_tests(const naos_options_t *options, const naos_suite_t *suites, size_t count)
{
    size_t i;
    size_t j;

    for (i = 0; i < count; i++)
    {
        for (j = 0; j < suites[i].count; j++)
        {
            if (naos_filter_matches(&suites[i], &suites[i].tests[j], options->filter) != 0)
            {
                naos_report_test_name(suites[i].name, suites[i].tests[j].name);
            }
        }
    }

    naos_report_flush();
}

int naos_runner_run(const naos_options_t *options, const naos_suite_t *suites, size_t count)
{
    size_t selected;

    selected = count_selected(options, suites, count);
    if ((options->filter != NULL) && (selected == 0))
    {
        naos_report_error("no test matches: %s", options->filter);
        naos_options_print_usage(stderr, options->program);

        return NAOS_EXIT_USAGE;
    }

    if (options->list_only == 0)
    {
        return run_rounds(options, suites, count, selected);
    }

    list_tests(options, suites, count);

    return NAOS_EXIT_PASS;
}
