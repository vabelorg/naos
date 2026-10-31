/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * order.c: the seeds of --naos-shuffle and the orders they give. These tests restart the random
 * order that this executable shares with the harness; under --naos-shuffle that changes the order
 * of the tests after them, the same way for the same seed.
 */

#include <naos/naos.h>

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "order.h"

#define ORDER_COUNT 8

/* Whether order[0, count) holds each of 0 to count - 1 once. */
static int is_permutation(const size_t *order, size_t count)
{
    unsigned char seen[ORDER_COUNT];
    size_t i;

    memset(seen, 0, sizeof(seen));
    for (i = 0; i < count; i++)
    {
        if ((order[i] >= count) || (seen[order[i]] != 0))
        {
            return 0;
        }
        seen[order[i]] = 1;
    }

    return 1;
}

NAOS_TEST(pick_seed_keeps_given)
{
    NAOS_CHECK_UINT_EQ(1, naos_order_pick_seed(1));
    NAOS_CHECK_UINT_EQ(12345, naos_order_pick_seed(12345));
    NAOS_CHECK_UINT_EQ(NAOS_ORDER_SEED_MAX, naos_order_pick_seed(NAOS_ORDER_SEED_MAX));
}

/* 0 stands for none given: one is chosen from the time, within the range. */
NAOS_TEST(pick_seed_chooses_in_range)
{
    unsigned long seed;

    seed = naos_order_pick_seed(0);
    NAOS_CHECK_UINT_GE(seed, 1);
    NAOS_CHECK_UINT_LE(seed, NAOS_ORDER_SEED_MAX);
}

/* --naos-repeat takes the seeds one after another, from the largest back to 1. */
NAOS_TEST(next_seed_wraps)
{
    NAOS_CHECK_UINT_EQ(2, naos_order_next_seed(1));
    NAOS_CHECK_UINT_EQ(NAOS_ORDER_SEED_MAX, naos_order_next_seed(NAOS_ORDER_SEED_MAX - 1));
    NAOS_CHECK_UINT_EQ(1, naos_order_next_seed(NAOS_ORDER_SEED_MAX));
}

NAOS_TEST(seed_max_string_matches)
{
    char text[16];

    snprintf(text, sizeof(text), "%lu", (unsigned long)NAOS_ORDER_SEED_MAX);
    NAOS_CHECK_STR_EQ(NAOS_ORDER_SEED_MAX_STRING, text);
}

/* Without shuffling the order is the order of registration, whatever the state. */
NAOS_TEST(no_shuffle_keeps_order)
{
    static const size_t expected[ORDER_COUNT] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    size_t order[ORDER_COUNT];

    naos_order_start(1);
    naos_order_make(order, ORDER_COUNT, 0);
    NAOS_CHECK_MEM_EQ(expected, sizeof(expected), order, sizeof(order));
}

/* No element, or one, has nothing to shuffle. */
NAOS_TEST(shuffle_small_counts)
{
    size_t order[1];

    naos_order_start(1);
    naos_order_make(NULL, 0, 1);
    order[0] = 99;
    naos_order_make(order, 1, 1);
    NAOS_CHECK_UINT_EQ(0, order[0]);
}

NAOS_TEST(shuffle_is_permutation)
{
    size_t order[ORDER_COUNT];
    unsigned long seed;
    size_t count;

    for (seed = 1; seed <= 50; seed++)
    {
        naos_order_start(seed);
        for (count = 2; count <= ORDER_COUNT; count++)
        {
            naos_order_make(order, count, 1);
            NAOS_NOTE("seed %lu, count %u", seed, (unsigned int)count);
            NAOS_CHECK_TRUE(is_permutation(order, count));
        }
    }
}

/*
 * A seed gives the same orders on every platform: xorshift on 32 bits, whatever the width of
 * unsigned long. The orders are written out, so a change of the algorithm shows here first, and
 * the seeds users have noted keep reproducing their order.
 */
NAOS_TEST(seed_gives_fixed_orders)
{
    static const size_t seed_1_first[ORDER_COUNT] = { 1, 5, 6, 0, 7, 2, 4, 3 };
    static const size_t seed_1_second[ORDER_COUNT] = { 6, 3, 7, 0, 1, 5, 2, 4 };
    static const size_t seed_2_first[ORDER_COUNT] = { 6, 3, 5, 0, 1, 7, 2, 4 };
    size_t order[ORDER_COUNT];

    naos_order_start(1);
    naos_order_make(order, ORDER_COUNT, 1);
    NAOS_CHECK_MEM_EQ(seed_1_first, sizeof(seed_1_first), order, sizeof(order));
    naos_order_make(order, ORDER_COUNT, 1);
    NAOS_CHECK_MEM_EQ(seed_1_second, sizeof(seed_1_second), order, sizeof(order));

    naos_order_start(2);
    naos_order_make(order, ORDER_COUNT, 1);
    NAOS_CHECK_MEM_EQ(seed_2_first, sizeof(seed_2_first), order, sizeof(order));
}

/* Starting the same seed again gives the same order: --naos-random-seed reproduces a run. */
NAOS_TEST(restart_repeats_order)
{
    size_t first[ORDER_COUNT];
    size_t again[ORDER_COUNT];

    naos_order_start(4242);
    naos_order_make(first, ORDER_COUNT, 1);
    naos_order_start(4242);
    naos_order_make(again, ORDER_COUNT, 1);
    NAOS_CHECK_MEM_EQ(first, sizeof(first), again, sizeof(again));
}

/*
 * Over many seeds, every element lands in every position: no position is stuck, as an off-by-one in
 * Fisher-Yates would leave one.
 */
NAOS_TEST(every_position_reached)
{
    unsigned char reached[4][4];
    size_t order[4];
    unsigned long seed;
    size_t position;
    size_t element;

    memset(reached, 0, sizeof(reached));
    for (seed = 1; seed <= 200; seed++)
    {
        naos_order_start(seed);
        naos_order_make(order, 4, 1);
        for (position = 0; position < 4; position++)
        {
            reached[position][order[position]] = 1;
        }
    }

    for (position = 0; position < 4; position++)
    {
        for (element = 0; element < 4; element++)
        {
            NAOS_NOTE("element %u at position %u", (unsigned int)element, (unsigned int)position);
            NAOS_CHECK_UINT_EQ(1, reached[position][element]);
        }
    }
}

NAOS_SUITE(order,
    NAOS_ADD_TEST(pick_seed_keeps_given),
    NAOS_ADD_TEST(pick_seed_chooses_in_range),
    NAOS_ADD_TEST(next_seed_wraps),
    NAOS_ADD_TEST(seed_max_string_matches),
    NAOS_ADD_TEST(no_shuffle_keeps_order),
    NAOS_ADD_TEST(shuffle_small_counts),
    NAOS_ADD_TEST(shuffle_is_permutation),
    NAOS_ADD_TEST(seed_gives_fixed_orders),
    NAOS_ADD_TEST(restart_repeats_order),
    NAOS_ADD_TEST(every_position_reached)
)
