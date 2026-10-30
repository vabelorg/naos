/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * Linkage and compiler attributes of the public Naos functions, used by every public header.
 */

#ifndef NAOS_EXPORT_H
#define NAOS_EXPORT_H

/*
 * NAOS_API marks the functions exported from a shared Naos.
 * - Static library (the default, also without CMake): define nothing.
 * - Shared library: define NAOS_SHARED when using it (CMake does this for consumers) and
 *   NAOS_BUILDING while building Naos itself.
 */
#if defined(NAOS_BUILDING) || defined(NAOS_SHARED)
#if defined(_WIN32) || defined(__CYGWIN__)
#if defined(NAOS_BUILDING)
#define NAOS_API __declspec(dllexport)
#else
#define NAOS_API __declspec(dllimport)
#endif
#elif defined(__GNUC__) || defined(__clang__)
#define NAOS_API __attribute__((visibility("default")))
#else
#define NAOS_API
#endif
#else
#define NAOS_API
#endif

/* Lets GCC and Clang check the arguments of a printf-like function against its format. */
#if defined(__GNUC__) || defined(__clang__)
#define NAOS_PRINTF_FORMAT(format_index, first_arg_index) __attribute__((format(printf, format_index, first_arg_index)))
#else
#define NAOS_PRINTF_FORMAT(format_index, first_arg_index)
#endif

/*
 * Marks the functions that leave through longjmp. Not C11 _Noreturn (Naos is C99, and C23 deprecates
 * it) nor C++ [[noreturn]] (which would have to come before NAOS_API); empty for other compilers.
 */
#if defined(__GNUC__) || defined(__clang__)
#define NAOS_NORETURN __attribute__((noreturn))
#elif defined(_MSC_VER)
#define NAOS_NORETURN __declspec(noreturn)
#else
#define NAOS_NORETURN
#endif

/*
 * C linkage for C++ consumers: NAOS_BEGIN_DECLS and NAOS_END_DECLS enclose the declarations of a
 * header, and NAOS_EXTERN_C declares one function from a macro.
 */
#ifdef __cplusplus
#define NAOS_BEGIN_DECLS extern "C" {
#define NAOS_END_DECLS }
#define NAOS_EXTERN_C extern "C"
#else
#define NAOS_BEGIN_DECLS
#define NAOS_END_DECLS
#define NAOS_EXTERN_C extern
#endif

#endif
