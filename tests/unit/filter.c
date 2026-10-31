/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * naos_filter_matches: the patterns of --naos-filter against the whole name <suite>.<test>.
 */

#include <naos/naos.h>

#include <stddef.h>
#include <string.h>

#include "filter.h"

/* Whether filter selects the test suite_name.test_name. */
static int selects(const char *filter, const char *suite_name, const char *test_name)
{
    naos_suite_t suite;
    naos_test_t test;

    memset(&suite, 0, sizeof(suite));
    memset(&test, 0, sizeof(test));
    suite.name = suite_name;
    test.name = test_name;

    return naos_filter_matches(&suite, &test, filter);
}

NAOS_TEST(no_filter_selects_all)
{
    NAOS_CHECK_TRUE(selects(NULL, "parser", "empty"));
}

/* A pattern matches the whole name, never a part of it. */
NAOS_TEST(whole_name)
{
    NAOS_CHECK_TRUE(selects("parser.empty", "parser", "empty"));
    NAOS_CHECK_FALSE(selects("parser.empt", "parser", "empty"));
    NAOS_CHECK_FALSE(selects("parser.emptyx", "parser", "empty"));
    NAOS_CHECK_FALSE(selects("arser.empty", "parser", "empty"));
    NAOS_CHECK_FALSE(selects("parser", "parser", "empty"));
    NAOS_CHECK_FALSE(selects("empty", "parser", "empty"));
}

NAOS_TEST(star)
{
    NAOS_CHECK_TRUE(selects("*", "parser", "empty"));
    NAOS_CHECK_TRUE(selects("parser.*", "parser", "empty"));
    NAOS_CHECK_TRUE(selects("*.empty", "parser", "empty"));
    NAOS_CHECK_TRUE(selects("parser.empty*", "parser", "empty"));
    NAOS_CHECK_TRUE(selects("parser.empty**", "parser", "empty"));
    NAOS_CHECK_TRUE(selects("p*y", "parser", "empty"));
    NAOS_CHECK_FALSE(selects("writer.*", "parser", "empty"));
}

/* After a mismatch, the last '*' takes one more character and the rest is tried again. */
NAOS_TEST(star_backtracks)
{
    NAOS_CHECK_TRUE(selects("*a*b", "aab", "xb"));
    NAOS_CHECK_TRUE(selects("a*b*c", "axb", "yc"));
    NAOS_CHECK_FALSE(selects("a*b*c", "axb", "yd"));
    NAOS_CHECK_TRUE(selects("*.*", "a", "b"));
    NAOS_CHECK_FALSE(selects("*.*.*", "a", "b"));
    NAOS_CHECK_TRUE(selects("*ss*ss", "assa", "ssss"));
}

NAOS_TEST(question_mark)
{
    NAOS_CHECK_TRUE(selects("?arser.empty", "parser", "empty"));
    NAOS_CHECK_TRUE(selects("parser?empty", "parser", "empty"));
    NAOS_CHECK_TRUE(selects("parser.e????", "parser", "empty"));
    NAOS_CHECK_FALSE(selects("parser.e???", "parser", "empty"));
    NAOS_CHECK_FALSE(selects("parser.empty?", "parser", "empty"));
}

/* ':' separates the patterns; an empty one matches nothing. */
NAOS_TEST(several_patterns)
{
    NAOS_CHECK_TRUE(selects("writer.*:parser.*", "parser", "empty"));
    NAOS_CHECK_TRUE(selects("::parser.empty:", "parser", "empty"));
    NAOS_CHECK_FALSE(selects("writer.*:reader.*", "parser", "empty"));
    NAOS_CHECK_FALSE(selects(":", "parser", "empty"));
}

/* Patterns after the first '-' exclude; without positive ones, everything else is selected. */
NAOS_TEST(negative_patterns)
{
    NAOS_CHECK_FALSE(selects("-parser.*", "parser", "empty"));
    NAOS_CHECK_TRUE(selects("-writer.*", "parser", "empty"));
    NAOS_CHECK_TRUE(selects("-", "parser", "empty"));
    NAOS_CHECK_FALSE(selects("*-parser.e*", "parser", "empty"));
    NAOS_CHECK_TRUE(selects("parser.*-*.slow_*", "parser", "empty"));
    NAOS_CHECK_FALSE(selects("parser.*-*.slow_*", "parser", "slow_big"));
    NAOS_CHECK_FALSE(selects("parser.*-writer.*:*.slow_*", "parser", "slow_big"));
    NAOS_CHECK_FALSE(selects("writer.*-*.slow_*", "parser", "empty"));
}

/* Names never contain '-', so a later '-' is part of a negative pattern that matches nothing. */
NAOS_TEST(second_minus_is_literal)
{
    NAOS_CHECK_TRUE(selects("*-parser-empty", "parser", "empty"));
}

NAOS_SUITE(filter,
    NAOS_ADD_TEST(no_filter_selects_all),
    NAOS_ADD_TEST(whole_name),
    NAOS_ADD_TEST(star),
    NAOS_ADD_TEST(star_backtracks),
    NAOS_ADD_TEST(question_mark),
    NAOS_ADD_TEST(several_patterns),
    NAOS_ADD_TEST(negative_patterns),
    NAOS_ADD_TEST(second_minus_is_literal)
)
