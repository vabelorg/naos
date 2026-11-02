/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * Tests for --naos-shuffle and --naos-repeat: three suites of three passing tests, whose order the
 * expected output holds, and a test that depends on running after another one. Adding a test here
 * changes the order of every seed.
 */

#include <naos/naos.h>

static int first_ran = 0;

NAOS_TEST(t1)
{
    NAOS_CHECK_TRUE(1);
}

NAOS_TEST(t2)
{
    NAOS_CHECK_TRUE(1);
}

NAOS_TEST(t3)
{
    NAOS_CHECK_TRUE(1);
}

NAOS_SUITE(a,
    NAOS_ADD_TEST(t1),
    NAOS_ADD_TEST(t2),
    NAOS_ADD_TEST(t3)
)

NAOS_SUITE(b,
    NAOS_ADD_TEST(t1),
    NAOS_ADD_TEST(t2),
    NAOS_ADD_TEST(t3)
)

NAOS_SUITE(c,
    NAOS_ADD_TEST(t1),
    NAOS_ADD_TEST(t2),
    NAOS_ADD_TEST(t3)
)

NAOS_TEST(first)
{
    first_ran = 1;
}

/* Passes only after first in the same process: the kind of dependency the shuffle brings out. */
NAOS_TEST(second)
{
    NAOS_CHECK_TRUE(first_ran);
}

NAOS_SUITE(dependent,
    NAOS_ADD_TEST(first),
    NAOS_ADD_TEST(second)
)

NAOS_MAIN(
    NAOS_ADD_SUITE(a),
    NAOS_ADD_SUITE(b),
    NAOS_ADD_SUITE(c),
    NAOS_ADD_SUITE(dependent)
)
