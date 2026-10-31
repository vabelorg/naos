/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 */

#include "strbuf.h"

#include <stdlib.h>
#include <string.h>

int strbuf_init(struct strbuf *sb)
{
    sb->length = 0;
    sb->capacity = 16;
    sb->data = malloc(sb->capacity);
    if (sb->data == NULL)
    {
        sb->capacity = 0;
        return -1;
    }

    sb->data[0] = '\0';
    return 0;
}

void strbuf_free(struct strbuf *sb)
{
    free(sb->data);
    sb->data = NULL;
    sb->length = 0;
    sb->capacity = 0;
}

int strbuf_append(struct strbuf *sb, const char *text)
{
    size_t size = strlen(text);
    size_t needed = sb->length + size + 1;

    if (needed > sb->capacity)
    {
        size_t capacity = sb->capacity * 2;
        char *data;

        if (capacity < needed)
        {
            capacity = needed;
        }

        data = realloc(sb->data, capacity);
        if (data == NULL)
        {
            return -1;
        }

        sb->data = data;
        sb->capacity = capacity;
    }

    memcpy(sb->data + sb->length, text, size + 1);
    sb->length += size;
    return 0;
}

const char *strbuf_cstr(const struct strbuf *sb)
{
    return sb->data;
}
