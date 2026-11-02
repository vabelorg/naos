/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * The events of a run, told to every writer on (writer.h): the human-readable output on stdout
 * always, and the file of --naos-output when given. The other modules decide what to tell and when;
 * only the writers know how it looks. The names of --naos-list are left to plan.h, the messages of
 * Naos itself to message.h, and the help and the usage of the command line to options.c.
 */

#ifndef NAOS_REPORT_H
#define NAOS_REPORT_H

#include <stddef.h>

#include "writer.h"

/* The formats --naos-output writes besides the human-readable output. */
typedef enum naos_output_format
{
    NAOS_OUTPUT_NONE,
    NAOS_OUTPUT_XML /* "xml": JUnit XML */
} naos_output_format_t;

/*
 * Starts the run: turns on the writer of format, if any, to write to path, and every writer on gets
 * ready to write; with quiet, stdout leaves out the tests that pass or skip. program is argv[0],
 * possibly NULL; path lives until the process ends. Returns 0 after reporting why a writer cannot
 * write, and then no test may run.
 */
int naos_report_begin(const char *program, naos_output_format_t format, const char *path, int quiet);

/* The events of the run, told to every writer on; see naos_writer_t for what each means. */
void naos_report_round_begin(const naos_round_t *round);
void naos_report_test_start(const char *suite, const char *test, const char *xfail_reason);
void naos_report_diagnostic(const naos_diagnostic_record_t *record);
void naos_report_test_result(const char *suite, const char *test, naos_outcome_t outcome, int failure_count, naos_nanoseconds_t elapsed);
void naos_report_round_end(const naos_round_t *round, const naos_tally_t *tally, size_t not_run);

/* Ends the run. Returns 0 when a writer could not write all of it, after reporting why. */
int naos_report_end(long failed_rounds, long repeat, long rounds_not_run);

void naos_report_flush(void);

#endif
