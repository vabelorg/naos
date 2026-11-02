/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * NAOS_FAIL records a fatal failure and stops the test, from the test or from a helper.
 */

#include <naos/naos.h>

#include <stddef.h>
#include <stdio.h>

/* Ends with NAOS_FAIL rather than return: NAOS_NORETURN keeps -Wreturn-type and C4715 quiet. */
static int find_index(int value)
{
    if (value == 1)
    {
        return 0;
    }

    NAOS_FAIL("not found");
}

/*
 * The printf calls after these helpers are unreachable on purpose: the expected output shows they
 * print nothing. Optimized builds of MSVC inline the helpers, see that the call does not return and
 * warn about them (C4702). C4702 comes from the optimizer, so the pragma has to cover the whole
 * functions.
 */
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4702)
#endif

static void fail_in_helper(void)
{
    NAOS_FAIL("helper failure");
}

NAOS_TEST(stops)
{
    NAOS_FAIL("explicit failure");
    printf("reached-after-fail\n");
}

NAOS_TEST(from_helper)
{
    fail_in_helper();
    printf("reached-after-helper\n");
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

NAOS_TEST(no_message)
{
    NAOS_FAIL(NULL);
}

NAOS_TEST(ends_value_helper)
{
    int index;

    index = find_index(1);
    NAOS_CHECK_INT_EQ(0, index);
    index = find_index(2);
    printf("reached-after-helper %d\n", index);
}

NAOS_SUITE(fail,
    NAOS_ADD_TEST(stops),
    NAOS_ADD_TEST(no_message),
    NAOS_ADD_TEST(from_helper),
    NAOS_ADD_TEST(ends_value_helper)
)
