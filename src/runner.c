/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * The whole run of the selected tests (runner.h): the rounds of --naos-repeat, the order of
 * --naos-shuffle, --naos-fail-fast, the events around the rounds and the exit code.
 */

#include "runner.h"

#include <stdint.h>
#include <stdlib.h>

#include <naos/exit.h>

#include "filter.h"
#include "message.h"
#include "order.h"
#include "report.h"
#include "test.h"

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
 * Runs the selected tests once, adding their results and times to tally. --naos-shuffle
 * shuffles the suites, then the tests of each suite, so that the output stays grouped by suite.
 * Every test takes part in the shuffle, selected or not, so that a seed gives the selected tests the
 * same order whatever the filter. With --naos-fail-fast, the round ends after the first failing test,
 * its teardown included.
 */
static void run_round(const naos_options_t *options, const naos_suite_t *suites, size_t count, const naos_orders_t *orders, naos_tally_t *tally)
{
    const naos_suite_t *suite;
    const naos_test_t *test;
    naos_outcome_t outcome;
    naos_nanoseconds_t elapsed;
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

            outcome = naos_test_run(suite, test, &elapsed);
            tally->elapsed += elapsed;
            switch (outcome)
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
    total->elapsed += tally->elapsed;
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

/*
 * Ends a round and tells it. Returns 1 when --naos-fail-fast stops the run after this round, and 0
 * otherwise.
 */
static int end_round(const naos_options_t *options, const naos_round_t *round, size_t selected, const naos_tally_t *tally)
{
    size_t not_run;
    int stopped;

    stopped = 0;
    not_run = 0;
    if ((options->fail_fast != 0) && (tally->failed > 0))
    {
        /* The rest of this round, then every later round; --naos-repeat may make it more than size_t holds. */
        stopped = 1;
        not_run = add_saturated(selected - (tally->passed + tally->failed + tally->skipped), multiply_saturated(selected, (size_t)(options->repeat - round->index)));
    }

    naos_report_round_end(round, tally, not_run);

    return stopped;
}

/*
 * Runs the selected tests options->repeat times; the seed of --naos-shuffle goes up by 1 each
 * time, from NAOS_ORDER_SEED_MAX back to 1. --naos-fail-fast stops them all at the first failing
 * test. Returns the result of all the runs together.
 */
static int run_rounds(const naos_options_t *options, const naos_suite_t *suites, size_t count, size_t selected)
{
    naos_orders_t orders;
    naos_tally_t total = NAOS_TALLY_EMPTY;
    naos_round_t round;
    unsigned long seed;
    long failed_rounds;
    int stopped;
    int result;

    if (alloc_orders(&orders, suites, count) == 0)
    {
        naos_message("out of memory while ordering the tests");

        return NAOS_EXIT_FAIL;
    }

    if (naos_report_begin(options->program, options->output_format, options->output_path, options->quiet) == 0)
    {
        free_orders(&orders);

        return NAOS_EXIT_USAGE;
    }

    seed = choose_seed(options);
    failed_rounds = 0;
    stopped = 0;
    round.repeat = options->repeat;
    round.shuffled = options->shuffle;
    for (round.index = 1; (round.index <= options->repeat) && (stopped == 0); round.index++)
    {
        naos_tally_t tally = NAOS_TALLY_EMPTY;

        round.seed = seed;
        naos_report_round_begin(&round);
        if (options->shuffle != 0)
        {
            /* --naos-shuffle --naos-random-seed <seed> runs the same order again. */
            naos_order_start(seed);
        }

        run_round(options, suites, count, &orders, &tally);
        stopped = end_round(options, &round, selected, &tally);

        failed_rounds += (tally.failed > 0) ? 1 : 0;
        tally_add(&total, &tally);
        seed = naos_order_next_seed(seed);
    }

    free_orders(&orders);
    result = tally_result(&total);

    /* round.index is one past the last round run. */
    if (naos_report_end(failed_rounds, options->repeat, options->repeat - round.index + 1) == 0)
    {
        result = NAOS_EXIT_FAIL;
    }

    return result;
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
        naos_message("no test matches: %s", options->filter);
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
