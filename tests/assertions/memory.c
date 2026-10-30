/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * NAOS_*_MEM_EQ: both sizes, lined up as left and right, then the first difference or what is
 * invalid.
 */

#include <naos/naos.h>

#include <stddef.h>

/* The bytes themselves are not written. */
NAOS_TEST(byte)
{
    static const unsigned char expected[3] = { 0x01, 0x02, 0x03 };
    static const unsigned char actual[3] = { 0x01, 0xFF, 0x03 };

    NAOS_CHECK_MEM_EQ(expected, sizeof(expected), actual, sizeof(actual));
}

NAOS_TEST(right_shorter)
{
    static const char bytes[3] = { 'a', 'b', 'c' };

    NAOS_CHECK_MEM_EQ(bytes, 3, bytes, 2);
}

NAOS_TEST(left_shorter)
{
    static const char bytes[3] = { 'a', 'b', 'c' };

    NAOS_CHECK_MEM_EQ(bytes, 2, bytes, 3);
}

/* invalid is not part of the left/right pair, so its value follows one space after the colon. */
NAOS_TEST(null_with_size)
{
    static const char bytes[2] = { 'a', 'b' };
    const char *none;

    none = NULL;
    NAOS_CHECK_MEM_EQ(none, 2, bytes, 2);
    NAOS_CHECK_MEM_EQ(bytes, 2, none, 2);
    NAOS_CHECK_MEM_EQ(none, 2, none, 2);
}

NAOS_SUITE(memory,
    NAOS_ADD_TEST(byte),
    NAOS_ADD_TEST(right_shorter),
    NAOS_ADD_TEST(left_shorter),
    NAOS_ADD_TEST(null_with_size)
)
