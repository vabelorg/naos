/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * Does not compile, on purpose: a setup named in NAOS_ADD_TEST. check-misuse.cmake compiles it and
 * expects the mistake to be reported.
 */

#include <naos/naos.h>

NAOS_SETUP(prepare)
{
}

NAOS_SUITE(setup_as_test,
    NAOS_ADD_TEST(prepare)
)
