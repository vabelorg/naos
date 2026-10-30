/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * NAOS_NOTE: written by the next assertion if it fails, as a detail line after its values, and used
 * up by it either way. Each test, and its teardown, starts without one.
 */

#include <naos/naos.h>

#include <stdio.h>

NAOS_TEST(in_loop)
{
    int i;

    for (i = 0; i < 5; i++)
    {
        NAOS_NOTE("i=%d of %s", i, "loop");
        NAOS_CHECK_INT_LT(i, 3);
    }
}

NAOS_TEST(used_up_by_pass)
{
    NAOS_NOTE("stale-note");
    NAOS_CHECK_TRUE(1);
    NAOS_CHECK_TRUE(0);
}

NAOS_TEST(replaced)
{
    NAOS_NOTE("first-note");
    NAOS_NOTE("second-note");
    NAOS_CHECK_TRUE(0);
    NAOS_CHECK_TRUE(0);
}

NAOS_TEST(on_require)
{
    NAOS_NOTE("before require");
    NAOS_REQUIRE_STR_EQ("a", "b");
}

NAOS_TEST(on_fail)
{
    NAOS_NOTE("context %d", 42);
    NAOS_FAIL("explicit failure");
}

NAOS_TEST(on_skip)
{
    NAOS_NOTE("why skipped");
    NAOS_SKIP("not applicable");
}

/* Longer than the buffer of the note: the end shows "...". */
NAOS_TEST(cut_short)
{
    NAOS_NOTE("%0600d", 7);
    NAOS_CHECK_TRUE(0);
}

NAOS_TEST(with_double)
{
    NAOS_NOTE("double-note");
    NAOS_CHECK_DOUBLE_NEAR(1.0, 2.0, 0.5);
}

NAOS_TEST(with_mem)
{
    static const unsigned char expected[3] = { 0x01, 0x02, 0x03 };
    static const unsigned char actual[3] = { 0x01, 0xFF, 0x03 };

    NAOS_NOTE("mem-note");
    NAOS_CHECK_MEM_EQ(expected, sizeof(expected), actual, sizeof(actual));
}

NAOS_SUITE(note,
    NAOS_ADD_TEST(in_loop),
    NAOS_ADD_TEST(used_up_by_pass),
    NAOS_ADD_TEST(replaced),
    NAOS_ADD_TEST(on_require),
    NAOS_ADD_TEST(on_fail),
    NAOS_ADD_TEST(on_skip),
    NAOS_ADD_TEST(cut_short),
    NAOS_ADD_TEST(with_double),
    NAOS_ADD_TEST(with_mem)
)

NAOS_TEST(leaves_note)
{
    NAOS_NOTE("leaked-note");
}

NAOS_TEST(fails_without_note)
{
    NAOS_CHECK_TRUE(0);
}

/* A note left unused by one test does not reach the next one. */
NAOS_SUITE(note_reset,
    NAOS_ADD_TEST(leaves_note),
    NAOS_ADD_TEST(fails_without_note)
)

NAOS_TEARDOWN(check_in_teardown)
{
    NAOS_CHECK_TRUE(1 == 2);
    printf("teardown-ran\n");
}

/* Nor does teardown see a note left unused by the body. */
NAOS_SUITE(note_before_teardown,
    NAOS_SET_TEARDOWN(check_in_teardown),
    NAOS_ADD_TEST(leaves_note)
)
