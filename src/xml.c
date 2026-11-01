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
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <naos/export.h>

#include "format.h"
#include "value.h"

/* Size of the first allocation of a text; each next one doubles it. */
#define NAOS_TEXT_FIRST_SIZE 256

/* Size of what add_format() formats: a number of any %d, %ld, %lu or %zu, with a few chars around it. */
#define NAOS_NUMBER_SIZE 48

/* A text that grows as it is added to; malloc. */
typedef struct naos_text
{
    char *data;
    size_t length;
    size_t capacity;
} naos_text_t;

/* Where a text is added, which decides how it is escaped. */
typedef enum naos_xml_place
{
    NAOS_XML_ATTRIBUTE, /* the value of an attribute */
    NAOS_XML_TEXT,      /* the text of an element, as it is */
    NAOS_XML_LINE       /* a value of a line of a <failure>: kept on its line */
} naos_xml_place_t;

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
static int g_out_of_memory = 0;

/* The <testcase> elements of the tests that have ended. */
static naos_text_t g_cases = { NULL, 0, 0 };

/* What the running test has so far: its location, its <failure> elements and the message of its skip. */
static naos_text_t g_location = { NULL, 0, 0 };
static naos_text_t g_failures = { NULL, 0, 0 };
static naos_text_t g_skip = { NULL, 0, 0 };
static int g_has_location = 0;
static int g_has_skip = 0;

/* The suites of every round so far, the last one being the suite of the running test. */
static naos_xml_suite_t *g_suites = NULL;
static size_t g_suite_count = 0;
static size_t g_suite_capacity = 0;
static int g_suite_open = 0;
static naos_round_t g_round;

static void add_format(naos_text_t *text, const char *format, ...) NAOS_PRINTF_FORMAT(2, 3);

/* Makes room for more chars in text. Returns 0, and from then on adds nothing, when memory runs out. */
static int reserve(naos_text_t *text, size_t more)
{
    size_t capacity;
    char *data;

    if (g_out_of_memory != 0)
    {
        return 0;
    }

    if (more <= (text->capacity - text->length))
    {
        return 1;
    }

    capacity = (text->capacity == 0) ? NAOS_TEXT_FIRST_SIZE : text->capacity;
    while ((capacity - text->length) < more)
    {
        if (capacity > (SIZE_MAX / 2))
        {
            g_out_of_memory = 1;

            return 0;
        }

        capacity *= 2;
    }

    data = (char *)realloc(text->data, capacity);
    if (data == NULL)
    {
        g_out_of_memory = 1;

        return 0;
    }

    text->data = data;
    text->capacity = capacity;

    return 1;
}

/* bytes may be NULL when length is 0, as the data of a text nothing was added to: memcpy may not be called then. */
static void add_bytes(naos_text_t *text, const char *bytes, size_t length)
{
    if (length == 0)
    {
        return;
    }

    if (reserve(text, length) != 0)
    {
        memcpy(text->data + text->length, bytes, length);
        text->length += length;
    }
}

/* Adds markup, or any text that holds nothing to escape. */
static void add(naos_text_t *text, const char *markup)
{
    add_bytes(text, markup, strlen(markup));
}

/* Adds numbers formatted as printf does; the result is shorter than NAOS_NUMBER_SIZE. */
static void add_format(naos_text_t *text, const char *format, ...)
{
    char buffer[NAOS_NUMBER_SIZE];
    va_list args;

    va_start(args, format);
    naos_vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    add(text, buffer);
}

static void release(naos_text_t *text)
{
    free(text->data);
    text->data = NULL;
    text->length = 0;
    text->capacity = 0;
}

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
 * The escape of byte in a value of a line of a <failure>, which keeps the value on its line and lets
 * it be read back: \\, \n and \r. NULL for the other bytes.
 */
static const char *line_escape(unsigned char byte)
{
    switch (byte)
    {
    case '\\':
        return "\\\\";
    case '\n':
        return "\\n";
    case '\r':
        return "\\r";
    default:
        return NULL;
    }
}

/*
 * Adds a text of the test or of Naos, such as an expression, a message or a file name, escaped for
 * XML: in an attribute, in the text of an element, or as a value of a line of a <failure>. Valid
 * UTF-8 is kept as it is; a byte that is not part of it, and a control character that XML 1.0 does
 * not allow, is written as \xNN.
 */
static void add_escaped(naos_text_t *text, const char *value, naos_xml_place_t place)
{
    char escape[NAOS_VALUE_BYTE_SIZE];
    const unsigned char *byte;
    const char *reference;
    size_t length;

    byte = (const unsigned char *)value;
    while (*byte != '\0')
    {
        reference = (place == NAOS_XML_LINE) ? line_escape(*byte) : NULL;
        if (reference == NULL)
        {
            reference = xml_reference(*byte, place == NAOS_XML_ATTRIBUTE);
        }

        length = utf8_sequence(byte);
        if (reference != NULL)
        {
            add(text, reference);
            length = 1;
        }
        else if ((length == 0) || ((*byte < 0x20) && (*byte != '\n') && (*byte != '\t')))
        {
            naos_snprintf(escape, sizeof(escape), "\\x%02X", (unsigned int)*byte);
            add(text, escape);
            length = 1;
        }
        else
        {
            add_bytes(text, (const char *)byte, length);
        }

        byte += length;
    }
}

/* Adds a C string of the test, possibly NULL, as the human-readable output writes it, escaped for XML. */
static void add_string_value(naos_text_t *text, const char *value)
{
    char escape[NAOS_VALUE_BYTE_SIZE];
    const unsigned char *byte;

    if (value == NULL)
    {
        add(text, "(null)");
        return;
    }

    add(text, "\"");
    for (byte = (const unsigned char *)value; *byte != '\0'; byte++)
    {
        naos_value_escape_byte(*byte, escape);
        add_escaped(text, escape, NAOS_XML_TEXT);
    }
    add(text, "\"");
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
        add(text, "naos");
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
    add_escaped(text, copy, NAOS_XML_ATTRIBUTE);
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
    suite->start = g_cases.length;
    suite->end = g_cases.length;
    g_suite_count++;
    g_suite_open = 1;
}

/* Adds the counts of tally as the attributes of <testsuites> or <testsuite>. */
static void add_counts(naos_text_t *text, const naos_tally_t *tally)
{
    add_format(text, " tests=\"%zu\"", tally->passed + tally->failed + tally->skipped);
    add_format(text, " failures=\"%zu\"", tally->failed);
    add(text, " errors=\"0\"");
    add_format(text, " skipped=\"%zu\"", tally->skipped);
}

/* Adds one suite of one round: its counts, the round it ran in when there are several or shuffled, and its test cases. */
static void add_suite(naos_text_t *text, const naos_xml_suite_t *suite)
{
    add(text, "  <testsuite name=\"");
    add_escaped(text, suite->name, NAOS_XML_ATTRIBUTE);
    add(text, "\"");
    add_counts(text, &suite->tally);
    add(text, ">\n");

    if ((suite->round.repeat > 1) || (suite->round.shuffled != 0))
    {
        add(text, "    <properties>\n");
        if (suite->round.repeat > 1)
        {
            add(text, "      <property name=\"iteration\"");
            add_format(text, " value=\"%ld\"/>\n", suite->round.index);
        }

        if (suite->round.shuffled != 0)
        {
            add(text, "      <property name=\"random_seed\"");
            add_format(text, " value=\"%lu\"/>\n", suite->round.seed);
        }
        add(text, "    </properties>\n");
    }

    /* NULL + 0 is undefined too, so the test cases are added only when there are some. */
    if (suite->end > suite->start)
    {
        add_bytes(text, g_cases.data + suite->start, suite->end - suite->start);
    }
    add(text, "  </testsuite>\n");
}

static void release_all(void)
{
    release(&g_cases);
    release(&g_location);
    release(&g_failures);
    release(&g_skip);
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
static void test_begin(const char *suite, const char *test)
{
    (void)test;

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
 * The first diagnostic gives the test case its location. A failure becomes a <failure> element whose
 * text holds its location, values, note and traces, one "<label>: <value>" line each, each value
 * kept on its line (a string value is already); a skip keeps its message for <skipped>.
 */
static void diagnostic(const naos_diagnostic_record_t *record)
{
    const naos_trace_t *trace;
    size_t i;
    int j;

    if (g_has_location == 0)
    {
        g_has_location = 1;
        add(&g_location, " file=\"");
        add_escaped(&g_location, record->file, NAOS_XML_ATTRIBUTE);
        add_format(&g_location, "\" line=\"%d\"", record->line);
    }

    if (record->kind == NAOS_REPORT_SKIP)
    {
        if (g_has_skip == 0)
        {
            g_has_skip = 1;
            add_escaped(&g_skip, record->text, NAOS_XML_ATTRIBUTE);
        }

        return;
    }

    add(&g_failures, "      <failure type=\"error\" message=\"");
    add_escaped(&g_failures, record->text, NAOS_XML_ATTRIBUTE);
    add(&g_failures, "\">file: ");
    add_escaped(&g_failures, record->file, NAOS_XML_LINE);
    add_format(&g_failures, "\nline: %d", record->line);

    for (i = 0; i < record->detail_count; i++)
    {
        add(&g_failures, "\n");
        add_escaped(&g_failures, record->details[i].label, NAOS_XML_LINE);
        add(&g_failures, ": ");
        if (record->details[i].kind == NAOS_DETAIL_STRING)
        {
            add_string_value(&g_failures, record->details[i].value);
        }
        else
        {
            add_escaped(&g_failures, record->details[i].value, NAOS_XML_LINE);
        }
    }

    if (record->note != NULL)
    {
        add(&g_failures, "\nnote: ");
        add_escaped(&g_failures, record->note, NAOS_XML_LINE);
    }

    if (record->traces_not_kept > 0)
    {
        add_format(&g_failures, "\ntraces: %d", record->traces_not_kept);
        add(&g_failures, " innermost not kept");
    }

    for (j = 0; j < record->trace_count; j++)
    {
        /* The location on a line of its own, read from its last ':', since a file name may hold ': '. */
        trace = record->traces[j];
        add(&g_failures, "\ntrace: ");
        add_escaped(&g_failures, trace->text, NAOS_XML_LINE);
        add(&g_failures, "\nat: ");
        add_escaped(&g_failures, trace->file, NAOS_XML_LINE);
        add_format(&g_failures, ":%d", trace->line);
    }

    add(&g_failures, "</failure>\n");
}

static void test_end(const char *suite_name, const char *test, naos_outcome_t outcome, int failure_count)
{
    naos_xml_suite_t *suite;

    (void)failure_count;

    if (g_suite_count == 0)
    {
        return;
    }

    suite = &g_suites[g_suite_count - 1];
    add(&g_cases, "    <testcase classname=\"");
    add_escaped(&g_cases, suite_name, NAOS_XML_ATTRIBUTE);
    add(&g_cases, "\" name=\"");
    add_escaped(&g_cases, test, NAOS_XML_ATTRIBUTE);
    add(&g_cases, "\"");
    add_bytes(&g_cases, g_location.data, g_location.length);

    switch (outcome)
    {
    case NAOS_OUTCOME_FAIL:
        add(&g_cases, ">\n");
        add_bytes(&g_cases, g_failures.data, g_failures.length);
        add(&g_cases, "    </testcase>\n");
        suite->tally.failed++;
        break;
    case NAOS_OUTCOME_SKIP:
        add(&g_cases, ">\n      <skipped message=\"");
        add_bytes(&g_cases, g_skip.data, g_skip.length);
        add(&g_cases, "\"/>\n    </testcase>\n");
        suite->tally.skipped++;
        break;
    case NAOS_OUTCOME_PASS:
        add(&g_cases, "/>\n");
        suite->tally.passed++;
        break;
    }

    suite->end = g_cases.length;
}

static void round_end(const naos_round_t *round, size_t selected, const naos_tally_t *tally, size_t not_run)
{
    (void)round;
    (void)selected;
    (void)tally;
    (void)not_run;

    g_suite_open = 0;
}

/* Puts the whole file together and writes it at once, then closes it. */
static int end(long failed_rounds, long repeat, long rounds_not_run)
{
    naos_text_t document = { NULL, 0, 0 };
    naos_tally_t total = { 0, 0, 0 };
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
    }

    add(&document, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<testsuites name=\"");
    add_program_name(&document, g_program);
    add(&document, "\"");
    add_counts(&document, &total);
    add(&document, ">\n");
    for (i = 0; i < g_suite_count; i++)
    {
        add_suite(&document, &g_suites[i]);
    }
    add(&document, "</testsuites>\n");

    written = 0;
    if (g_out_of_memory == 0)
    {
        written = (fwrite(document.data, 1, document.length, g_file) == document.length) && (fflush(g_file) == 0);
    }

    if (fclose(g_file) != 0)
    {
        written = 0;
    }

    g_file = NULL;
    release(&document);
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
