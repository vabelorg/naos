/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * A test that ends the process during the run, as a crash would, leaving the placeholder in the
 * JUnit XML file of --naos-output.
 */

#include <stdlib.h>

#include <naos/naos.h>

NAOS_TEST(exits)
{
    exit(3);
}

NAOS_SUITE(ending,
    NAOS_ADD_TEST(exits)
)

NAOS_MAIN(
    NAOS_ADD_SUITE(ending)
)
