/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * The clock that times the tests. C99 has no portable wall clock finer than a second, so clock.c
 * uses the monotonic clock of the operating system, and is the only file of src/ that depends on
 * the platform. Uses no other Naos module.
 */

#ifndef NAOS_CLOCK_H
#define NAOS_CLOCK_H

/* A time in nanoseconds. */
typedef unsigned long long naos_nanoseconds_t;

/*
 * Now, from a start that only the clock knows: only the difference of two calls means something.
 * The clock never goes back, so the difference of a later call and an earlier one is never negative.
 */
naos_nanoseconds_t naos_clock_now(void);

/*
 * What the clock measures, for the help: "wall-clock time, from a monotonic clock", or with no such
 * clock "processor time, from clock(), not wall-clock time".
 */
const char *naos_clock_description(void);

#endif
