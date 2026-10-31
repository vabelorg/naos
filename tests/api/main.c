/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * Tests of the public API that must pass, in C, one file per public header; cpp.cpp is the same in
 * C++. Each test is a CTest test of its own (naos_add_executable), and the run as a whole must pass
 * in any order and any number of times, since the tests share the process.
 */

#include <naos/naos.h>

NAOS_DECLARE_SUITE(check);
NAOS_DECLARE_SUITE(control);
NAOS_DECLARE_SUITE(fixture);
NAOS_DECLARE_SUITE(version);

NAOS_MAIN(
    NAOS_ADD_SUITE(check),
    NAOS_ADD_SUITE(control),
    NAOS_ADD_SUITE(fixture),
    NAOS_ADD_SUITE(version)
)
