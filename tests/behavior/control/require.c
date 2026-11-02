/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * NAOS_REQUIRE_* records a failure and stops the test, however deep in helpers.
 */

#include <naos/naos.h>

#include <stdio.h>

static void require_in_helper(int value)
{
    NAOS_REQUIRE_INT_EQ(1, value);
    printf("reached-after-require\n");
}

static void require_three_deep(int value)
{
    NAOS_REQUIRE_INT_EQ(1, value);
    printf("reached-after-require\n");
}

static void require_two_deep(int value)
{
    require_three_deep(value);
    printf("reached-after-three\n");
}

static void require_one_deep(int value)
{
    require_two_deep(value);
    printf("reached-after-two\n");
}

NAOS_TEST(stops)
{
    NAOS_REQUIRE_TRUE(1 == 2);
    printf("reached-after-require\n");
}

NAOS_TEST(false_stops)
{
    int done;

    done = 1;
    NAOS_REQUIRE_FALSE(done);
    printf("reached-after-require\n");
}

NAOS_TEST(int_eq_stops)
{
    NAOS_REQUIRE_INT_EQ(1, 2);
    printf("reached-after-require\n");
}

NAOS_TEST(int_lt_stops)
{
    NAOS_REQUIRE_INT_LT(2, 1);
    printf("reached-after-require\n");
}

NAOS_TEST(uint_ge_stops)
{
    NAOS_REQUIRE_UINT_GE(1u, 2u);
    printf("reached-after-require\n");
}

NAOS_TEST(leaves_helper)
{
    require_in_helper(2);
    printf("reached-after-helper\n");
}

NAOS_TEST(leaves_nested_helpers)
{
    require_one_deep(2);
    printf("reached-after-helper\n");
}

/* Each form is an expression of type void, usable in a comma expression. */
NAOS_TEST(as_expression)
{
    int value;

    (void)NAOS_REQUIRE_TRUE(1 == 1);
    value = (NAOS_REQUIRE_INT_EQ(1, 1), 5);
    NAOS_CHECK_INT_EQ(5, value);
    (void)(NAOS_REQUIRE_TRUE(1 == 2), printf("reached-after-require\n"));
}

NAOS_SUITE(require,
    NAOS_ADD_TEST(stops),
    NAOS_ADD_TEST(false_stops),
    NAOS_ADD_TEST(int_eq_stops),
    NAOS_ADD_TEST(int_lt_stops),
    NAOS_ADD_TEST(uint_ge_stops),
    NAOS_ADD_TEST(leaves_helper),
    NAOS_ADD_TEST(leaves_nested_helpers),
    NAOS_ADD_TEST(as_expression)
)
