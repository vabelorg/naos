/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * A text that grows as it is added to (text.h): it doubles its memory when it runs out of room.
 */

#include "text.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "format.h"

/* Size of the first allocation of a text; each next one doubles it. */
#define NAOS_TEXT_FIRST_SIZE 256

/* Size of what naos_text_add_format() formats: a number of any %d, %ld, %lu or %zu, with a few chars around it. */
#define NAOS_TEXT_NUMBER_SIZE 48

/* Makes room for more chars in text. Returns 0, and from then on adds nothing, when memory runs out. */
static int reserve(naos_text_t *text, size_t more)
{
    size_t capacity;
    char *data;

    if (text->failed != 0)
    {
        return 0;
    }

    if (more <= (text->capacity - text->length))
    {
        return 1;
    }

    capacity = (text->capacity == 0) ? NAOS_TEXT_FIRST_SIZE : text->capacity;
    while ((capacity - text->length) < more)
    {
        if (capacity > (SIZE_MAX / 2))
        {
            text->failed = 1;

            return 0;
        }

        capacity *= 2;
    }

    data = (char *)realloc(text->data, capacity);
    if (data == NULL)
    {
        text->failed = 1;

        return 0;
    }

    text->data = data;
    text->capacity = capacity;

    return 1;
}

/* bytes may be NULL when length is 0, as the data of a text nothing was added to: memcpy may not be called then. */
void naos_text_add_bytes(naos_text_t *text, const char *bytes, size_t length)
{
    if (length == 0)
    {
        return;
    }

    if (reserve(text, length) != 0)
    {
        memcpy(text->data + text->length, bytes, length);
        text->length += length;
    }
}

void naos_text_add(naos_text_t *text, const char *string)
{
    naos_text_add_bytes(text, string, strlen(string));
}

void naos_text_add_format(naos_text_t *text, const char *format, ...)
{
    char buffer[NAOS_TEXT_NUMBER_SIZE];
    va_list args;

    va_start(args, format);
    naos_vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    naos_text_add(text, buffer);
}

void naos_text_release(naos_text_t *text)
{
    free(text->data);
    text->data = NULL;
    text->length = 0;
    text->capacity = 0;
    text->failed = 0;
}
