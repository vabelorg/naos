/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * Stub of module_b, linked by the tests in place of module_b.c.
 */

#include "fake_module_b.h"

#include "module_b.h"

int fake_b_return;
int fake_b_calls;
int fake_b_last_arg;

int func_b(int x)
{
    fake_b_calls++;
    fake_b_last_arg = x;

    return fake_b_return;
}
