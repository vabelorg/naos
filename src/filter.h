/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * Selecting the tests with the patterns of --naos-filter.
 */

#ifndef NAOS_FILTER_H
#define NAOS_FILTER_H

#include "suite_internal.h"

/*
 * Tells whether filter, the patterns of --naos-filter, selects the test. The filter is
 * "<positive>[-<negative>]", each a ':'-separated list of patterns. A test is
 * selected when it matches a positive pattern, or there are none, and no negative one. Names hold
 * only letters, digits and '_' (suite.c rejects any other), so never '*', '?', ':' or '-'.
 */
int naos_filter_matches(const naos_suite_t *suite, const naos_test_t *test, const char *filter);

#endif
