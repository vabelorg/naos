/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * naos_text_*: a text that grows as it is added to, and is made empty again when released.
 */

#include <naos/naos.h>

#include <stddef.h>
#include <string.h>

#include "text.h"

/* Whether text holds exactly expected. */
static int holds(const naos_text_t *text, const char *expected)
{
    size_t length;

    length = strlen(expected);

    return (text->length == length) && ((length == 0) || (memcmp(text->data, expected, length) == 0));
}

NAOS_TEST(starts_empty)
{
    naos_text_t text = NAOS_TEXT_EMPTY;

    NAOS_CHECK_NULL(text.data);
    NAOS_CHECK_UINT_EQ(0, text.length);
    NAOS_CHECK_INT_EQ(0, text.failed);
}

NAOS_TEST(adds_in_order)
{
    naos_text_t text = NAOS_TEXT_EMPTY;

    naos_text_add(&text, "ab");
    naos_text_add_bytes(&text, "cde", 2);
    naos_text_add(&text, "");
    naos_text_add_bytes(&text, NULL, 0);
    naos_text_add_format(&text, "[%d %zu %s]", -1, (size_t)42, "x");

    NAOS_CHECK_TRUE(holds(&text, "abcd[-1 42 x]"));
    NAOS_CHECK_INT_EQ(0, text.failed);
    naos_text_release(&text);
}

/* The bytes are kept as they are, a NUL included, since the length counts them. */
NAOS_TEST(keeps_any_byte)
{
    naos_text_t text = NAOS_TEXT_EMPTY;

    naos_text_add_bytes(&text, "a\0b\xFF", 4);

    NAOS_REQUIRE_UINT_EQ(4, text.length);
    NAOS_CHECK_MEM_EQ("a\0b\xFF", 4, text.data, text.length);
    naos_text_release(&text);
}

NAOS_TEST(grows_beyond_its_capacity)
{
    naos_text_t text = NAOS_TEXT_EMPTY;
    char chunk[100];
    size_t i;

    memset(chunk, 'x', sizeof(chunk));
    for (i = 0; i < 1000; i++)
    {
        naos_text_add_bytes(&text, chunk, sizeof(chunk));
    }

    NAOS_CHECK_UINT_EQ(100000, text.length);
    NAOS_CHECK_UINT_GE(text.capacity, text.length);
    NAOS_CHECK_INT_EQ('x', text.data[99999]);
    NAOS_CHECK_INT_EQ(0, text.failed);
    naos_text_release(&text);
}

NAOS_TEST(release_makes_it_empty)
{
    naos_text_t text = NAOS_TEXT_EMPTY;

    naos_text_add(&text, "abc");
    text.failed = 1;
    naos_text_release(&text);

    NAOS_CHECK_NULL(text.data);
    NAOS_CHECK_UINT_EQ(0, text.length);
    NAOS_CHECK_UINT_EQ(0, text.capacity);
    NAOS_CHECK_INT_EQ(0, text.failed);

    naos_text_add(&text, "d");
    NAOS_CHECK_TRUE(holds(&text, "d"));
    naos_text_release(&text);
}

/* A failed text takes nothing more until it is released, so that what it holds is not out of order. */
NAOS_TEST(failed_takes_nothing)
{
    naos_text_t text = NAOS_TEXT_EMPTY;

    naos_text_add(&text, "a");
    text.failed = 1;
    naos_text_add(&text, "b");
    naos_text_add_format(&text, "%d", 1);

    NAOS_CHECK_TRUE(holds(&text, "a"));
    naos_text_release(&text);
}

NAOS_SUITE(text,
    NAOS_ADD_TEST(starts_empty),
    NAOS_ADD_TEST(adds_in_order),
    NAOS_ADD_TEST(keeps_any_byte),
    NAOS_ADD_TEST(grows_beyond_its_capacity),
    NAOS_ADD_TEST(release_makes_it_empty),
    NAOS_ADD_TEST(failed_takes_nothing)
)
