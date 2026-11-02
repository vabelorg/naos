/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * Failing assertions, one file and suite per kind, whose output expected/ holds: the error line
 * with the expression as written, then the values as detail lines. Every test fails on purpose.
 */

#include <naos/naos.h>

NAOS_DECLARE_SUITE(boolean);
NAOS_DECLARE_SUITE(integer);
NAOS_DECLARE_SUITE(unsigned_integer);
NAOS_DECLARE_SUITE(floating);
NAOS_DECLARE_SUITE(string);
NAOS_DECLARE_SUITE(memory);
NAOS_DECLARE_SUITE(pointer);
NAOS_DECLARE_SUITE(comparison);

NAOS_MAIN(
    NAOS_ADD_SUITE(boolean),
    NAOS_ADD_SUITE(integer),
    NAOS_ADD_SUITE(unsigned_integer),
    NAOS_ADD_SUITE(floating),
    NAOS_ADD_SUITE(string),
    NAOS_ADD_SUITE(memory),
    NAOS_ADD_SUITE(pointer),
    NAOS_ADD_SUITE(comparison)
)
