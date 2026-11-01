/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * The human-readable output (human.h): the form of every line a run writes on stdout, and which of
 * them a run writes.
 */

#include "human.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <naos/export.h>

#include "format.h"
#include "text.h"
#include "value.h"

/* Size of what put_format() formats: a number with a few chars around it. */
#define NAOS_HUMAN_NUMBER_SIZE 48

/* --naos-quiet: the tests that pass or skip, and ITERATION:, are left out. */
static int g_quiet = 0;

/*
 * The running test, between test_begin and test_end. With --naos-quiet, its RUN line waits for its
 * first failure, and so do its skips before it, since a failure in teardown may still follow them.
 */
static const char *g_suite = NULL;
static const char *g_test = NULL;
static int g_in_test = 0;
static int g_run_written = 0;
static naos_text_t g_held = NAOS_TEXT_EMPTY;

static void put_format(naos_human_sink_t sink, void *context, const char *format, ...) NAOS_PRINTF_FORMAT(3, 4);

/* Gives sink numbers formatted as printf does; the result is shorter than NAOS_HUMAN_NUMBER_SIZE. */
static void put_format(naos_human_sink_t sink, void *context, const char *format, ...)
{
    char buffer[NAOS_HUMAN_NUMBER_SIZE];
    va_list args;

    va_start(args, format);
    naos_vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    sink(context, buffer);
}

/*
 * Gives where a diagnostic happened, followed by a space, in the form of the diagnostics of the
 * compiler Naos is built with: "<file>(<line>): " for MSVC, "<file>:<line>: " otherwise. This is
 * the only place that depends on the compiler.
 */
static void put_location(naos_human_sink_t sink, void *context, const char *file, int line)
{
    sink(context, file);
#if defined(_MSC_VER)
    put_format(sink, context, "(%d): ", line);
#else
    put_format(sink, context, ":%d: ", line);
#endif
}

/* Word of a diagnostic line, after the location. */
static const char *report_kind_word(naos_report_kind_t kind)
{
    switch (kind)
    {
    case NAOS_REPORT_ERROR:
        return "error";
    case NAOS_REPORT_SKIP:
        return "skip";
    case NAOS_REPORT_NOTE:
        return "note";
    }

    return "error";
}

/* Gives "<location> <kind>: <text>\n"; file and text are given as they are. */
static void put_diagnostic_line(naos_human_sink_t sink, void *context, naos_report_kind_t kind, const char *file, int line, const char *text)
{
    put_location(sink, context, file, line);
    sink(context, report_kind_word(kind));
    sink(context, ": ");
    sink(context, text);
    sink(context, "\n");
}

/* Gives the value of one detail as its kind says: a string between double quotes, escaped one byte at a time, or (null). */
static void put_detail_value(naos_human_sink_t sink, void *context, const naos_detail_t *detail)
{
    char escape[NAOS_VALUE_BYTE_SIZE];
    const unsigned char *byte;

    if (detail->kind != NAOS_DETAIL_STRING)
    {
        sink(context, detail->value);
        return;
    }

    if (detail->value == NULL)
    {
        sink(context, "(null)");
        return;
    }

    sink(context, "\"");
    for (byte = (const unsigned char *)detail->value; *byte != '\0'; byte++)
    {
        naos_value_escape_byte(*byte, escape);
        sink(context, escape);
    }
    sink(context, "\"");
}

/* The sink of the lines of stdout. */
static void put_stdout(void *context, const char *text)
{
    (void)context;
    fputs(text, stdout);
}

/* The sink that adds the lines to a text, to write them later. */
static void put_text(void *context, const char *text)
{
    naos_text_add((naos_text_t *)context, text);
}

/* Writes the RUN line of the running test, if not yet written, then the skips held for it. */
static void write_run(void)
{
    if (g_run_written != 0)
    {
        return;
    }

    g_run_written = 1;
    naos_printf("RUN: %s.%s\n", g_suite, g_test);

    /* data is NULL until something is held, and fwrite may not be given NULL. */
    if (g_held.length > 0)
    {
        fwrite(g_held.data, 1, g_held.length, stdout);
        g_held.length = 0;
    }

    fflush(stdout);
}

/*
 * Whether label is "left" or "left <rest>" and details[0, count) also holds its partner "right" or
 * "right <rest>". Such a label gets one more space so that its value lines up with the partner's.
 */
static int has_right_partner(const char *label, const naos_detail_t *details, size_t count)
{
    const char *rest;
    size_t i;

    if (strncmp(label, "left", 4) != 0)
    {
        return 0;
    }

    rest = label + 4;
    if ((*rest != '\0') && (*rest != ' '))
    {
        return 0;
    }

    for (i = 0; i < count; i++)
    {
        if ((strncmp(details[i].label, "right", 5) == 0) && (strcmp(details[i].label + 5, rest) == 0))
        {
            return 1;
        }
    }

    return 0;
}

static int begin(const char *program, const char *path)
{
    (void)program;
    (void)path;

    return 1;
}

static void round_begin(const naos_round_t *round)
{
    if ((round->repeat > 1) && (g_quiet == 0))
    {
        naos_printf("ITERATION: %ld of %ld\n", round->index, round->repeat);
    }

    if (round->shuffled != 0)
    {
        /* --naos-shuffle --naos-random-seed <seed> runs the same order again. */
        naos_printf("RANDOM SEED: %lu\n", round->seed);
    }
}

/*
 * The RUN line is flushed at once, so that the name of the test stays when the process dies in it.
 * With --naos-quiet, it waits for the first failure of the test, if any.
 */
static void test_begin(const char *suite, const char *test)
{
    g_suite = suite;
    g_test = test;
    g_in_test = 1;
    g_run_written = 0;
    g_held.length = 0;

    if (g_quiet == 0)
    {
        write_run();
    }
}

/*
 * The diagnostic line, its details as "  <label>: <value>", the note and the count of the traces not
 * kept, then the traces as note lines. "left" and "left <rest>" get one more space when "right" or
 * "right <rest>" is in the block, so the two values line up.
 */
void naos_human_put_diagnostic(const naos_diagnostic_record_t *record, naos_human_sink_t sink, void *context)
{
    const naos_trace_t *trace;
    size_t i;
    int j;

    put_diagnostic_line(sink, context, record->kind, record->file, record->line, record->text);

    for (i = 0; i < record->detail_count; i++)
    {
        sink(context, "  ");
        sink(context, record->details[i].label);
        sink(context, has_right_partner(record->details[i].label, record->details, record->detail_count) ? ":  " : ": ");
        put_detail_value(sink, context, &record->details[i]);
        sink(context, "\n");
    }

    if (record->note != NULL)
    {
        sink(context, "  note: ");
        sink(context, record->note);
        sink(context, "\n");
    }

    if (record->traces_not_kept > 0)
    {
        put_format(sink, context, "  traces: %d innermost not kept\n", record->traces_not_kept);
    }

    for (j = 0; j < record->trace_count; j++)
    {
        trace = record->traces[j];
        put_diagnostic_line(sink, context, NAOS_REPORT_NOTE, trace->file, trace->line, trace->text);
    }
}

/*
 * With --naos-quiet, a skip before any failure of its test is held, to be written only when a
 * failure follows. When memory runs out to hold it, it is written at once rather than lost.
 */
static void diagnostic(const naos_diagnostic_record_t *record)
{
    size_t held;

    if ((g_in_test != 0) && (g_run_written == 0) && (record->kind == NAOS_REPORT_SKIP))
    {
        held = g_held.length;
        naos_human_put_diagnostic(record, put_text, &g_held);
        if (g_held.failed == 0)
        {
            return;
        }

        g_held.length = held;
        g_held.failed = 0;
    }

    /* A diagnostic out of a test, such as a misuse before naos_run(), has no RUN line. */
    if (g_in_test != 0)
    {
        write_run();
    }

    naos_human_put_diagnostic(record, put_stdout, NULL);
    fflush(stdout);
}

/* With --naos-quiet, a test that passes or skips leaves no line, and its skips held are dropped. */
static void test_end(const char *suite, const char *test, naos_outcome_t outcome, int failure_count)
{
    if ((g_quiet != 0) && (outcome != NAOS_OUTCOME_FAIL))
    {
        g_in_test = 0;
        g_held.length = 0;

        return;
    }

    /* A failure comes with its diagnostic, which wrote the RUN line; this keeps it before FAIL anyway. */
    write_run();
    g_in_test = 0;

    switch (outcome)
    {
    case NAOS_OUTCOME_FAIL:
        naos_printf("FAIL: %s.%s (%d %s)\n", suite, test, failure_count, (failure_count == 1) ? "failure" : "failures");
        break;
    case NAOS_OUTCOME_SKIP:
        naos_printf("SKIP: %s.%s\n", suite, test);
        break;
    case NAOS_OUTCOME_PASS:
        naos_printf("PASS: %s.%s\n", suite, test);
        break;
    }

    fflush(stdout);
}

/* The FAIL FAST, SUMMARY and ITERATION FAILED lines; every round ends with its summary, so that a run always says what happened. */
static void round_end(const naos_round_t *round, const naos_tally_t *tally, size_t not_run)
{
    if (not_run > 0)
    {
        naos_printf("FAIL FAST: stopped at the first failure; %zu not run\n", not_run);
    }

    naos_printf("SUMMARY: %zu passed, %zu failed, %zu skipped\n", tally->passed, tally->failed, tally->skipped);

    if ((round->repeat > 1) && (tally->failed > 0))
    {
        naos_printf("ITERATION FAILED: %ld of %ld", round->index, round->repeat);
        if (round->shuffled != 0)
        {
            naos_printf(" (random seed %lu)", round->seed);
        }
        naos_printf("\n");
    }

    fflush(stdout);
}

static int end(long failed_rounds, long repeat, long rounds_not_run)
{
    naos_text_release(&g_held);

    if (repeat > 1)
    {
        naos_printf("REPEAT: %ld of %ld iterations failed", failed_rounds, repeat);
        if (rounds_not_run > 0)
        {
            naos_printf(", %ld not run", rounds_not_run);
        }
        naos_printf("\n");
        fflush(stdout);
    }

    return 1;
}

void naos_human_set_quiet(int quiet)
{
    g_quiet = quiet;
}

const naos_writer_t *naos_human_writer(void)
{
    static const naos_writer_t writer =
    {
        begin,
        round_begin,
        test_begin,
        diagnostic,
        test_end,
        round_end,
        end
    };

    return &writer;
}
