/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * Controlling the running test: its note and traces, failing, skipping and stopping it, a part of
 * naos/naos.h: include that header.
 *
 * NAOS_FAIL, NAOS_SKIP and NAOS_REQUIRE_* of naos/check.h stop the test through longjmp, from any
 * function, however deep in helpers and whatever it returns: from setup or the body they go to
 * teardown, and from teardown to the end of teardown. NAOS_FAIL and a failed NAOS_REQUIRE_* are
 * fatal failures; NAOS_CHECK_* failures and NAOS_SKIP are not. Since the frames in between are left
 * at once:
 * - cleanup they would do (free, for example) does not run; keep it in teardown
 * - in C++, those frames must not hold objects with destructors (skipping them is undefined), and
 *   constructors and destructors must not use these macros
 * - outside a running test (before naos_run(), in a suite function) there is nowhere to go: they
 *   print a message and exit with NAOS_EXIT_USAGE
 * - they cannot leave from another thread than the one running the test
 *
 * The macros call the functions below, and NAOS_REQUIRE_* calls naos_stop(); none is meant to
 * be called directly.
 */

#ifndef NAOS_CONTROL_H
#define NAOS_CONTROL_H

#include <naos/export.h>

NAOS_BEGIN_DECLS

/*
 * Formats a note as printf does (C99 vsnprintf; long notes are cut short at 511 bytes). The next
 * assertion prints it as its "note" detail line, after the values, if it fails, and uses it up
 * either way; NAOS_FAIL and NAOS_SKIP print it too. A new note replaces one not yet used. Each
 * test, and its teardown, starts without a note.
 *     for (i = 0; i < n; i++)
 *     {
 *         NAOS_NOTE("i=%d", i);
 *         NAOS_CHECK_INT_EQ(expected[i], actual[i]);
 *     }
 * The format is part of __VA_ARGS__, so a note without arguments, NAOS_NOTE("text"), is valid C99.
 */
#define NAOS_NOTE(...) naos_note(__VA_ARGS__)

/*
 * Pushes a note, formatted as NAOS_NOTE does (long ones cut short at 255 bytes), that every failure
 * and skip prints until the matching NAOS_TRACE_POP: after its detail lines, each trace is a
 * "<file>:<line>: note: <text>" line (MSVC form as in naos/check.h) with the file and line of the
 * push, innermost first. Pushes and pops must pair up:
 *     for (i = 0; i < n; i++)
 *     {
 *         NAOS_TRACE_PUSH("i=%d", i);
 *         check_item(&items[i]);
 *         NAOS_TRACE_POP();
 *     }
 * Setup and the body share one stack, teardown starts its own: both start empty, so a failure in
 * teardown never prints what the body pushed. A fatal failure or a skip leaves the pops after it
 * out, which is fine. Setup and the body, or teardown, returning with traces still pushed is
 * recorded as a failure (naming the innermost one), as is a pop without a push and a push beyond
 * 16 levels. Those beyond 16 are counted for the pops but not kept: while they are pushed, a
 * failure or skip ends its detail lines with "traces: <n> innermost not kept".
 */
#define NAOS_TRACE_PUSH(...) naos_trace_push(__FILE__, __LINE__, __VA_ARGS__)
#define NAOS_TRACE_POP() naos_trace_pop(__FILE__, __LINE__)

/* Records a fatal failure and stops the test. */
#define NAOS_FAIL(message) naos_fail((message), __FILE__, __LINE__)

/* Marks the test as skipped and stops it. A failure still wins. */
#define NAOS_SKIP(message) naos_skip((message), __FILE__, __LINE__)

NAOS_API void naos_note(const char *fmt, ...) NAOS_PRINTF_FORMAT(1, 2);
NAOS_API void naos_trace_push(const char *file, int line, const char *fmt, ...) NAOS_PRINTF_FORMAT(3, 4);
NAOS_API void naos_trace_pop(const char *file, int line);

/* Record a fatal failure (naos_fail) or a skip (naos_skip), then leave through naos_stop(). */
NAOS_API NAOS_NORETURN void naos_fail(const char *msg, const char *file, int line);
NAOS_API NAOS_NORETURN void naos_skip(const char *msg, const char *file, int line);

/* Leaves setup and the body, or teardown, of the running test through longjmp. */
NAOS_API NAOS_NORETURN void naos_stop(void);

NAOS_END_DECLS

#endif
