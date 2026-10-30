/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * Running one test: its parts, and the jump out of them that stops it.
 */

#ifndef NAOS_TEST_H
#define NAOS_TEST_H

#include <naos/export.h>

#include "report.h"
#include "suite_internal.h"

/*
 * Runs one test with its outcome cleared, and returns it. Other static state, of Naos or of the
 * tests, carries over from the previous test. A fatal failure or a skip in setup leaves the body
 * out, and teardown always runs. Writes "RUN: <suite>.<test>" before setup, and the result line
 * after teardown.
 */
naos_outcome_t naos_test_run(const naos_suite_t *suite, const naos_test_t *test);

/*
 * Leaves setup and the body, or teardown, of the running test through longjmp. Outside a running
 * test there is nowhere to go: it prints a message and exits with NAOS_EXIT_USAGE.
 */
NAOS_NORETURN void naos_test_leave(void);

#endif
