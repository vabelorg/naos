/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * The diagnostics of the running test, what goes with them (the note of NAOS_NOTE and the stack of
 * NAOS_TRACE_PUSH), and the outcome they add up to.
 *
 * A diagnostic is told in three steps: naos_diagnostic_begin() counts it, naos_diagnostic_add*()
 * collect its values, and naos_diagnostic_end() tells all of it to the writers with the note and the
 * traces. One diagnostic is open at a time.
 */

#ifndef NAOS_DIAGNOSTIC_H
#define NAOS_DIAGNOSTIC_H

#include <stdarg.h>

#include "report.h"

/* Traces kept; deeper pushes are only counted. The string form is for the messages. */
#define NAOS_DIAGNOSTIC_TRACE_DEPTH 16
#define NAOS_DIAGNOSTIC_TRACE_DEPTH_STRING "16"

/* Starts the outcome of a new test: no failure, no skip and no assertion. */
void naos_diagnostic_start_test(void);

/* Outcome of the running test so far: FAIL after any failure, else SKIP after a skip, else PASS. */
naos_outcome_t naos_diagnostic_outcome(void);

/* Number of failures of the running test so far. */
int naos_diagnostic_failure_count(void);

/*
 * Opens a diagnostic: counts the failure of an error or records the skip, and counts an assertion
 * run. kind is NAOS_DIAGNOSTIC_ERROR or NAOS_DIAGNOSTIC_SKIP; text is not NULL; file is NULL only
 * for a failure without a location. file and text are borrowed until naos_diagnostic_end() returns.
 */
void naos_diagnostic_begin(naos_diagnostic_kind_t kind, const char *file, int line, const char *text);

/*
 * Adds a value to the open diagnostic; one beyond the eighth is dropped. label is the bare
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
 * Closes the open diagnostic: tells it to the writers with its values, the note, the count of the
 * traces not kept and the traces, innermost first. Uses the note up.
 */
void naos_diagnostic_end(void);

/* Ends an assertion that passed: counts it, and its note is used up without being written. */
void naos_diagnostic_pass(void);

/*
 * Called when the test has ended, teardown included: a test that would pass but ran no assertion
 * fails, with a diagnostic without a location, note or traces, and with a hint as its one value.
 * Every diagnostic, and every assertion that passed, counts as an assertion run.
 */
void naos_diagnostic_check_assertion_ran(void);

/*
 * Called when a test expected to fail has ended, teardown included, without a failure or a skip: it
 * fails with a diagnostic at file and line, where the test is marked, without note or traces, and
 * with reason as its one value. file is NULL for no location.
 */
void naos_diagnostic_unexpected_pass(const char *file, int line, const char *reason);

/*
 * Tells a misuse of NAOS_TRACE_PUSH or NAOS_TRACE_POP as a failure without values, the note, which
 * is left for the next assertion, or the traces.
 */
void naos_diagnostic_misuse(const char *file, int line, const char *message);

/*
 * Called when setup and the body, or teardown, end without a jump: a trace still pushed lacks its
 * NAOS_TRACE_POP. The failure points at the innermost trace kept and names its text; the traces
 * outside it go with it.
 */
void naos_diagnostic_check_traces_popped(void);

/*
 * Starts setup and the body, or teardown, without a note or trace: the ones left by the previous
 * part would read as if they belonged to this one.
 */
void naos_diagnostic_clear_context(void);

/* Formats the note as vsnprintf does, replacing one not yet used; a NULL format makes it empty. */
void naos_diagnostic_set_note(const char *format, va_list args);

/*
 * Pushes a trace at file and line, formatted as naos_diagnostic_set_note() does. Returns 0 when the
 * stack is full: the push is then counted, so that the pops stay balanced, but not kept.
 */
int naos_diagnostic_push_trace(const char *file, int line, const char *format, va_list args);

/* Pops the innermost trace. Returns 0, changing nothing, when none is pushed. */
int naos_diagnostic_pop_trace(void);

#endif
