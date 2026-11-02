/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * The program of the contract cases: a test of each outcome, the diagnostics of each kind, and a
 * test that ends the process. The first argument chooses what main() does besides adding the
 * suites:
 *   no-suites      adds none, for a run with no test registered
 *   mistake        adds a suite with two tests of the same name
 *   out-of-order   calls naos_run() without naos_init()
 */

#include <naos/naos.h>

#include <stdlib.h>
#include <string.h>

NAOS_TEST(passes)
{
    NAOS_CHECK_TRUE(1);
}

NAOS_TEST(fails)
{
    NAOS_CHECK_INT_EQ(1, 2);
}

NAOS_TEST(skips)
{
    NAOS_SKIP("skipped");
}

NAOS_SUITE(outcome,
    NAOS_ADD_TEST(passes),
    NAOS_ADD_TEST(fails),
    NAOS_ADD_TEST(skips)
)

NAOS_TEST(first)
{
    NAOS_SKIP("first skipped");
}

NAOS_TEST(second)
{
    NAOS_SKIP("second skipped");
}

NAOS_SUITE(skipping,
    NAOS_ADD_TEST(first),
    NAOS_ADD_TEST(second)
)

/* An error line, then a note line for the trace that goes with it. */
NAOS_TEST(error_with_trace)
{
    NAOS_TRACE_PUSH("trace");
    NAOS_CHECK_TRUE(0);
    NAOS_TRACE_POP();
}

NAOS_TEST(skip)
{
    NAOS_SKIP("skip message");
}

NAOS_SUITE(diagnostic,
    NAOS_ADD_TEST(error_with_trace),
    NAOS_ADD_TEST(skip)
)

/* Ends the process during the run, as a crash would. */
NAOS_TEST(exits)
{
    exit(3);
}

NAOS_SUITE(crash,
    NAOS_ADD_TEST(exits)
)

NAOS_TEST(same)
{
}

NAOS_SUITE(mistake,
    NAOS_ADD_TEST(same),
    NAOS_ADD_TEST(same)
)

int main(int argc, char **argv)
{
    const char *mode;

    mode = (argc >= 2) ? argv[1] : "";
    if (strcmp(mode, "out-of-order") == 0)
    {
        return naos_run();
    }

    naos_init(&argc, argv);
    if (strcmp(mode, "no-suites") == 0)
    {
        return naos_run();
    }
    if (strcmp(mode, "mistake") == 0)
    {
        NAOS_ADD_SUITE(mistake);
    }
    NAOS_ADD_SUITE(outcome);
    NAOS_ADD_SUITE(skipping);
    NAOS_ADD_SUITE(diagnostic);
    NAOS_ADD_SUITE(crash);
    return naos_run();
}
