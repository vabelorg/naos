/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * naos/check.h: the comparison of each kind of assertion, and the values its failure prints.
 */

#include <naos/check.h>

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "diagnostic.h"
#include "value.h"

/* Size of the buffer one value is formatted into: more than any %jd, %ju, %p or %zu needs. */
#define NAOS_VALUE_SIZE 64

/* Ends an assertion that passed. */
static int passed(void)
{
    naos_diagnostic_pass();
    return 1;
}

/* -1, 0 or 1 as left is below, equal to or above right. */
static int order_int(intmax_t left, intmax_t right)
{
    if (left < right)
    {
        return -1;
    }

    return (left > right) ? 1 : 0;
}

static int order_uint(uintmax_t left, uintmax_t right)
{
    if (left < right)
    {
        return -1;
    }

    return (left > right) ? 1 : 0;
}

/* Whether cmp holds for order, the result of order_int() or order_uint(). */
static int holds_for_order(naos_cmp_t cmp, int order)
{
    switch (cmp)
    {
    case NAOS_CMP_EQ:
        return order == 0;
    case NAOS_CMP_NE:
        return order != 0;
    case NAOS_CMP_LT:
        return order < 0;
    case NAOS_CMP_LE:
        return order <= 0;
    case NAOS_CMP_GT:
        return order > 0;
    case NAOS_CMP_GE:
        return order >= 0;
    }

    return 0;
}

/* %p leaves the form of NULL to the C library, so NULL is formatted as (null) here. */
static void format_pointer(char *buffer, size_t size, const void *ptr)
{
    if (ptr == NULL)
    {
        snprintf(buffer, size, "(null)");
    }
    else
    {
        snprintf(buffer, size, "%p", ptr);
    }
}

/* Records a failure with the values left and right, already formatted. */
static int fail_with_values(const char *left, const char *right, const char *expr, const char *file, int line)
{
    naos_diagnostic_begin(NAOS_REPORT_ERROR, file, line, expr);
    naos_diagnostic_add("left", left);
    naos_diagnostic_add("right", right);
    naos_diagnostic_end();

    return 0;
}

/* Two NULLs are equal; NULL and a string are not. */
static int str_equal(const char *left, const char *right)
{
    if ((left == NULL) || (right == NULL))
    {
        return left == right;
    }

    return strcmp(left, right) == 0;
}

/* Offset of the first byte where left and right differ, or the shorter size when one is a prefix of the other. */
static size_t first_difference_offset(const unsigned char *left, size_t left_size, const unsigned char *right, size_t right_size)
{
    size_t common_size;
    size_t offset;

    common_size = (left_size < right_size) ? left_size : right_size;

    offset = 0;
    while ((offset < common_size) && (left[offset] == right[offset]))
    {
        offset++;
    }

    return offset;
}

/* Formats the first difference at offset, which first_difference_offset() found, as the value of a detail. */
static void format_first_difference(char *buffer, size_t size, size_t offset, const unsigned char *left, size_t left_size, const unsigned char *right, size_t right_size)
{
    if ((offset < left_size) && (offset < right_size))
    {
        snprintf(buffer, size, "offset %zu, left 0x%02X, right 0x%02X", offset, (unsigned int)left[offset], (unsigned int)right[offset]);
    }
    else
    {
        /* Only the sizes differ: the difference is where the shorter side ends. */
        snprintf(buffer, size, "offset %zu, end of %s", offset, (left_size < right_size) ? "left" : "right");
    }
}

/* Describes a pointer that is NULL with a non-zero size, or returns NULL when neither side is one. */
static const char *describe_null_with_size(const void *left, size_t left_size, const void *right, size_t right_size)
{
    int left_invalid;
    int right_invalid;

    left_invalid = (left == NULL) && (left_size != 0);
    right_invalid = (right == NULL) && (right_size != 0);

    if ((left_invalid != 0) && (right_invalid != 0))
    {
        return "left and right are NULL with non-zero size";
    }
    else if (left_invalid != 0)
    {
        return "left is NULL with non-zero size";
    }
    else if (right_invalid != 0)
    {
        return "right is NULL with non-zero size";
    }

    return NULL;
}

/* With show_difference non-zero, the failure also gets the first difference; both values must then be non-NULL. */
static int fail_with_strings(const char *left, const char *right, int show_difference, const char *expr, const char *file, int line)
{
    const unsigned char *left_bytes;
    const unsigned char *right_bytes;
    size_t left_size;
    size_t right_size;
    char difference_text[NAOS_VALUE_SIZE];

    naos_diagnostic_begin(NAOS_REPORT_ERROR, file, line, expr);
    naos_diagnostic_add_string("left", left);
    naos_diagnostic_add_string("right", right);

    if (show_difference != 0)
    {
        left_bytes = (const unsigned char *)left;
        right_bytes = (const unsigned char *)right;
        left_size = strlen(left);
        right_size = strlen(right);
        format_first_difference(difference_text, sizeof(difference_text), first_difference_offset(left_bytes, left_size, right_bytes, right_size), left_bytes, left_size, right_bytes, right_size);
        naos_diagnostic_add("first difference", difference_text);
    }

    naos_diagnostic_end();

    return 0;
}

/* Records a failure of NULL or NOT_NULL: the pointer is a value only when it is not NULL. */
static int fail_with_pointer(const void *ptr, const char *expr, const char *file, int line)
{
    char value_text[NAOS_VALUE_SIZE];

    naos_diagnostic_begin(NAOS_REPORT_ERROR, file, line, expr);

    if (ptr != NULL)
    {
        snprintf(value_text, sizeof(value_text), "%p", ptr);
        naos_diagnostic_add("value", value_text);
    }

    naos_diagnostic_end();

    return 0;
}

int naos_check(int cond, const char *expr, const char *file, int line)
{
    if (cond != 0)
    {
        return passed();
    }

    naos_diagnostic_begin(NAOS_REPORT_ERROR, file, line, expr);
    naos_diagnostic_end();

    return 0;
}

int naos_check_int(naos_cmp_t cmp, intmax_t left, intmax_t right, const char *expr, const char *file, int line)
{
    char left_text[NAOS_VALUE_SIZE];
    char right_text[NAOS_VALUE_SIZE];

    if (holds_for_order(cmp, order_int(left, right)) != 0)
    {
        return passed();
    }

    snprintf(left_text, sizeof(left_text), "%jd", left);
    snprintf(right_text, sizeof(right_text), "%jd", right);

    return fail_with_values(left_text, right_text, expr, file, line);
}

int naos_check_uint(naos_cmp_t cmp, uintmax_t left, uintmax_t right, const char *expr, const char *file, int line)
{
    char left_text[NAOS_VALUE_SIZE];
    char right_text[NAOS_VALUE_SIZE];

    if (holds_for_order(cmp, order_uint(left, right)) != 0)
    {
        return passed();
    }

    snprintf(left_text, sizeof(left_text), "%ju", left);
    snprintf(right_text, sizeof(right_text), "%ju", right);

    return fail_with_values(left_text, right_text, expr, file, line);
}

int naos_check_double_near(double left, double right, double tol, const char *expr, const char *file, int line)
{
    char left_text[NAOS_VALUE_DOUBLE_SIZE];
    char right_text[NAOS_VALUE_DOUBLE_SIZE];
    char tol_text[NAOS_VALUE_DOUBLE_SIZE];

    /* Exact equality first so that matching infinities pass; NaN fails every comparison. */
    if ((left == right) || (fabs(left - right) <= tol))
    {
        return passed();
    }

    naos_value_format_double(left_text, sizeof(left_text), left);
    naos_value_format_double(right_text, sizeof(right_text), right);
    naos_value_format_double(tol_text, sizeof(tol_text), tol);

    naos_diagnostic_begin(NAOS_REPORT_ERROR, file, line, expr);
    naos_diagnostic_add("left", left_text);
    naos_diagnostic_add("right", right_text);
    naos_diagnostic_add("tolerance", tol_text);
    naos_diagnostic_end();

    return 0;
}

int naos_check_str(naos_cmp_t cmp, const char *left, const char *right, const char *expr, const char *file, int line)
{
    int equal;

    equal = str_equal(left, right);
    if (((cmp == NAOS_CMP_EQ) && (equal != 0)) || ((cmp == NAOS_CMP_NE) && (equal == 0)))
    {
        return passed();
    }

    /* Only a failed EQ of two strings has a first difference to show. */
    return fail_with_strings(left, right, (cmp == NAOS_CMP_EQ) && (left != NULL) && (right != NULL), expr, file, line);
}

int naos_check_mem_eq(const void *left, size_t left_size, const void *right, size_t right_size, const char *expr, const char *file, int line)
{
    const unsigned char *left_bytes;
    const unsigned char *right_bytes;
    size_t offset;
    char left_size_text[NAOS_VALUE_SIZE];
    char right_size_text[NAOS_VALUE_SIZE];
    char difference_text[NAOS_VALUE_SIZE];
    const char *last_label;
    const char *last_value;

    left_bytes = (const unsigned char *)left;
    right_bytes = (const unsigned char *)right;

    last_value = describe_null_with_size(left, left_size, right, right_size);
    if (last_value != NULL)
    {
        last_label = "invalid";
    }
    else
    {
        offset = first_difference_offset(left_bytes, left_size, right_bytes, right_size);
        if ((offset == left_size) && (offset == right_size))
        {
            return passed();
        }

        format_first_difference(difference_text, sizeof(difference_text), offset, left_bytes, left_size, right_bytes, right_size);

        last_label = "first difference";
        last_value = difference_text;
    }

    snprintf(left_size_text, sizeof(left_size_text), "%zu", left_size);
    snprintf(right_size_text, sizeof(right_size_text), "%zu", right_size);

    naos_diagnostic_begin(NAOS_REPORT_ERROR, file, line, expr);
    naos_diagnostic_add("left size", left_size_text);
    naos_diagnostic_add("right size", right_size_text);
    naos_diagnostic_add(last_label, last_value);
    naos_diagnostic_end();

    return 0;
}

int naos_check_ptr_eq(const void *left, const void *right, const char *expr, const char *file, int line)
{
    char left_text[NAOS_VALUE_SIZE];
    char right_text[NAOS_VALUE_SIZE];

    if (left == right)
    {
        return passed();
    }

    format_pointer(left_text, sizeof(left_text), left);
    format_pointer(right_text, sizeof(right_text), right);

    return fail_with_values(left_text, right_text, expr, file, line);
}

int naos_check_null(const void *ptr, const char *expr, const char *file, int line)
{
    if (ptr == NULL)
    {
        return passed();
    }

    return fail_with_pointer(ptr, expr, file, line);
}

int naos_check_not_null(const void *ptr, const char *expr, const char *file, int line)
{
    if (ptr != NULL)
    {
        return passed();
    }

    return fail_with_pointer(ptr, expr, file, line);
}
