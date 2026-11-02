/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * naos_human_put_diagnostic: the lines of a diagnostic as standard output shows them, collected
 * from the sink it writes to. The other lines of the human-readable output go to standard output
 * directly, and are covered by the golden files of behavior/.
 */

#include <naos/naos.h>

#include <stddef.h>
#include <string.h>

#include "human.h"

#if defined(_MSC_VER)
#define AT(line) "f.c(" #line "): "
#else
#define AT(line) "f.c:" #line ": "
#endif

static char g_lines[2048];

static void collect(void *context, const char *part)
{
    size_t length;
    size_t added;

    (void)context;
    length = strlen(g_lines);
    added = strlen(part);
    NAOS_REQUIRE_UINT_LT(length + added, sizeof(g_lines));
    memcpy(g_lines + length, part, added + 1);
}

/* The lines of record, valid until the next call. */
static const char *lines_of(const naos_diagnostic_record_t *record)
{
    g_lines[0] = '\0';
    naos_human_put_diagnostic(record, collect, NULL);

    return g_lines;
}

/* A record of kind for text, at f.c:3, with the details given and nothing else. */
static naos_diagnostic_record_t record_of(naos_report_kind_t kind, const char *text, const naos_detail_t *details, size_t count)
{
    naos_diagnostic_record_t record;

    memset(&record, 0, sizeof(record));
    record.kind = kind;
    record.file = "f.c";
    record.line = 3;
    record.text = text;
    record.details = details;
    record.detail_count = count;

    return record;
}

NAOS_TEST(error_line)
{
    naos_diagnostic_record_t record;

    record = record_of(NAOS_REPORT_ERROR, "a == b", NULL, 0);
    NAOS_CHECK_STR_EQ(AT(3) "error: a == b\n", lines_of(&record));
}

NAOS_TEST(skip_line)
{
    naos_diagnostic_record_t record;

    record = record_of(NAOS_REPORT_SKIP, "not here", NULL, 0);
    NAOS_CHECK_STR_EQ(AT(3) "skip: not here\n", lines_of(&record));
}

/* The file and the text are written as they are, whatever they hold. */
NAOS_TEST(file_and_text_as_given)
{
    naos_diagnostic_record_t record;

    record = record_of(NAOS_REPORT_ERROR, "\"x\"\t\\", NULL, 0);
    record.file = "dir/sub dir\\f.c";
#if defined(_MSC_VER)
    NAOS_CHECK_STR_EQ("dir/sub dir\\f.c(3): error: \"x\"\t\\\n", lines_of(&record));
#else
    NAOS_CHECK_STR_EQ("dir/sub dir\\f.c:3: error: \"x\"\t\\\n", lines_of(&record));
#endif
}

/* "left" lines up with "right", and "left size" with "right size". */
NAOS_TEST(left_lines_up_with_right)
{
    static const naos_detail_t details[] = {
        { "left size", NAOS_DETAIL_TEXT, "3" },
        { "right size", NAOS_DETAIL_TEXT, "4" },
        { "left", NAOS_DETAIL_TEXT, "1" },
        { "right", NAOS_DETAIL_TEXT, "2" },
    };
    naos_diagnostic_record_t record;

    record = record_of(NAOS_REPORT_ERROR, "e", details, 4);
    NAOS_CHECK_STR_EQ(AT(3) "error: e\n  left size:  3\n  right size: 4\n  left:  1\n  right: 2\n", lines_of(&record));
}

/* Only a label with its partner in the block gets the space. */
NAOS_TEST(left_alone_not_aligned)
{
    static const naos_detail_t alone[] = {
        { "left", NAOS_DETAIL_TEXT, "1" },
        { "right size", NAOS_DETAIL_TEXT, "2" },
        { "leftover", NAOS_DETAIL_TEXT, "3" },
        { "tolerance", NAOS_DETAIL_TEXT, "4" },
    };
    naos_diagnostic_record_t record;

    record = record_of(NAOS_REPORT_ERROR, "e", alone, 4);
    NAOS_CHECK_STR_EQ(AT(3) "error: e\n  left: 1\n  right size: 2\n  leftover: 3\n  tolerance: 4\n", lines_of(&record));
}

/* A string value is quoted and escaped byte by byte; NULL is (null). */
NAOS_TEST(string_values)
{
    static const naos_detail_t details[] = {
        { "left", NAOS_DETAIL_STRING, "a\"b\\c\n\t\r\x01\x7F\xC3\xA9" },
        { "right", NAOS_DETAIL_STRING, NULL },
        { "value", NAOS_DETAIL_TEXT, "\"as is\"" },
    };
    naos_diagnostic_record_t record;

    record = record_of(NAOS_REPORT_ERROR, "e", details, 3);
    NAOS_CHECK_STR_EQ(AT(3) "error: e\n"
                      "  left:  \"a\\\"b\\\\c\\n\\t\\r\\x01\\x7F\\xC3\\xA9\"\n"
                      "  right: (null)\n"
                      "  value: \"as is\"\n",
        lines_of(&record));
}

/* The note, then the count of traces not kept, then the traces as note lines, innermost first. */
NAOS_TEST(note_and_traces)
{
    static const naos_detail_t details[] = {
        { "value", NAOS_DETAIL_TEXT, "1" },
    };
    naos_trace_t inner;
    naos_trace_t outer;
    const naos_trace_t *traces[2];
    naos_diagnostic_record_t record;

    inner.file = "f.c";
    inner.line = 5;
    memcpy(inner.text, "inner", sizeof("inner"));
    outer.file = "g.c";
    outer.line = 1;
    memcpy(outer.text, "outer", sizeof("outer"));
    traces[0] = &inner;
    traces[1] = &outer;

    record = record_of(NAOS_REPORT_ERROR, "e", details, 1);
    record.note = "the note";
    record.traces_not_kept = 2;
    record.traces = traces;
    record.trace_count = 2;

#if defined(_MSC_VER)
    NAOS_CHECK_STR_EQ(AT(3) "error: e\n  value: 1\n  note: the note\n  traces: 2 innermost not kept\n"
                      "f.c(5): note: inner\ng.c(1): note: outer\n",
        lines_of(&record));
#else
    NAOS_CHECK_STR_EQ(AT(3) "error: e\n  value: 1\n  note: the note\n  traces: 2 innermost not kept\n"
                      "f.c:5: note: inner\ng.c:1: note: outer\n",
        lines_of(&record));
#endif
}

NAOS_TEST(traces_of_a_skip)
{
    naos_trace_t trace;
    const naos_trace_t *traces[1];
    naos_diagnostic_record_t record;

    trace.file = "f.c";
    trace.line = 2;
    memcpy(trace.text, "while skipping", sizeof("while skipping"));
    traces[0] = &trace;

    record = record_of(NAOS_REPORT_SKIP, "no", NULL, 0);
    record.traces = traces;
    record.trace_count = 1;
    NAOS_CHECK_STR_EQ(AT(3) "skip: no\n" AT(2) "note: while skipping\n", lines_of(&record));
}

NAOS_SUITE(human,
    NAOS_ADD_TEST(error_line),
    NAOS_ADD_TEST(skip_line),
    NAOS_ADD_TEST(file_and_text_as_given),
    NAOS_ADD_TEST(left_lines_up_with_right),
    NAOS_ADD_TEST(left_alone_not_aligned),
    NAOS_ADD_TEST(string_values),
    NAOS_ADD_TEST(note_and_traces),
    NAOS_ADD_TEST(traces_of_a_skip)
)
