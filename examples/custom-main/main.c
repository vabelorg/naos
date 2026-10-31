/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 */

#include <naos/naos.h>

#include <stdio.h>
#include <string.h>

NAOS_DECLARE_SUITE(quick);
NAOS_DECLARE_SUITE(slow);

int main(int argc, char **argv)
{
    int i;

    /* Removes the --naos-* options: only the arguments of the program are left. */
    naos_init(&argc, argv);

    /* The shared library loaded may be older than the headers the tests were built with. */
    if (naos_version_number() != NAOS_VERSION_NUMBER)
    {
        fprintf(stderr, "naos %s is loaded, but the tests were built with %s\n",
            naos_version_string(), NAOS_VERSION_STRING);
        return 1;
    }

    NAOS_ADD_SUITE(quick);

    for (i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--slow") == 0)
        {
            NAOS_ADD_SUITE(slow);
        }
    }

    return naos_run();
}
