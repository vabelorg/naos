/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * naos/session.h: naos_init(), NAOS_ADD_SUITE and naos_run(), and the order they are called in.
 */

#include <naos/session.h>

#include <stdlib.h>

#include "options.h"
#include "report.h"
#include "runner.h"
#include "suite_internal.h"

/*
 * The calls come in this order, each moving to the next phase: naos_init(), then NAOS_ADD_SUITE any
 * number of times, then naos_run(). A call out of its phase is a usage error.
 */
typedef enum naos_phase
{
    NAOS_PHASE_START,   /* before naos_init() */
    NAOS_PHASE_ADDING,  /* after naos_init(): NAOS_ADD_SUITE and naos_run() */
    NAOS_PHASE_RUNNING  /* naos_run() has started; nothing more until the process ends */
} naos_phase_t;

static naos_phase_t g_phase = NAOS_PHASE_START;

/*
 * Checks that what (naos_init(), NAOS_ADD_SUITE or naos_run()) is called in the phase expected.
 * Otherwise prints which call came out of order and returns 0.
 */
static int check_phase(naos_phase_t expected, const char *what)
{
    static const char *const when[] =
    {
        "before naos_init()",
        "after naos_init()",
        "after naos_run() has started"
    };

    if (g_phase == expected)
    {
        return 1;
    }

    naos_report_error("%s %s; the order is naos_init(), NAOS_ADD_SUITE, naos_run()", what, when[g_phase]);

    return 0;
}

void naos_init(int *argc, char **argv)
{
    if (check_phase(NAOS_PHASE_START, "naos_init()") == 0)
    {
        exit(NAOS_EXIT_USAGE);
    }

    /* Even when the arguments are a usage error: naos_run() then returns NAOS_EXIT_USAGE. */
    g_phase = NAOS_PHASE_ADDING;
    naos_options_parse(argc, argv);
}

void naos_add_suite(naos_suite_func_t suite)
{
    /* NAOS_ADD_SUITE returns nothing, so a misuse ends the process, as NAOS_FAIL does outside a test. */
    if (check_phase(NAOS_PHASE_ADDING, "NAOS_ADD_SUITE") == 0)
    {
        exit(NAOS_EXIT_USAGE);
    }

    naos_suite_keep(suite);
}

int naos_run(void)
{
    const naos_options_t *options;
    naos_builder_t builder;
    int result;

    if (check_phase(NAOS_PHASE_ADDING, "naos_run()") == 0)
    {
        return NAOS_EXIT_USAGE;
    }

    g_phase = NAOS_PHASE_RUNNING;

    /*
     * Mistakes in the suites and a lack of memory, here or in NAOS_ADD_SUITE, are reported by
     * naos_suite_collect().
     */
    result = naos_suite_collect(&builder);
    options = naos_options_get();

    if (result == NAOS_EXIT_PASS)
    {
        if (options->usage_error != 0)
        {
            result = NAOS_EXIT_USAGE;
        }
        else if (options->help_requested == 0)
        {
            result = naos_runner_run(options, builder.suites, builder.count);
        }
    }

    naos_suite_release(&builder);

    return result;
}
