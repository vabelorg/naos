/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * The JUnit XML file of --naos-output xml:<path> (xml.h). The test cases are kept in memory while
 * the tests run, and the whole file is written when the run ends, so that the counts of each suite
 * come before its test cases; a process that ends during the run leaves the file empty.
 */

/* fopen() is standard C; MSVC would warn that fopen_s() is safer. */
#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "xml.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "format.h"
#include "human.h"
#include "text.h"
#include "value.h"

/* The test cases of one suite in one round: g_cases from start to end. */
typedef struct naos_xml_suite
{
    const char *name;
    naos_round_t round;
    naos_tally_t tally;
    size_t start;
    size_t end;
} naos_xml_suite_t;

static FILE *g_file = NULL;
static const char *g_program = NULL;

/* Memory ran out for the suites or the name of the program; each text keeps its own failed. */
static int g_out_of_memory = 0;

/* The <testcase> elements of the tests that have ended. */
static naos_text_t g_cases = NAOS_TEXT_EMPTY;

/* What the running test has so far: its location, its <failure> elements and the message of its skip. */
static naos_text_t g_location = NAOS_TEXT_EMPTY;
static naos_text_t g_failures = NAOS_TEXT_EMPTY;
static naos_text_t g_skip = NAOS_TEXT_EMPTY;
static int g_has_location = 0;
static int g_has_skip = 0;
static const char *g_xfail_reason = NULL;

/* The suites of every round so far, the last one being the suite of the running test. */
static naos_xml_suite_t *g_suites = NULL;
static size_t g_suite_count = 0;
static size_t g_suite_capacity = 0;
static int g_suite_open = 0;
static naos_round_t g_round;

/*
 * Length of the UTF-8 sequence that bytes starts with, from 1 to 4, or 0 when it is not one that
 * XML 1.0 allows: a byte that cannot start one, a sequence cut short or longer than it needs, a
 * surrogate, U+FFFE, U+FFFF or a code point beyond U+10FFFF. A NUL ends a sequence short.
 */
static size_t utf8_sequence(const unsigned char *bytes)
{
    unsigned long code;
    size_t length;
    size_t i;

    if (bytes[0] < 0x80)
    {
        return 1;
    }

    if ((bytes[0] >= 0xC2) && (bytes[0] <= 0xDF))
    {
        length = 2;
        code = bytes[0] & 0x1FUL;
    }
    else if ((bytes[0] >= 0xE0) && (bytes[0] <= 0xEF))
    {
        length = 3;
        code = bytes[0] & 0x0FUL;
    }
    else if ((bytes[0] >= 0xF0) && (bytes[0] <= 0xF4))
    {
        length = 4;
        code = bytes[0] & 0x07UL;
    }
    else
    {
        return 0;
    }

    for (i = 1; i < length; i++)
    {
        if ((bytes[i] & 0xC0) != 0x80)
        {
            return 0;
        }

        code = (code << 6) | (bytes[i] & 0x3FUL);
    }

    if (((length == 3) && (code < 0x800UL)) || ((length == 4) && ((code < 0x10000UL) || (code > 0x10FFFFUL))))
    {
        return 0;
    }

    if (((code >= 0xD800UL) && (code <= 0xDFFFUL)) || (code == 0xFFFEUL) || (code == 0xFFFFUL))
    {
        return 0;
    }

    return length;
}

/*
 * The entity or character reference byte is written as, or NULL when it needs none. In an attribute,
 * the quotes and the white space that a parser would turn into spaces need one too.
 */
static const char *xml_reference(unsigned char byte, int in_attribute)
{
    switch (byte)
    {
    case '&':
        return "&amp;";
    case '<':
        return "&lt;";
    case '>':
        return "&gt;";
    case '"':
        return in_attribute ? "&quot;" : NULL;
    case '\'':
        return in_attribute ? "&apos;" : NULL;
    case '\r':
        return "&#13;";
    case '\n':
        return in_attribute ? "&#10;" : NULL;
    case '\t':
        return in_attribute ? "&#9;" : NULL;
    default:
        return NULL;
    }
}

/*
 * Adds a text of the test or of Naos, such as an expression, a message or a file name, escaped for
 * XML, in an attribute or in the text of an element. Valid UTF-8 is kept as it is; a byte that is
 * not part of it, and a control character that XML 1.0 does not allow, is written as \xNN.
 */
static void add_escaped(naos_text_t *text, const char *value, int in_attribute)
{
    char escape[NAOS_VALUE_BYTE_SIZE];
    const unsigned char *byte;
    const char *reference;
    size_t length;

    byte = (const unsigned char *)value;
    while (*byte != '\0')
    {
        reference = xml_reference(*byte, in_attribute);
        length = utf8_sequence(byte);
        if (reference != NULL)
        {
            naos_text_add(text, reference);
            length = 1;
        }
        else if ((length == 0) || ((*byte < 0x20) && (*byte != '\n') && (*byte != '\t')))
        {
            naos_snprintf(escape, sizeof(escape), "\\x%02X", (unsigned int)*byte);
            naos_text_add(text, escape);
            length = 1;
        }
        else
        {
            naos_text_add_bytes(text, (const char *)byte, length);
        }

        byte += length;
    }
}

/* The sink of naos_human_put_diagnostic() that adds each part to the text of a <failure>. */
static void put_failure_text(void *context, const char *part)
{
    add_escaped((naos_text_t *)context, part, 0);
}

/* The name of the program as the name of <testsuites>: without its directory, and without ".exe". */
static void add_program_name(naos_text_t *text, const char *program)
{
    static const char extension[] = ".exe";
    const char *name;
    const char *cursor;
    char *copy;
    size_t length;
    size_t i;

    if (program == NULL)
    {
        naos_text_add(text, "naos");
        return;
    }

    name = program;
    for (cursor = program; *cursor != '\0'; cursor++)
    {
        if ((*cursor == '/') || (*cursor == '\\'))
        {
            name = cursor + 1;
        }
    }

    length = strlen(name);
    if (length > (sizeof(extension) - 1))
    {
        for (i = 0; i < (sizeof(extension) - 1); i++)
        {
            if (tolower((unsigned char)name[length - (sizeof(extension) - 1) + i]) != extension[i])
            {
                break;
            }
        }

        if (i == (sizeof(extension) - 1))
        {
            length -= sizeof(extension) - 1;
        }
    }

    copy = (char *)malloc(length + 1);
    if (copy == NULL)
    {
        g_out_of_memory = 1;
        return;
    }

    memcpy(copy, name, length);
    copy[length] = '\0';
    add_escaped(text, copy, 1);
    free(copy);
}

/* Starts the record of a suite in the running round, for its tests that follow. */
static void open_suite(const char *name)
{
    naos_xml_suite_t *suites;
    naos_xml_suite_t *suite;
    size_t capacity;

    if (g_suite_count == g_suite_capacity)
    {
        capacity = (g_suite_capacity == 0) ? 16 : (g_suite_capacity * 2);
        suites = (capacity > (SIZE_MAX / sizeof(*suites))) ? NULL : (naos_xml_suite_t *)realloc(g_suites, capacity * sizeof(*suites));
        if (suites == NULL)
        {
            g_out_of_memory = 1;
            return;
        }

        g_suites = suites;
        g_suite_capacity = capacity;
    }

    suite = &g_suites[g_suite_count];
    suite->name = name;
    suite->round = g_round;
    suite->tally.passed = 0;
    suite->tally.failed = 0;
    suite->tally.skipped = 0;
    suite->tally.elapsed = 0;
    suite->start = g_cases.length;
    suite->end = g_cases.length;
    g_suite_count++;
    g_suite_open = 1;
}

/* Adds a time as the attribute time: seconds with three decimals, rounded down, as JUnit XML has it. */
static void add_time(naos_text_t *text, naos_nanoseconds_t elapsed)
{
    naos_nanoseconds_t milliseconds;

    milliseconds = elapsed / 1000000ULL;
    naos_text_add_format(text, " time=\"%llu.%03llu\"", milliseconds / 1000ULL, milliseconds % 1000ULL);
}

/* Adds the counts and the time of tally as the attributes of <testsuites> or <testsuite>. */
static void add_counts(naos_text_t *text, const naos_tally_t *tally)
{
    naos_text_add_format(text, " tests=\"%zu\"", tally->passed + tally->failed + tally->skipped);
    naos_text_add_format(text, " failures=\"%zu\"", tally->failed);
    naos_text_add(text, " errors=\"0\"");
    naos_text_add_format(text, " skipped=\"%zu\"", tally->skipped);
    add_time(text, tally->elapsed);
}

/* Adds one suite of one round: its counts, the round it ran in when there are several or shuffled, and its test cases. */
static void add_suite(naos_text_t *text, const naos_xml_suite_t *suite)
{
    naos_text_add(text, "  <testsuite name=\"");
    add_escaped(text, suite->name, 1);
    naos_text_add(text, "\"");
    add_counts(text, &suite->tally);
    naos_text_add(text, ">\n");

    if ((suite->round.repeat > 1) || (suite->round.shuffled != 0))
    {
        naos_text_add(text, "    <properties>\n");
        if (suite->round.repeat > 1)
        {
            naos_text_add(text, "      <property name=\"iteration\"");
            naos_text_add_format(text, " value=\"%ld\"/>\n", suite->round.index);
        }

        if (suite->round.shuffled != 0)
        {
            naos_text_add(text, "      <property name=\"random_seed\"");
            naos_text_add_format(text, " value=\"%lu\"/>\n", suite->round.seed);
        }
        naos_text_add(text, "    </properties>\n");
    }

    /* NULL + 0 is undefined too, so the test cases are added only when there are some. */
    if (suite->end > suite->start)
    {
        naos_text_add_bytes(text, g_cases.data + suite->start, suite->end - suite->start);
    }
    naos_text_add(text, "  </testsuite>\n");
}

static void release_all(void)
{
    naos_text_release(&g_cases);
    naos_text_release(&g_location);
    naos_text_release(&g_failures);
    naos_text_release(&g_skip);
    free(g_suites);
    g_suites = NULL;
    g_suite_count = 0;
    g_suite_capacity = 0;
}

/* Binary, so that the lines end with LF on every platform. */
static int begin(const char *program, const char *path)
{
    g_file = fopen(path, "wb");
    g_program = program;
    g_out_of_memory = 0;
    g_suite_open = 0;

    return g_file != NULL;
}

static void round_begin(const naos_round_t *round)
{
    g_round = *round;
    g_suite_open = 0;
}

/* The tests of a suite run together in a round, so a new name starts a new suite. */
static void test_begin(const char *suite, const char *test, const char *xfail_reason)
{
    (void)test;

    g_xfail_reason = xfail_reason;

    if ((g_suite_open == 0) || (strcmp(g_suites[g_suite_count - 1].name, suite) != 0))
    {
        open_suite(suite);
    }

    g_location.length = 0;
    g_failures.length = 0;
    g_skip.length = 0;
    g_has_location = 0;
    g_has_skip = 0;
}

/*
 * The first diagnostic with a location gives the test case its location. A failure becomes a <failure> element whose
 * text is the diagnostic as stdout shows it, for people to read; a skip keeps its message for
 * <skipped>.
 */
static void diagnostic(const naos_diagnostic_record_t *record)
{
    if ((g_has_location == 0) && (record->file != NULL))
    {
        g_has_location = 1;
        naos_text_add(&g_location, " file=\"");
        add_escaped(&g_location, record->file, 1);
        naos_text_add_format(&g_location, "\" line=\"%d\"", record->line);
    }

    if (record->kind == NAOS_DIAGNOSTIC_SKIP)
    {
        if (g_has_skip == 0)
        {
            g_has_skip = 1;
            add_escaped(&g_skip, record->text, 1);
        }

        return;
    }

    naos_text_add(&g_failures, "      <failure type=\"error\" message=\"");
    add_escaped(&g_failures, record->text, 1);
    naos_text_add(&g_failures, "\">");
    naos_human_put_diagnostic(record, put_failure_text, &g_failures);
    naos_text_add(&g_failures, "</failure>\n");
}

static void test_end(const char *suite_name, const char *test, naos_outcome_t outcome, int failure_count, naos_nanoseconds_t elapsed)
{
    naos_xml_suite_t *suite;

    (void)failure_count;

    if (g_suite_count == 0)
    {
        return;
    }

    suite = &g_suites[g_suite_count - 1];
    naos_text_add(&g_cases, "    <testcase classname=\"");
    add_escaped(&g_cases, suite_name, 1);
    naos_text_add(&g_cases, "\" name=\"");
    add_escaped(&g_cases, test, 1);
    naos_text_add(&g_cases, "\"");
    add_time(&g_cases, elapsed);
    naos_text_add_bytes(&g_cases, g_location.data, g_location.length);
    suite->tally.elapsed += elapsed;

    switch (outcome)
    {
    case NAOS_OUTCOME_FAIL:
    case NAOS_OUTCOME_XPASS:
        naos_text_add(&g_cases, ">\n");
        naos_text_add_bytes(&g_cases, g_failures.data, g_failures.length);
        naos_text_add(&g_cases, "    </testcase>\n");
        suite->tally.failed++;
        break;
    case NAOS_OUTCOME_SKIP:
        naos_text_add(&g_cases, ">\n      <skipped message=\"");
        naos_text_add_bytes(&g_cases, g_skip.data, g_skip.length);
        naos_text_add(&g_cases, "\"/>\n    </testcase>\n");
        suite->tally.skipped++;
        break;
    case NAOS_OUTCOME_PASS:
        naos_text_add(&g_cases, "/>\n");
        suite->tally.passed++;
        break;
    case NAOS_OUTCOME_XFAIL:
        /* Not a pass to the reader, nor a failure that would fail the run: skipped, as pytest has it. */
        naos_text_add(&g_cases, ">\n      <skipped message=\"expected failure: ");
        add_escaped(&g_cases, (g_xfail_reason != NULL) ? g_xfail_reason : "", 1);
        naos_text_add(&g_cases, "\"/>\n    </testcase>\n");
        suite->tally.skipped++;
        break;
    }

    suite->end = g_cases.length;
}

static void round_end(const naos_round_t *round, const naos_tally_t *tally, size_t not_run)
{
    (void)round;
    (void)tally;
    (void)not_run;

    g_suite_open = 0;
}

/* Puts the whole file together and writes it at once, then closes it. */
static int end(long failed_rounds, long repeat, long rounds_not_run)
{
    naos_text_t document = NAOS_TEXT_EMPTY;
    naos_tally_t total = NAOS_TALLY_EMPTY;
    size_t i;
    int written;

    (void)failed_rounds;
    (void)repeat;
    (void)rounds_not_run;

    for (i = 0; i < g_suite_count; i++)
    {
        total.passed += g_suites[i].tally.passed;
        total.failed += g_suites[i].tally.failed;
        total.skipped += g_suites[i].tally.skipped;
        total.elapsed += g_suites[i].tally.elapsed;
    }

    naos_text_add(&document, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<testsuites name=\"");
    add_program_name(&document, g_program);
    naos_text_add(&document, "\"");
    add_counts(&document, &total);
    naos_text_add(&document, ">\n");
    for (i = 0; i < g_suite_count; i++)
    {
        add_suite(&document, &g_suites[i]);
    }
    naos_text_add(&document, "</testsuites>\n");

    written = 0;
    if ((g_out_of_memory == 0) && (g_cases.failed == 0) && (g_location.failed == 0) && (g_failures.failed == 0) && (g_skip.failed == 0) && (document.failed == 0))
    {
        written = (fwrite(document.data, 1, document.length, g_file) == document.length) && (fflush(g_file) == 0);
    }

    if (fclose(g_file) != 0)
    {
        written = 0;
    }

    g_file = NULL;
    naos_text_release(&document);
    release_all();

    return written;
}

const naos_writer_t *naos_xml_writer(void)
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
