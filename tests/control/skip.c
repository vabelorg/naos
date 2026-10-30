/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * NAOS_SKIP marks the test skipped and stops it; a failure recorded before still wins.
 */

#include <naos/naos.h>

#include <stdio.h>

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

static void skip_in_helper(void)
{
    NAOS_SKIP("helper skip");
}

NAOS_TEST(stops)
{
    NAOS_SKIP("not applicable");
    printf("reached-after-skip\n");
}

NAOS_TEST(from_helper)
{
    skip_in_helper();
    printf("reached-after-helper\n");
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

NAOS_TEST(failure_wins)
{
    NAOS_CHECK_TRUE(0);
    NAOS_SKIP("too late");
}

NAOS_SUITE(skip,
    NAOS_ADD_TEST(stops),
    NAOS_ADD_TEST(from_helper),
    NAOS_ADD_TEST(failure_wins)
)
