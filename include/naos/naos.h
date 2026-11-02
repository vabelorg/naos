/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * Naos
 * A test harness for C and C++ that needs only a C99 compiler.
 *
 * GitHub: https://github.com/vabelorg/naos
 * Docs: https://github.com/vabelorg/naos/tree/main/docs (for a release, read them at its tag)
 *
 * Usage:
 *     parser_test.c:
 *         NAOS_TEST(empty_object)
 *         {
 *             NAOS_REQUIRE_NOT_NULL(ptr);
 *             NAOS_CHECK_INT_EQ(0, value);
 *         }
 *
 *         NAOS_SUITE(parser,
 *             NAOS_ADD_TEST(empty_object)
 *         )
 *
 *     main.c:
 *         NAOS_DECLARE_SUITE(parser);
 *
 *         NAOS_MAIN(
 *             NAOS_ADD_SUITE(parser)
 *         )
 *
 * This header includes the parts (naos/session.h, exit.h, suite.h, check.h, control.h and
 * version.h); include this one. The public API, its compatibility and the command line are
 * described in docs/reference/. Not thread-safe.
 */

#ifndef NAOS_NAOS_H
#define NAOS_NAOS_H

#include <naos/check.h>
#include <naos/control.h>
#include <naos/exit.h>
#include <naos/session.h>
#include <naos/suite.h>
#include <naos/version.h>

#endif
