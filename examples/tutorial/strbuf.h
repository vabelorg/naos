/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * A growable, NUL-terminated string buffer.
 */

#ifndef STRBUF_H
#define STRBUF_H

#include <stddef.h>

struct strbuf
{
    char *data;
    size_t length;   /* bytes in use, not counting the NUL */
    size_t capacity; /* bytes allocated */
};

/* Makes sb an empty buffer. Returns 0, or -1 when out of memory (sb is then zeroed). */
int strbuf_init(struct strbuf *sb);

/* Frees the memory of sb and zeroes it. Safe on a zeroed or already freed buffer. */
void strbuf_free(struct strbuf *sb);

/* Appends text to sb. Returns 0, or -1 when out of memory (sb is then unchanged). */
int strbuf_append(struct strbuf *sb, const char *text);

/* Returns the content of sb as a C string, valid until sb changes. */
const char *strbuf_cstr(const struct strbuf *sb);

#endif
