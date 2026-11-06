/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * The messages of Naos itself on stderr (message.h).
 */

#include "message.h"


#include "format.h"

void naos_message(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    naos_message_v(format, args);
    va_end(args);
}

void naos_message_v(const char *format, va_list args)
{
    fflush(stdout);
    fputs("naos: ", stderr);
    naos_vfprintf(stderr, format, args);
    fputc('\n', stderr);
    fflush(stderr);
}
