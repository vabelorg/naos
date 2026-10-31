/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * The main() side: adding the suites and running them, a part of naos/naos.h: include that header.
 *
 * NAOS_MAIN defines main(), which adds the suites with NAOS_ADD_SUITE and runs them. A program that
 * needs its own main() (to parse its own arguments, for example) adds the suites one at a time, as
 * a suite adds its tests, and may pick them from the arguments:
 *     int main(int argc, char **argv)
 *     {
 *         naos_init(&argc, argv);
 *         ... parse the arguments left in argv ...
 *         NAOS_ADD_SUITE(parser);
 *         if (slow)
 *         {
 *             NAOS_ADD_SUITE(parser_slow);
 *         }
 *         return naos_run();
 *     }
 * The order is naos_init() once, NAOS_ADD_SUITE any number of times, naos_run() once. A call out of
 * order prints a message; naos_run() then returns NAOS_EXIT_USAGE, and naos_init() and NAOS_ADD_SUITE
 * exit with it (NAOS_ADD_SUITE in a test or a suite function too, as naos_run() has started).
 */

#ifndef NAOS_SESSION_H
#define NAOS_SESSION_H

#include <naos/exit.h>
#include <naos/export.h>
#include <naos/suite.h>

NAOS_BEGIN_DECLS

/* Adds a suite, defined by NAOS_SUITE, for naos_run() to run, after those added before it. */
#define NAOS_ADD_SUITE(name) naos_add_suite(NAOS_SUITE_FUNC_(name))

/* Defines main() running the suites given, as NAOS_ADD_SUITE entries. Use once per executable. */
#define NAOS_MAIN(...) \
    int main(int argc, char **argv) \
    { \
        naos_init(&argc, argv); \
        (void)(__VA_ARGS__); \
        return naos_run(); \
    }

/*
 * Takes the Naos options (--naos-*) out of argv wherever they are, shifting the other arguments
 * down in order and decrementing *argc. --help is left in argv so that the program can print its
 * own help too. Naos prints its help and its usage errors here; naos_run() then runs no test, and
 * returns NAOS_EXIT_USAGE after a usage error or a mistake in the suites, NAOS_EXIT_PASS after the
 * help otherwise.
 */
NAOS_API void naos_init(int *argc, char **argv);

/*
 * Calls every suite function added with NAOS_ADD_SUITE once, in order, to collect the suites, reports
 * any mistake in them, then runs the tests selected by naos_init(). Returns the process exit code
 * (NAOS_EXIT_*); when the suites cannot be added or collected for lack of memory, it prints one line
 * and returns NAOS_EXIT_FAIL without running.
 */
NAOS_API int naos_run(void);

/* Used by NAOS_ADD_SUITE: only keeps the suite function, which naos_run() calls. */
NAOS_API void naos_add_suite(naos_suite_func_t suite);

NAOS_END_DECLS

#endif
