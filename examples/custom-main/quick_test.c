/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 */

#include <naos/naos.h>

NAOS_TEST(adds)
{
    NAOS_CHECK_INT_EQ(4, 2 + 2);
}

NAOS_SUITE(quick,
    NAOS_ADD_TEST(adds)
)
