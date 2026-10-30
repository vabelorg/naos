/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * The diagnostics of the running test, one at a time (begin, details, end), and the outcome they
 * add up to (diagnostic.h).
 */

#include "diagnostic.h"

#include <stdio.h>

#include "context.h"

/* Most detail lines of one diagnostic: values, the note and the count of traces not kept. */
#define NAOS_DETAIL_COUNT 8

/* Which of the note and the traces a diagnostic is closed with. */
typedef enum naos_context_use
{
    NAOS_CONTEXT_ALL,   /* a failure, NAOS_FAIL or NAOS_SKIP */
    NAOS_CONTEXT_OUTER, /* NAOS_TRACE_PUSH without NAOS_TRACE_POP: its first line is the innermost trace */
    NAOS_CONTEXT_NONE   /* NAOS_TRACE_POP without NAOS_TRACE_PUSH, or a push too deep: the note is kept */
} naos_context_use_t;

/* What the diagnostics of the running test add up to, cleared by naos_diagnostic_start_test(). */
typedef struct naos_result
{
    int failure_count;
    int skipped;
} naos_result_t;

static naos_result_t g_result = { 0, 0 };

/* Detail lines of the open diagnostic; their labels and values are borrowed until it ends. */
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
 * Writes the traces still pushed as note lines at the place of their push, innermost first. With
 * NAOS_CONTEXT_OUTER, the innermost one is left out: the first line already names it.
 */
static void write_traces(naos_context_use_t use)
{
    const naos_trace_t *trace;
    int i;

    i = naos_context_trace_count() - 1;
    if (use == NAOS_CONTEXT_OUTER)
    {
        i--;
    }

    while (i >= 0)
    {
        trace = naos_context_trace(i);
        naos_report_diagnostic(NAOS_REPORT_NOTE, trace->file, trace->line, trace->text);
        i--;
    }
}

/*
 * Closes the open diagnostic and flushes stdout. Unless use is NAOS_CONTEXT_NONE, the note and the
 * count of the traces not kept follow its values, the note is used up, and the traces follow the
 * details as note lines.
 */
static void end_with(naos_context_use_t use)
{
    char not_kept[32]; /* "<int> innermost not kept" */
    int left_out;

    if (use != NAOS_CONTEXT_NONE)
    {
        if (naos_context_note() != NULL)
        {
            add_detail("note", NAOS_DETAIL_TEXT, naos_context_note());
        }

        left_out = naos_context_trace_depth() - naos_context_trace_count();
        if (left_out > 0)
        {
            snprintf(not_kept, sizeof(not_kept), "%d innermost not kept", left_out);
            add_detail("traces", NAOS_DETAIL_TEXT, not_kept);
        }
    }

    naos_report_details(g_details, g_detail_count);
    g_detail_count = 0;

    if (use != NAOS_CONTEXT_NONE)
    {
        naos_context_clear_note();
        write_traces(use);
    }

    naos_report_flush();
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

void naos_diagnostic_begin(naos_report_kind_t kind, const char *file, int line, const char *text)
{
    if (kind == NAOS_REPORT_SKIP)
    {
        g_result.skipped = 1;
    }
    else
    {
        g_result.failure_count++;
    }

    g_detail_count = 0;
    naos_report_diagnostic(kind, file, line, text);
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
    naos_context_clear_note();
}

void naos_diagnostic_misuse(const char *file, int line, const char *message)
{
    naos_diagnostic_begin(NAOS_REPORT_ERROR, file, line, message);
    end_with(NAOS_CONTEXT_NONE);
}

void naos_diagnostic_check_traces_popped(void)
{
    static const char prefix[] = "NAOS_TRACE_PUSH without NAOS_TRACE_POP: ";
    char message[NAOS_CONTEXT_TRACE_SIZE + sizeof(prefix)];
    const naos_trace_t *innermost;

    if (naos_context_trace_depth() == 0)
    {
        return;
    }

    innermost = naos_context_trace(naos_context_trace_count() - 1);
    snprintf(message, sizeof(message), "%s%s", prefix, innermost->text);
    naos_diagnostic_begin(NAOS_REPORT_ERROR, innermost->file, innermost->line, message);
    end_with(NAOS_CONTEXT_OUTER);
}
