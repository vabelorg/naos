/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * The writer of the JUnit XML file, driven through its events (writer.h) as report.c drives it, and
 * the file it writes read back. Each test writes a file of its own in the working directory, which
 * CTest makes the build directory of unit/. The escapes are covered here, byte by byte, rather than
 * by the golden files, which need a test program that fails with each text.
 */

/* fopen() is standard C; MSVC would warn that fopen_s() is safer. */
#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <naos/naos.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "writer.h"
#include "xml.h"

#define FILE_SIZE 8192

static char g_file_text[FILE_SIZE];

static const naos_round_t g_one_round = { 1, 1, 0, 0 };
static const naos_tally_t g_no_tally = NAOS_TALLY_EMPTY;

/* Reads the file at path into g_file_text, or makes it empty when there is none. */
static const char *read_back(const char *path)
{
    FILE *file;
    size_t length;

    g_file_text[0] = '\0';
    file = fopen(path, "rb");
    if (file == NULL)
    {
        return g_file_text;
    }

    length = fread(g_file_text, 1, sizeof(g_file_text) - 1, file);
    g_file_text[length] = '\0';
    fclose(file);
    remove(path);

    return g_file_text;
}

/* A record of kind for text, at file and line, with nothing else. */
static naos_diagnostic_record_t record_of(naos_report_kind_t kind, const char *file, int line, const char *text)
{
    naos_diagnostic_record_t record;

    memset(&record, 0, sizeof(record));
    record.kind = kind;
    record.file = file;
    record.line = line;
    record.text = text;

    return record;
}

/* Writes one round of one test of the suite s, named t, with one diagnostic when record is not NULL. */
static const char *one_test(const char *path, naos_outcome_t outcome, const naos_diagnostic_record_t *record, naos_nanoseconds_t elapsed)
{
    const naos_writer_t *writer;
    int written;

    writer = naos_xml_writer();
    NAOS_REQUIRE_INT_EQ(1, writer->begin("prog", path));
    writer->round_begin(&g_one_round);
    writer->test_begin("s", "t");
    if (record != NULL)
    {
        writer->diagnostic(record);
    }
    writer->test_end("s", "t", outcome, (outcome == NAOS_OUTCOME_FAIL) ? 1 : 0, elapsed);
    writer->round_end(&g_one_round, &g_no_tally, 0);
    written = writer->end(0, 1, 0);
    NAOS_CHECK_INT_EQ(1, written);

    return read_back(path);
}

/* The <failure> of a test that fails with text, at f.c:7. */
static const char *failure_of(const char *path, const char *text)
{
    naos_diagnostic_record_t record;

    record = record_of(NAOS_REPORT_ERROR, "f.c", 7, text);

    return one_test(path, NAOS_OUTCOME_FAIL, &record, 0);
}

/* Whether text holds part, noting text when it does not. */
static int has(const char *text, const char *part)
{
    if (strstr(text, part) != NULL)
    {
        return 1;
    }

    NAOS_NOTE("%s", text);

    return 0;
}

#if defined(_MSC_VER)
#define LOCATION "f.c(7): "
#else
#define LOCATION "f.c:7: "
#endif

NAOS_TEST(passed_test)
{
    NAOS_CHECK_STR_EQ(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<testsuites name=\"prog\" tests=\"1\" failures=\"0\" errors=\"0\" skipped=\"0\" time=\"0.003\">\n"
        "  <testsuite name=\"s\" tests=\"1\" failures=\"0\" errors=\"0\" skipped=\"0\" time=\"0.003\">\n"
        "    <testcase classname=\"s\" name=\"t\" time=\"0.003\"/>\n"
        "  </testsuite>\n"
        "</testsuites>\n",
        one_test("unit_xml_passed.xml", NAOS_OUTCOME_PASS, NULL, 3999999ULL));
}

NAOS_TEST(failed_test)
{
    NAOS_CHECK_STR_EQ(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<testsuites name=\"prog\" tests=\"1\" failures=\"1\" errors=\"0\" skipped=\"0\" time=\"0.000\">\n"
        "  <testsuite name=\"s\" tests=\"1\" failures=\"1\" errors=\"0\" skipped=\"0\" time=\"0.000\">\n"
        "    <testcase classname=\"s\" name=\"t\" time=\"0.000\" file=\"f.c\" line=\"7\">\n"
        "      <failure type=\"error\" message=\"a == b\">" LOCATION "error: a == b\n</failure>\n"
        "    </testcase>\n"
        "  </testsuite>\n"
        "</testsuites>\n",
        failure_of("unit_xml_failed.xml", "a == b"));
}

NAOS_TEST(skipped_test)
{
    naos_diagnostic_record_t record;
    const char *xml;

    record = record_of(NAOS_REPORT_SKIP, "f.c", 9, "not here");
    xml = one_test("unit_xml_skipped.xml", NAOS_OUTCOME_SKIP, &record, 0);

    NAOS_CHECK_TRUE(has(xml, "skipped=\"1\" time=\"0.000\">\n"));
    NAOS_CHECK_TRUE(has(xml,
        "    <testcase classname=\"s\" name=\"t\" time=\"0.000\" file=\"f.c\" line=\"9\">\n"
        "      <skipped message=\"not here\"/>\n"
        "    </testcase>\n"));
}

/* Seconds with three decimals, rounded down to the millisecond. */
NAOS_TEST(times)
{
    NAOS_CHECK_TRUE(has(one_test("unit_xml_times_0.xml", NAOS_OUTCOME_PASS, NULL, 999999ULL), "name=\"t\" time=\"0.000\"/>"));
    NAOS_CHECK_TRUE(has(one_test("unit_xml_times_1.xml", NAOS_OUTCOME_PASS, NULL, 1000000ULL), "name=\"t\" time=\"0.001\"/>"));
    NAOS_CHECK_TRUE(has(one_test("unit_xml_times_2.xml", NAOS_OUTCOME_PASS, NULL, 3120000000ULL), "name=\"t\" time=\"3.120\"/>"));
    NAOS_CHECK_TRUE(has(one_test("unit_xml_times_3.xml", NAOS_OUTCOME_PASS, NULL, 61000000000ULL), "name=\"t\" time=\"61.000\"/>"));
}

/* The name of <testsuites> is the program without its directory and without ".exe", in any case. */
NAOS_TEST(program_name)
{
    static const char *const programs[][2] = {
        { "/a/b/tests", "tests" },
        { "C:\\a\\b\\tests.exe", "tests" },
        { "tests.EXE", "tests" },
        { ".exe", ".exe" },
        { "a&b", "a&amp;b" },
    };
    const naos_writer_t *writer;
    char expected[64];
    size_t i;

    writer = naos_xml_writer();
    for (i = 0; i < sizeof(programs) / sizeof(programs[0]); i++)
    {
        NAOS_TRACE_PUSH("program %s", programs[i][0]);
        NAOS_REQUIRE_INT_EQ(1, writer->begin(programs[i][0], "unit_xml_program.xml"));
        NAOS_REQUIRE_INT_EQ(1, writer->end(0, 1, 0));
        snprintf(expected, sizeof(expected), "<testsuites name=\"%s\" tests=\"0\"", programs[i][1]);
        NAOS_CHECK_TRUE(has(read_back("unit_xml_program.xml"), expected));
        NAOS_TRACE_POP();
    }

    NAOS_REQUIRE_INT_EQ(1, writer->begin(NULL, "unit_xml_program.xml"));
    NAOS_REQUIRE_INT_EQ(1, writer->end(0, 1, 0));
    NAOS_CHECK_TRUE(has(read_back("unit_xml_program.xml"), "<testsuites name=\"naos\" tests=\"0\""));
}

/* No round: the file holds <testsuites> alone. */
NAOS_TEST(no_test)
{
    const naos_writer_t *writer;

    writer = naos_xml_writer();
    NAOS_REQUIRE_INT_EQ(1, writer->begin("prog", "unit_xml_no_test.xml"));
    NAOS_REQUIRE_INT_EQ(1, writer->end(0, 1, 0));
    NAOS_CHECK_STR_EQ(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<testsuites name=\"prog\" tests=\"0\" failures=\"0\" errors=\"0\" skipped=\"0\" time=\"0.000\">\n"
        "</testsuites>\n",
        read_back("unit_xml_no_test.xml"));
}

NAOS_TEST(cannot_open)
{
    NAOS_CHECK_INT_EQ(0, naos_xml_writer()->begin("prog", "no_such_directory/unit_xml.xml"));
}

/* In an attribute, the markup, the quotes and the white space that a parser would turn into spaces. */
NAOS_TEST(escapes_in_attributes)
{
    NAOS_CHECK_TRUE(has(failure_of("unit_xml_attribute.xml", "<a & \"b\" 'c'>\n\t\r"),
        "message=\"&lt;a &amp; &quot;b&quot; &apos;c&apos;&gt;&#10;&#9;&#13;\">"));
}

/* In the text of an element, the markup and carriage returns only; ]]> cannot end a section there. */
NAOS_TEST(escapes_in_text)
{
    NAOS_CHECK_TRUE(has(failure_of("unit_xml_text.xml", "<a & \"b\" 'c'> ]]>\n\tx\ry"),
        "error: &lt;a &amp; \"b\" 'c'&gt; ]]&gt;\n\tx&#13;y\n</failure>"));
}

/* Valid UTF-8 is kept as it is, from two bytes to four. */
NAOS_TEST(keeps_valid_utf8)
{
    NAOS_CHECK_TRUE(has(failure_of("unit_xml_utf8.xml", "\xC3\xA9 \xE3\x81\x82 \xF0\x9F\x98\x80 \xEF\xBF\xBD"),
        "message=\"\xC3\xA9 \xE3\x81\x82 \xF0\x9F\x98\x80 \xEF\xBF\xBD\""));
}

/* What XML 1.0 does not allow, or what is not UTF-8, is written byte by byte as \xNN. */
NAOS_TEST(escapes_what_xml_does_not_allow)
{
    static const char *const cases[][2] = {
        { "a\x01" "b", "a\\x01b" },                      /* a control character */
        { "\x1F", "\\x1F" },
        { "\x7F", "\x7F" },                              /* DEL is allowed */
        { "\xFF", "\\xFF" },                             /* a byte that never starts a sequence */
        { "\x80", "\\x80" },                             /* a continuation byte alone */
        { "\xC0\x80", "\\xC0\\x80" },                    /* an overlong NUL */
        { "\xE0\x80\x80", "\\xE0\\x80\\x80" },           /* an overlong sequence of three bytes */
        { "\xED\xA0\x80", "\\xED\\xA0\\x80" },           /* a surrogate */
        { "\xEF\xBF\xBE", "\\xEF\\xBF\\xBE" },           /* U+FFFE */
        { "\xEF\xBF\xBF", "\\xEF\\xBF\\xBF" },           /* U+FFFF */
        { "\xF4\x90\x80\x80", "\\xF4\\x90\\x80\\x80" },  /* beyond U+10FFFF */
        { "\xE3\x81", "\\xE3\\x81" },                    /* cut short by the end */
        { "\xE3\x81" "a", "\\xE3\\x81a" },               /* cut short by another byte */
    };
    char expected[64];
    size_t i;

    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); i++)
    {
        NAOS_TRACE_PUSH("case %u", (unsigned int)i);
        snprintf(expected, sizeof(expected), "message=\"%s\"", cases[i][1]);
        NAOS_CHECK_TRUE(has(failure_of("unit_xml_bytes.xml", cases[i][0]), expected));
        NAOS_TRACE_POP();
    }
}

/* Each suite of each round has its counts; the file has the sum of them. */
NAOS_TEST(rounds_and_suites)
{
    static const naos_round_t first = { 1, 2, 1, 7 };
    static const naos_round_t second = { 2, 2, 1, 8 };
    const naos_writer_t *writer;
    naos_diagnostic_record_t record;
    const char *xml;

    writer = naos_xml_writer();
    record = record_of(NAOS_REPORT_ERROR, "f.c", 7, "x");
    NAOS_REQUIRE_INT_EQ(1, writer->begin("prog", "unit_xml_rounds.xml"));

    writer->round_begin(&first);
    writer->test_begin("a", "one");
    writer->test_end("a", "one", NAOS_OUTCOME_PASS, 0, 1000000ULL);
    writer->test_begin("b", "two");
    writer->diagnostic(&record);
    writer->test_end("b", "two", NAOS_OUTCOME_FAIL, 1, 2000000ULL);
    writer->round_end(&first, &g_no_tally, 0);

    writer->round_begin(&second);
    writer->test_begin("a", "one");
    writer->test_end("a", "one", NAOS_OUTCOME_PASS, 0, 4000000ULL);
    writer->round_end(&second, &g_no_tally, 0);

    NAOS_REQUIRE_INT_EQ(1, writer->end(1, 2, 0));
    xml = read_back("unit_xml_rounds.xml");

    NAOS_CHECK_TRUE(has(xml, "<testsuites name=\"prog\" tests=\"3\" failures=\"1\" errors=\"0\" skipped=\"0\" time=\"0.007\">"));
    NAOS_CHECK_TRUE(has(xml,
        "  <testsuite name=\"a\" tests=\"1\" failures=\"0\" errors=\"0\" skipped=\"0\" time=\"0.001\">\n"
        "    <properties>\n"
        "      <property name=\"iteration\" value=\"1\"/>\n"
        "      <property name=\"random_seed\" value=\"7\"/>\n"
        "    </properties>\n"
        "    <testcase classname=\"a\" name=\"one\" time=\"0.001\"/>\n"
        "  </testsuite>\n"
        "  <testsuite name=\"b\" tests=\"1\" failures=\"1\""));
    NAOS_CHECK_TRUE(has(xml,
        "      <property name=\"iteration\" value=\"2\"/>\n"
        "      <property name=\"random_seed\" value=\"8\"/>\n"));
}

/* The time of a suite is the sum of the exact times of its tests, rounded down once. */
NAOS_TEST(suite_time_rounded_once)
{
    const naos_writer_t *writer;
    const char *xml;

    writer = naos_xml_writer();
    NAOS_REQUIRE_INT_EQ(1, writer->begin("prog", "unit_xml_sum.xml"));
    writer->round_begin(&g_one_round);
    writer->test_begin("s", "one");
    writer->test_end("s", "one", NAOS_OUTCOME_PASS, 0, 1500000ULL);
    writer->test_begin("s", "two");
    writer->test_end("s", "two", NAOS_OUTCOME_PASS, 0, 1500000ULL);
    writer->round_end(&g_one_round, &g_no_tally, 0);
    NAOS_REQUIRE_INT_EQ(1, writer->end(0, 1, 0));
    xml = read_back("unit_xml_sum.xml");

    NAOS_CHECK_TRUE(has(xml, "<testsuite name=\"s\" tests=\"2\" failures=\"0\" errors=\"0\" skipped=\"0\" time=\"0.003\">"));
    NAOS_CHECK_TRUE(has(xml, "name=\"one\" time=\"0.001\"/>"));
}

/* end() leaves nothing for the next file: one process may write several. */
NAOS_TEST(next_file_starts_clean)
{
    naos_diagnostic_record_t record;
    const char *xml;

    record = record_of(NAOS_REPORT_ERROR, "f.c", 7, "first");
    (void)one_test("unit_xml_first.xml", NAOS_OUTCOME_FAIL, &record, 0);
    xml = one_test("unit_xml_second.xml", NAOS_OUTCOME_PASS, NULL, 0);

    NAOS_CHECK_TRUE(strstr(xml, "first") == NULL);
    NAOS_CHECK_TRUE(has(xml, "<testsuites name=\"prog\" tests=\"1\" failures=\"0\""));
}

NAOS_SUITE(xml,
    NAOS_ADD_TEST(passed_test),
    NAOS_ADD_TEST(failed_test),
    NAOS_ADD_TEST(skipped_test),
    NAOS_ADD_TEST(times),
    NAOS_ADD_TEST(program_name),
    NAOS_ADD_TEST(no_test),
    NAOS_ADD_TEST(cannot_open),
    NAOS_ADD_TEST(escapes_in_attributes),
    NAOS_ADD_TEST(escapes_in_text),
    NAOS_ADD_TEST(keeps_valid_utf8),
    NAOS_ADD_TEST(escapes_what_xml_does_not_allow),
    NAOS_ADD_TEST(rounds_and_suites),
    NAOS_ADD_TEST(suite_time_rounded_once),
    NAOS_ADD_TEST(next_file_starts_clean)
)
