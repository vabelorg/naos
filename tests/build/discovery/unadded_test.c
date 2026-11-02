/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * A test defined and never added, forgotten, which NaosCheckRegistration.cmake reports.
 */

#include <naos/naos.h>

NAOS_TEST(forgotten)
{
    NAOS_FAIL("never runs");
}

NAOS_TEST(runs)
{
    NAOS_PASS();
}

NAOS_SUITE(kept,
    NAOS_ADD_TEST(runs)
)

NAOS_MAIN(
    NAOS_ADD_SUITE(kept)
)
