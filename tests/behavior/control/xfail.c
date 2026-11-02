/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * Tests expected to fail, registered with NAOS_ADD_TEST_XFAIL: XFAIL when they record a failure, in
 * setup, the body or teardown, XPASS (a failure) when they end without one, a skip when they skip
 * without failing. A test prints "reached-..." after a point, so its expected output shows whether it
 * got there.
 */

#include <naos/naos.h>

#include <stdio.h>

static int g_ready = 0;

static void check_ready(void)
{
    NAOS_CHECK_TRUE(g_ready);
}

NAOS_TEST(check_fails)
{
    NAOS_CHECK_INT_EQ(-42, 42);
    printf("reached-after-check\n");
}

NAOS_TEST(require_fails)
{
    NAOS_REQUIRE_TRUE(g_ready);
    printf("reached-after-require\n");
}

NAOS_TEST(fails)
{
    NAOS_FAIL("not implemented yet");
}

NAOS_TEST(in_helper)
{
    check_ready();
}

NAOS_TEST(passes)
{
    NAOS_CHECK_INT_EQ(42, 42);
}

/* No assertion ran: the test did not fail, so it is XPASS rather than failing for the count. */
NAOS_TEST(empty)
{
}

NAOS_TEST(skips)
{
    NAOS_SKIP("needs a network");
}

/* A failure before the skip: the failure wins, as for any test. */
NAOS_TEST(fails_then_skips)
{
    NAOS_CHECK_TRUE(g_ready);
    NAOS_SKIP("needs a network");
}

NAOS_TEST(ok)
{
    NAOS_PASS();
}

NAOS_SUITE(expected,
    NAOS_ADD_TEST(ok),
    NAOS_ADD_TEST_XFAIL(check_fails, "sign handling not implemented yet"),
    NAOS_ADD_TEST_XFAIL(require_fails, "not ready"),
    NAOS_ADD_TEST_XFAIL(fails, "not implemented yet"),
    NAOS_ADD_TEST_XFAIL(in_helper, "not ready"),
    NAOS_ADD_TEST_XFAIL(passes, "bug #1, \"fixed\" since"),
    NAOS_ADD_TEST_XFAIL(empty, "nothing checked"),
    NAOS_ADD_TEST_XFAIL(skips, "skips first"),
    NAOS_ADD_TEST_XFAIL(fails_then_skips, "fails first")
)

NAOS_SETUP(fail_in_setup)
{
    NAOS_REQUIRE_TRUE(g_ready);
}

NAOS_TEARDOWN(fail_in_teardown)
{
    NAOS_CHECK_TRUE(g_ready);
}

NAOS_TEST(body)
{
    NAOS_PASS();
    printf("reached-body\n");
}

/* A failure in setup or teardown counts as the expected failure, as it counts for any test. */
NAOS_SUITE(in_setup,
    NAOS_SET_SETUP(fail_in_setup),
    NAOS_ADD_TEST_XFAIL(body, "setup not ready")
)

NAOS_SUITE(in_teardown,
    NAOS_SET_TEARDOWN(fail_in_teardown),
    NAOS_ADD_TEST_XFAIL(body, "teardown not ready")
)

NAOS_MAIN(
    NAOS_ADD_SUITE(expected),
    NAOS_ADD_SUITE(in_setup),
    NAOS_ADD_SUITE(in_teardown)
)
