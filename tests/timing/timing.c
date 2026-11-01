/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * Tests for the times of the result lines and SUMMARY: tests that keep the processor busy for a
 * known time, in the body, or in setup and teardown, so that the time shown must be at least that.
 */

#include <naos/naos.h>

#include <time.h>

/*
 * Keeps the processor busy for milliseconds of the processor time of clock(). The wall-clock time of
 * a single thread is never less than its processor time, so the test takes at least that long.
 */
static void stay_busy(long milliseconds)
{
    clock_t start;
    clock_t now;

    start = clock();
    NAOS_REQUIRE_TRUE(start != (clock_t)-1);
    do
    {
        now = clock();
    } while ((double)(now - start) * 1000.0 < (double)milliseconds * (double)CLOCKS_PER_SEC);
}

NAOS_TEST(passes)
{
    stay_busy(50);
}

NAOS_TEST(fails)
{
    stay_busy(50);
    NAOS_FAIL("after 50 ms");
}

NAOS_TEST(skips)
{
    stay_busy(50);
    NAOS_SKIP("after 50 ms");
}

NAOS_SUITE(timing,
    NAOS_ADD_TEST(passes),
    NAOS_ADD_TEST(fails),
    NAOS_ADD_TEST(skips)
)

NAOS_SETUP(busy_setup)
{
    stay_busy(30);
}

NAOS_TEARDOWN(busy_teardown)
{
    stay_busy(30);
}

NAOS_TEST(between)
{
    NAOS_CHECK_TRUE(1);
}

/* The body takes no time: what is shown comes from setup and teardown. */
NAOS_SUITE(fixtures,
    NAOS_SET_SETUP(busy_setup),
    NAOS_SET_TEARDOWN(busy_teardown),
    NAOS_ADD_TEST(between)
)

NAOS_MAIN(
    NAOS_ADD_SUITE(timing),
    NAOS_ADD_SUITE(fixtures)
)
