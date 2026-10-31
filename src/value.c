/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * Writing the values of the detail lines in one fixed form (value.h): escaped strings and shortest
 * doubles.
 */

#include "value.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/* %.16e writes 17 significant digits, which read back to every double. */
#define MAX_PRECISION 16
#define MAX_DIGITS (MAX_PRECISION + 1)

/*
 * Large enough for %.16e of any double with a decimal point of several bytes, which a locale may
 * have; the decimal point is read past and never copied into the result.
 */
#define SCIENTIFIC_SIZE 64

/*
 * Writes one byte of a string value: the escape of C for the bytes that have a short one, \xNN for
 * the other bytes that are not printable ASCII, and the byte itself otherwise. The digits do not
 * depend on the locale, so the same bytes are always written the same way.
 */
static void print_string_byte(unsigned char byte)
{
    static const char hex_digits[] = "0123456789ABCDEF";

    switch (byte)
    {
    case '\n':
        fputs("\\n", stdout);
        return;
    case '\t':
        fputs("\\t", stdout);
        return;
    case '\r':
        fputs("\\r", stdout);
        return;
    case '"':
        fputs("\\\"", stdout);
        return;
    case '\\':
        fputs("\\\\", stdout);
        return;
    default:
        break;
    }

    if ((byte < 0x20) || (byte >= 0x7F))
    {
        fputs("\\x", stdout);
        fputc(hex_digits[byte >> 4], stdout);
        fputc(hex_digits[byte & 0x0F], stdout);
    }
    else
    {
        fputc(byte, stdout);
    }
}

/*
 * Writes magnitude, a finite double above zero, with %.*e at the first precision from 0 whose text
 * reads back to it, and at precision 16 when no shorter one does. The locale decides only the
 * decimal point of this text, and strtod reads it with the same locale.
 */
static void format_shortest_scientific(char *scientific, size_t size, double magnitude)
{
    int precision;

    for (precision = 0;; precision++)
    {
        if (snprintf(scientific, size, "%.*e", precision, magnitude) < 0)
        {
            scientific[0] = '\0';
        }

        if ((precision == MAX_PRECISION) || (strtod(scientific, NULL) == magnitude))
        {
            return;
        }
    }
}

/*
 * Reads the digits of the mantissa of a %e text into digits, past whatever the decimal point is, and
 * returns their count without the trailing '0' digits; *exponent is the decimal exponent.
 */
static size_t read_scientific(const char *scientific, char *digits, int *exponent)
{
    const char *cursor;
    size_t count;

    count = 0;
    for (cursor = scientific; (*cursor != '\0') && (*cursor != 'e') && (*cursor != 'E'); cursor++)
    {
        if ((*cursor >= '0') && (*cursor <= '9') && (count < MAX_DIGITS))
        {
            digits[count] = *cursor;
            count++;
        }
    }

    *exponent = (*cursor != '\0') ? (int)strtol(cursor + 1, NULL, 10) : 0;

    /* A text without digits (a failed snprintf) reads as 0, so digits always has one. */
    if (count == 0)
    {
        digits[count++] = '0';
    }

    while ((count > 1) && (digits[count - 1] == '0'))
    {
        count--;
    }

    return count;
}

/* Writes digits with the decimal exponent exponent, from -4 to 16, without an exponent. */
static size_t write_plain(char *text, size_t length, const char *digits, size_t count, int exponent)
{
    size_t index;
    size_t integer_count;

    if (exponent < 0)
    {
        text[length++] = '0';
        text[length++] = '.';
        for (index = 1; index < (size_t)(-exponent); index++)
        {
            text[length++] = '0';
        }

        for (index = 0; index < count; index++)
        {
            text[length++] = digits[index];
        }

        return length;
    }

    integer_count = (size_t)exponent + 1;
    for (index = 0; index < integer_count; index++)
    {
        text[length++] = (index < count) ? digits[index] : '0';
    }

    if (count > integer_count)
    {
        text[length++] = '.';
        for (index = integer_count; index < count; index++)
        {
            text[length++] = digits[index];
        }
    }

    return length;
}

/* Writes digits as d[.ddd]e followed by the sign and at least two digits of exponent. */
static size_t write_exponent(char *text, size_t length, const char *digits, size_t count, int exponent)
{
    size_t index;
    int magnitude;

    text[length++] = digits[0];
    if (count > 1)
    {
        text[length++] = '.';
        for (index = 1; index < count; index++)
        {
            text[length++] = digits[index];
        }
    }

    text[length++] = 'e';
    text[length++] = (exponent < 0) ? '-' : '+';
    magnitude = (exponent < 0) ? -exponent : exponent;
    if (magnitude >= 100)
    {
        text[length++] = (char)('0' + (magnitude / 100));
    }

    text[length++] = (char)('0' + ((magnitude / 10) % 10));
    text[length++] = (char)('0' + (magnitude % 10));

    return length;
}

/* Every byte up to the terminating NUL is written; nothing is cut and nothing is buffered. */
void naos_value_print_string(const char *value)
{
    const unsigned char *byte;

    if (value == NULL)
    {
        fputs("(null)", stdout);
        return;
    }

    fputc('"', stdout);
    for (byte = (const unsigned char *)value; *byte != '\0'; byte++)
    {
        print_string_byte(*byte);
    }
    fputc('"', stdout);
}

/*
 * The digits and the decimal exponent come from %e, and the result is put together here, so neither
 * the form of %g nor the decimal point of the locale reaches it.
 */
void naos_value_format_double(char *buffer, size_t size, double value)
{
    char scientific[SCIENTIFIC_SIZE];
    char digits[MAX_DIGITS];
    char text[NAOS_VALUE_DOUBLE_SIZE];
    size_t count;
    size_t length;
    int exponent;

    if (isnan(value))
    {
        snprintf(buffer, size, "nan");
        return;
    }

    if (isinf(value))
    {
        snprintf(buffer, size, "%s", (value < 0) ? "-inf" : "inf");
        return;
    }

    if (value == 0)
    {
        snprintf(buffer, size, "%s", signbit(value) ? "-0" : "0");
        return;
    }

    format_shortest_scientific(scientific, sizeof(scientific), (value < 0) ? -value : value);
    count = read_scientific(scientific, digits, &exponent);

    length = 0;
    if (value < 0)
    {
        text[length++] = '-';
    }

    if ((exponent >= -4) && (exponent < 17))
    {
        length = write_plain(text, length, digits, count, exponent);
    }
    else
    {
        length = write_exponent(text, length, digits, count, exponent);
    }

    text[length] = '\0';

    snprintf(buffer, size, "%s", text);
}
