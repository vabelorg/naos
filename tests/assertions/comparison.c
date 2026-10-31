/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * The recording functions called directly with a comparison outside naos_cmp_t, as no macro does:
 * it never holds, whatever the values.
 */

#include <naos/naos.h>

NAOS_TEST(unknown)
{
    naos_cmp_t unknown;

    unknown = (naos_cmp_t)99;
    naos_check_int(unknown, 1, 1, "int", __FILE__, __LINE__);
    naos_check_uint(unknown, 1u, 1u, "uint", __FILE__, __LINE__);
    naos_check_str(unknown, "a", "a", "str", __FILE__, __LINE__);
    naos_check_str(NAOS_CMP_LT, "a", "b", "str_lt", __FILE__, __LINE__);
}

NAOS_SUITE(comparison,
    NAOS_ADD_TEST(unknown)
)
