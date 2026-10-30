/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * The diagnostics of the running test, and the outcome they add up to.
 *
 * A diagnostic is written in three steps: naos_diagnostic_begin() counts it and writes its first
 * line, naos_diagnostic_add*() collect its values, and naos_diagnostic_end() writes them with the
 * note and the traces of context.h. One diagnostic is open at a time.
 */

#ifndef NAOS_DIAGNOSTIC_H
#define NAOS_DIAGNOSTIC_H

#include "report.h"

/* Starts the outcome of a new test: no failure and no skip. */
void naos_diagnostic_start_test(void);

/* Outcome of the running test so far: FAIL after any failure, else SKIP after a skip, else PASS. */
naos_outcome_t naos_diagnostic_outcome(void);

/* Number of failures of the running test so far. */
int naos_diagnostic_failure_count(void);

/*
 * Opens a diagnostic: counts the failure of an error or records the skip, and writes the first
 * line. kind is NAOS_REPORT_ERROR or NAOS_REPORT_SKIP; text is not NULL.
 */
void naos_diagnostic_begin(naos_report_kind_t kind, const char *file, int line, const char *text);

/*
 * Adds a detail line to the open diagnostic; one beyond the eighth is dropped. label is the bare
 * label, without indent, colon or padding. label and value are borrowed until
 * naos_diagnostic_end() returns; value is written as it is.
 */
void naos_diagnostic_add(const char *label, const char *value);

/*
 * As naos_diagnostic_add(), for a C string of the test (possibly NULL), written as (null) or as
 * "value" on one line, with \n, \t, \r, \", \\ and \xNN for the other bytes that are not printable ASCII.
 */
void naos_diagnostic_add_string(const char *label, const char *value);

/*
 * Closes the open diagnostic: writes its details, then the note and the count of the traces not
 * kept as details, then the traces as note lines, innermost first. Uses the note up and flushes
 * stdout.
 */
void naos_diagnostic_end(void);

/* Ends an assertion that passed: its note is used up without being written. */
void naos_diagnostic_pass(void);

/*
 * Writes a misuse of NAOS_TRACE_PUSH or NAOS_TRACE_POP as a failure of one line, without the note,
 * which is left for the next assertion, or the traces.
 */
void naos_diagnostic_misuse(const char *file, int line, const char *message);

/*
 * Called when setup and the body, or teardown, end without a jump: a trace still pushed lacks its
 * NAOS_TRACE_POP. The failure points at the innermost trace kept and names its text; the traces
 * outside it follow as note lines.
 */
void naos_diagnostic_check_traces_popped(void);

#endif
