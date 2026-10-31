/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * NAOS_CHECK_* records a failure and goes on, in the test as in a helper.
 */

#include <naos/naos.h>

#include <stdio.h>

static void check_in_helper(int value)
{
    NAOS_CHECK_INT_EQ(1, value);
}

NAOS_TEST(goes_on)
{
    NAOS_CHECK_TRUE(1 == 2);
    printf("reached-after-check\n");
}

NAOS_TEST(false_goes_on)
{
    int done;

    done = 1;
    NAOS_CHECK_FALSE(done);
    printf("reached-after-check\n");
}

NAOS_TEST(in_helper_goes_on)
{
    check_in_helper(2);
    printf("reached-after-helper\n");
}

NAOS_SUITE(check,
    NAOS_ADD_TEST(goes_on),
    NAOS_ADD_TEST(false_goes_on),
    NAOS_ADD_TEST(in_helper_goes_on)
)
