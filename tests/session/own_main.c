/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * A main() of its own: naos_init() takes the Naos options out of argv and leaves the rest, in order,
 * to the program, which adds the suites one at a time and may pick them from the arguments.
 * The argument "with-extra" adds the suite extra too.
 * The argument "require-outside" makes it call NAOS_REQUIRE_TRUE before naos_run(), outside any test.
 * The argument "add-after-run" makes it call NAOS_ADD_SUITE after naos_run().
 * The argument "run-twice" makes it call naos_run() a second time.
 * The arguments "init-twice" and "init-after-run" make it call naos_init() a second time, before and
 * after naos_run().
 * The first argument "add-before-init" makes it call NAOS_ADD_SUITE before naos_init(), and
 * "run-without-init" makes it call naos_run() without naos_init().
 */

#include <naos/naos.h>

#include <stdio.h>
#include <string.h>

static int program_argc;
static char **program_argv;

NAOS_TEST(sees_args_without_options)
{
    NAOS_REQUIRE_INT_EQ(3, program_argc);
    NAOS_CHECK_STR_EQ("first", program_argv[1]);
    NAOS_CHECK_STR_EQ("second", program_argv[2]);
    NAOS_CHECK_NULL(program_argv[3]);
}

NAOS_TEST(runs)
{
}

NAOS_SUITE(own_main,
    NAOS_ADD_TEST(sees_args_without_options)
)

NAOS_SUITE(extra,
    NAOS_ADD_TEST(runs)
)

int main(int argc, char **argv)
{
    int add_after_run;
    int run_twice;
    int init_after_run;
    int result;
    int i;

    /* Read before naos_init(), which these two misuses must come before. */
    if ((argc >= 2) && (strcmp(argv[1], "add-before-init") == 0))
    {
        NAOS_ADD_SUITE(own_main);
        printf("reached-after-add\n");
    }
    else if ((argc >= 2) && (strcmp(argv[1], "run-without-init") == 0))
    {
        return naos_run();
    }

    naos_init(&argc, argv);
    NAOS_ADD_SUITE(own_main);

    add_after_run = 0;
    run_twice = 0;
    init_after_run = 0;
    for (i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--help") == 0)
        {
            printf("program saw --help\n");
        }
        else if (strcmp(argv[i], "with-extra") == 0)
        {
            NAOS_ADD_SUITE(extra);
        }
        else if (strcmp(argv[i], "require-outside") == 0)
        {
            NAOS_REQUIRE_TRUE(1 == 2);
            printf("reached-after-require\n");
        }
        else if (strcmp(argv[i], "add-after-run") == 0)
        {
            add_after_run = 1;
        }
        else if (strcmp(argv[i], "run-twice") == 0)
        {
            run_twice = 1;
        }
        else if (strcmp(argv[i], "init-twice") == 0)
        {
            naos_init(&argc, argv);
            printf("reached-after-init\n");
        }
        else if (strcmp(argv[i], "init-after-run") == 0)
        {
            init_after_run = 1;
        }
    }

    program_argc = argc;
    program_argv = argv;

    result = naos_run();

    if (add_after_run != 0)
    {
        NAOS_ADD_SUITE(extra);
        printf("reached-after-add\n");
    }

    if (run_twice != 0)
    {
        result = naos_run();
    }

    if (init_after_run != 0)
    {
        naos_init(&argc, argv);
        printf("reached-after-init\n");
    }

    return result;
}
