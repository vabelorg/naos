/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * naos/version.h: the version of the library linked and of the headers agree.
 */

#include <naos/naos.h>

NAOS_TEST(linked_matches_headers)
{
    /* The library linked here was built from the same headers. */
    NAOS_CHECK_STR_EQ(NAOS_VERSION_STRING, naos_version_string());
    NAOS_CHECK_UINT_EQ(NAOS_VERSION_NUMBER, naos_version_number());
}

NAOS_TEST(conversion_macros)
{
    NAOS_CHECK_STR_EQ("1.2.3", NAOS_VERSION_STRINGIFY(1, 2, 3));
    NAOS_CHECK_UINT_EQ(0x010203UL, NAOS_VERSION_ENCODE(1, 2, 3));
}

NAOS_SUITE(version,
    NAOS_ADD_TEST(linked_matches_headers),
    NAOS_ADD_TEST(conversion_macros)
)
