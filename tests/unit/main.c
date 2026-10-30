/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * Unit tests of the internal modules of Naos whose results depend only on their arguments, run by
 * Naos itself, one file per module. The executable is built from the sources of Naos rather than
 * linked to naos::naos, so it reaches the functions that a shared Naos keeps hidden.
 */

#include <naos/naos.h>

NAOS_DECLARE_SUITE(value);
NAOS_DECLARE_SUITE(filter);
NAOS_DECLARE_SUITE(order);

NAOS_MAIN(
    NAOS_ADD_SUITE(value),
    NAOS_ADD_SUITE(filter),
    NAOS_ADD_SUITE(order)
)
