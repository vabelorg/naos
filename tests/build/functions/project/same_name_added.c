/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * A test named shared, added to the suite added_here; same_name_left_out.c defines a test of the
 * same name and adds it to no suite, which this one does not hide.
 */

#include <naos/naos.h>

NAOS_DECLARE_SUITE(other_file);

NAOS_TEST(shared)
{
    NAOS_PASS();
}

NAOS_SUITE(added_here,
    NAOS_ADD_TEST(shared)
)

NAOS_MAIN(
    NAOS_ADD_SUITE(added_here),
    NAOS_ADD_SUITE(other_file)
)
