/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 */

/* The real module_b, linked by the program and never by the tests. */

#include "module_b.h"

#include <stdio.h>

int func_b(int x)
{
    return printf("device <- %d\n", x) < 0 ? -1 : 0;
}
