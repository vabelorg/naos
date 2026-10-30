/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * Matching the names of the tests against the patterns of --naos-filter (filter.h).
 */

#include "filter.h"

#include <string.h>

/* Character index of the name <suite>.<test>, which is suite_length + 1 + strlen(test) long. */
static char full_name_char(const char *suite, size_t suite_length, const char *test, size_t index)
{
    if (index < suite_length)
    {
        return suite[index];
    }

    return (index == suite_length) ? '.' : test[index - suite_length - 1];
}

/*
 * Matches the whole name <suite>.<test> against pattern[0, length), where '*' matches any string and
 * '?' any one character. On a mismatch after a '*', that '*' takes one more character and the rest
 * is tried again; only the last '*' needs retrying, so this takes no recursion.
 */
static int match_pattern(const char *pattern, size_t length, const char *suite, const char *test)
{
    size_t suite_length;
    size_t name_length;
    size_t p;
    size_t n;
    size_t star;
    size_t star_n;
    int has_star;

    suite_length = strlen(suite);
    name_length = suite_length + 1 + strlen(test);
    p = 0;
    n = 0;
    star = 0;
    star_n = 0;
    has_star = 0;

    while (n < name_length)
    {
        if ((p < length) && (pattern[p] == '*'))
        {
            has_star = 1;
            star = p++;
            star_n = n;
        }
        else if ((p < length) && ((pattern[p] == '?') || (pattern[p] == full_name_char(suite, suite_length, test, n))))
        {
            p++;
            n++;
        }
        else if (has_star != 0)
        {
            p = star + 1;
            n = ++star_n;
        }
        else
        {
            return 0;
        }
    }

    while ((p < length) && (pattern[p] == '*'))
    {
        p++;
    }

    return (p == length) ? 1 : 0;
}

/* Matches the name against any of the ':'-separated patterns in [begin, end); empty ones match nothing. */
static int match_any_pattern(const char *begin, const char *end, const char *suite, const char *test)
{
    const char *separator;

    for (;;)
    {
        separator = begin;
        while ((separator < end) && (*separator != ':'))
        {
            separator++;
        }

        if ((separator > begin) && (match_pattern(begin, (size_t)(separator - begin), suite, test) != 0))
        {
            return 1;
        }

        if (separator == end)
        {
            return 0;
        }

        begin = separator + 1;
    }
}

int naos_filter_matches(const naos_suite_t *suite, const naos_test_t *test, const char *filter)
{
    const char *minus;
    const char *end;

    if (filter == NULL)
    {
        return 1;
    }

    end = filter + strlen(filter);
    minus = strchr(filter, '-');

    if ((minus != filter) && (match_any_pattern(filter, (minus != NULL) ? minus : end, suite->name, test->name) == 0))
    {
        return 0;
    }

    if ((minus != NULL) && (match_any_pattern(minus + 1, end, suite->name, test->name) != 0))
    {
        return 0;
    }

    return 1;
}
