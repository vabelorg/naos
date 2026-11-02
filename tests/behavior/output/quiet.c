/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * Tests for --naos-quiet: each outcome, a skip that a failure in teardown follows, a test that
 * prints itself, and a process ending in the middle of a test.
 */

#include <naos/naos.h>

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

NAOS_TEST(passes)
{
    NAOS_CHECK_TRUE(1);
}

NAOS_TEST(fails)
{
    NAOS_CHECK_STR_EQ("hello\n", "hello");
}

NAOS_TEST(skips)
{
    NAOS_SKIP("not here");
}

/* The failure wins, and the skip after it is written as it comes. */
NAOS_TEST(fails_then_skips)
{
    NAOS_CHECK_INT_EQ(1, 2);
    NAOS_SKIP("later");
}

NAOS_TEST(fails_with_trace)
{
    NAOS_TRACE_PUSH("case %d", 3);
    NAOS_REQUIRE_TRUE(0);
}

/* What a test prints itself is written as it is, without a RUN line to tell whose it is. */
NAOS_TEST(prints)
{
    printf("printed by the test\n");
    NAOS_PASS();
}

NAOS_SUITE(results,
    NAOS_ADD_TEST(passes),
    NAOS_ADD_TEST(fails),
    NAOS_ADD_TEST(skips),
    NAOS_ADD_TEST(fails_then_skips),
    NAOS_ADD_TEST(fails_with_trace),
    NAOS_ADD_TEST(prints)
)

NAOS_SETUP(skip_in_setup)
{
    NAOS_SKIP("not applicable");
}

NAOS_TEARDOWN(check_in_teardown)
{
    NAOS_CHECK_TRUE(1 == 2);
}

NAOS_TEST(body)
{
    NAOS_CHECK_TRUE(1);
}

/* The skip of setup is held until teardown fails, then written before the failure. */
NAOS_SUITE(skip_then_teardown_fails,
    NAOS_SET_SETUP(skip_in_setup),
    NAOS_SET_TEARDOWN(check_in_teardown),
    NAOS_ADD_TEST(body)
)

NAOS_TEST(skipped_first)
{
    NAOS_SKIP("first");
}

NAOS_TEST(skipped_second)
{
    NAOS_SKIP(NULL);
}

NAOS_SUITE(skipping,
    NAOS_ADD_TEST(skipped_first),
    NAOS_ADD_TEST(skipped_second)
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
    NAOS_ADD_SUITE(skip_then_teardown_fails),
    NAOS_ADD_SUITE(skipping),
    NAOS_ADD_SUITE(crash)
)
