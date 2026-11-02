/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * A test defined and never added to a suite, left_out: discovery fails.
 */

#include <naos/naos.h>

NAOS_TEST(passes)
{
    NAOS_PASS();
}

NAOS_TEST(left_out)
{
    NAOS_FAIL("never runs");
}

NAOS_SUITE(kept,
    NAOS_ADD_TEST(passes)
)

NAOS_MAIN(
    NAOS_ADD_SUITE(kept)
)
