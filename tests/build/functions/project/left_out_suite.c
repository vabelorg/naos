/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * A suite defined and never added, left_out: discovery fails unless IGNORE_SUITES names it. The
 * test only_in_left_out is in no other suite; it is not reported on its own, since its suite is.
 */

#include <naos/naos.h>

NAOS_TEST(passes)
{
    NAOS_PASS();
}

NAOS_TEST(only_in_left_out)
{
    NAOS_PASS();
}

NAOS_SUITE(kept,
    NAOS_ADD_TEST(passes)
)

NAOS_SUITE(left_out,
    NAOS_ADD_TEST(passes),
    NAOS_ADD_TEST(only_in_left_out)
)

NAOS_MAIN(
    NAOS_ADD_SUITE(kept)
)
