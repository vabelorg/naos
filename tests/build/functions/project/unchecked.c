/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * A test that runs no assertion: it passes only when registered with ALLOW_NO_ASSERTIONS.
 */

#include <naos/naos.h>

NAOS_TEST(runs)
{
}

NAOS_SUITE(unchecked,
    NAOS_ADD_TEST(runs)
)

NAOS_MAIN(
    NAOS_ADD_SUITE(unchecked)
)
