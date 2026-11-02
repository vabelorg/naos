/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * The diagnostics of the running test, one at a time (begin, details, end), the note and the traces
 * that go with them, and the outcome they add up to (diagnostic.h).
 */

#include "diagnostic.h"

#include <string.h>

#include "format.h"

/* Most values of one diagnostic. */
#define NAOS_DETAIL_COUNT 8

/* Size of the note buffer: longer notes are cut short, ending in "...". */
#define NAOS_NOTE_SIZE 512

/* Which of the note and the traces a diagnostic is closed with. */
typedef enum naos_context_use
{
    NAOS_CONTEXT_ALL,   /* a failure, NAOS_FAIL or NAOS_SKIP */
    NAOS_CONTEXT_OUTER, /* NAOS_TRACE_PUSH without NAOS_TRACE_POP: its text names the innermost trace */
    NAOS_CONTEXT_NONE   /* NAOS_TRACE_POP without NAOS_TRACE_PUSH, or a push too deep: the note is kept */
} naos_context_use_t;

/*
 * Notes of NAOS_TRACE_PUSH, outermost first. Pushes beyond NAOS_DIAGNOSTIC_TRACE_DEPTH are counted in
 * depth, so that the pops stay balanced, but not kept.
 */
typedef struct naos_trace_stack
{
    int depth;
    naos_trace_t items[NAOS_DIAGNOSTIC_TRACE_DEPTH];
} naos_trace_stack_t;

/* Note of NAOS_NOTE for the next assertion; empty when there is none. */
static char g_note[NAOS_NOTE_SIZE] = "";

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

    length = naos_vsnprintf(buffer, size, format, args);
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

/* The note, or NULL when there is none. */
static const char *current_note(void)
{
    return (g_note[0] != '\0') ? g_note : NULL;
}

/* Uses the note up. */
static void clear_note(void)
{
    g_note[0] = '\0';
}

/* Number of traces kept, at most NAOS_DIAGNOSTIC_TRACE_DEPTH: the outermost ones. */
static int kept_trace_count(void)
{
    return (g_traces.depth < NAOS_DIAGNOSTIC_TRACE_DEPTH) ? g_traces.depth : NAOS_DIAGNOSTIC_TRACE_DEPTH;
}

/* What the diagnostics of the running test add up to, cleared by naos_diagnostic_start_test(). */
typedef struct naos_result
{
    int failure_count;
    int skipped;
} naos_result_t;

static naos_result_t g_result = { 0, 0 };

/* The open diagnostic; its file, text, labels and values are borrowed until it ends. */
static naos_diagnostic_record_t g_record;
static naos_detail_t g_details[NAOS_DETAIL_COUNT];
static size_t g_detail_count = 0;

static void add_detail(const char *label, naos_detail_kind_t kind, const char *value)
{
    if (g_detail_count >= NAOS_DETAIL_COUNT)
    {
        return;
    }

    g_details[g_detail_count].label = label;
    g_details[g_detail_count].kind = kind;
    g_details[g_detail_count].value = value;
    g_detail_count++;
}

/*
 * Closes the open diagnostic and tells it to the writers. Unless use is NAOS_CONTEXT_NONE, the note,
 * the count of the traces not kept and the traces still pushed go with it, innermost first, and the
 * note is used up. With NAOS_CONTEXT_OUTER, the innermost trace is left out: the text names it.
 */
static void end_with(naos_context_use_t use)
{
    const naos_trace_t *traces[NAOS_DIAGNOSTIC_TRACE_DEPTH];
    int trace_count;
    int i;

    g_record.details = g_details;
    g_record.detail_count = g_detail_count;
    g_record.note = NULL;
    g_record.traces_not_kept = 0;
    trace_count = 0;

    if (use != NAOS_CONTEXT_NONE)
    {
        g_record.note = current_note();
        g_record.traces_not_kept = g_traces.depth - kept_trace_count();

        i = kept_trace_count() - 1;
        if (use == NAOS_CONTEXT_OUTER)
        {
            i--;
        }

        while (i >= 0)
        {
            traces[trace_count] = &g_traces.items[i];
            trace_count++;
            i--;
        }
    }

    g_record.traces = traces;
    g_record.trace_count = trace_count;
    naos_report_diagnostic(&g_record);
    g_detail_count = 0;

    if (use != NAOS_CONTEXT_NONE)
    {
        clear_note();
    }
}

void naos_diagnostic_start_test(void)
{
    g_result.failure_count = 0;
    g_result.skipped = 0;
}

naos_outcome_t naos_diagnostic_outcome(void)
{
    if (g_result.failure_count > 0)
    {
        return NAOS_OUTCOME_FAIL;
    }

    return (g_result.skipped != 0) ? NAOS_OUTCOME_SKIP : NAOS_OUTCOME_PASS;
}

int naos_diagnostic_failure_count(void)
{
    return g_result.failure_count;
}

void naos_diagnostic_begin(naos_diagnostic_kind_t kind, const char *file, int line, const char *text)
{
    if (kind == NAOS_DIAGNOSTIC_SKIP)
    {
        g_result.skipped = 1;
    }
    else
    {
        g_result.failure_count++;
    }

    g_record.kind = kind;
    g_record.file = file;
    g_record.line = line;
    g_record.text = text;
    g_detail_count = 0;
}

void naos_diagnostic_add(const char *label, const char *value)
{
    add_detail(label, NAOS_DETAIL_TEXT, value);
}

void naos_diagnostic_add_string(const char *label, const char *value)
{
    add_detail(label, NAOS_DETAIL_STRING, value);
}

void naos_diagnostic_end(void)
{
    end_with(NAOS_CONTEXT_ALL);
}

void naos_diagnostic_pass(void)
{
    clear_note();
}

void naos_diagnostic_misuse(const char *file, int line, const char *message)
{
    naos_diagnostic_begin(NAOS_DIAGNOSTIC_ERROR, file, line, message);
    end_with(NAOS_CONTEXT_NONE);
}

void naos_diagnostic_check_traces_popped(void)
{
    static const char prefix[] = "NAOS_TRACE_PUSH without NAOS_TRACE_POP: ";
    char message[NAOS_TRACE_TEXT_SIZE + sizeof(prefix)];
    const naos_trace_t *innermost;

    if (g_traces.depth == 0)
    {
        return;
    }

    innermost = &g_traces.items[kept_trace_count() - 1];
    naos_snprintf(message, sizeof(message), "%s%s", prefix, innermost->text);
    naos_diagnostic_begin(NAOS_DIAGNOSTIC_ERROR, innermost->file, innermost->line, message);
    end_with(NAOS_CONTEXT_OUTER);
}

void naos_diagnostic_clear_context(void)
{
    g_note[0] = '\0';
    g_traces.depth = 0;
}

void naos_diagnostic_set_note(const char *format, va_list args)
{
    format_into(g_note, sizeof(g_note), format, args);
}

int naos_diagnostic_push_trace(const char *file, int line, const char *format, va_list args)
{
    naos_trace_t *trace;

    if (g_traces.depth >= NAOS_DIAGNOSTIC_TRACE_DEPTH)
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

int naos_diagnostic_pop_trace(void)
{
    if (g_traces.depth == 0)
    {
        return 0;
    }

    g_traces.depth--;

    return 1;
}
