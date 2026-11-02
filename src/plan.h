/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * Which tests run, and in what order: the selection of --naos-filter, the order of --naos-shuffle in
 * each round, and the names of --naos-list.
 */

#ifndef NAOS_PLAN_H
#define NAOS_PLAN_H

#include <stddef.h>

#include "options.h"
#include "suite_internal.h"

/* The tests of the suites that the options select, and where a round has come to. */
typedef struct naos_plan
{
    const naos_suite_t *suites;
    size_t count;
    const char *filter; /* NULL selects every test */
    int shuffle;
    size_t selected; /* the tests the filter selects, in every round */

    /* Order of the suites, and of the tests of one suite at a time; NULL until naos_plan_prepare(). */
    size_t *suite_order;
    size_t *test_order;

    /* The round going on: the suite gone through, NULL before the first, and what comes next. */
    const naos_suite_t *suite;
    size_t next_suite; /* index into suite_order */
    size_t next_test;  /* index into test_order */
} naos_plan_t;

/*
 * Makes the plan of the tests of suites that naos_suite_collect() accepted, as options say. Returns
 * NAOS_EXIT_PASS, or NAOS_EXIT_USAGE after reporting that --naos-filter selects no test. Allocates
 * nothing: listing the tests needs no order.
 */
int naos_plan_make(naos_plan_t *plan, const naos_options_t *options, const naos_suite_t *suites, size_t count);

/* Prints the selected tests for --naos-list: always in the order of registration, one name per line. */
void naos_plan_list(const naos_plan_t *plan);

/* Allocates the orders the rounds need. Returns 0, with nothing allocated, after reporting a lack of memory. */
int naos_plan_prepare(naos_plan_t *plan);

/* Releases what naos_plan_prepare() allocated, if anything. */
void naos_plan_release(naos_plan_t *plan);

/*
 * Starts a round: with --naos-shuffle, the order of seed (1 to NAOS_SHUFFLE_SEED_MAX), so that the
 * same seed runs the same order again; otherwise the order of registration.
 */
void naos_plan_start_round(naos_plan_t *plan, unsigned long seed);

/*
 * Gives the next selected test of the round in *suite and *test, and returns 1; returns 0 once the
 * round has none left. --naos-shuffle shuffles the suites when the round starts, then the tests of
 * each suite when the round comes to it, so that the output stays grouped by suite. Every test takes
 * part in the shuffle, selected or not, so that a seed gives the selected tests the same order
 * whatever the filter.
 */
int naos_plan_next(naos_plan_t *plan, const naos_suite_t **suite, const naos_test_t **test);

#endif
