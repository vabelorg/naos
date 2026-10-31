/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 */

#include <naos/naos.h>

#include "clamp.h"

NAOS_TEST(inside_range)
{
    NAOS_CHECK_INT_EQ(5, clamp(5, 0, 10));
}

NAOS_TEST(below_range)
{
    NAOS_CHECK_INT_EQ(0, clamp(-3, 0, 10));
}

NAOS_TEST(above_range)
{
    NAOS_CHECK_INT_EQ(10, clamp(42, 0, 10));
}

NAOS_SUITE(clamp,
    NAOS_ADD_TEST(inside_range),
    NAOS_ADD_TEST(below_range),
    NAOS_ADD_TEST(above_range)
)

NAOS_MAIN(
    NAOS_ADD_SUITE(clamp)
)
