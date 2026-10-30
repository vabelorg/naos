# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# Notes and traces: where they are written in a failure, when they are used up or popped, and the
# misuses of NAOS_TRACE_PUSH and NAOS_TRACE_POP.

set(exe naos_selftest_context)
naos_selftest_program(${exe} main.c note.c trace.c)

naos_expect_tests(${exe}
    note.in_loop note.used_up_by_pass note.replaced note.on_require note.on_fail note.on_skip
    note.cut_short note.with_double note.with_mem note_before_teardown.leaves_note
    trace.nested trace.popped trace.left_by_jump trace.missing_pop trace.missing_pop_nested
    trace.pop_without_push trace.too_deep trace.too_deep_with_note trace.on_skip
    trace_phases.pops_setup_trace trace_phases.jumps_with_trace trace_teardown.body
)
naos_expect(note_reset ${exe} ARGS --naos-filter note_reset.*)
# Every test in one process: a note or trace left by one test must not reach the next.
naos_expect(run_all ${exe})
