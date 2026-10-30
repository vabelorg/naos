/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * Does not compile, on purpose: a test named in NAOS_SET_SETUP. check-misuse.cmake compiles it and
 * expects the mistake to be reported.
 */

#include <naos/naos.h>

NAOS_TEST(only)
{
}

NAOS_SUITE(test_as_setup,
    NAOS_SET_SETUP(only),
    NAOS_ADD_TEST(only)
)
