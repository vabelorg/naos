/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * Suites that must be rejected before anything runs. The first argument naos_init() leaves picks
 * the suites to add, so one executable covers every case.
 */

#include <naos/naos.h>

#include <stdio.h>
#include <string.h>

NAOS_TEST(same_name)
{
    printf("test-ran\n");
}

NAOS_SETUP(setup_fixture)
{
}

NAOS_TEARDOWN(teardown_fixture)
{
}

NAOS_SUITE(duplicate_test,
    NAOS_ADD_TEST(same_name),
    NAOS_ADD_TEST(same_name)
)

NAOS_SUITE(valid,
    NAOS_ADD_TEST(same_name)
)

NAOS_SUITE(setup_twice,
    NAOS_SET_SETUP(setup_fixture),
    NAOS_SET_SETUP(setup_fixture),
    NAOS_ADD_TEST(same_name)
)

NAOS_SUITE(teardown_twice,
    NAOS_SET_TEARDOWN(teardown_fixture),
    NAOS_ADD_TEST(same_name),
    NAOS_SET_TEARDOWN(teardown_fixture)
)

/* The macros cannot produce the mistakes below, so these suite functions are written by hand. */
static void dotted_test(naos_builder_t *builder)
{
    naos_begin_suite(builder, "dotted_test");
    naos_add_test(builder, "a.b", NAOS_TEST_FUNC_(same_name));
}

static void dotted_suite(naos_builder_t *builder)
{
    naos_begin_suite(builder, "a.b");
    naos_add_test(builder, "same_name", NAOS_TEST_FUNC_(same_name));
}

/*
 * Names with a character that means something to --naos-filter or to a shell, which no filter could
 * select on its own, and an empty one: each is reported.
 */
static void invalid_test_names(naos_builder_t *builder)
{
    naos_begin_suite(builder, "invalid_test_names");
    naos_add_test(builder, "a-b", NAOS_TEST_FUNC_(same_name));
    naos_add_test(builder, "a:b", NAOS_TEST_FUNC_(same_name));
    naos_add_test(builder, "a*", NAOS_TEST_FUNC_(same_name));
    naos_add_test(builder, "a?", NAOS_TEST_FUNC_(same_name));
    naos_add_test(builder, "a b", NAOS_TEST_FUNC_(same_name));
    naos_add_test(builder, "", NAOS_TEST_FUNC_(same_name));
    naos_add_test(builder, NULL, NAOS_TEST_FUNC_(same_name));
    naos_add_test(builder, "Valid_name_9", NAOS_TEST_FUNC_(same_name));
}

static void invalid_suite_name(naos_builder_t *builder)
{
    naos_begin_suite(builder, "a-b");
    naos_add_test(builder, "same_name", NAOS_TEST_FUNC_(same_name));
}

static void no_begin(naos_builder_t *builder)
{
    naos_add_test(builder, "same_name", NAOS_TEST_FUNC_(same_name));
}

/* Adding a suite once naos_run() has started is a misuse, from a test as from a suite function. */
NAOS_TEST(adds_suite)
{
    NAOS_ADD_SUITE(valid);
    printf("reached-after-add\n");
}

NAOS_SUITE(add_in_test,
    NAOS_ADD_TEST(adds_suite)
)

static void add_in_suite(naos_builder_t *builder)
{
    naos_begin_suite(builder, "add_in_suite");
    NAOS_ADD_SUITE(valid);
    printf("reached-after-add\n");
}

static void add_duplicate_test(void)
{
    NAOS_ADD_SUITE(duplicate_test);
}

static void add_duplicate_suite(void)
{
    NAOS_ADD_SUITE(valid);
    NAOS_ADD_SUITE(valid);
}

static void add_dotted_test(void)
{
    naos_add_suite(dotted_test);
}

static void add_dotted_suite(void)
{
    naos_add_suite(dotted_suite);
}

static void add_invalid_test_names(void)
{
    naos_add_suite(invalid_test_names);
}

static void add_invalid_suite_name(void)
{
    naos_add_suite(invalid_suite_name);
}

static void add_setup_twice(void)
{
    NAOS_ADD_SUITE(setup_twice);
}

static void add_teardown_twice(void)
{
    NAOS_ADD_SUITE(teardown_twice);
}

static void add_no_begin(void)
{
    naos_add_suite(no_begin);
}

static void add_null_suite(void)
{
    naos_add_suite(NULL);
}

/* Every mistake is reported, not only the first one. */
static void add_many(void)
{
    NAOS_ADD_SUITE(valid);
    NAOS_ADD_SUITE(duplicate_test);
    naos_add_suite(dotted_test);
    NAOS_ADD_SUITE(setup_twice);
    NAOS_ADD_SUITE(valid);
}

static void add_add_in_test(void)
{
    NAOS_ADD_SUITE(add_in_test);
}

static void add_add_in_suite(void)
{
    naos_add_suite(add_in_suite);
}

struct case_
{
    const char *name;
    void (*add)(void);
};

#define CASE_(name) { #name, add_##name }

static const struct case_ cases[] =
{
    CASE_(duplicate_test),
    CASE_(duplicate_suite),
    CASE_(dotted_test),
    CASE_(dotted_suite),
    CASE_(invalid_test_names),
    CASE_(invalid_suite_name),
    CASE_(setup_twice),
    CASE_(teardown_twice),
    CASE_(no_begin),
    CASE_(null_suite),
    CASE_(many),
    CASE_(add_in_test),
    CASE_(add_in_suite)
};

int main(int argc, char **argv)
{
    size_t i;

    naos_init(&argc, argv);

    for (i = 0; (argc >= 2) && (i < (sizeof(cases) / sizeof(cases[0]))); i++)
    {
        if (strcmp(argv[1], cases[i].name) == 0)
        {
            cases[i].add();
            return naos_run();
        }
    }

    return NAOS_EXIT_USAGE;
}
