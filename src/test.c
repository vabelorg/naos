/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * Running one test (test.h): setup, the body and teardown under setjmp, and the longjmp that stops
 * them.
 */

#include "test.h"

#include <setjmp.h>
#include <stdlib.h>

#include <naos/exit.h>

#include "clock.h"
#include "diagnostic.h"
#include "message.h"

/*
 * Where naos_test_leave() goes back to: setup and the body, or teardown. Only run_guarded() calls
 * setjmp, so no caller's local variable changes between setjmp and longjmp and none needs to be
 * volatile; jmp_buf stays out of the public API.
 */
static jmp_buf g_jump;
static int g_jump_armed = 0;

/*
 * Runs first, then second, either of which may be NULL: setup and the body, or teardown alone. A
 * fatal failure or a skip in either, or in anything they call, comes back here, so a jump out of
 * first leaves second out without further bookkeeping. A jump skips the pops after it, so the
 * traces are checked only when second returns.
 */
static void run_guarded(naos_func_t first, naos_func_t second)
{
    naos_diagnostic_clear_context();
    g_jump_armed = 1;
    if (setjmp(g_jump) == 0)
    {
        if (first != NULL)
        {
            first();
        }

        if (second != NULL)
        {
            second();
        }

        naos_diagnostic_check_traces_popped();
    }
    g_jump_armed = 0;
}

/*
 * The time is taken here once, between the events that start and end the test, so that every writer
 * shows the same: it covers setup, the body and teardown, and not the writing of RUN and the result.
 */
naos_outcome_t naos_test_run(const naos_suite_t *suite, const naos_test_t *test, int require_assertion, naos_nanoseconds_t *elapsed)
{
    naos_outcome_t outcome;
    naos_nanoseconds_t start;
    naos_nanoseconds_t end;

    naos_diagnostic_start_test();
    naos_report_test_start(suite->name, test->name);
    start = naos_clock_now();

    run_guarded(suite->setup, test->func);
    run_guarded(NULL, suite->teardown); /* always runs; a jump out of it ends teardown only */

    end = naos_clock_now();
    *elapsed = (end > start) ? (end - start) : 0;
    if (require_assertion != 0)
    {
        naos_diagnostic_check_assertion_ran();
    }
    outcome = naos_diagnostic_outcome();
    naos_report_test_result(suite->name, test->name, outcome, naos_diagnostic_failure_count(), *elapsed);

    return outcome;
}

void naos_test_leave(void)
{
    if (g_jump_armed == 0)
    {
        naos_message("NAOS_REQUIRE_*, NAOS_FAIL or NAOS_SKIP outside a running test");
        exit(NAOS_EXIT_USAGE);
    }

    naos_report_flush();
    longjmp(g_jump, 1);
}
