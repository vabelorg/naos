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

/*
 * Writes a C string of the test to stdout on one line: (null) for NULL, otherwise the bytes between
 * double quotes, with \n, \t, \r, \", \\ and \xNN (two upper-case hex digits) for every other byte
 * below 0x20, 0x7F and every byte from 0x80. Nothing is flushed.
 */
void naos_value_print_string(const char *value);

/*
 * Formats value into buffer as the shortest decimal that reads back to it: plain for decimal
 * exponents from -4 to 16, otherwise d.ddde+XX; nan, inf, -inf and -0 for the special values; the
 * decimal point is always '.'. size is at least NAOS_VALUE_DOUBLE_SIZE.
 */
void naos_value_format_double(char *buffer, size_t size, double value);

#endif
