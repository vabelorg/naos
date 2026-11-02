/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * Matching the name of a test against the patterns of --naos-filter. Uses no other Naos module.
 */

#ifndef NAOS_FILTER_H
#define NAOS_FILTER_H

/*
 * Tells whether filter, the patterns of --naos-filter, selects the test <suite>.<test>; a NULL
 * filter selects every test. The filter is
 * "<positive>[-<negative>]", each a ':'-separated list of patterns. A test is
 * selected when it matches a positive pattern, or there are none, and no negative one. Names hold
 * only letters, digits and '_' (suite.c rejects any other), so never '*', '?', ':' or '-'.
 */
int naos_filter_matches(const char *filter, const char *suite, const char *test);

#endif
