/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * The human-readable output: the lines a run writes on stdout.
 */

#ifndef NAOS_HUMAN_H
#define NAOS_HUMAN_H

#include "writer.h"

/* The writer of the human-readable output; it is always on. */
const naos_writer_t *naos_human_writer(void);

/*
 * With quiet set, the writer leaves out the tests that pass or skip, and ITERATION: a failing test
 * is written as without it, its RUN line coming just before its first failure. Set before the run.
 */
void naos_human_set_quiet(int quiet);

/* Takes the next part of a text, NUL-terminated and not NULL, borrowed for the call. */
typedef void (*naos_human_sink_t)(void *context, const char *text);

/*
 * Gives sink, part by part, the lines of a diagnostic as stdout shows them, each ending with '\n',
 * so that another writer can show them too. A part is a whole text of the test (a file name, a
 * message, a note, a trace) or a part made by Naos; nothing is flushed.
 */
void naos_human_put_diagnostic(const naos_diagnostic_record_t *record, naos_human_sink_t sink, void *context);

#endif
