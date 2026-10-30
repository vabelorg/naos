/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * The random order of --naos-shuffle and its seeds.
 */

#ifndef NAOS_ORDER_H
#define NAOS_ORDER_H

#include <stddef.h>

/* The seeds go from 1 to NAOS_ORDER_SEED_MAX; 0 stands for none given. The string is for messages. */
#define NAOS_ORDER_SEED_MAX 99999UL
#define NAOS_ORDER_SEED_MAX_STRING "99999"

/* Seed of the first round: seed when given, or else one from the time. */
unsigned long naos_order_pick_seed(unsigned long seed);

/* Seed of the round after the one of seed: 1 more, from NAOS_ORDER_SEED_MAX back to 1. */
unsigned long naos_order_next_seed(unsigned long seed);

/* Starts the random order of seed, from 1 to NAOS_ORDER_SEED_MAX. */
void naos_order_start(unsigned long seed);

/* Fills order with 0 to count - 1, shuffled (Fisher-Yates) when shuffle is not 0. */
void naos_order_make(size_t *order, size_t count, int shuffle);

#endif
