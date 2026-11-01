/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * Everything a run writes. The events of a run go to every writer on (writer.h): the human-readable
 * output on stdout always, and the file of --naos-output when given. The names of --naos-list on stdout and the messages of Naos itself
 * ("naos: ...") on stderr are written here. The other modules decide what to tell and when; only the
 * writers know how it looks. The help and the usage of the command line are left to options.c.
 */

#ifndef NAOS_REPORT_H
#define NAOS_REPORT_H

#include <stdarg.h>
#include <stddef.h>

#include <naos/export.h>

#include "writer.h"

/* The formats --naos-output writes besides the human-readable output. */
typedef enum naos_format
{
    NAOS_FORMAT_NONE,
    NAOS_FORMAT_XML /* "xml": JUnit XML */
} naos_format_t;

/*
 * Starts the run: turns on the writer of format, if any, to write to path, and every writer on gets
 * ready to write. program is argv[0], possibly NULL; path lives until the process ends. Returns 0
 * after reporting why a writer cannot write, and then no test may run.
 */
int naos_report_begin(const char *program, naos_format_t format, const char *path);

/* The events of the run, told to every writer on; see naos_writer_t for what each means. */
void naos_report_round_begin(const naos_round_t *round);
void naos_report_test_start(const char *suite, const char *test);
void naos_report_diagnostic(const naos_diagnostic_record_t *record);
void naos_report_test_result(const char *suite, const char *test, naos_outcome_t outcome, int failure_count);
void naos_report_round_end(const naos_round_t *round, size_t selected, const naos_tally_t *tally, size_t not_run);

/* Ends the run. Returns 0 when a writer could not write all of it, after reporting why. */
int naos_report_end(long failed_rounds, long repeat, long rounds_not_run);

/* Writes the name of a test as --naos-list prints it: "<suite>.<test>". */
void naos_report_test_name(const char *suite, const char *test);

void naos_report_flush(void);

/*
 * Writes a message of Naos itself to stderr as "naos: <message>\n", formatted as printf does, after
 * flushing stdout so that it comes after what the tests have written; then flushes stderr.
 */
void naos_report_error(const char *format, ...) NAOS_PRINTF_FORMAT(1, 2);
void naos_report_error_v(const char *format, va_list args);

#endif
