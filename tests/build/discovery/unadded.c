/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * A suite defined and never added, left_out, which NaosCheckRegistration.cmake reports.
 */

#include <naos/naos.h>

NAOS_TEST(runs)
{
    NAOS_PASS();
}

NAOS_SUITE(added,
    NAOS_ADD_TEST(runs)
)

NAOS_SUITE(left_out,
    NAOS_ADD_TEST(runs)
)

NAOS_MAIN(
    NAOS_ADD_SUITE(added)
)
