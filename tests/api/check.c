/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * naos/check.h: assertions given matching values do not fail, whatever their form.
 */

#include <naos/naos.h>

#include <math.h>
#include <stddef.h>
#include <stdint.h>

/* The boolean assertions are NAOS_CHECK_TRUE and the like; the old names must stay removed. */
#if defined(NAOS_CHECK) || defined(NAOS_REQUIRE)
#error "NAOS_CHECK and NAOS_REQUIRE were removed: use NAOS_CHECK_TRUE, NAOS_CHECK_FALSE and their NAOS_REQUIRE_ forms"
#endif

NAOS_TEST(condition)
{
    int value;
    int calls;
    int *some;
    int *none;

    value = 3;
    some = &value;
    none = NULL;
    NAOS_CHECK_TRUE(value == 3);
    NAOS_REQUIRE_TRUE(value > 0);
    NAOS_CHECK_FALSE(value == 4);
    NAOS_REQUIRE_FALSE(value < 0);

    /* Any non-zero value is true, pointers included. */
    NAOS_CHECK_TRUE(5);
    NAOS_CHECK_TRUE(some);
    NAOS_CHECK_FALSE(0);
    NAOS_CHECK_FALSE(none);

    /* Each argument is evaluated once. */
    calls = 0;
    NAOS_CHECK_TRUE(++calls);
    NAOS_CHECK_FALSE(--calls);
    NAOS_CHECK_INT_EQ(0, calls);
}

NAOS_TEST(int_eq)
{
    NAOS_CHECK_INT_EQ(-5, -5);
    NAOS_CHECK_INT_EQ(INTMAX_MIN, INTMAX_MIN);
    NAOS_REQUIRE_INT_EQ(INTMAX_MAX, INTMAX_MAX);
}

NAOS_TEST(uint_eq)
{
    NAOS_CHECK_UINT_EQ(0u, 0u);
    NAOS_REQUIRE_UINT_EQ(UINTMAX_MAX, UINTMAX_MAX);
}

NAOS_TEST(int_compare)
{
    NAOS_CHECK_INT_NE(-1, 1);
    NAOS_CHECK_INT_LT(INTMAX_MIN, INTMAX_MAX);
    NAOS_CHECK_INT_LE(-2, -2);
    NAOS_CHECK_INT_LE(-3, -2);
    NAOS_CHECK_INT_GT(0, -1);
    NAOS_CHECK_INT_GE(5, 5);
    NAOS_CHECK_INT_GE(6, 5);
    NAOS_REQUIRE_INT_NE(0, 1);
    NAOS_REQUIRE_INT_LT(-1, 0);
    NAOS_REQUIRE_INT_LE(0, 0);
    NAOS_REQUIRE_INT_GT(1, 0);
    NAOS_REQUIRE_INT_GE(0, 0);
}

NAOS_TEST(uint_compare)
{
    /* Compared as uintmax_t, not as signed values. */
    NAOS_CHECK_UINT_NE(0u, UINTMAX_MAX);
    NAOS_CHECK_UINT_LT(0u, UINTMAX_MAX);
    NAOS_CHECK_UINT_LE(7u, 7u);
    NAOS_CHECK_UINT_GT(UINTMAX_MAX, 0u);
    NAOS_CHECK_UINT_GE(UINTMAX_MAX, UINTMAX_MAX);
    NAOS_REQUIRE_UINT_NE(1u, 2u);
    NAOS_REQUIRE_UINT_LT(1u, 2u);
    NAOS_REQUIRE_UINT_LE(2u, 2u);
    NAOS_REQUIRE_UINT_GT(2u, 1u);
    NAOS_REQUIRE_UINT_GE(2u, 1u);
}

NAOS_TEST(double_near)
{
    double infinity;

    infinity = HUGE_VAL;
    NAOS_CHECK_DOUBLE_NEAR(0.3, 0.1 + 0.2, 1e-12);
    NAOS_CHECK_DOUBLE_NEAR(1.5, 1.5, 0.0);
    NAOS_REQUIRE_DOUBLE_NEAR(infinity, infinity, 0.0);
}

NAOS_TEST(str_eq)
{
    const char *none;

    none = NULL;
    NAOS_CHECK_STR_EQ("abc", "abc");
    NAOS_CHECK_STR_EQ("", "");
    NAOS_REQUIRE_STR_EQ(none, none);
}

NAOS_TEST(str_ne)
{
    const char *none;

    none = NULL;
    NAOS_CHECK_STR_NE("abc", "abd");
    NAOS_CHECK_STR_NE("", none);
    NAOS_REQUIRE_STR_NE(none, "");
}

NAOS_TEST(mem_eq)
{
    /* Deliberately not NUL-terminated. */
    static const char expected[3] = { 'a', '\0', 'b' };
    char actual[3];

    actual[0] = 'a';
    actual[1] = '\0';
    actual[2] = 'b';
    NAOS_CHECK_MEM_EQ(expected, sizeof(expected), actual, sizeof(actual));
    NAOS_REQUIRE_MEM_EQ(NULL, 0, NULL, 0);
}

NAOS_TEST(null)
{
    int value;
    int *none;

    value = 0;
    none = NULL;
    NAOS_CHECK_NULL(none);
    NAOS_CHECK_NOT_NULL(&value);
    NAOS_REQUIRE_NOT_NULL(&value);
}

NAOS_TEST(ptr_eq)
{
    static const int values[2] = { 1, 2 };
    const int *none;

    none = NULL;
    NAOS_CHECK_PTR_EQ(&values[1], values + 1);
    NAOS_CHECK_PTR_EQ(NULL, none);
    NAOS_REQUIRE_PTR_EQ(values, &values[0]);
}

static void passing_helper(void)
{
    NAOS_REQUIRE_TRUE(1 == 1);
}

NAOS_TEST(passing_helper_returns)
{
    passing_helper();
}

NAOS_SUITE(check,
    NAOS_ADD_TEST(condition),
    NAOS_ADD_TEST(int_eq),
    NAOS_ADD_TEST(uint_eq),
    NAOS_ADD_TEST(int_compare),
    NAOS_ADD_TEST(uint_compare),
    NAOS_ADD_TEST(double_near),
    NAOS_ADD_TEST(str_eq),
    NAOS_ADD_TEST(str_ne),
    NAOS_ADD_TEST(mem_eq),
    NAOS_ADD_TEST(ptr_eq),
    NAOS_ADD_TEST(null),
    NAOS_ADD_TEST(passing_helper_returns)
)
