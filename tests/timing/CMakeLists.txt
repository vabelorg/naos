# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# The times of the result lines and SUMMARY. They change from run to run, so the golden cases of the
# other subjects show them as N ms; here regexes check that they are measured: a test that keeps the
# processor busy for a known time shows at least that time.

set(exe naos_selftest_timing)
naos_selftest_program(${exe} timing.c)

# At least 50 ms, and at least 60 ms for a test whose setup and teardown take 30 ms each.
set(at_least_50 "([5-9][0-9]|[1-9][0-9][0-9]+) ms")
set(at_least_60 "([6-9][0-9]|[1-9][0-9][0-9]+) ms")
set(at_least_150 "(1[5-9][0-9]|[2-9][0-9][0-9]|[1-9][0-9][0-9][0-9]+) ms")

# Each outcome shows its time; SUMMARY shows the sum.
naos_expect_match(outcomes ${exe} EXIT_CODE 1 ARGS --naos-filter timing.*
    EXPECT "PASS: timing\\.passes \\(${at_least_50}\\)"
    EXPECT "FAIL: timing\\.fails \\(1 failure, ${at_least_50}\\)"
    EXPECT "SKIP: timing\\.skips \\(${at_least_50}\\)"
    EXPECT "SUMMARY: 1 passed, 1 failed, 1 skipped \\(${at_least_150}\\)")
# The time covers setup and teardown.
naos_expect_match(fixtures ${exe} EXIT_CODE 0 ARGS --naos-filter fixtures.*
    EXPECT "PASS: fixtures\\.between \\(${at_least_60}\\)")
# Each iteration has its own total.
naos_expect_match(repeat ${exe} EXIT_CODE 0 ARGS --naos-repeat 2 --naos-filter timing.passes
    EXPECT "ITERATION: 1 of 2.RUN: timing\\.passes.PASS: timing\\.passes \\(${at_least_50}\\).SUMMARY: 1 passed, 0 failed, 0 skipped \\(${at_least_50}\\).ITERATION: 2 of 2.RUN: timing\\.passes.PASS: timing\\.passes \\(${at_least_50}\\).SUMMARY: 1 passed, 0 failed, 0 skipped \\(${at_least_50}\\).REPEAT")
