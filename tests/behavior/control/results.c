/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * The result of a run: the suites that open and close a full run, and those for the exit codes.
 */

#include <naos/naos.h>

NAOS_TEST(passes)
{
    NAOS_CHECK_TRUE(1);
}

NAOS_SUITE(pass,
    NAOS_ADD_TEST(passes)
)

/* For the exit codes: every selected test skipped is 77, a skip among passes is 0. */
NAOS_TEST(skipped_one)
{
    NAOS_SKIP("not applicable");
}

NAOS_TEST(skipped_two)
{
    NAOS_SKIP("not applicable either");
}

NAOS_TEST(passed)
{
    NAOS_CHECK_TRUE(1);
}

NAOS_SUITE(skips,
    NAOS_ADD_TEST(skipped_one),
    NAOS_ADD_TEST(skipped_two),
    NAOS_ADD_TEST(passed)
)

/*
 * Runs last in a full run: neither the failures nor the jumps of the earlier tests may leak into
 * it, so that it passes is the check.
 */
NAOS_TEST(passes_after_failures)
{
    NAOS_CHECK_TRUE(1);
}

NAOS_SUITE(last,
    NAOS_ADD_TEST(passes_after_failures)
)
