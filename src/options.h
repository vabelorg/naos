/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * The command line of naos_init().
 */

#ifndef NAOS_OPTIONS_H
#define NAOS_OPTIONS_H

#include <stdio.h>

/* Command line, set by naos_options_parse() and only read afterwards. */
typedef struct naos_options
{
    const char *program;
    const char *filter;
    int list_only;
    int shuffle;
    unsigned long random_seed; /* 0 until given, or when given as 0: chosen at run */
    long repeat;
    int fail_fast;
    int help_requested;
    int usage_error;
} naos_options_t;

const naos_options_t *naos_options_get(void);

/*
 * Does the work of naos_init(): takes the Naos options out of argv into the options, then prints
 * the usage after an error, or else the help when asked.
 */
void naos_options_parse(int *argc, char **argv);

void naos_options_print_usage(FILE *out, const char *program);

#endif
