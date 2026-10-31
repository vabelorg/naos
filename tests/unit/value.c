/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * naos_value_format_double: the shortest decimal that reads back to the value, in a fixed form.
 */

#include <naos/naos.h>

#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "value.h"

/* The text of value; valid until the next call. */
static const char *double_text(double value)
{
    static char text[NAOS_VALUE_DOUBLE_SIZE];

    naos_value_format_double(text, sizeof(text), value);

    return text;
}

NAOS_TEST(special_values)
{
    NAOS_CHECK_STR_EQ("0", double_text(0.0));
    NAOS_CHECK_STR_EQ("-0", double_text(-0.0));
    NAOS_CHECK_STR_EQ("inf", double_text(INFINITY));
    NAOS_CHECK_STR_EQ("-inf", double_text(-INFINITY));
    NAOS_CHECK_STR_EQ("nan", double_text(NAN));
    NAOS_CHECK_STR_EQ("nan", double_text(-NAN));
}

NAOS_TEST(shortest_digits)
{
    NAOS_CHECK_STR_EQ("0.1", double_text(0.1));
    NAOS_CHECK_STR_EQ("0.3", double_text(0.3));
    NAOS_CHECK_STR_EQ("0.30000000000000004", double_text(0.1 + 0.2));
    NAOS_CHECK_STR_EQ("0.3333333333333333", double_text(1.0 / 3.0));
    NAOS_CHECK_STR_EQ("-1.25", double_text(-1.25));
    NAOS_CHECK_STR_EQ("123456.789", double_text(123456.789));
}

NAOS_TEST(integers_written_out)
{
    NAOS_CHECK_STR_EQ("1", double_text(1.0));
    NAOS_CHECK_STR_EQ("100", double_text(100.0));
    NAOS_CHECK_STR_EQ("-42", double_text(-42.0));
    NAOS_CHECK_STR_EQ("9007199254740992", double_text(9007199254740992.0));
}

/* Plain from 1e-4 up to below 1e17; the exponent outside, with its sign and at least two digits. */
NAOS_TEST(exponent_bounds)
{
    NAOS_CHECK_STR_EQ("0.0001", double_text(1e-4));
    NAOS_CHECK_STR_EQ("0.000123", double_text(0.000123));
    NAOS_CHECK_STR_EQ("1e-05", double_text(1e-5));
    NAOS_CHECK_STR_EQ("10000000000000000", double_text(1e16));
    NAOS_CHECK_STR_EQ("1e+17", double_text(1e17));
    NAOS_CHECK_STR_EQ("1.5e+20", double_text(1.5e20));
    NAOS_CHECK_STR_EQ("1e+300", double_text(1e300));
    NAOS_CHECK_STR_EQ("-2.5e-100", double_text(-2.5e-100));
}

NAOS_TEST(extremes)
{
    NAOS_CHECK_STR_EQ("1.7976931348623157e+308", double_text(DBL_MAX));
    NAOS_CHECK_STR_EQ("2.2250738585072014e-308", double_text(DBL_MIN));
    NAOS_CHECK_STR_EQ("5e-324", double_text(5e-324));
    NAOS_CHECK_STR_EQ("-1.7976931348623157e+308", double_text(-DBL_MAX));
}

/* Every text reads back to the value it came from. */
NAOS_TEST(reads_back)
{
    static const double values[] =
    {
        0.1, 0.7, 2.0 / 3.0, 1e-7, 123.456e-10, 6.02214076e23, 1.0 - DBL_EPSILON, 1.0 + DBL_EPSILON,
        3.141592653589793, 2.718281828459045, 4.9406564584124654e-324, 1e22, 1e23
    };
    size_t i;

    for (i = 0; i < (sizeof(values) / sizeof(values[0])); i++)
    {
        NAOS_NOTE("values[%u]", (unsigned int)i);
        NAOS_CHECK_TRUE(strtod(double_text(values[i]), NULL) == values[i]);
        NAOS_NOTE("values[%u]", (unsigned int)i);
        NAOS_CHECK_TRUE(strtod(double_text(-values[i]), NULL) == -values[i]);
    }
}

/* NAOS_VALUE_DOUBLE_SIZE holds the longest texts, plain and with an exponent. */
NAOS_TEST(longest_texts_fit)
{
    NAOS_CHECK_UINT_LT(strlen(double_text(-0.00012345678901234567)), NAOS_VALUE_DOUBLE_SIZE);
    NAOS_CHECK_UINT_LT(strlen(double_text(-2.2250738585072014e-308)), NAOS_VALUE_DOUBLE_SIZE);
    NAOS_CHECK_UINT_LT(strlen(double_text(-1234567890123456.7)), NAOS_VALUE_DOUBLE_SIZE);
}

/* A smaller buffer gets the start of the text, NUL-terminated. */
NAOS_TEST(small_buffer_cuts_short)
{
    char text[4];

    memset(text, 'x', sizeof(text));
    naos_value_format_double(text, sizeof(text), 1.0 / 3.0);
    NAOS_CHECK_STR_EQ("0.3", text);
}

NAOS_SUITE(value,
    NAOS_ADD_TEST(special_values),
    NAOS_ADD_TEST(shortest_digits),
    NAOS_ADD_TEST(integers_written_out),
    NAOS_ADD_TEST(exponent_bounds),
    NAOS_ADD_TEST(extremes),
    NAOS_ADD_TEST(reads_back),
    NAOS_ADD_TEST(longest_texts_fit),
    NAOS_ADD_TEST(small_buffer_cuts_short)
)
