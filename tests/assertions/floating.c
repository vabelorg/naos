/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * NAOS_*_DOUBLE_NEAR: both values and the tolerance, each the shortest text that reads back to
 * it.
 */

#include <naos/naos.h>

#include <locale.h>
#include <math.h>
#include <stddef.h>

NAOS_TEST(near)
{
    NAOS_CHECK_DOUBLE_NEAR(1.0, 1.5, 0.25);
}

NAOS_TEST(nan_never_matches)
{
    NAOS_CHECK_DOUBLE_NEAR(NAN, NAN, 1.0);
}

/* Each value is the shortest text that reads back to it: 0.1, not 0.10000000000000001. */
NAOS_TEST(shortest)
{
    double third = 1.0 / 3.0;

    NAOS_CHECK_DOUBLE_NEAR(0.1, 0.2, 0.05);
    NAOS_CHECK_DOUBLE_NEAR(third, 100.0, 0.001);
}

/*
 * The exponent is used from 1e17 on and below 1e-4, with its sign and at least two digits; 1e16 is
 * written out in full. 5e-324 is the smallest subnormal double.
 */
NAOS_TEST(exponent)
{
    NAOS_CHECK_DOUBLE_NEAR(1e-5, 1.5e20, 1.0);
    NAOS_CHECK_DOUBLE_NEAR(1e300, 1e16, 1e17);
    NAOS_CHECK_DOUBLE_NEAR(5e-324, 1.0, 0.5);
}

NAOS_TEST(special)
{
    NAOS_CHECK_DOUBLE_NEAR(-0.0, 1.0, 0.5);
    NAOS_CHECK_DOUBLE_NEAR(INFINITY, -INFINITY, 1.0);
    NAOS_CHECK_DOUBLE_NEAR(1.0, 2.0, NAN);
}

/*
 * Compares under the first locale found whose decimal point is ','; the values still have '.'.
 * Where there is none, the C locale stays and the output is the same. The check does not leave the
 * test, so the C locale is back for the tests after it.
 */
NAOS_TEST(locale)
{
    static const char *const names[] = { "de_DE.UTF-8", "de_DE", "German" };
    size_t index;

    for (index = 0; index < (sizeof(names) / sizeof(names[0])); index++)
    {
        if (setlocale(LC_NUMERIC, names[index]) != NULL)
        {
            break;
        }
    }
    NAOS_CHECK_DOUBLE_NEAR(1.5, 2.5, 0.25);
    setlocale(LC_NUMERIC, "C");
}

NAOS_SUITE(floating,
    NAOS_ADD_TEST(near),
    NAOS_ADD_TEST(nan_never_matches),
    NAOS_ADD_TEST(shortest),
    NAOS_ADD_TEST(exponent),
    NAOS_ADD_TEST(special),
    NAOS_ADD_TEST(locale)
)
