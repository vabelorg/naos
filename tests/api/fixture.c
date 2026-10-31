/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * naos/suite.h: a suite with setup and teardown. Each test checks that setup has run once more
 * than teardown, which holds both in a full run and when the test runs alone.
 */

#include <naos/naos.h>

static int setup_calls;
static int teardown_calls;
static int value;

NAOS_SETUP(setup)
{
    setup_calls++;
    value = 42;
}

NAOS_TEARDOWN(teardown)
{
    teardown_calls++;
    value = 0;
}

NAOS_TEST(setup_runs_before_each_test)
{
    NAOS_CHECK_INT_EQ(teardown_calls + 1, setup_calls);
    NAOS_CHECK_INT_EQ(42, value);
    value = 7;
}

/* The same name as a test of check.c: test functions are static, so files do not clash. */
NAOS_TEST(condition)
{
    NAOS_CHECK_INT_EQ(teardown_calls + 1, setup_calls);
    NAOS_CHECK_TRUE(value == 42);
}

NAOS_SUITE(fixture,
    NAOS_SET_SETUP(setup),
    NAOS_SET_TEARDOWN(teardown),
    NAOS_ADD_TEST(setup_runs_before_each_test),
    NAOS_ADD_TEST(condition)
)
