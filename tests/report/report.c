/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * Tests whose results the JUnit XML file of --naos-output holds: each outcome, values, notes and
 * traces, the text that XML must escape, and a process ending in the middle of a test.
 */

#include <naos/naos.h>

#include <stdlib.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

NAOS_TEST(passes)
{
    NAOS_CHECK_TRUE(1);
}

/* Each failure of the test is a <failure> of its own. */
NAOS_TEST(fails_twice)
{
    NAOS_CHECK_INT_EQ(1, 2);
    NAOS_CHECK_STR_EQ("hello\n", "hello");
}

NAOS_TEST(skips)
{
    NAOS_SKIP("not here");
}

/* The failure wins: the test case has the <failure>, not <skipped>. */
NAOS_TEST(fails_then_skips)
{
    NAOS_CHECK_TRUE(0);
    NAOS_SKIP("later");
}

NAOS_TEST(with_context)
{
    NAOS_TRACE_PUSH("outer");
    NAOS_TRACE_PUSH("inner %d", 2);
    NAOS_NOTE("locale %s", "C");
    NAOS_CHECK_DOUBLE_NEAR(0.1, 0.25, 1e-3);
    NAOS_CHECK_MEM_EQ("ab", 2, "ac", 2);
    NAOS_TRACE_POP();
    NAOS_TRACE_POP();
}

/* 17 levels: the innermost one is only counted. */
NAOS_TEST(too_deep)
{
    int i;

    for (i = 0; i < 17; i++)
    {
        NAOS_TRACE_PUSH("level %d", i);
    }

    NAOS_CHECK_TRUE(0);

    for (i = 0; i < 17; i++)
    {
        NAOS_TRACE_POP();
    }
}

NAOS_SUITE(results,
    NAOS_ADD_TEST(passes),
    NAOS_ADD_TEST(fails_twice),
    NAOS_ADD_TEST(skips),
    NAOS_ADD_TEST(fails_then_skips),
    NAOS_ADD_TEST(with_context),
    NAOS_ADD_TEST(too_deep)
)

NAOS_TEST(markup)
{
    NAOS_FAIL("<a & \"b\" 'c'>");
}

/* Valid UTF-8 is kept as it is: U+65E5 U+672C. */
NAOS_TEST(utf8)
{
    NAOS_FAIL("\xE6\x97\xA5\xE6\x9C\xAC");
}

NAOS_TEST(invalid_utf8)
{
    NAOS_FAIL("bad \xFF and \xE6\x97 bytes");
}

/* A line break and a tab survive in an attribute; other control characters are not allowed in XML. */
NAOS_TEST(control)
{
    NAOS_FAIL("one\ntwo\tthree\r\x01");
}

/* A string value is written as the human-readable output writes it, then escaped for XML. */
NAOS_TEST(string_value)
{
    NAOS_CHECK_STR_EQ("\xE6\x97\xA5 <&>", "x");
}

NAOS_TEST(skip_markup)
{
    NAOS_SKIP("needs <a> & \"b\"");
}

NAOS_SUITE(text,
    NAOS_ADD_TEST(markup),
    NAOS_ADD_TEST(utf8),
    NAOS_ADD_TEST(invalid_utf8),
    NAOS_ADD_TEST(control),
    NAOS_ADD_TEST(string_value),
    NAOS_ADD_TEST(skip_markup)
)

/*
 * Ends the process with exit code 3 without writing out the buffers of stdio. _Exit of the Windows
 * CRT still writes them out, and only TerminateProcess leaves them.
 */
static void exit_without_flush(void)
{
#ifdef _WIN32
    TerminateProcess(GetCurrentProcess(), 3);
#endif
    _Exit(3);
}

/* The full run leaves this suite out, since its test ends the process. */
NAOS_TEST(exits)
{
    exit_without_flush();
}

NAOS_SUITE(crash,
    NAOS_ADD_TEST(exits)
)

NAOS_MAIN(
    NAOS_ADD_SUITE(results),
    NAOS_ADD_SUITE(text),
    NAOS_ADD_SUITE(crash)
)
