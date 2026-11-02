/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * The headers and the library a project gets are those of the version built; one test skips, so
 * that CTest shows the SKIP_RETURN_CODE that discovery gives every test.
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

NAOS_TEST(skips)
{
    NAOS_SKIP("a skip shows as a skip under CTest");
}

NAOS_SUITE(consumer,
    NAOS_ADD_TEST(headers),
    NAOS_ADD_TEST(library),
    NAOS_ADD_TEST(skips)
)

NAOS_MAIN(
    NAOS_ADD_SUITE(consumer)
)
