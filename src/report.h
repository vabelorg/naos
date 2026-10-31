/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * Everything a run writes: each line of the output format on stdout, and the messages of Naos
 * itself ("naos: ...") on stderr. The other modules decide what to write and when; only this one
 * knows how a line looks. The help and the usage of the command line are left to options.c.
 *
 * Nothing on stdout is flushed here unless said so; naos_report_flush() does it.
 */

#ifndef NAOS_REPORT_H
#define NAOS_REPORT_H

#include <stdarg.h>
#include <stddef.h>

#include <naos/export.h>

/* Outcome of one test, as its result line names it. */
typedef enum naos_outcome
{
    NAOS_OUTCOME_PASS, /* "PASS" */
    NAOS_OUTCOME_FAIL, /* "FAIL", with the number of failures */
    NAOS_OUTCOME_SKIP  /* "SKIP" */
} naos_outcome_t;

/* Kind of a diagnostic line. */
typedef enum naos_report_kind
{
    NAOS_REPORT_ERROR, /* "error" */
    NAOS_REPORT_SKIP,  /* "skip" */
    NAOS_REPORT_NOTE   /* "note" */
} naos_report_kind_t;

/* How the value of a detail line is written. */
typedef enum naos_detail_kind
{
    NAOS_DETAIL_TEXT,  /* value is written as it is */
    NAOS_DETAIL_STRING /* value is a C string of the test, possibly NULL: (null) or "value", escaped */
} naos_detail_kind_t;

/* One detail line; label and value are borrowed, and only read while naos_report_details() runs. */
typedef struct naos_detail
{
    const char *label;
    naos_detail_kind_t kind;
    const char *value;
} naos_detail_t;

/* Writes "RUN: <suite>.<test>" and flushes stdout. */
void naos_report_test_start(const char *suite, const char *test);

/*
 * Writes the result line of a test, "PASS: <suite>.<test>", "FAIL: <suite>.<test> (<n> failure[s])"
 * or "SKIP: <suite>.<test>", and flushes stdout. failure_count is used by NAOS_OUTCOME_FAIL only.
 */
void naos_report_test_result(const char *suite, const char *test, naos_outcome_t outcome, int failure_count);

/*
 * Writes "<location> <kind>: <text>". file and text are written as they are; text is not NULL. The
 * location takes the form of the diagnostics of the compiler Naos is built with.
 */
void naos_report_diagnostic(naos_report_kind_t kind, const char *file, int line, const char *text);

/*
 * Writes details[0, count) as "  <label>: <value>", one line each; "left" and "left <rest>" get one
 * more space when "right" or "right <rest>" is in the block, so the two values line up. Every label
 * is a bare label (no indent, colon or padding) and not NULL, and neither is the value of a
 * NAOS_DETAIL_TEXT.
 */
void naos_report_details(const naos_detail_t *details, size_t count);

/* Writes the name of a test as --naos-list prints it: "<suite>.<test>". */
void naos_report_test_name(const char *suite, const char *test);

/* The lines around the rounds of --naos-repeat and --naos-shuffle; see runner.c for when. */
void naos_report_iteration(long round, long repeat);
void naos_report_random_seed(unsigned long seed);
void naos_report_fail_fast(size_t not_run);
void naos_report_summary(size_t passed, size_t failed, size_t skipped);

/* "ITERATION FAILED: <round> of <repeat>", followed by " (random seed <seed>)" when shuffled. */
void naos_report_iteration_failed(long round, long repeat, int shuffled, unsigned long seed);

/* "REPEAT: <failed> of <repeat> iterations failed", followed by ", <not_run> not run" when not 0. */
void naos_report_repeat(long failed_rounds, long repeat, long not_run);

void naos_report_flush(void);

/*
 * Writes a message of Naos itself to stderr as "naos: <message>\n", formatted as printf does, after
 * flushing stdout so that it comes after what the tests have written; then flushes stderr.
 */
void naos_report_error(const char *format, ...) NAOS_PRINTF_FORMAT(1, 2);
void naos_report_error_v(const char *format, va_list args);

#endif
