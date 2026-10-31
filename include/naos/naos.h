/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * Naos: C99 test harness.
 *
 * Tests are grouped in suites. A suite lists the tests of one source file; a file may hold several
 * suites, but a suite never spans files. A test is named <suite>.<test>.
 *
 * The executable runs every test in one process, in the order the suites and tests are added:
 *     prog                               runs all tests
 *     prog --naos-filter <suite>.<test>  runs only that test
 *     prog --naos-filter <patterns>      runs only the tests whose name matches
 *     prog --naos-list                   prints one test name per line
 *     prog --naos-shuffle                runs the suites, and the tests of each suite, in a random order
 *     prog --naos-random-seed <n>        seed of that order (1 to 99999; 0 or none: from the time)
 *     prog --naos-repeat <n>             runs the tests n times
 *     prog --naos-fail-fast              stops at the first failing test
 *     prog --help                        prints the usage
 * A filter pattern matches the whole name <suite>.<test>: '*' matches any string, including an
 * empty one or a dot, and '?' any one character. Patterns are separated by ':', and those after the
 * first '-' exclude tests: "parser.*:writer.*-*.slow_*". Without positive patterns
 * ("-*.slow_*") every test not excluded is selected. A filter selecting no test is a usage error.
 * Each test prints RUN when it starts, then PASS, FAIL or SKIP; with several tests a SUMMARY line
 * follows. The exit code reports the result (NAOS_EXIT_*): FAIL if any test failed, SKIP if every
 * selected test was skipped, PASS otherwise. The options may appear anywhere; naos_init() takes
 * them out of argv and leaves the other arguments in argv for the program.
 * Tests share the process, so static state is not reset between them; --naos-shuffle brings out a
 * test that passes only after another one. It prints "RANDOM SEED: <n>", and --naos-shuffle
 * --naos-random-seed <n> runs the same order again; --naos-repeat adds 1 to the seed each time.
 * A caller that needs crash isolation, parallelism or timeouts runs one test per process with
 * --naos-list and --naos-filter; those and reports are left to the caller.
 *
 * Usage:
 *     parser_test.c:
 *         NAOS_TEST(empty_object)
 *         {
 *             NAOS_REQUIRE_NOT_NULL(ptr);
 *             NAOS_CHECK_INT_EQ(0, value);
 *         }
 *
 *         NAOS_SUITE(parser,
 *             NAOS_ADD_TEST(empty_object)
 *         )
 *
 *     main.c:
 *         NAOS_DECLARE_SUITE(parser);
 *
 *         NAOS_MAIN(
 *             NAOS_ADD_SUITE(parser)
 *         )
 *
 * NAOS_SUITE defines a function that registers the suite, as NAOS_TEST and NAOS_MAIN define
 * functions, so none of the three is followed by a semicolon; NAOS_DECLARE_SUITE declares one and
 * is. The entries of a suite are separated by commas, with none after the last one. The suite
 * comes after the tests it lists, usually at the end of the file. NAOS_DECLARE_SUITE is needed only
 * for a suite defined in another file.
 *
 * This header only includes the parts, each in a header of its own: naos/session.h adds and runs
 * the suites (main()), naos/exit.h has the exit codes, naos/suite.h defines the tests and suites,
 * naos/check.h has the assertions, naos/control.h the running test's notes, traces, failures,
 * skips and stop and naos/version.h the version. Include this header.
 *
 * Public API (kept compatible within a minor version before 1.0 and within a major version from 1.0
 * on, also as a shared library):
 * - the macros documented in naos/session.h, naos/exit.h, naos/suite.h, naos/check.h,
 *   naos/control.h and naos/version.h
 * - the types naos_func_t, naos_suite_func_t and naos_builder_t (opaque)
 * - the functions naos_init, naos_run and those of naos/version.h
 * - the output format described in docs/reference/output.md
 * The functions, types and helper macros (ending with '_') the macros use (naos_check*, naos_cmp_t,
 * naos_note, naos_trace_*, naos_fail, naos_skip, naos_stop, naos_begin_suite, naos_add_test,
 * naos_set_* and naos_add_suite) are not meant to be used directly, but they are public too and
 * kept compatible in the same way.
 * Before 1.0 a new minor version may change the macros, the functions they call (signatures and
 * ABI) and the output format incompatibly, without keeping the old functions.
 *
 * Not thread-safe.
 */

#ifndef NAOS_NAOS_H
#define NAOS_NAOS_H

#include <naos/check.h>
#include <naos/control.h>
#include <naos/exit.h>
#include <naos/session.h>
#include <naos/suite.h>
#include <naos/version.h>

#endif
