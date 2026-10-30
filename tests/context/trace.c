/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * NAOS_TRACE_PUSH and NAOS_TRACE_POP: written by every failure until popped, as note lines after
 * its details, innermost first, and checked for misuse. Setup and the body share one stack;
 * teardown starts its own.
 */

#include <naos/naos.h>

#include <stdio.h>

NAOS_TEST(nested)
{
    int i;

    NAOS_TRACE_PUSH("outer");
    for (i = 0; i < 3; i++)
    {
        NAOS_TRACE_PUSH("i=%d", i);
        NAOS_NOTE("inner note");
        NAOS_CHECK_INT_LT(i, 2);
        NAOS_TRACE_POP();
    }
    NAOS_TRACE_POP();
}

NAOS_TEST(popped)
{
    NAOS_TRACE_PUSH("popped-trace");
    NAOS_TRACE_POP();
    NAOS_CHECK_TRUE(0);
}

static void require_with_trace(int value)
{
    NAOS_TRACE_PUSH("helper value=%d", value);
    NAOS_REQUIRE_INT_EQ(1, value);
    NAOS_TRACE_POP();
}

/* The pop skipped by the jump is not a missing pop. */
NAOS_TEST(left_by_jump)
{
    require_with_trace(2);
}

NAOS_TEST(missing_pop)
{
    NAOS_TRACE_PUSH("forgotten");
    NAOS_CHECK_TRUE(1);
}

/*
 * Two traces left pushed: the outer one follows as a note line, and the note is written as a
 * detail.
 */
NAOS_TEST(missing_pop_nested)
{
    NAOS_TRACE_PUSH("outer-forgotten");
    NAOS_TRACE_PUSH("inner-forgotten");
    NAOS_NOTE("missing-pop-note");
}

/* The misuse is one line and leaves the note for the next failure. */
NAOS_TEST(pop_without_push)
{
    NAOS_NOTE("kept-note");
    NAOS_TRACE_POP();
    NAOS_CHECK_TRUE(0);
}

NAOS_TEST(too_deep)
{
    int i;

    for (i = 0; i < 18; i++)
    {
        NAOS_TRACE_PUSH("level %d", i);
    }
    NAOS_CHECK_TRUE(0);
    for (i = 0; i < 18; i++)
    {
        NAOS_TRACE_POP();
    }
}

/*
 * The note is set before the push too deep, which leaves it for the failure after it. That block
 * has the most details any diagnostic has: three values, the note and the count of traces not kept.
 */
NAOS_TEST(too_deep_with_note)
{
    int i;

    NAOS_NOTE("deep-note");
    for (i = 0; i < 17; i++)
    {
        NAOS_TRACE_PUSH("level %d", i);
    }
    NAOS_CHECK_DOUBLE_NEAR(1.0, 2.0, 0.5);
    for (i = 0; i < 17; i++)
    {
        NAOS_TRACE_POP();
    }
}

NAOS_TEST(on_skip)
{
    NAOS_TRACE_PUSH("skip-trace");
    NAOS_SKIP("not applicable");
}

NAOS_SUITE(trace,
    NAOS_ADD_TEST(nested),
    NAOS_ADD_TEST(popped),
    NAOS_ADD_TEST(left_by_jump),
    NAOS_ADD_TEST(missing_pop),
    NAOS_ADD_TEST(missing_pop_nested),
    NAOS_ADD_TEST(pop_without_push),
    NAOS_ADD_TEST(too_deep),
    NAOS_ADD_TEST(too_deep_with_note),
    NAOS_ADD_TEST(on_skip)
)

NAOS_TEARDOWN(check_in_teardown)
{
    NAOS_CHECK_TRUE(1 == 2);
    printf("teardown-ran\n");
}

NAOS_SETUP(push_in_setup)
{
    NAOS_TRACE_PUSH("setup-trace");
}

NAOS_TEARDOWN(push_in_teardown)
{
    NAOS_TRACE_PUSH("teardown-trace");
}

NAOS_TEST(pops_setup_trace)
{
    NAOS_CHECK_TRUE(0);
    NAOS_TRACE_POP();
}

NAOS_TEST(jumps_with_trace)
{
    NAOS_TRACE_PUSH("body-trace");
    NAOS_REQUIRE_TRUE(1 == 2);
}

/* Setup and the body share the traces; teardown starts without them, so its failure prints none. */
NAOS_SUITE(trace_phases,
    NAOS_SET_SETUP(push_in_setup),
    NAOS_SET_TEARDOWN(check_in_teardown),
    NAOS_ADD_TEST(pops_setup_trace),
    NAOS_ADD_TEST(jumps_with_trace)
)

NAOS_TEST(body)
{
    printf("body-ran\n");
}

/* Teardown returning with a trace pushed is a missing pop too. */
NAOS_SUITE(trace_teardown,
    NAOS_SET_TEARDOWN(push_in_teardown),
    NAOS_ADD_TEST(body)
)
