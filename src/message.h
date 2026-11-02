/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * The messages of Naos itself, such as a usage error or a lack of memory, on stderr. Not an event of
 * the run: the writers never see them. Uses no other Naos module.
 */

#ifndef NAOS_MESSAGE_H
#define NAOS_MESSAGE_H

#include <stdarg.h>

#include <naos/export.h>

/*
 * Writes a message of Naos itself to stderr as "naos: <message>\n", formatted as printf does, after
 * flushing stdout so that it comes after what the tests have written; then flushes stderr.
 */
void naos_message(const char *format, ...) NAOS_PRINTF_FORMAT(1, 2);
void naos_message_v(const char *format, va_list args);

#endif
