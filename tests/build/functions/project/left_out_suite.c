/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * A suite defined and never added, left_out: discovery fails unless IGNORE_SUITES names it.
 */

#include <naos/naos.h>

NAOS_TEST(passes)
{
    NAOS_PASS();
}

NAOS_SUITE(kept,
    NAOS_ADD_TEST(passes)
)

NAOS_SUITE(left_out,
    NAOS_ADD_TEST(passes)
)

NAOS_MAIN(
    NAOS_ADD_SUITE(kept)
)
