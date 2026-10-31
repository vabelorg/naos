/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * Running or listing the selected tests, once the suites are collected.
 */

#ifndef NAOS_RUNNER_H
#define NAOS_RUNNER_H

#include "options.h"
#include "suite_internal.h"

/*
 * Runs or lists, as options say, the tests of suites that naos_suite_collect() accepted, and returns
 * the exit code.
 */
int naos_runner_run(const naos_options_t *options, const naos_suite_t *suites, size_t count);

#endif
