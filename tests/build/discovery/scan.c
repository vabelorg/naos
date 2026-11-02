/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * Not built: the text FindDefinitions.cmake reads with NaosCheckRegistration.cmake. Each line with a
 * comment "found: <name>" is where a suite of that name must be found, and "test: <name>" where a
 * test must be; no other suite or test may be.
 */

#include <naos/naos.h>

/* NAOS_SUITE(in_block_comment, x) NAOS_TEST(in_block_comment) */
// NAOS_SUITE(in_line_comment, x) NAOS_TEST(in_line_comment)
/*
 * NAOS_SUITE(in_long_comment, x)
 */
#define WRAP_SUITE(name) NAOS_SUITE(name, \
    NAOS_ADD_TEST(one))
  #  define ALIAS NAOS_SUITE(in_directive, x)
#define WRAP_TEST(name) NAOS_TEST(name)

NAOS_TEST(one) // test: one
{
    NAOS_PASS();
}

NAOS_SUITE(first, NAOS_ADD_TEST(one)) // found: first
NAOS_SUITE (  // found: spaced
    spaced,
    NAOS_ADD_TEST(one)
)
NAOS_SUITE(  // found: name_on_next_line
    name_on_next_line, NAOS_ADD_TEST(one))
/* [;] \ */ NAOS_SUITE(after_comment, NAOS_ADD_TEST(one)) /* found: after_comment */
NAOS_SUITE(left, NAOS_ADD_TEST(one)) NAOS_SUITE(right, NAOS_ADD_TEST(one)) // found: left found: right
NAOS_TEST ( // test: spaced_test
    spaced_test) { NAOS_PASS(); }

/* Inside conditionals: not checked, whatever the condition. */
#ifdef NAOS_SCAN_NEVER_DEFINED
NAOS_TEST(in_ifdef) { NAOS_PASS(); }
NAOS_SUITE(in_ifdef, NAOS_ADD_TEST(one))
#  if defined(A) \
      && defined(B)
NAOS_SUITE(in_nested_if, NAOS_ADD_TEST(one))
#  endif
NAOS_SUITE(in_ifdef_after_nested, NAOS_ADD_TEST(one))
#else
NAOS_SUITE(in_else, NAOS_ADD_TEST(one))
#endif
#ifndef NAOS_SCAN_NEVER_DEFINED
NAOS_SUITE(in_ifndef, NAOS_ADD_TEST(one))
#endif /* NAOS_SCAN_NEVER_DEFINED */
NAOS_SUITE(after_endif, NAOS_ADD_TEST(one)) // found: after_endif

/* Not definitions: another name, a longer name, a definition through a macro of the project's own. */
static void (*suite_function)(naos_builder_t *) = NAOS_SUITE_FUNC_(first);
static void (*test_function)(void) = NAOS_TEST_FUNC_(one);
MY_NAOS_SUITE(prefixed, x)
MY_NAOS_TEST(prefixed)
WRAP_SUITE(wrapped)
WRAP_TEST(wrapped)

NAOS_MAIN(
    NAOS_ADD_SUITE(first)
)
