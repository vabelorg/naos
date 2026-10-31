/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * Tests and suites, a part of naos/naos.h: include that header.
 *
 * NAOS_SET_SETUP(name) and NAOS_SET_TEARDOWN(name) among the entries name two functions, defined
 * by NAOS_SETUP(name) and NAOS_TEARDOWN(name), called around each test of the suite, wherever they
 * are listed (first, preferably): when setup records a fatal failure or a skip the test body is not
 * run, teardown always runs, and failures in either count for the test. State they share with the
 * tests lives in static variables of the file.
 *
 * Mistakes in the suites (a name that is empty or holds anything but letters, digits and '_', a
 * duplicate name, a second NAOS_SET_*) are all reported before anything runs, and naos_run()
 * returns NAOS_EXIT_USAGE. The macros always give valid names.
 *
 * The names the macros define all look like naos_gen_<role>_<name> (test, setup, teardown, suite),
 * which is the name a debugger shows. Names starting with naos_gen_ are reserved. Suite names are
 * global symbols: two suites of the same name in one executable fail to link.
 */

#ifndef NAOS_SUITE_H
#define NAOS_SUITE_H

#include <naos/export.h>

NAOS_BEGIN_DECLS

/* A test, setup or teardown function. */
typedef void (*naos_func_t)(void);

/* Collects the suites while naos_run() calls the suite functions. Its contents are private. */
typedef struct naos_builder naos_builder_t;

/* A suite function, defined by NAOS_SUITE and added with NAOS_ADD_SUITE. */
typedef void (*naos_suite_func_t)(naos_builder_t *builder);

/*
 * Defines a test, setup or teardown function; list it in a suite with NAOS_ADD_TEST,
 * NAOS_SET_SETUP or NAOS_SET_TEARDOWN, given the same name. The function name gets a prefix by its
 * role, so that naming a function for the wrong role (a teardown in NAOS_SET_SETUP, a setup in
 * NAOS_ADD_TEST, ...) does not compile.
 */
#define NAOS_TEST_FUNC_(name) naos_gen_test_##name
#define NAOS_SETUP_FUNC_(name) naos_gen_setup_##name
#define NAOS_TEARDOWN_FUNC_(name) naos_gen_teardown_##name
#define NAOS_TEST(name) static void NAOS_TEST_FUNC_(name)(void)
#define NAOS_SETUP(name) static void NAOS_SETUP_FUNC_(name)(void)
#define NAOS_TEARDOWN(name) static void NAOS_TEARDOWN_FUNC_(name)(void)

/* The suite function of the suite <name>: the one name NAOS_SUITE gives it. */
#define NAOS_SUITE_FUNC_(name) naos_gen_suite_##name

/* Declares a suite defined in another file so that NAOS_ADD_SUITE can add it. */
#define NAOS_DECLARE_SUITE(name) NAOS_EXTERN_C void NAOS_SUITE_FUNC_(name)(naos_builder_t *builder)

/*
 * Defines the suite <name> from the entries given, separated by commas. It defines the function
 * NAOS_SUITE_FUNC_(name), which registers the suite and is the only thing visible from other files.
 */
#define NAOS_SUITE(name, ...) \
    NAOS_DECLARE_SUITE(name); \
    void NAOS_SUITE_FUNC_(name)(naos_builder_t *builder) \
    { \
        naos_begin_suite(builder, #name); \
        (void)(__VA_ARGS__); \
    }

/* Entries of NAOS_SUITE. Each may appear once per suite, except NAOS_ADD_TEST. */
#define NAOS_ADD_TEST(name) naos_add_test(builder, #name, NAOS_TEST_FUNC_(name))
#define NAOS_SET_SETUP(name) naos_set_setup(builder, NAOS_SETUP_FUNC_(name))
#define NAOS_SET_TEARDOWN(name) naos_set_teardown(builder, NAOS_TEARDOWN_FUNC_(name))

/*
 * Registration functions used by the macros above. A suite function calls naos_begin_suite first.
 * The names must outlive naos_run().
 */
NAOS_API void naos_begin_suite(naos_builder_t *builder, const char *name);
NAOS_API void naos_add_test(naos_builder_t *builder, const char *name, naos_func_t func);
NAOS_API void naos_set_setup(naos_builder_t *builder, naos_func_t func);
NAOS_API void naos_set_teardown(naos_builder_t *builder, naos_func_t func);

NAOS_END_DECLS

#endif
