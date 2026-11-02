/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * A test named shared, as in same_name_added.c, which no suite of this file adds.
 */

#include <naos/naos.h>

NAOS_TEST(shared)
{
    NAOS_FAIL("never runs");
}

NAOS_TEST(own)
{
    NAOS_PASS();
}

NAOS_SUITE(other_file,
    NAOS_ADD_TEST(own)
)
