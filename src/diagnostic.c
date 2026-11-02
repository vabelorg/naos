/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * The diagnostics of the running test, one at a time (begin, details, end), and the outcome they
 * add up to (diagnostic.h).
 */

#include "diagnostic.h"

#include "context.h"
#include "format.h"

/* Most values of one diagnostic. */
#define NAOS_DETAIL_COUNT 8

/* Which of the note and the traces a diagnostic is closed with. */
typedef enum naos_context_use
{
    NAOS_CONTEXT_ALL,   /* a failure, NAOS_FAIL or NAOS_SKIP */
    NAOS_CONTEXT_OUTER, /* NAOS_TRACE_PUSH without NAOS_TRACE_POP: its text names the innermost trace */
    NAOS_CONTEXT_NONE   /* NAOS_TRACE_POP without NAOS_TRACE_PUSH, or a push too deep: the note is kept */
} naos_context_use_t;

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
    const naos_trace_t *traces[NAOS_CONTEXT_TRACE_DEPTH];
    int trace_count;
    int i;

    g_record.details = g_details;
    g_record.detail_count = g_detail_count;
    g_record.note = NULL;
    g_record.traces_not_kept = 0;
    trace_count = 0;

    if (use != NAOS_CONTEXT_NONE)
    {
        g_record.note = naos_context_note();
        g_record.traces_not_kept = naos_context_trace_depth() - naos_context_trace_count();

        i = naos_context_trace_count() - 1;
        if (use == NAOS_CONTEXT_OUTER)
        {
            i--;
        }

        while (i >= 0)
        {
            traces[trace_count] = naos_context_trace(i);
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
        naos_context_clear_note();
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
    naos_context_clear_note();
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

    if (naos_context_trace_depth() == 0)
    {
        return;
    }

    innermost = naos_context_trace(naos_context_trace_count() - 1);
    naos_snprintf(message, sizeof(message), "%s%s", prefix, innermost->text);
    naos_diagnostic_begin(NAOS_DIAGNOSTIC_ERROR, innermost->file, innermost->line, message);
    end_with(NAOS_CONTEXT_OUTER);
}
