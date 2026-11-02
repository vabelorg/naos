/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * naos/control.h: notes, traces, and the stops of NAOS_FAIL, NAOS_SKIP and NAOS_REQUIRE_*.
 */

#include <naos/control.h>

#include <stdarg.h>
#include <stddef.h>

#include "diagnostic.h"
#include "test.h"

void naos_note(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    naos_diagnostic_set_note(fmt, args);
    va_end(args);
}

void naos_trace_push(const char *file, int line, const char *fmt, ...)
{
    va_list args;
    int kept;

    va_start(args, fmt);
    kept = naos_diagnostic_push_trace(file, line, fmt, args);
    va_end(args);

    if (kept == 0)
    {
        naos_diagnostic_misuse(file, line, "NAOS_TRACE_PUSH beyond " NAOS_DIAGNOSTIC_TRACE_DEPTH_STRING " levels");
    }
}

void naos_trace_pop(const char *file, int line)
{
    if (naos_diagnostic_pop_trace() == 0)
    {
        naos_diagnostic_misuse(file, line, "NAOS_TRACE_POP without NAOS_TRACE_PUSH");
    }
}

void naos_fail(const char *msg, const char *file, int line)
{
    naos_diagnostic_begin(NAOS_DIAGNOSTIC_ERROR, file, line, (msg != NULL) ? msg : "(no message)");
    naos_diagnostic_end();
    naos_test_leave();
}

void naos_skip(const char *msg, const char *file, int line)
{
    naos_diagnostic_begin(NAOS_DIAGNOSTIC_SKIP, file, line, (msg != NULL) ? msg : "(no message)");
    naos_diagnostic_end();
    naos_test_leave();
}

void naos_stop(void)
{
    naos_test_leave();
}
