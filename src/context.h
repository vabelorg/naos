/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * The context the diagnostics of the running test are written with: the note of NAOS_NOTE and the
 * stack of NAOS_TRACE_PUSH. Only keeps them; diagnostic.c writes them and reports their misuse.
 */

#ifndef NAOS_CONTEXT_H
#define NAOS_CONTEXT_H

#include <stdarg.h>

/* Size of the note buffer: longer notes are cut short, ending in "...". */
#define NAOS_CONTEXT_NOTE_SIZE 512

/* Traces kept; deeper pushes are only counted. The string form is for the messages. */
#define NAOS_CONTEXT_TRACE_DEPTH 16
#define NAOS_CONTEXT_TRACE_DEPTH_STRING "16"

/* Size of the text of one trace: longer ones are cut short, ending in "...". */
#define NAOS_CONTEXT_TRACE_SIZE 256

typedef struct naos_trace
{
    const char *file;
    int line;
    char text[NAOS_CONTEXT_TRACE_SIZE];
} naos_trace_t;

/*
 * Starts setup and the body, or teardown, without a note or trace: the ones left by the previous
 * part would read as if they belonged to this one.
 */
void naos_context_clear(void);

/* Formats the note as vsnprintf does, replacing one not yet used; a NULL format makes it empty. */
void naos_context_set_note(const char *format, va_list args);

/* The note, or NULL when there is none. */
const char *naos_context_note(void);

/* Uses the note up. */
void naos_context_clear_note(void);

/*
 * Pushes a trace at file and line, formatted as naos_context_set_note() does. Returns 0 when the
 * stack is full: the push is then counted, so that the pops stay balanced, but not kept.
 */
int naos_context_push_trace(const char *file, int line, const char *format, va_list args);

/* Pops the innermost trace. Returns 0, changing nothing, when none is pushed. */
int naos_context_pop_trace(void);

/* Number of traces pushed and not popped, those not kept included. */
int naos_context_trace_depth(void);

/* Number of traces kept, at most NAOS_CONTEXT_TRACE_DEPTH: the outermost ones. */
int naos_context_trace_count(void);

/* The trace kept at index, from 0 (outermost) to naos_context_trace_count() - 1 (innermost). */
const naos_trace_t *naos_context_trace(int index);

#endif
