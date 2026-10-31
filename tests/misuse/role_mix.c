/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * Does not compile, on purpose: a teardown named in NAOS_SET_SETUP and a setup named in
 * NAOS_SET_TEARDOWN. check-misuse.cmake compiles it and expects both mistakes to be reported.
 */

#include <naos/naos.h>

NAOS_SETUP(prepare)
{
}

NAOS_TEARDOWN(cleanup)
{
}

NAOS_TEST(only)
{
}

NAOS_SUITE(role_mix,
    NAOS_SET_SETUP(cleanup),
    NAOS_SET_TEARDOWN(prepare),
    NAOS_ADD_TEST(only)
)
