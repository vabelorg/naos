/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * Tests that only pass, for the cases of the command line: the options, the filter patterns, the
 * listing and the help. The names are chosen for the patterns to tell apart, and kept stable, since
 * the expected output of most cases lists them.
 */

#include <naos/naos.h>

NAOS_TEST(empty)
{
    NAOS_CHECK_TRUE(1);
}

NAOS_TEST(nested)
{
    NAOS_CHECK_TRUE(1);
}

NAOS_TEST(slow_big)
{
    NAOS_CHECK_TRUE(1);
}

NAOS_TEST(slow_small)
{
    NAOS_CHECK_TRUE(1);
}

NAOS_SUITE(parser,
    NAOS_ADD_TEST(empty),
    NAOS_ADD_TEST(nested),
    NAOS_ADD_TEST(slow_big)
)

NAOS_SUITE(writer,
    NAOS_ADD_TEST(empty),
    NAOS_ADD_TEST(slow_small)
)

NAOS_MAIN(
    NAOS_ADD_SUITE(parser),
    NAOS_ADD_SUITE(writer)
)
