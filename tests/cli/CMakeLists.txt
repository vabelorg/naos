# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# The command line: options anywhere, with a value after ' ' or '=', their mistakes, the filter
# patterns against the whole name <suite>.<test>, --naos-list and --help. The matching itself is
# covered by unit/filter.c; these cases check that the options reach it.

set(exe naos_selftest_cli)
naos_selftest_program(${exe} cli.c)

# Running and listing
naos_expect(run_all ${exe})
naos_expect(list ${exe} ARGS --naos-list)
naos_expect(list_filtered ${exe} ARGS --naos-filter parser.* --naos-list)
naos_expect(help ${exe} ARGS --help)
naos_expect(help_wins_over_run ${exe} ARGS --naos-filter parser.empty --naos-list --help)
naos_expect(help_among_program_args ${exe} ARGS --naos-filter parser.empty first --help)
naos_expect(options_anywhere ${exe} ARGS first --naos-filter parser.empty second)

# Filter patterns
naos_expect(filter_one ${exe} ARGS --naos-filter parser.empty)
naos_expect(filter_suite ${exe} ARGS --naos-filter writer.*)
naos_expect(filter_any_suite ${exe} ARGS --naos-filter *.empty --naos-list)
naos_expect(filter_question_mark ${exe} ARGS --naos-filter ?riter.*_small --naos-list)
naos_expect(filter_star_spans_dot ${exe} ARGS --naos-filter pa*y --naos-list)
naos_expect(filter_several ${exe} ARGS --naos-filter parser.empty::writer.*: --naos-list)
naos_expect(filter_exclude_only ${exe} ARGS --naos-filter -parser.*:writer.slow_* --naos-list)
naos_expect(filter_include_exclude ${exe} ARGS --naos-filter *-*.slow_*:parser.nested --naos-list)
naos_expect(filter_with_equals ${exe} ARGS --naos-filter=parser.*-*.slow_*)
naos_expect(filter_splits_at_first_equals ${exe} ARGS --naos-filter=parser.empty=x)
naos_expect(filter_suite_name_alone ${exe} ARGS --naos-filter parser)
naos_expect(filter_no_match ${exe} ARGS --naos-filter nothing.* --naos-list)
naos_expect(filter_all_excluded ${exe} ARGS --naos-filter parser.*-*)

# Mistakes in the options: each prints what is wrong and the usage, and runs nothing.
naos_expect(unknown_option ${exe} ARGS --naos-unknown)
naos_expect(longer_option_name ${exe} ARGS --naos-filterx=parser.empty)
naos_expect(filter_without_value ${exe} ARGS --naos-filter)
naos_expect(filter_empty_value ${exe} ARGS --naos-filter=)
naos_expect(list_with_value ${exe} ARGS --naos-list=1)
naos_expect(shuffle_with_value ${exe} ARGS --naos-shuffle=1)
naos_expect(fail_fast_with_value ${exe} ARGS --naos-fail-fast=1)
naos_expect(seed_too_large ${exe} ARGS --naos-random-seed 100000)
naos_expect(seed_negative ${exe} ARGS --naos-random-seed=-1)
naos_expect(seed_not_number ${exe} ARGS --naos-random-seed 7x)
naos_expect(seed_huge ${exe} ARGS --naos-random-seed 99999999999999999999999)
naos_expect(seed_empty ${exe} ARGS --naos-random-seed=)
naos_expect(seed_without_value ${exe} ARGS --naos-random-seed)
naos_expect(repeat_0 ${exe} ARGS --naos-repeat 0)
naos_expect(repeat_not_number ${exe} ARGS --naos-repeat=x)
naos_expect(repeat_without_value ${exe} ARGS --naos-repeat)
naos_expect(several_mistakes ${exe} ARGS --naos-unknown --naos-repeat 0)
