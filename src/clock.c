/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * The clock that times the tests (clock.h): the monotonic clock of the operating system, so that
 * a test that sleeps or waits shows the time it took, the same on every platform.
 *
 *   Windows                       QueryPerformanceCounter, in kernel32, which is always linked
 *   macOS                         clock_gettime(CLOCK_MONOTONIC), in libc since 10.12
 *   other POSIX systems           clock_gettime(CLOCK_MONOTONIC), in libc since glibc 2.17
 *   anything else, or no clock    clock(): processor time, which the help says
 *
 * These belong to the operating system and need no library to install or link, so Naos still
 * depends on nothing besides the standard C library (docs/development/architecture.md). This is
 * the only file of src/ that depends on the platform.
 */

/*
 * clock_gettime() is POSIX, which -std=c99 leaves out of <time.h> unless asked for, before any
 * include. macOS is left as it is: there, _POSIX_C_SOURCE would hide more than it shows, and
 * without it the headers declare everything.
 */
#if !defined(_WIN32) && !defined(__APPLE__) && (defined(__unix__) || defined(__unix)) && !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 199309L
#endif

#include "clock.h"

#include <time.h>

#if defined(_WIN32)
#define NAOS_CLOCK_WINDOWS
#elif defined(__APPLE__)
#define NAOS_CLOCK_POSIX
#elif defined(__unix__) || defined(__unix)
/*
 * <unistd.h> tells what POSIX provides. _POSIX_TIMERS must be greater than 0. _POSIX_MONOTONIC_CLOCK
 * may be 0, as glibc has it, which means that the clock is known only at run time; -1 means none.
 */
#include <unistd.h>
#if defined(_POSIX_TIMERS) && (_POSIX_TIMERS > 0) && defined(_POSIX_MONOTONIC_CLOCK) && (_POSIX_MONOTONIC_CLOCK >= 0)
#define NAOS_CLOCK_POSIX
#endif
#endif

#if defined(NAOS_CLOCK_WINDOWS)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#define NAOS_CLOCK_NANOSECONDS_PER_SECOND 1000000000ULL

#define NAOS_CLOCK_WALL "wall-clock time, from a monotonic clock"
#define NAOS_CLOCK_PROCESSOR "processor time, from clock(), not wall-clock time"

/* ticks of a clock that counts per_second of them in a second, in nanoseconds, without overflowing first. */
static naos_nanoseconds_t to_nanoseconds(naos_nanoseconds_t ticks, naos_nanoseconds_t per_second)
{
    return ((ticks / per_second) * NAOS_CLOCK_NANOSECONDS_PER_SECOND) + (((ticks % per_second) * NAOS_CLOCK_NANOSECONDS_PER_SECOND) / per_second);
}

#if !defined(NAOS_CLOCK_WINDOWS)

/* The processor time of clock(), when there is no monotonic clock; 0 when clock() has none either. */
static naos_nanoseconds_t processor_now(void)
{
    clock_t now;

    now = clock();
    if (now == (clock_t)-1)
    {
        return 0;
    }

    return to_nanoseconds((naos_nanoseconds_t)now, (naos_nanoseconds_t)CLOCKS_PER_SEC);
}

#endif

#if defined(NAOS_CLOCK_WINDOWS)

/*
 * QueryPerformanceFrequency() never changes while the system runs, and never fails from Windows XP
 * on; 1 all the same if it did, so that nothing is divided by 0.
 */
static naos_nanoseconds_t g_frequency = 0;

naos_nanoseconds_t naos_clock_now(void)
{
    LARGE_INTEGER value;

    if (g_frequency == 0)
    {
        g_frequency = 1;
        if ((QueryPerformanceFrequency(&value) != 0) && (value.QuadPart > 0))
        {
            g_frequency = (naos_nanoseconds_t)value.QuadPart;
        }
    }

    QueryPerformanceCounter(&value);

    return to_nanoseconds((naos_nanoseconds_t)value.QuadPart, g_frequency);
}

const char *naos_clock_description(void)
{
    return NAOS_CLOCK_WALL;
}

#elif defined(NAOS_CLOCK_POSIX)

/* Whether CLOCK_MONOTONIC works, known at the first call: -1 until then. */
static int g_monotonic = -1;

/* Whether CLOCK_MONOTONIC works, which a system may tell only at run time (_POSIX_MONOTONIC_CLOCK 0). */
static int monotonic(void)
{
    struct timespec now;

    if (g_monotonic == -1)
    {
        g_monotonic = (clock_gettime(CLOCK_MONOTONIC, &now) == 0) ? 1 : 0;
    }

    return g_monotonic;
}

/*
 * Once CLOCK_MONOTONIC has worked, it is always used: a clock that changed during the run could go
 * back. It cannot fail then, since its id is valid and now is a valid address.
 */
naos_nanoseconds_t naos_clock_now(void)
{
    struct timespec now;

    if (monotonic() == 0)
    {
        return processor_now();
    }

    now.tv_sec = 0;
    now.tv_nsec = 0;
    (void)clock_gettime(CLOCK_MONOTONIC, &now);

    return ((naos_nanoseconds_t)now.tv_sec * NAOS_CLOCK_NANOSECONDS_PER_SECOND) + (naos_nanoseconds_t)now.tv_nsec;
}

const char *naos_clock_description(void)
{
    return (monotonic() != 0) ? NAOS_CLOCK_WALL : NAOS_CLOCK_PROCESSOR;
}

#else

naos_nanoseconds_t naos_clock_now(void)
{
    return processor_now();
}

const char *naos_clock_description(void)
{
    return NAOS_CLOCK_PROCESSOR;
}

#endif
