# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# The main() side: the order of naos_init(), NAOS_ADD_SUITE and naos_run(), the arguments
# naos_init() leaves to the program, and the mistakes in the suites, all reported before anything
# runs.

# suites.c adds the suites its first argument names.
naos_selftest_program(naos_selftest_suites suites.c)
foreach(case IN ITEMS
        duplicate_test duplicate_suite dotted_test dotted_suite invalid_test_names invalid_suite_name
        setup_twice teardown_twice no_begin null_suite many add_in_test add_in_suite)
    naos_expect(${case} naos_selftest_suites ARGS ${case})
endforeach()
naos_expect(mistakes_win_over_help naos_selftest_suites ARGS many --help)

# own_main.c has a main() of its own, whose arguments select a misuse.
set(exe naos_selftest_own_main)
naos_selftest_program(${exe} own_main.c)
naos_expect(args_without_options ${exe} ARGS first --naos-filter own_main.sees_args_without_options second)
naos_expect(default_suites ${exe} ARGS --naos-list)
naos_expect(suite_by_arg ${exe} ARGS with-extra --naos-list)
naos_expect(program_sees_help ${exe} ARGS --help)
naos_expect(add_after_run ${exe} ARGS add-after-run --naos-list)
naos_expect(run_twice ${exe} ARGS run-twice --naos-list)
naos_expect(init_twice ${exe} ARGS init-twice --naos-list)
naos_expect(init_after_run ${exe} ARGS init-after-run --naos-list)
naos_expect(add_before_init ${exe} ARGS add-before-init --naos-list)
naos_expect(run_without_init ${exe} ARGS run-without-init --naos-list)
naos_expect(require_outside_test ${exe} ARGS require-outside)
