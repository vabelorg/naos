/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * How a test goes on or stops, one file and suite per way: NAOS_CHECK_* records and goes on,
 * NAOS_REQUIRE_*, NAOS_FAIL and NAOS_SKIP leave the test from any depth, and the result line and
 * exit code follow. A test prints "reached-..." after a point it must not pass, so its expected
 * output shows whether it got there.
 */

#include <naos/naos.h>

NAOS_DECLARE_SUITE(pass);
NAOS_DECLARE_SUITE(check);
NAOS_DECLARE_SUITE(require);
NAOS_DECLARE_SUITE(fail);
NAOS_DECLARE_SUITE(skip);
NAOS_DECLARE_SUITE(skips);
NAOS_DECLARE_SUITE(last);
NAOS_DECLARE_SUITE(crash);

NAOS_MAIN(
    NAOS_ADD_SUITE(pass),
    NAOS_ADD_SUITE(check),
    NAOS_ADD_SUITE(require),
    NAOS_ADD_SUITE(fail),
    NAOS_ADD_SUITE(skip),
    NAOS_ADD_SUITE(skips),
    NAOS_ADD_SUITE(last),
    NAOS_ADD_SUITE(crash)
)
