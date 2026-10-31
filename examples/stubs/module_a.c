/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 */

#include "module_a.h"

#include "module_b.h"

int func_a(int x)
{
    if (func_b(x) != 0)
    {
        return A_ERROR;
    }

    return A_OK;
}
