/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * The rounds of the planned tests (runner.h): --naos-repeat and the seed of each round,
 * --naos-fail-fast, the tally, the events around the rounds and the exit code.
 */

#include "runner.h"

#include <stdint.h>

#include <naos/exit.h>

#include "report.h"
#include "shuffle.h"
#include "test.h"

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
 * Runs the tests of one round of plan, adding their results and times to tally. With
 * --naos-fail-fast, the round ends after the first failing test, its teardown included.
 */
static void run_round(const naos_options_t *options, naos_plan_t *plan, naos_tally_t *tally)
{
    const naos_suite_t *suite;
    const naos_test_t *test;
    naos_outcome_t outcome;
    naos_nanoseconds_t elapsed;

    while (naos_plan_next(plan, &suite, &test) != 0)
    {
        outcome = naos_test_run(suite, test, options->allow_no_assertions == 0, &elapsed);
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

/* Seed of the first round: the one given, or with --naos-shuffle and none given, one from the time. */
static unsigned long choose_seed(const naos_options_t *options)
{
    return (options->shuffle != 0) ? naos_shuffle_pick_seed(options->random_seed) : options->random_seed;
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

int naos_runner_run(const naos_options_t *options, naos_plan_t *plan)
{
    naos_tally_t total = NAOS_TALLY_EMPTY;
    naos_round_t round;
    unsigned long seed;
    long failed_rounds;
    int stopped;
    int result;

    if (naos_plan_prepare(plan) == 0)
    {
        return NAOS_EXIT_FAIL;
    }

    if (naos_report_begin(options->program, options->output_format, options->output_path, options->quiet) == 0)
    {
        naos_plan_release(plan);

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
        naos_plan_start_round(plan, seed);

        run_round(options, plan, &tally);
        stopped = end_round(options, &round, plan->selected, &tally);

        failed_rounds += (tally.failed > 0) ? 1 : 0;
        tally_add(&total, &tally);
        seed = naos_shuffle_next_seed(seed);
    }

    naos_plan_release(plan);
    result = tally_result(&total);

    /* round.index is one past the last round run. */
    if (naos_report_end(failed_rounds, options->repeat, options->repeat - round.index + 1) == 0)
    {
        result = NAOS_EXIT_FAIL;
    }

    return result;
}
