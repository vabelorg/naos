# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
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
