/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * naos/control.h: notes and traces around assertions that pass leave no trace in the result.
 */

#include <naos/naos.h>

NAOS_TEST(note_with_passing_assertions)
{
    int i;

    NAOS_NOTE("no arguments");
    NAOS_CHECK_TRUE(1);
    for (i = 0; i < 3; i++)
    {
        NAOS_NOTE("i=%d", i);
        NAOS_CHECK_INT_LT(i, 3);
    }
    NAOS_NOTE("left over at the end of the test");
}

NAOS_TEST(trace_with_passing_assertions)
{
    int i;

    NAOS_TRACE_PUSH("no arguments");
    for (i = 0; i < 3; i++)
    {
        NAOS_TRACE_PUSH("i=%d", i);
        NAOS_CHECK_INT_LT(i, 3);
        NAOS_TRACE_POP();
    }
    NAOS_TRACE_POP();
}

NAOS_SUITE(control,
    NAOS_ADD_TEST(note_with_passing_assertions),
    NAOS_ADD_TEST(trace_with_passing_assertions)
)
