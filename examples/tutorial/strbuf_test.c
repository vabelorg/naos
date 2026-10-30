/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 */

#include <naos/naos.h>

#include "strbuf.h"

/* Shared by setup, the tests and teardown. */
static struct strbuf sb;

NAOS_SETUP(setup)
{
    NAOS_REQUIRE_INT_EQ(0, strbuf_init(&sb));
}

NAOS_TEARDOWN(teardown)
{
    strbuf_free(&sb);
}

NAOS_TEST(starts_empty)
{
    NAOS_CHECK_UINT_EQ(0, sb.length);
    NAOS_CHECK_STR_EQ("", strbuf_cstr(&sb));
}

NAOS_TEST(append_joins)
{
    NAOS_REQUIRE_INT_EQ(0, strbuf_append(&sb, "hello, "));
    NAOS_REQUIRE_INT_EQ(0, strbuf_append(&sb, "world"));

    NAOS_CHECK_STR_EQ("hello, world", strbuf_cstr(&sb));
    NAOS_CHECK_UINT_EQ(12, sb.length);
}

NAOS_TEST(append_grows)
{
    int i;

    for (i = 0; i < 100; i++)
    {
        NAOS_NOTE("append %d", i);
        NAOS_REQUIRE_INT_EQ(0, strbuf_append(&sb, "x"));
    }

    NAOS_CHECK_UINT_EQ(100, sb.length);
    NAOS_CHECK_UINT_GT(sb.capacity, sb.length);
}

NAOS_SUITE(strbuf,
    NAOS_SET_SETUP(setup),
    NAOS_SET_TEARDOWN(teardown),
    NAOS_ADD_TEST(starts_empty),
    NAOS_ADD_TEST(append_joins),
    NAOS_ADD_TEST(append_grows)
)
