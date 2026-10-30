/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * The installed headers and library are those of the version built.
 */

#include <naos/naos.h>

NAOS_TEST(headers)
{
    NAOS_CHECK_STR_EQ(EXPECTED_VERSION, NAOS_VERSION_STRING);
}

NAOS_TEST(library)
{
    NAOS_CHECK_STR_EQ(EXPECTED_VERSION, naos_version_string());
}

NAOS_SUITE(package,
    NAOS_ADD_TEST(headers),
    NAOS_ADD_TEST(library)
)

NAOS_MAIN(
    NAOS_ADD_SUITE(package)
)
