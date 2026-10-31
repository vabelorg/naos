/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * Setup and teardown around each test: a fatal failure or a skip in setup leaves the body out,
 * teardown always runs, one in the body or in teardown leaves the rest of it out, and failures in
 * any of them count for the test. Each suite is one arrangement, around a test named body.
 */

#include <naos/naos.h>

#include <stdio.h>

NAOS_SETUP(require_in_setup)
{
    NAOS_REQUIRE_TRUE(1 == 2);
}

NAOS_SETUP(skip_in_setup)
{
    NAOS_SKIP("not applicable");
}

NAOS_SETUP(check_in_setup)
{
    NAOS_CHECK_TRUE(1 == 2);
}

NAOS_TEARDOWN(print_in_teardown)
{
    printf("teardown-ran\n");
}

NAOS_TEARDOWN(check_in_teardown)
{
    NAOS_CHECK_TRUE(1 == 2);
    printf("teardown-ran\n");
}

NAOS_TEARDOWN(require_in_teardown)
{
    NAOS_REQUIRE_TRUE(3 == 4);
    printf("reached-after-teardown-require\n");
}

NAOS_TEST(body)
{
    printf("body-ran\n");
}

NAOS_TEST(fatal_body)
{
    NAOS_REQUIRE_TRUE(1 == 2);
    printf("reached-after-require\n");
}

NAOS_SUITE(setup_fails,
    NAOS_SET_SETUP(require_in_setup),
    NAOS_SET_TEARDOWN(print_in_teardown),
    NAOS_ADD_TEST(body)
)

NAOS_SUITE(setup_skips,
    NAOS_SET_SETUP(skip_in_setup),
    NAOS_SET_TEARDOWN(print_in_teardown),
    NAOS_ADD_TEST(body)
)

NAOS_SUITE(setup_check_continues,
    NAOS_SET_SETUP(check_in_setup),
    NAOS_SET_TEARDOWN(print_in_teardown),
    NAOS_ADD_TEST(body)
)

/* Listed after the test: setup and teardown apply to the whole suite wherever they are. */
NAOS_SUITE(teardown_fails,
    NAOS_ADD_TEST(body),
    NAOS_SET_TEARDOWN(check_in_teardown)
)

/* A fatal failure in the body leaves the rest of the body out, but not teardown. */
NAOS_SUITE(body_fails,
    NAOS_SET_TEARDOWN(print_in_teardown),
    NAOS_ADD_TEST(fatal_body)
)

/* A fatal failure in teardown leaves the rest of teardown out. */
NAOS_SUITE(teardown_requires,
    NAOS_SET_TEARDOWN(require_in_teardown),
    NAOS_ADD_TEST(body)
)

/* Both jumps happen in one test and both failures count. */
NAOS_SUITE(body_and_teardown_require,
    NAOS_SET_TEARDOWN(require_in_teardown),
    NAOS_ADD_TEST(fatal_body)
)

NAOS_MAIN(
    NAOS_ADD_SUITE(setup_fails),
    NAOS_ADD_SUITE(setup_skips),
    NAOS_ADD_SUITE(setup_check_continues),
    NAOS_ADD_SUITE(teardown_fails),
    NAOS_ADD_SUITE(body_fails),
    NAOS_ADD_SUITE(teardown_requires),
    NAOS_ADD_SUITE(body_and_teardown_require)
)
