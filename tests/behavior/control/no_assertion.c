/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * A test that runs no assertion fails, unless it fails or skips otherwise: what counts as an
 * assertion (NAOS_CHECK_* and NAOS_REQUIRE_* that pass or fail, NAOS_PASS, NAOS_FAIL, NAOS_SKIP, in
 * setup, the body, teardown or a helper) and what does not (notes, traces, printing, an assertion
 * never reached). A test prints "reached-..." after a point, so its expected output shows whether it
 * got there.
 */

#include <naos/naos.h>

#include <stdio.h>
#include <stddef.h>

/* No case: the loop below runs no assertion, as a table not yet filled in. */
static const int g_no_values[1] = { 0 };
static const size_t g_no_value_count = 0;

static void check_positive(int value)
{
    NAOS_CHECK_INT_GT(value, 0);
}

static int g_ready = 0;

NAOS_TEST(empty)
{
}

NAOS_TEST(only_prints)
{
    printf("reached-body\n");
}

NAOS_TEST(only_note)
{
    NAOS_NOTE("unused note");
}

NAOS_TEST(only_trace)
{
    NAOS_TRACE_PUSH("trace");
    NAOS_TRACE_POP();
}

NAOS_TEST(loop_over_no_case)
{
    size_t i;

    for (i = 0; i < g_no_value_count; i++)
    {
        NAOS_CHECK_INT_GT(g_no_values[i], 0);
    }
}

NAOS_TEST(returns_early)
{
    if (g_ready == 0)
    {
        return;
    }

    NAOS_CHECK_TRUE(g_ready);
}

NAOS_SUITE(none,
    NAOS_ADD_TEST(empty),
    NAOS_ADD_TEST(only_prints),
    NAOS_ADD_TEST(only_note),
    NAOS_ADD_TEST(only_trace),
    NAOS_ADD_TEST(loop_over_no_case),
    NAOS_ADD_TEST(returns_early)
)

NAOS_TEST(check_passes)
{
    NAOS_CHECK_TRUE(1);
}

/* One failure: the assertion that failed ran, so no failure is added for the count. */
NAOS_TEST(check_fails)
{
    NAOS_CHECK_TRUE(0);
}

NAOS_TEST(require_fails)
{
    NAOS_REQUIRE_TRUE(0);
}

NAOS_TEST(in_helper)
{
    check_positive(1);
}

/* NAOS_PASS counts and goes on. */
NAOS_TEST(pass)
{
    NAOS_PASS();
    printf("reached-after-pass\n");
}

NAOS_TEST(fails)
{
    NAOS_FAIL("failed on purpose");
}

NAOS_TEST(skips)
{
    NAOS_SKIP("skipped on purpose");
}

NAOS_SUITE(counted,
    NAOS_ADD_TEST(check_passes),
    NAOS_ADD_TEST(check_fails),
    NAOS_ADD_TEST(require_fails),
    NAOS_ADD_TEST(in_helper),
    NAOS_ADD_TEST(pass),
    NAOS_ADD_TEST(fails),
    NAOS_ADD_TEST(skips)
)

NAOS_SETUP(check_in_setup)
{
    NAOS_CHECK_TRUE(1);
}

NAOS_TEARDOWN(check_in_teardown)
{
    NAOS_CHECK_TRUE(1);
}

NAOS_SETUP(skip_in_setup)
{
    NAOS_SKIP("skipped in setup");
}

NAOS_TEARDOWN(fail_in_teardown)
{
    NAOS_CHECK_TRUE(0);
}

NAOS_TEST(body)
{
    printf("reached-body\n");
}

/* Setup, the body and teardown count together. */
NAOS_SUITE(in_setup,
    NAOS_SET_SETUP(check_in_setup),
    NAOS_ADD_TEST(body)
)

NAOS_SUITE(in_teardown,
    NAOS_SET_TEARDOWN(check_in_teardown),
    NAOS_ADD_TEST(body)
)

NAOS_SUITE(skipped_in_setup,
    NAOS_SET_SETUP(skip_in_setup),
    NAOS_ADD_TEST(body)
)

NAOS_SUITE(failed_in_teardown,
    NAOS_SET_TEARDOWN(fail_in_teardown),
    NAOS_ADD_TEST(body)
)

NAOS_MAIN(
    NAOS_ADD_SUITE(none),
    NAOS_ADD_SUITE(counted),
    NAOS_ADD_SUITE(in_setup),
    NAOS_ADD_SUITE(in_teardown),
    NAOS_ADD_SUITE(skipped_in_setup),
    NAOS_ADD_SUITE(failed_in_teardown)
)
