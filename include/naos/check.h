/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * Assertions, a part of naos/naos.h: include that header.
 *
 * NAOS_CHECK_* record a failure and continue. A failed assertion prints an error line,
 * "<file>:<line>: error: <expression>" ("<file>(<line>): error: ..." when Naos is built with MSVC),
 * then detail lines "  <label>: <value>", with one space after the colon; left and left size get one
 * more, so their values line up with those of right and right size. The comparisons print both
 * values, as left and right, and the values they used besides (tolerance, sizes, first
 * difference); the note and traces of naos/control.h come after them.
 * NAOS_REQUIRE_* record a failure and stop the test, as NAOS_FAIL of naos/control.h does: a failed
 * NAOS_REQUIRE_* is a fatal failure. naos/control.h tells how a test is stopped.
 *
 * Every argument is evaluated once. The expression of the error line is the arguments as written,
 * before macro expansion.
 */

#ifndef NAOS_CHECK_H
#define NAOS_CHECK_H

#include <stddef.h>
#include <stdint.h>

#include <naos/control.h>
#include <naos/export.h>

NAOS_BEGIN_DECLS

/*
 * The comparison an assertion makes, as the recording functions below take it. The values are part
 * of the ABI, kept compatible as the rest of the public API (see naos/naos.h).
 */
typedef enum naos_cmp
{
    NAOS_CMP_EQ = 0, /* left == right */
    NAOS_CMP_NE = 1, /* left != right */
    NAOS_CMP_LT = 2, /* left < right */
    NAOS_CMP_LE = 3, /* left <= right */
    NAOS_CMP_GT = 4, /* left > right */
    NAOS_CMP_GE = 5  /* left >= right */
} naos_cmp_t;

/* What a failed check does: NAOS_CHECK_ goes on, NAOS_REQUIRE_ stops the test. */
#define NAOS_CHECK_(call) ((void)(call))
#define NAOS_REQUIRE_(call) ((call) ? (void)0 : naos_stop())

/*
 * The forms of each kind of assertion; mode is NAOS_CHECK_ or NAOS_REQUIRE_. expr is the text of the
 * error line, made by the public macro so that its arguments are stringified as written.
 */
#define NAOS_INT_(mode, cmp, a, b, expr) mode(naos_check_int((cmp), (intmax_t)(a), (intmax_t)(b), (expr), __FILE__, __LINE__))
#define NAOS_UINT_(mode, cmp, a, b, expr) mode(naos_check_uint((cmp), (uintmax_t)(a), (uintmax_t)(b), (expr), __FILE__, __LINE__))
#define NAOS_STR_(mode, cmp, a, b, expr) mode(naos_check_str((cmp), (a), (b), (expr), __FILE__, __LINE__))

/* Boolean condition: true is any non-zero value. TRUE prints the expression, FALSE prints it as !(expr). */
#define NAOS_CHECK_TRUE(expr) NAOS_CHECK_(naos_check(((expr) ? 1 : 0), #expr, __FILE__, __LINE__))
#define NAOS_CHECK_FALSE(expr) NAOS_CHECK_(naos_check(((expr) ? 0 : 1), "!(" #expr ")", __FILE__, __LINE__))
#define NAOS_REQUIRE_TRUE(expr) NAOS_REQUIRE_(naos_check(((expr) ? 1 : 0), #expr, __FILE__, __LINE__))
#define NAOS_REQUIRE_FALSE(expr) NAOS_REQUIRE_(naos_check(((expr) ? 0 : 1), "!(" #expr ")", __FILE__, __LINE__))

/* Signed integers, compared as intmax_t. */
#define NAOS_CHECK_INT_EQ(a, b) NAOS_INT_(NAOS_CHECK_, NAOS_CMP_EQ, a, b, #a " == " #b)
#define NAOS_CHECK_INT_NE(a, b) NAOS_INT_(NAOS_CHECK_, NAOS_CMP_NE, a, b, #a " != " #b)
#define NAOS_CHECK_INT_LT(a, b) NAOS_INT_(NAOS_CHECK_, NAOS_CMP_LT, a, b, #a " < " #b)
#define NAOS_CHECK_INT_LE(a, b) NAOS_INT_(NAOS_CHECK_, NAOS_CMP_LE, a, b, #a " <= " #b)
#define NAOS_CHECK_INT_GT(a, b) NAOS_INT_(NAOS_CHECK_, NAOS_CMP_GT, a, b, #a " > " #b)
#define NAOS_CHECK_INT_GE(a, b) NAOS_INT_(NAOS_CHECK_, NAOS_CMP_GE, a, b, #a " >= " #b)
#define NAOS_REQUIRE_INT_EQ(a, b) NAOS_INT_(NAOS_REQUIRE_, NAOS_CMP_EQ, a, b, #a " == " #b)
#define NAOS_REQUIRE_INT_NE(a, b) NAOS_INT_(NAOS_REQUIRE_, NAOS_CMP_NE, a, b, #a " != " #b)
#define NAOS_REQUIRE_INT_LT(a, b) NAOS_INT_(NAOS_REQUIRE_, NAOS_CMP_LT, a, b, #a " < " #b)
#define NAOS_REQUIRE_INT_LE(a, b) NAOS_INT_(NAOS_REQUIRE_, NAOS_CMP_LE, a, b, #a " <= " #b)
#define NAOS_REQUIRE_INT_GT(a, b) NAOS_INT_(NAOS_REQUIRE_, NAOS_CMP_GT, a, b, #a " > " #b)
#define NAOS_REQUIRE_INT_GE(a, b) NAOS_INT_(NAOS_REQUIRE_, NAOS_CMP_GE, a, b, #a " >= " #b)

/* Unsigned integers, compared as uintmax_t, printed as NAOS_*_INT_*. */
#define NAOS_CHECK_UINT_EQ(a, b) NAOS_UINT_(NAOS_CHECK_, NAOS_CMP_EQ, a, b, #a " == " #b)
#define NAOS_CHECK_UINT_NE(a, b) NAOS_UINT_(NAOS_CHECK_, NAOS_CMP_NE, a, b, #a " != " #b)
#define NAOS_CHECK_UINT_LT(a, b) NAOS_UINT_(NAOS_CHECK_, NAOS_CMP_LT, a, b, #a " < " #b)
#define NAOS_CHECK_UINT_LE(a, b) NAOS_UINT_(NAOS_CHECK_, NAOS_CMP_LE, a, b, #a " <= " #b)
#define NAOS_CHECK_UINT_GT(a, b) NAOS_UINT_(NAOS_CHECK_, NAOS_CMP_GT, a, b, #a " > " #b)
#define NAOS_CHECK_UINT_GE(a, b) NAOS_UINT_(NAOS_CHECK_, NAOS_CMP_GE, a, b, #a " >= " #b)
#define NAOS_REQUIRE_UINT_EQ(a, b) NAOS_UINT_(NAOS_REQUIRE_, NAOS_CMP_EQ, a, b, #a " == " #b)
#define NAOS_REQUIRE_UINT_NE(a, b) NAOS_UINT_(NAOS_REQUIRE_, NAOS_CMP_NE, a, b, #a " != " #b)
#define NAOS_REQUIRE_UINT_LT(a, b) NAOS_UINT_(NAOS_REQUIRE_, NAOS_CMP_LT, a, b, #a " < " #b)
#define NAOS_REQUIRE_UINT_LE(a, b) NAOS_UINT_(NAOS_REQUIRE_, NAOS_CMP_LE, a, b, #a " <= " #b)
#define NAOS_REQUIRE_UINT_GT(a, b) NAOS_UINT_(NAOS_REQUIRE_, NAOS_CMP_GT, a, b, #a " > " #b)
#define NAOS_REQUIRE_UINT_GE(a, b) NAOS_UINT_(NAOS_REQUIRE_, NAOS_CMP_GE, a, b, #a " >= " #b)

/*
 * |a - b| <= tol. Equal values (including infinities) always match; NaN never does. The values
 * print as the shortest decimal that reads back to them (0.1, 1e-05, 1.5e+20), or as nan, inf, -inf
 * and -0.
 */
#define NAOS_CHECK_DOUBLE_NEAR(a, b, tol) NAOS_CHECK_(naos_check_double_near((a), (b), (tol), #a " ~= " #b, __FILE__, __LINE__))
#define NAOS_REQUIRE_DOUBLE_NEAR(a, b, tol) NAOS_REQUIRE_(naos_check_double_near((a), (b), (tol), #a " ~= " #b, __FILE__, __LINE__))

/*
 * NUL-terminated strings. Two NULLs are equal; NULL and a string are not. The values print on one
 * line in double quotes, with \n, \t, \r, \", \\ and \xNN for the other bytes below 0x20, 0x7F
 * and the bytes from 0x80, or as (null); STR_EQ adds the first difference, as MEM_EQ does.
 */
#define NAOS_CHECK_STR_EQ(a, b) NAOS_STR_(NAOS_CHECK_, NAOS_CMP_EQ, a, b, #a " == " #b)
#define NAOS_CHECK_STR_NE(a, b) NAOS_STR_(NAOS_CHECK_, NAOS_CMP_NE, a, b, #a " != " #b)
#define NAOS_REQUIRE_STR_EQ(a, b) NAOS_STR_(NAOS_REQUIRE_, NAOS_CMP_EQ, a, b, #a " == " #b)
#define NAOS_REQUIRE_STR_NE(a, b) NAOS_STR_(NAOS_REQUIRE_, NAOS_CMP_NE, a, b, #a " != " #b)

/* Byte ranges with explicit sizes. Pointers may be NULL when their size is 0. */
#define NAOS_CHECK_MEM_EQ(a, a_size, b, b_size) NAOS_CHECK_(naos_check_mem_eq((a), (a_size), (b), (b_size), #a " == " #b, __FILE__, __LINE__))
#define NAOS_REQUIRE_MEM_EQ(a, a_size, b, b_size) NAOS_REQUIRE_(naos_check_mem_eq((a), (a_size), (b), (b_size), #a " == " #b, __FILE__, __LINE__))

/* Object pointers, compared as const void *. */
#define NAOS_CHECK_PTR_EQ(a, b) NAOS_CHECK_(naos_check_ptr_eq((a), (b), #a " == " #b, __FILE__, __LINE__))
#define NAOS_CHECK_NULL(ptr) NAOS_CHECK_(naos_check_null((ptr), #ptr " == NULL", __FILE__, __LINE__))
#define NAOS_CHECK_NOT_NULL(ptr) NAOS_CHECK_(naos_check_not_null((ptr), #ptr " != NULL", __FILE__, __LINE__))
#define NAOS_REQUIRE_PTR_EQ(a, b) NAOS_REQUIRE_(naos_check_ptr_eq((a), (b), #a " == " #b, __FILE__, __LINE__))
#define NAOS_REQUIRE_NULL(ptr) NAOS_REQUIRE_(naos_check_null((ptr), #ptr " == NULL", __FILE__, __LINE__))
#define NAOS_REQUIRE_NOT_NULL(ptr) NAOS_REQUIRE_(naos_check_not_null((ptr), #ptr " != NULL", __FILE__, __LINE__))

/*
 * Recording functions used by the macros above, one per kind of assertion (NAOS_CHECK_* and
 * NAOS_REQUIRE_* share it). They return 1 when the check passes and 0 when it fails.
 * naos_check_str takes NAOS_CMP_EQ or NAOS_CMP_NE; a comparison outside naos_cmp_t never holds.
 */
NAOS_API int naos_check(int cond, const char *expr, const char *file, int line);
NAOS_API int naos_check_int(naos_cmp_t cmp, intmax_t left, intmax_t right, const char *expr, const char *file, int line);
NAOS_API int naos_check_uint(naos_cmp_t cmp, uintmax_t left, uintmax_t right, const char *expr, const char *file, int line);
NAOS_API int naos_check_double_near(double left, double right, double tol, const char *expr, const char *file, int line);
NAOS_API int naos_check_str(naos_cmp_t cmp, const char *left, const char *right, const char *expr, const char *file, int line);
NAOS_API int naos_check_mem_eq(const void *left, size_t left_size, const void *right, size_t right_size, const char *expr, const char *file, int line);
NAOS_API int naos_check_ptr_eq(const void *left, const void *right, const char *expr, const char *file, int line);
NAOS_API int naos_check_null(const void *ptr, const char *expr, const char *file, int line);
NAOS_API int naos_check_not_null(const void *ptr, const char *expr, const char *file, int line);

NAOS_END_DECLS

#endif
