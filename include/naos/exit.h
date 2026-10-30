/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * Exit codes of a test executable, a part of naos/naos.h: include that header.
 *
 * naos_run() returns one of them for main() to return: FAIL if any test failed, SKIP if every
 * selected test was skipped, PASS otherwise, and USAGE for a mistake in the arguments or the suites.
 * The CMake functions register the tests with SKIP_RETURN_CODE 77.
 */

#ifndef NAOS_EXIT_H
#define NAOS_EXIT_H

#define NAOS_EXIT_PASS 0
#define NAOS_EXIT_FAIL 1
#define NAOS_EXIT_USAGE 2
#define NAOS_EXIT_SKIP 77

#endif
