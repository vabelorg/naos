/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * NAOS_*_TRUE and NAOS_*_FALSE: the expression as written, !(expr) for FALSE, and no value.
 */

#include <naos/naos.h>

NAOS_TEST(true_fails)
{
    int done;

    done = 0;
    NAOS_CHECK_TRUE(done);
}

NAOS_TEST(false_fails)
{
    int done;

    done = 1;
    NAOS_CHECK_FALSE(done);
}

NAOS_SUITE(boolean,
    NAOS_ADD_TEST(true_fails),
    NAOS_ADD_TEST(false_fails)
)
