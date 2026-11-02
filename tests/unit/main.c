/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * Unit tests of the internal modules of Naos, run by Naos itself, one file per module: the ones whose
 * results depend only on their arguments, and the writers driven through their events. The executable is built from the sources of Naos rather than
 * linked to Naos::naos, so it reaches the functions that a shared Naos keeps hidden.
 */

#include <naos/naos.h>

NAOS_DECLARE_SUITE(value);
NAOS_DECLARE_SUITE(filter);
NAOS_DECLARE_SUITE(order);
NAOS_DECLARE_SUITE(text);
NAOS_DECLARE_SUITE(xml);
NAOS_DECLARE_SUITE(human);

NAOS_MAIN(
    NAOS_ADD_SUITE(value),
    NAOS_ADD_SUITE(filter),
    NAOS_ADD_SUITE(order),
    NAOS_ADD_SUITE(text),
    NAOS_ADD_SUITE(xml),
    NAOS_ADD_SUITE(human)
)
