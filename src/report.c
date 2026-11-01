/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * The form of every line Naos writes, on stdout and on stderr (report.h).
 */

#include "report.h"

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

/* Writes the value of one detail as its kind says. */
static void print_detail_value(const naos_detail_t *detail)
{
    if (detail->kind == NAOS_DETAIL_STRING)
    {
        naos_value_print_string(detail->value);
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

void naos_report_test_start(const char *suite, const char *test)
{
    naos_printf("RUN: %s.%s\n", suite, test);
    fflush(stdout);
}

void naos_report_test_result(const char *suite, const char *test, naos_outcome_t outcome, int failure_count)
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

void naos_report_diagnostic(naos_report_kind_t kind, const char *file, int line, const char *text)
{
    print_location(file, line);
    naos_printf("%s: %s\n", report_kind_word(kind), text);
}

void naos_report_details(const naos_detail_t *details, size_t count)
{
    size_t i;

    for (i = 0; i < count; i++)
    {
        naos_printf("  %s:%s", details[i].label, has_right_partner(details[i].label, details, count) ? "  " : " ");
        print_detail_value(&details[i]);
        fputc('\n', stdout);
    }
}

void naos_report_test_name(const char *suite, const char *test)
{
    naos_printf("%s.%s\n", suite, test);
}

void naos_report_iteration(long round, long repeat)
{
    naos_printf("ITERATION: %ld of %ld\n", round, repeat);
}

void naos_report_random_seed(unsigned long seed)
{
    naos_printf("RANDOM SEED: %lu\n", seed);
}

void naos_report_fail_fast(size_t not_run)
{
    naos_printf("FAIL FAST: stopped at the first failure; %zu not run\n", not_run);
}

void naos_report_summary(size_t passed, size_t failed, size_t skipped)
{
    naos_printf("SUMMARY: %zu passed, %zu failed, %zu skipped\n", passed, failed, skipped);
}

void naos_report_iteration_failed(long round, long repeat, int shuffled, unsigned long seed)
{
    naos_printf("ITERATION FAILED: %ld of %ld", round, repeat);
    if (shuffled != 0)
    {
        naos_printf(" (random seed %lu)", seed);
    }
    naos_printf("\n");
}

void naos_report_repeat(long failed_rounds, long repeat, long not_run)
{
    naos_printf("REPEAT: %ld of %ld iterations failed", failed_rounds, repeat);
    if (not_run > 0)
    {
        naos_printf(", %ld not run", not_run);
    }
    naos_printf("\n");
}

void naos_report_flush(void)
{
    fflush(stdout);
}

void naos_report_error(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    naos_report_error_v(format, args);
    va_end(args);
}

void naos_report_error_v(const char *format, va_list args)
{
    fflush(stdout);
    fputs("naos: ", stderr);
    naos_vfprintf(stderr, format, args);
    fputc('\n', stderr);
    fflush(stderr);
}
