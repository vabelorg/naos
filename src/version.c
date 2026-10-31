/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * naos/version.h: the version of the library actually linked.
 */

#include <naos/version.h>

const char *naos_version_string(void)
{
    return NAOS_VERSION_STRING;
}

unsigned long naos_version_number(void)
{
    return NAOS_VERSION_NUMBER;
}
