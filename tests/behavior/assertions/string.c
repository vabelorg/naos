/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * NAOS_*_STR_*: both values quoted and escaped on one line, and for a failed EQ of two strings the
 * first difference, as NAOS_*_MEM_EQ writes it.
 */

#include <naos/naos.h>

#include <stddef.h>
#include <string.h>

NAOS_TEST(eq)
{
    NAOS_CHECK_STR_EQ("abc", "abd");
}

/* Neither a NULL side nor a failed STR_NE has a first difference. */
NAOS_TEST(eq_null)
{
    const char *none;

    none = NULL;
    NAOS_CHECK_STR_EQ("", none);
}

NAOS_TEST(ne)
{
    NAOS_CHECK_STR_NE("abc", "abc");
}

NAOS_TEST(ne_null)
{
    const char *none;

    none = NULL;
    NAOS_REQUIRE_STR_NE(none, none);
}

/* Written escaped on one line: control bytes, quotes and backslashes. */
NAOS_TEST(escapes)
{
    NAOS_CHECK_STR_EQ("a\nb\tc\rd", "a\"b\\c\x01\x7F");
}

/* Bytes from 0x80 up are written as \xNN, whatever encoding they belong to. */
NAOS_TEST(non_ascii)
{
    NAOS_CHECK_STR_EQ("\xE3\x81\x82", "\xFF");
}

/* The quotes show where the value ends, so a trailing space is visible. */
NAOS_TEST(trailing_space)
{
    NAOS_CHECK_STR_EQ("abc ", "abc");
}

/* The left value is a prefix of the right one, so the difference is where the left one ends. */
NAOS_TEST(left_ends)
{
    NAOS_CHECK_STR_EQ("ab", "abc");
}

/* A long value is written in full, not cut short. */
NAOS_TEST(long_value)
{
    char value[1002];

    memset(value, 'a', 1000);
    value[1000] = 'Z';
    value[1001] = '\0';
    NAOS_CHECK_STR_EQ(value, "a");
}

NAOS_SUITE(string,
    NAOS_ADD_TEST(eq),
    NAOS_ADD_TEST(eq_null),
    NAOS_ADD_TEST(ne),
    NAOS_ADD_TEST(ne_null),
    NAOS_ADD_TEST(escapes),
    NAOS_ADD_TEST(non_ascii),
    NAOS_ADD_TEST(trailing_space),
    NAOS_ADD_TEST(left_ends),
    NAOS_ADD_TEST(long_value)
)
