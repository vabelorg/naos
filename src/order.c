/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * The random order of --naos-shuffle and its seeds (order.h), with a generator of its own so that
 * a seed gives the same order on every platform.
 */

#include "order.h"

#include <time.h>

/*
 * State of xorshift32 (Marsaglia), kept in an unsigned long cut to 32 bits: unlike rand(), the same
 * seed gives the same order on every platform.
 */
static unsigned long g_random_state = 1;

/* Returns a number below bound (> 0). The bias of the modulo is too small to matter for an order of tests. */
static size_t random_below(size_t bound)
{
    unsigned long x = g_random_state;

    x ^= (x << 13) & 0xFFFFFFFFUL;
    x ^= x >> 17;
    x ^= (x << 5) & 0xFFFFFFFFUL;
    g_random_state = x;

    return (size_t)(x % (unsigned long)bound);
}

unsigned long naos_order_pick_seed(unsigned long seed)
{
    if (seed == 0)
    {
        return ((unsigned long)time(NULL) % NAOS_ORDER_SEED_MAX) + 1;
    }

    return seed;
}

unsigned long naos_order_next_seed(unsigned long seed)
{
    return (seed % NAOS_ORDER_SEED_MAX) + 1;
}

void naos_order_start(unsigned long seed)
{
    unsigned long x = seed;

    /*
     * The finalizer of MurmurHash3 spreads the small seeds over 32 bits, so that seeds 1 apart, as
     * --naos-repeat takes them, give unrelated orders. It is a bijection keeping only 0 at 0, the one
     * state xorshift never leaves, and a seed is never 0.
     */
    x ^= x >> 16;
    x = (x * 0x85EBCA6BUL) & 0xFFFFFFFFUL;
    x ^= x >> 13;
    x = (x * 0xC2B2AE35UL) & 0xFFFFFFFFUL;
    x ^= x >> 16;
    g_random_state = x;
}

void naos_order_make(size_t *order, size_t count, int shuffle)
{
    size_t swap;
    size_t i;
    size_t j;

    for (i = 0; i < count; i++)
    {
        order[i] = i;
    }

    if (shuffle == 0)
    {
        return;
    }

    for (i = count; i > 1; i--)
    {
        j = random_below(i);
        swap = order[i - 1];
        order[i - 1] = order[j];
        order[j] = swap;
    }
}
