/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * A process ending in the middle of a test: what Naos has written out by then is left.
 */

#include <naos/naos.h>

#include <stdlib.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

/*
 * Ends the process with exit code 3 without writing out the buffers of stdio, so what is left of the
 * output is what Naos wrote out before. _Exit of the Windows CRT still writes them out, and only
 * TerminateProcess leaves them.
 */
static void exit_without_flush(void)
{
#ifdef _WIN32
    TerminateProcess(GetCurrentProcess(), 3);
#endif
    _Exit(3);
}

/* The full run leaves this suite out, since its tests end the process. */
NAOS_TEST(exits)
{
    exit_without_flush();
}

NAOS_TEST(exits_after_failure)
{
    NAOS_CHECK_INT_EQ(1, 2);
    exit_without_flush();
}

NAOS_SUITE(crash,
    NAOS_ADD_TEST(exits),
    NAOS_ADD_TEST(exits_after_failure)
)
