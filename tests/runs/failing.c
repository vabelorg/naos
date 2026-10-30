/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * Tests that fail or skip, for --naos-fail-fast and --naos-repeat: what stops a run, and what does
 * not.
 */

#include <naos/naos.h>

#include <stdio.h>

NAOS_TEARDOWN(check_in_teardown)
{
    NAOS_CHECK_TRUE(1 == 2);
    printf("teardown-ran\n");
}

NAOS_TEST(body)
{
    printf("body-ran\n");
}

/* The test fails in teardown, which runs to its end before fail fast stops the run. */
NAOS_SUITE(teardown_fails,
    NAOS_SET_TEARDOWN(check_in_teardown),
    NAOS_ADD_TEST(body)
)

NAOS_TEST(passes)
{
    NAOS_CHECK_TRUE(1);
}

NAOS_TEST(fails)
{
    NAOS_CHECK_INT_EQ(1, 2);
}

NAOS_TEST(fails_too)
{
    NAOS_CHECK_UINT_EQ(1u, 2u);
}

NAOS_TEST(passes_too)
{
    NAOS_CHECK_TRUE(1);
}

NAOS_SUITE(mixed,
    NAOS_ADD_TEST(passes),
    NAOS_ADD_TEST(fails),
    NAOS_ADD_TEST(fails_too),
    NAOS_ADD_TEST(passes_too)
)

NAOS_TEST(skip_one)
{
    NAOS_SKIP("not applicable");
}

NAOS_TEST(skip_two)
{
    NAOS_SKIP("not applicable either");
}

NAOS_SUITE(skips,
    NAOS_ADD_TEST(skip_one),
    NAOS_ADD_TEST(skip_two),
    NAOS_ADD_TEST(passes)
)

NAOS_MAIN(
    NAOS_ADD_SUITE(teardown_fails),
    NAOS_ADD_SUITE(mixed),
    NAOS_ADD_SUITE(skips)
)
