/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 */

#include <naos/naos.h>

#include "fake_module_b.h"
#include "module_a.h"

/* The stub keeps its values from the previous test. */
NAOS_SETUP(setup)
{
    fake_b_return = 0;
    fake_b_calls = 0;
    fake_b_last_arg = 0;
}

NAOS_TEST(passes_x_to_b)
{
    NAOS_CHECK_INT_EQ(A_OK, func_a(3));
    NAOS_CHECK_INT_EQ(1, fake_b_calls);
    NAOS_CHECK_INT_EQ(3, fake_b_last_arg);
}

NAOS_TEST(propagates_b_failure)
{
    fake_b_return = -1;

    NAOS_CHECK_INT_EQ(A_ERROR, func_a(3));
}

NAOS_SUITE(module_a,
    NAOS_SET_SETUP(setup),
    NAOS_ADD_TEST(passes_x_to_b),
    NAOS_ADD_TEST(propagates_b_failure)
)

NAOS_MAIN(
    NAOS_ADD_SUITE(module_a)
)
