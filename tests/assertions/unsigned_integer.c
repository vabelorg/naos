/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * NAOS_*_UINT_*: compared and written as uintmax_t.
 */

#include <naos/naos.h>

#include <stdint.h>

NAOS_TEST(eq)
{
    NAOS_CHECK_UINT_EQ(1u, 2u);
}

/* UINTMAX_MAX is the largest value, not -1. */
NAOS_TEST(compare)
{
    NAOS_CHECK_UINT_NE(8u, 8u);
    NAOS_CHECK_UINT_LT(UINTMAX_MAX, 0u);
    NAOS_CHECK_UINT_LE(10u, 9u);
    NAOS_CHECK_UINT_GT(0u, UINTMAX_MAX);
    NAOS_CHECK_UINT_GE(11u, 12u);
}

NAOS_SUITE(unsigned_integer,
    NAOS_ADD_TEST(eq),
    NAOS_ADD_TEST(compare)
)
