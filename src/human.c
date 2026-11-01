/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * The human-readable output (human.h): the form of every line a run writes on stdout, and which of
 * them a run writes.
 */

#include "human.h"

#include <stdio.h>
#include <string.h>

#include "format.h"
#include "value.h"

/*
 * Writes where a diagnostic happened, followed by a space, in the form of the diagnostics of the
 * compiler Naos is built with: "<file>(<line>): " for MSVC, "<file>:<line>: " otherwise. This is
 * the only place that depends on the compiler.
 */
static void print_location(const char *file, int line)
{
#if defined(_MSC_VER)
    naos_printf("%s(%d): ", file, line);
#else
    naos_printf("%s:%d: ", file, line);
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

/* Writes "<location> <kind>: <text>"; file and text are written as they are. */
static void print_diagnostic_line(naos_report_kind_t kind, const char *file, int line, const char *text)
{
    print_location(file, line);
    naos_printf("%s: %s\n", report_kind_word(kind), text);
}

/* Writes the value of one detail as its kind says. */
static void print_detail_value(const naos_detail_t *detail)
{
    if (detail->kind == NAOS_DETAIL_STRING)
    {
        naos_value_print_string(stdout, detail->value);
    }
    else
    {
        fputs(detail->value, stdout);
    }
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
    if (round->repeat > 1)
    {
        naos_printf("ITERATION: %ld of %ld\n", round->index, round->repeat);
    }

    if (round->shuffled != 0)
    {
        /* --naos-shuffle --naos-random-seed <seed> runs the same order again. */
        naos_printf("RANDOM SEED: %lu\n", round->seed);
    }
}

/* Flushed at once, so that the name of the test stays when the process dies in it. */
static void test_begin(const char *suite, const char *test)
{
    naos_printf("RUN: %s.%s\n", suite, test);
    fflush(stdout);
}

/*
 * Writes the diagnostic line, its details as "  <label>: <value>", the note and the count of the
 * traces not kept, then the traces as note lines. "left" and "left <rest>" get one more space when
 * "right" or "right <rest>" is in the block, so the two values line up.
 */
static void diagnostic(const naos_diagnostic_record_t *record)
{
    const naos_trace_t *trace;
    size_t i;
    int j;

    print_diagnostic_line(record->kind, record->file, record->line, record->text);

    for (i = 0; i < record->detail_count; i++)
    {
        naos_printf("  %s:%s", record->details[i].label, has_right_partner(record->details[i].label, record->details, record->detail_count) ? "  " : " ");
        print_detail_value(&record->details[i]);
        fputc('\n', stdout);
    }

    if (record->note != NULL)
    {
        naos_printf("  note: %s\n", record->note);
    }

    if (record->traces_not_kept > 0)
    {
        naos_printf("  traces: %d innermost not kept\n", record->traces_not_kept);
    }

    for (j = 0; j < record->trace_count; j++)
    {
        trace = record->traces[j];
        print_diagnostic_line(NAOS_REPORT_NOTE, trace->file, trace->line, trace->text);
    }

    fflush(stdout);
}

static void test_end(const char *suite, const char *test, naos_outcome_t outcome, int failure_count)
{
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

/* The FAIL FAST, SUMMARY and ITERATION FAILED lines; a summary of a single test would say nothing more. */
static void round_end(const naos_round_t *round, size_t selected, const naos_tally_t *tally, size_t not_run)
{
    if (not_run > 0)
    {
        naos_printf("FAIL FAST: stopped at the first failure; %zu not run\n", not_run);
    }

    if (selected > 1)
    {
        naos_printf("SUMMARY: %zu passed, %zu failed, %zu skipped\n", tally->passed, tally->failed, tally->skipped);
    }

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
