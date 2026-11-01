/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * The functions every formatted output of Naos goes through: the C99 ones of the C library Naos is
 * built with. src/ calls them by these names, never printf and the others directly.
 *
 * NAOS_PRINTF_FORMAT (naos/export.h) declares the printf-like functions of Naos with the formats of
 * C99, and these functions are what makes that true. On MinGW, the default functions are those of
 * the Microsoft C library, and the old one (msvcrt) does not know %zu, %jd or %ju and writes the
 * exponent of %e with three digits. The __mingw_ functions of mingw-w64 follow C99. Its <stdio.h>
 * declares them whatever __USE_MINGW_ANSI_STDIO is, so the result does not depend on how Naos is
 * built. This is the only place that depends on the C library.
 */

#ifndef NAOS_FORMAT_H
#define NAOS_FORMAT_H

#include <stdio.h>

#if defined(__MINGW32__)
#define naos_printf __mingw_printf
#define naos_fprintf __mingw_fprintf
#define naos_vfprintf __mingw_vfprintf
#define naos_snprintf __mingw_snprintf
#define naos_vsnprintf __mingw_vsnprintf
#else
#define naos_printf printf
#define naos_fprintf fprintf
#define naos_vfprintf vfprintf
#define naos_snprintf snprintf
#define naos_vsnprintf vsnprintf
#endif

#endif
