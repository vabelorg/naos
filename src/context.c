/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * The note of NAOS_NOTE and the stack of NAOS_TRACE_PUSH (context.h). Only keeps them; prints
 * nothing.
 */

#include "context.h"

#include <stdio.h>
#include <string.h>

/*
 * Notes of NAOS_TRACE_PUSH, outermost first. Pushes beyond NAOS_CONTEXT_TRACE_DEPTH are counted in
 * depth, so that the pops stay balanced, but not kept.
 */
typedef struct naos_trace_stack
{
    int depth;
    naos_trace_t items[NAOS_CONTEXT_TRACE_DEPTH];
} naos_trace_stack_t;

/* Note of NAOS_NOTE for the next assertion; empty when there is none. */
static char g_note[NAOS_CONTEXT_NOTE_SIZE] = "";

static naos_trace_stack_t g_traces = { 0 };

/* Formats into buffer as vsnprintf does; a NULL format or an error leaves it empty. */
static void format_into(char *buffer, size_t size, const char *format, va_list args)
{
    static const char ellipsis[] = "...";
    int length;

    if (format == NULL)
    {
        buffer[0] = '\0';
        return;
    }

    length = vsnprintf(buffer, size, format, args);
    if (length < 0)
    {
        buffer[0] = '\0';
    }
    else if ((size_t)length >= size)
    {
        /* Cut short: the end shows it. */
        memcpy(&buffer[size - sizeof(ellipsis)], ellipsis, sizeof(ellipsis));
    }
}

void naos_context_clear(void)
{
    g_note[0] = '\0';
    g_traces.depth = 0;
}

void naos_context_set_note(const char *format, va_list args)
{
    format_into(g_note, sizeof(g_note), format, args);
}

const char *naos_context_note(void)
{
    return (g_note[0] != '\0') ? g_note : NULL;
}

void naos_context_clear_note(void)
{
    g_note[0] = '\0';
}

int naos_context_push_trace(const char *file, int line, const char *format, va_list args)
{
    naos_trace_t *trace;

    if (g_traces.depth >= NAOS_CONTEXT_TRACE_DEPTH)
    {
        g_traces.depth++;
        return 0;
    }

    trace = &g_traces.items[g_traces.depth++];
    trace->file = file;
    trace->line = line;
    format_into(trace->text, sizeof(trace->text), format, args);

    return 1;
}

int naos_context_pop_trace(void)
{
    if (g_traces.depth == 0)
    {
        return 0;
    }

    g_traces.depth--;

    return 1;
}

int naos_context_trace_depth(void)
{
    return g_traces.depth;
}

int naos_context_trace_count(void)
{
    return (g_traces.depth < NAOS_CONTEXT_TRACE_DEPTH) ? g_traces.depth : NAOS_CONTEXT_TRACE_DEPTH;
}

const naos_trace_t *naos_context_trace(int index)
{
    return &g_traces.items[index];
}
