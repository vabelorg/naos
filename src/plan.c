/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * Which tests run, and in what order (plan.h).
 */

#include "plan.h"

#include <stdio.h>
#include <stdlib.h>

#include <naos/exit.h>

#include "filter.h"
#include "format.h"
#include "message.h"
#include "shuffle.h"

/* Whether the filter of plan selects test of suite. */
static int selects(const naos_plan_t *plan, const naos_suite_t *suite, const naos_test_t *test)
{
    return naos_filter_matches(plan->filter, suite->name, test->name);
}

int naos_plan_make(naos_plan_t *plan, const naos_options_t *options, const naos_suite_t *suites, size_t count)
{
    size_t i;
    size_t j;

    plan->suites = suites;
    plan->count = count;
    plan->filter = options->filter;
    plan->shuffle = options->shuffle;
    plan->suite_order = NULL;
    plan->test_order = NULL;
    plan->suite = NULL;
    plan->next_suite = 0;
    plan->next_test = 0;

    plan->selected = 0;
    for (i = 0; i < count; i++)
    {
        for (j = 0; j < suites[i].count; j++)
        {
            plan->selected += (size_t)selects(plan, &suites[i], &suites[i].tests[j]);
        }
    }

    if ((plan->filter != NULL) && (plan->selected == 0))
    {
        naos_message("no test matches: %s", plan->filter);
        naos_options_print_usage(stderr, options->program);

        return NAOS_EXIT_USAGE;
    }

    return NAOS_EXIT_PASS;
}

void naos_plan_list(const naos_plan_t *plan)
{
    size_t i;
    size_t j;

    for (i = 0; i < plan->count; i++)
    {
        for (j = 0; j < plan->suites[i].count; j++)
        {
            if (selects(plan, &plan->suites[i], &plan->suites[i].tests[j]) != 0)
            {
                naos_printf("%s.%s\n", plan->suites[i].name, plan->suites[i].tests[j].name);
            }
        }
    }

    fflush(stdout);
}

int naos_plan_prepare(naos_plan_t *plan)
{
    size_t most;
    size_t i;

    most = 1;
    for (i = 0; i < plan->count; i++)
    {
        most = (plan->suites[i].count > most) ? plan->suites[i].count : most;
    }

    plan->suite_order = (size_t *)malloc(((plan->count > 0) ? plan->count : 1) * sizeof(size_t));
    plan->test_order = (size_t *)malloc(most * sizeof(size_t));
    if ((plan->suite_order == NULL) || (plan->test_order == NULL))
    {
        naos_plan_release(plan);
        naos_message("out of memory while ordering the tests");

        return 0;
    }

    return 1;
}

void naos_plan_release(naos_plan_t *plan)
{
    free(plan->suite_order);
    free(plan->test_order);
    plan->suite_order = NULL;
    plan->test_order = NULL;
}

void naos_plan_start_round(naos_plan_t *plan, unsigned long seed)
{
    if (plan->shuffle != 0)
    {
        naos_shuffle_start(seed);
    }

    naos_shuffle_make(plan->suite_order, plan->count, plan->shuffle);
    plan->suite = NULL;
    plan->next_suite = 0;
    plan->next_test = 0;
}

int naos_plan_next(naos_plan_t *plan, const naos_suite_t **suite, const naos_test_t **test)
{
    const naos_test_t *candidate;

    for (;;)
    {
        if ((plan->suite != NULL) && (plan->next_test < plan->suite->count))
        {
            candidate = &plan->suite->tests[plan->test_order[plan->next_test]];
            plan->next_test++;
            if (selects(plan, plan->suite, candidate) != 0)
            {
                *suite = plan->suite;
                *test = candidate;

                return 1;
            }
        }
        else if (plan->next_suite < plan->count)
        {
            plan->suite = &plan->suites[plan->suite_order[plan->next_suite]];
            plan->next_suite++;
            naos_shuffle_make(plan->test_order, plan->suite->count, plan->shuffle);
            plan->next_test = 0;
        }
        else
        {
            return 0;
        }
    }
}
