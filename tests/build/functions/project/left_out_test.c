/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * A test and a setup defined and never added: naos_add_executable() makes them errors of the
 * compiler.
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

NAOS_SETUP(left_out)
{
}

NAOS_SUITE(kept,
    NAOS_ADD_TEST(passes)
)

NAOS_MAIN(
    NAOS_ADD_SUITE(kept)
)
