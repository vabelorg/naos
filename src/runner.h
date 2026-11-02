/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * Running the planned tests, round after round.
 */

#ifndef NAOS_RUNNER_H
#define NAOS_RUNNER_H

#include "options.h"
#include "plan.h"

/*
 * Runs the tests of plan options->repeat times, and returns the exit code of all the rounds
 * together. The seed of --naos-shuffle goes up by 1 each round, from NAOS_SHUFFLE_SEED_MAX back to
 * 1; --naos-fail-fast stops them all at the first failing test.
 */
int naos_runner_run(const naos_options_t *options, naos_plan_t *plan);

#endif
