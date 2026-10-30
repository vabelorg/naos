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

#include "context.h"
#include "diagnostic.h"

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
    naos_context_clear();
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

naos_outcome_t naos_test_run(const naos_suite_t *suite, const naos_test_t *test)
{
    naos_outcome_t outcome;

    naos_diagnostic_start_test();
    naos_report_test_start(suite->name, test->name);

    run_guarded(suite->setup, test->func);
    run_guarded(NULL, suite->teardown); /* always runs; a jump out of it ends teardown only */

    outcome = naos_diagnostic_outcome();
    naos_report_test_result(suite->name, test->name, outcome, naos_diagnostic_failure_count());

    return outcome;
}

void naos_test_leave(void)
{
    if (g_jump_armed == 0)
    {
        naos_report_error("NAOS_REQUIRE_*, NAOS_FAIL or NAOS_SKIP outside a running test");
        exit(NAOS_EXIT_USAGE);
    }

    naos_report_flush();
    longjmp(g_jump, 1);
}
