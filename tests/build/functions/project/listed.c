/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * Tests whose registration with CTest is checked: one of each outcome.
 */

#include <naos/naos.h>

NAOS_TEST(passes)
{
    NAOS_PASS();
}

NAOS_TEST(fails)
{
    NAOS_FAIL("fails on purpose");
}

NAOS_TEST(skips)
{
    NAOS_SKIP("skips on purpose");
}

NAOS_SUITE(listed,
    NAOS_ADD_TEST(passes),
    NAOS_ADD_TEST(fails),
    NAOS_ADD_TEST(skips)
)

NAOS_MAIN(
    NAOS_ADD_SUITE(listed)
)
