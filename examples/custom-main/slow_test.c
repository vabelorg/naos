/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 */

#include <naos/naos.h>

NAOS_TEST(sums_a_million)
{
    unsigned long sum = 0;
    unsigned long i;

    for (i = 1; i <= 1000000UL; i++)
    {
        sum += i % 7;
    }

    NAOS_CHECK_UINT_EQ(2999998UL, sum);
}

NAOS_SUITE(slow,
    NAOS_ADD_TEST(sums_a_million)
)
