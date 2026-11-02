# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# The behavior that docs/reference/control.md states, checked against golden
# files: a program per subject, and the cases that run it.

# control
# Going on and stopping: NAOS_CHECK_* goes on, NAOS_REQUIRE_*, NAOS_FAIL and NAOS_SKIP stop the test
# from any depth; the result line of each test, the summary and the exit code.

set(exe naos_selftest_control)
naos_selftest_program(${exe} main.c results.c check.c require.c fail.c skip.c crash.c)

naos_expect_tests(${exe}
    pass.passes
    check.goes_on check.false_goes_on check.in_helper_goes_on
    require.stops require.false_stops require.int_eq_stops require.int_lt_stops
    require.uint_ge_stops require.leaves_helper require.leaves_nested_helpers require.as_expression
    fail.stops fail.no_message fail.from_helper fail.ends_value_helper
    skip.stops skip.from_helper skip.failure_wins
)

# The run goes on after failures, each test starting clean, and the summary counts them all; the
# tests of crash end the process, so the full run leaves them out.
naos_expect(run_all ${exe} ARGS --naos-filter -crash.*)
# Exit codes: 77 when every selected test is skipped, 0 for a skip among passes.
naos_expect(all_skipped ${exe} ARGS --naos-filter skips.skipped_*)
naos_expect(skip_among_passes ${exe} ARGS --naos-filter skips.*)
# A process ending abruptly: stdout is a pipe here and fully buffered, so only what Naos has written
# out by then is left: the RUN line, and the diagnostic of a failure.
naos_expect_tests(${exe} crash.exits crash.exits_after_failure)

# context
# Notes and traces: where they are written in a failure, when they are used up or popped, and the
# misuses of NAOS_TRACE_PUSH and NAOS_TRACE_POP.

set(exe naos_selftest_context)
naos_selftest_program(${exe} context_main.c note.c trace.c)

naos_expect_tests(${exe}
    note.in_loop note.used_up_by_pass note.replaced note.on_require note.on_fail note.on_skip
    note.cut_short note.c99_formats note.with_double note.with_mem note_before_teardown.leaves_note
    trace.nested trace.popped trace.left_by_jump trace.missing_pop trace.missing_pop_nested
    trace.pop_without_push trace.too_deep trace.too_deep_with_note trace.on_skip
    trace_phases.pops_setup_trace trace_phases.jumps_with_trace trace_teardown.body
)
naos_expect(note_reset ${exe} ARGS --naos-filter note_reset.*)
# Every test in one process: a note or trace left by one test must not reach the next.
naos_expect(context_run_all ${exe})
