/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * NAOS_*_PTR_EQ, NAOS_*_NULL and NAOS_*_NOT_NULL. The values of pointers depend on the platform, so
 * the cases of PTR_EQ are matched by regexes; NULL and NOT_NULL write no value.
 */

#include <naos/naos.h>

#include <stddef.h>

NAOS_TEST(eq)
{
    static const int values[2] = { 1, 2 };

    NAOS_CHECK_PTR_EQ(&values[0], &values[1]);
}

NAOS_TEST(eq_null)
{
    int value;

    value = 0;
    NAOS_REQUIRE_PTR_EQ(NULL, &value);
}

/* A failed NULL writes no value either: the pointer says little, and changes from run to run. */
NAOS_TEST(null)
{
    int value;

    value = 0;
    NAOS_CHECK_NULL(&value);
}

/* A failed NOT_NULL is always NULL, so it writes no value. */
NAOS_TEST(not_null)
{
    int *none;

    none = NULL;
    NAOS_CHECK_NOT_NULL(none);
}

NAOS_SUITE(pointer,
    NAOS_ADD_TEST(eq),
    NAOS_ADD_TEST(eq_null),
    NAOS_ADD_TEST(null),
    NAOS_ADD_TEST(not_null)
)
