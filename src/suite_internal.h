/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * The suites as registered, for the other sources of Naos.
 */

#ifndef NAOS_SUITE_INTERNAL_H
#define NAOS_SUITE_INTERNAL_H

#include <naos/suite.h>

#include <stddef.h>

typedef struct naos_test
{
    const char *name;
    naos_func_t func;
} naos_test_t;

typedef struct naos_suite
{
    const char *name;
    naos_test_t *tests;
    size_t count;
    size_t capacity;
    naos_func_t setup;    /* called before each test, or NULL */
    naos_func_t teardown; /* called after each test, or NULL */
} naos_suite_t;

/*
 * The suites registered by the suite functions.
 */
struct naos_builder
{
    naos_suite_t *suites;  /* one per NAOS_ADD_SUITE, in registration order */
    size_t count;          /* suite functions reached so far; the one being called is count - 1 */
    naos_suite_t *current; /* suite being registered, NULL until naos_begin_suite */
    size_t test_calls;     /* naos_add_test calls of the current suite, accepted or not */
    size_t error_count;
};

/* Keeps a suite function of NAOS_ADD_SUITE, NULL included, for naos_suite_collect(). */
void naos_suite_keep(naos_suite_func_t suite);

/*
 * Calls every suite function kept once and checks what they registered. Returns NAOS_EXIT_PASS when
 * the suites can run, NAOS_EXIT_USAGE once every mistake is reported, or NAOS_EXIT_FAIL for a lack
 * of memory. naos_suite_release() is needed afterwards in every case.
 */
int naos_suite_collect(naos_builder_t *builder);

/* Releases the builder and the suite functions kept. */
void naos_suite_release(naos_builder_t *builder);

#endif
