/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * NAOS_*_INT_*: both values, as left and right, in decimal.
 */

#include <naos/naos.h>

#include <stdint.h>

NAOS_TEST(eq)
{
    NAOS_CHECK_INT_EQ(-1, 2);
}

NAOS_TEST(ne)
{
    NAOS_CHECK_INT_NE(3, 3);
}

NAOS_TEST(lt)
{
    NAOS_CHECK_INT_LT(2, -1);
}

NAOS_TEST(le)
{
    NAOS_CHECK_INT_LE(-4, -5);
}

NAOS_TEST(gt)
{
    NAOS_CHECK_INT_GT(6, 6);
}

NAOS_TEST(ge)
{
    NAOS_CHECK_INT_GE(INTMAX_MIN, 7);
}

/* A macro given to an assertion is printed as written, not expanded. */
#define EXPECTED_COUNT 16

NAOS_TEST(macro_argument_as_written)
{
    int count = 3;

    NAOS_CHECK_INT_EQ(EXPECTED_COUNT, count);
    NAOS_REQUIRE_INT_LT(EXPECTED_COUNT, count);
}

NAOS_SUITE(integer,
    NAOS_ADD_TEST(eq),
    NAOS_ADD_TEST(ne),
    NAOS_ADD_TEST(lt),
    NAOS_ADD_TEST(le),
    NAOS_ADD_TEST(gt),
    NAOS_ADD_TEST(ge),
    NAOS_ADD_TEST(macro_argument_as_written)
)
