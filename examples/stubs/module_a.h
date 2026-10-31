/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 */

#ifndef MODULE_A_H
#define MODULE_A_H

#define A_OK 0
#define A_ERROR 1

/* Sends x through module_b. Returns A_OK, or A_ERROR when module_b fails. */
int func_a(int x);

#endif
