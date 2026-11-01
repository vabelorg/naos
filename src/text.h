/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * A text that grows as it is added to, for a writer that keeps what it writes until later. Uses no
 * other Naos module.
 */

#ifndef NAOS_TEXT_H
#define NAOS_TEXT_H

#include <stddef.h>

#include <naos/export.h>

/*
 * The chars added so far, not NUL-terminated; data is NULL until the first add. When memory runs
 * out, failed is set and nothing more is added until the text is released, or failed is cleared by
 * the writer that knows what to do with what it has.
 */
typedef struct naos_text
{
    char *data;
    size_t length;
    size_t capacity;
    int failed;
} naos_text_t;

/* An empty text, to initialize one with. */
#define NAOS_TEXT_EMPTY { NULL, 0, 0, 0 }

/* Adds length bytes; bytes may be NULL when length is 0. */
void naos_text_add_bytes(naos_text_t *text, const char *bytes, size_t length);

/* Adds a NUL-terminated string. */
void naos_text_add(naos_text_t *text, const char *string);

/* Adds numbers formatted as printf does; the result has fewer than 48 chars. */
void naos_text_add_format(naos_text_t *text, const char *format, ...) NAOS_PRINTF_FORMAT(2, 3);

/* Frees the chars and makes text empty, failed cleared. */
void naos_text_release(naos_text_t *text);

#endif
