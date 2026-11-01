/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * What a run tells its writers, and the functions a writer has. report.c tells every writer the
 * same events in the same order; each writer decides what it writes of them, and where.
 */

#ifndef NAOS_WRITER_H
#define NAOS_WRITER_H

#include <stddef.h>

#include "context.h"

/* Outcome of one test. */
typedef enum naos_outcome
{
    NAOS_OUTCOME_PASS,
    NAOS_OUTCOME_FAIL, /* with the number of failures */
    NAOS_OUTCOME_SKIP
} naos_outcome_t;

/* Kind of a diagnostic. */
typedef enum naos_report_kind
{
    NAOS_REPORT_ERROR, /* "error" */
    NAOS_REPORT_SKIP,  /* "skip" */
    NAOS_REPORT_NOTE   /* "note": a trace, in the human-readable output only */
} naos_report_kind_t;

/* How the value of a detail is written. */
typedef enum naos_detail_kind
{
    NAOS_DETAIL_TEXT,  /* value is written as it is */
    NAOS_DETAIL_STRING /* value is a C string of the test, possibly NULL: (null) or "value", escaped */
} naos_detail_kind_t;

/* One value of a diagnostic, such as "left"; label and value are borrowed for the event only. */
typedef struct naos_detail
{
    const char *label;
    naos_detail_kind_t kind;
    const char *value;
} naos_detail_t;

/* One failure or skip, with what goes with it; everything is borrowed for the event only. */
typedef struct naos_diagnostic_record
{
    naos_report_kind_t kind; /* NAOS_REPORT_ERROR or NAOS_REPORT_SKIP */
    const char *file;
    int line;
    const char *text; /* the expression or the message; not NULL */
    const naos_detail_t *details;
    size_t detail_count;
    const char *note; /* the note of NAOS_NOTE, or NULL */
    int traces_not_kept; /* traces pushed beyond those kept, 0 when every one is kept */
    const naos_trace_t *const *traces; /* innermost first */
    int trace_count;
} naos_diagnostic_record_t;

/* Numbers of tests by outcome. */
typedef struct naos_tally
{
    size_t passed;
    size_t failed;
    size_t skipped;
} naos_tally_t;

/* One round of the run: the iteration of --naos-repeat, and the seed of --naos-shuffle. */
typedef struct naos_round
{
    long index; /* from 1 */
    long repeat;
    int shuffled;
    unsigned long seed; /* used only when shuffled */
} naos_round_t;

/*
 * The functions of a writer, called in this order: begin, then for each round round_begin, each test
 * (test_begin, diagnostic for each of its failures and skips, test_end) and round_end, then end. The
 * names of suites and tests live until the process ends; everything else is borrowed for the call.
 */
typedef struct naos_writer
{
    /* program is argv[0], possibly NULL; path is where to write, or NULL. Returns 0 when it cannot write there. */
    int (*begin)(const char *program, const char *path);
    void (*round_begin)(const naos_round_t *round);
    void (*test_begin)(const char *suite, const char *test);
    void (*diagnostic)(const naos_diagnostic_record_t *record);
    void (*test_end)(const char *suite, const char *test, naos_outcome_t outcome, int failure_count);

    /* not_run is how many tests --naos-fail-fast left out, this round and later. */
    void (*round_end)(const naos_round_t *round, const naos_tally_t *tally, size_t not_run);

    /* rounds_not_run is how many rounds --naos-fail-fast left out. Returns 0 when it could not write all of it. */
    int (*end)(long failed_rounds, long repeat, long rounds_not_run);
} naos_writer_t;

#endif
