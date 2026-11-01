/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * Telling the events of a run to every writer on, the names of --naos-list, and the messages of Naos
 * itself on stderr (report.h).
 */

#include "report.h"

#include <stdio.h>

#include "format.h"
#include "human.h"
#include "xml.h"

/* Most writers on at once: the human-readable one, and the one of --naos-output. */
#define NAOS_WRITER_COUNT 2

static const naos_writer_t *g_writers[NAOS_WRITER_COUNT];
static size_t g_writer_count = 0;

/* The path of --naos-output, for the messages; NULL without it. */
static const char *g_output_path = NULL;

/*
 * The writers on: the human-readable one alone until naos_report_begin(), so that a diagnostic out of
 * a run, such as a misuse of NAOS_TRACE_POP before naos_run(), is still written.
 */
static size_t writer_count(void)
{
    if (g_writer_count == 0)
    {
        g_writers[0] = naos_human_writer();
        g_writer_count = 1;
    }

    return g_writer_count;
}

/* The human-readable writer, first in g_writers, writes to stdout and never fails; only the one of --naos-output can. */
int naos_report_begin(const char *program, naos_format_t format, const char *path)
{
    size_t i;

    writer_count();
    if (format == NAOS_FORMAT_XML)
    {
        g_writers[g_writer_count] = naos_xml_writer();
        g_writer_count++;
        g_output_path = path;
    }

    for (i = 0; i < writer_count(); i++)
    {
        if (g_writers[i]->begin(program, (i == 0) ? NULL : path) == 0)
        {
            naos_report_error("cannot open the file of --naos-output: %s", path);

            return 0;
        }
    }

    return 1;
}

void naos_report_round_begin(const naos_round_t *round)
{
    size_t i;

    for (i = 0; i < writer_count(); i++)
    {
        g_writers[i]->round_begin(round);
    }
}

void naos_report_test_start(const char *suite, const char *test)
{
    size_t i;

    for (i = 0; i < writer_count(); i++)
    {
        g_writers[i]->test_begin(suite, test);
    }
}

void naos_report_diagnostic(const naos_diagnostic_record_t *record)
{
    size_t i;

    for (i = 0; i < writer_count(); i++)
    {
        g_writers[i]->diagnostic(record);
    }
}

void naos_report_test_result(const char *suite, const char *test, naos_outcome_t outcome, int failure_count)
{
    size_t i;

    for (i = 0; i < writer_count(); i++)
    {
        g_writers[i]->test_end(suite, test, outcome, failure_count);
    }
}

void naos_report_round_end(const naos_round_t *round, const naos_tally_t *tally, size_t not_run)
{
    size_t i;

    for (i = 0; i < writer_count(); i++)
    {
        g_writers[i]->round_end(round, tally, not_run);
    }
}

/* Every writer ends, even after one of them fails, so that each writes what it can. */
int naos_report_end(long failed_rounds, long repeat, long rounds_not_run)
{
    size_t i;
    int written;

    written = 1;
    for (i = 0; i < writer_count(); i++)
    {
        if (g_writers[i]->end(failed_rounds, repeat, rounds_not_run) == 0)
        {
            naos_report_error("cannot write the file of --naos-output, or out of memory for it: %s", g_output_path);
            written = 0;
        }
    }

    return written;
}

void naos_report_test_name(const char *suite, const char *test)
{
    naos_printf("%s.%s\n", suite, test);
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
