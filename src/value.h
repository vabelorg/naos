/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * Writing the values of the detail lines in one fixed form. Stateless, and uses no other Naos module.
 */

#ifndef NAOS_VALUE_H
#define NAOS_VALUE_H

#include <stddef.h>

/*
 * Size of a buffer that holds every double formatted by naos_value_format_double: the longest
 * results, -0.000 followed by 17 digits and -d.dddddddddddddddde-308, have 23 and 24 characters.
 */
#define NAOS_VALUE_DOUBLE_SIZE 32

/* Size of a buffer that holds how one byte of a string value is written: \xNN and its NUL. */
#define NAOS_VALUE_BYTE_SIZE 5

/*
 * Writes into text, NUL-terminated, how one byte of a C string of the test is written, so that the
 * string, between double quotes, stays on one line: \n, \t, \r, \", \\ and \xNN (two upper-case hex
 * digits) for every other byte below 0x20, 0x7F and every byte from 0x80, the byte itself otherwise.
 * text has NAOS_VALUE_BYTE_SIZE chars. Returns the length written, from 1 to 4.
 */
size_t naos_value_escape_byte(unsigned char byte, char *text);

/*
 * Formats value into buffer as the shortest decimal that reads back to it: plain for decimal
 * exponents from -4 to 16, otherwise d.ddde+XX; nan, inf, -inf and -0 for the special values; the
 * decimal point is always '.'. size is at least NAOS_VALUE_DOUBLE_SIZE.
 */
void naos_value_format_double(char *buffer, size_t size, double value);

#endif
