# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# The behavior that docs/reference/output.md states, checked against golden
# files: a program per subject, and the cases that run it.

# quiet
# --naos-quiet: stdout leaves out the tests that pass or skip, and ITERATION; a failing test is
# written as without the option, and the JUnit XML file does not change.

set(exe naos_selftest_quiet)
naos_selftest_program(${exe} quiet.c)

# Every outcome; the skip of setup comes out only because teardown fails after it.
naos_expect(all ${exe} ARGS --naos-quiet --naos-filter -crash.*)
# Only the summary is left when every test passes.
naos_expect(all_pass ${exe} ARGS --naos-quiet --naos-filter results.passes)
# Exit code 77, and a summary that counts the skips.
naos_expect(all_skipped ${exe} ARGS --naos-quiet --naos-filter skipping.*)
# The seed of each iteration and the failed iterations are kept; ITERATION is not.
naos_expect(repeat_shuffle ${exe} ARGS --naos-quiet --naos-repeat 2 --naos-shuffle --naos-random-seed 3 --naos-filter results.*)
naos_expect(fail_fast ${exe} ARGS --naos-quiet --naos-fail-fast --naos-filter results.*)
# A test that ends the process before any failure leaves no line naming it.
naos_expect(crash ${exe} ARGS --naos-quiet --naos-filter crash.*)
# The file is the same as without --naos-quiet, skips included.
naos_expect(xml ${exe} ARGS --naos-quiet --naos-output xml:{output} --naos-filter -crash.* OUTPUT_FILE xml.xml)

# timing
# The times of the result lines and SUMMARY. They change from run to run, so the golden cases of the
# other subjects show them as N ms; here regexes check that they are measured: a test that keeps the
# processor busy for a known time shows at least half of it. Not all of it: the test counts processor
# time, which a system counting it by ticks, as a virtual machine may, can count ahead of the clock,
# and the time shown is rounded down.

set(exe naos_selftest_timing)
naos_selftest_program(${exe} timing.c)

# A test busy for 100 ms shows at least 50 ms, and one whose setup and teardown are busy for 60 ms each
# at least 60 ms.
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
