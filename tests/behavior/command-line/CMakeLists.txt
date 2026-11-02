# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# The behavior that docs/reference/command-line.md states, checked against golden
# files: a program per subject, and the cases that run it.

# cli
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
naos_expect(quiet_with_value ${exe} ARGS --naos-quiet=1)
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
naos_expect(output_without_format ${exe} ARGS --naos-output out.xml)
naos_expect(output_empty_format ${exe} ARGS --naos-output :out.xml)
naos_expect(output_empty_path ${exe} ARGS --naos-output xml:)
naos_expect(output_unknown_format ${exe} ARGS --naos-output=json:out.json)
naos_expect(output_twice ${exe} ARGS --naos-output xml:a.xml --naos-output xml:b.xml)
naos_expect(output_without_value ${exe} ARGS --naos-output)

# runs
# Running the tests more than once or in another order: --naos-shuffle, --naos-random-seed,
# --naos-repeat and --naos-fail-fast. The order of a seed is fixed by the PRNG of Naos, the same on
# every platform, so the expected orders hold everywhere.

# shuffle.c: three suites of three passing tests, and a test that passes only after another one.
set(exe naos_selftest_shuffle)
naos_selftest_program(${exe} shuffle.c)
naos_expect(registration_order ${exe} ARGS --naos-random-seed 1)
naos_expect(shuffle_seed ${exe} ARGS --naos-shuffle --naos-random-seed 1)
naos_expect(shuffle_seed_with_equals ${exe} ARGS --naos-random-seed=1 --naos-shuffle)
naos_expect(shuffle_other_seed ${exe} ARGS --naos-shuffle --naos-random-seed 2)
naos_expect(shuffle_seed_max ${exe} ARGS --naos-shuffle --naos-random-seed 99999 --naos-filter a.*)

# Every test takes part in the shuffle, so a filter keeps the order of the tests it selects.
naos_expect(shuffle_order_kept_by_filter ${exe} ARGS --naos-shuffle --naos-random-seed 1 --naos-filter a.*:b.t1:b.t3)
naos_expect(shuffle_finds_dependency ${exe} ARGS --naos-shuffle --naos-random-seed 2 --naos-filter dependent.*)

naos_expect(list_ignores_shuffle ${exe} ARGS --naos-list --naos-shuffle --naos-random-seed 1 --naos-repeat 2)
naos_expect(repeat ${exe} ARGS --naos-repeat 3 --naos-filter a.t1)
naos_expect(repeat_once ${exe} ARGS --naos-repeat 1 --naos-filter a.t1)
naos_expect(repeat_with_summaries ${exe} ARGS --naos-repeat=2)
naos_expect(repeat_shuffle_next_seed ${exe} ARGS --naos-shuffle --naos-random-seed 1 --naos-repeat 2)
naos_expect(repeat_shuffle_seed_wraps ${exe} ARGS --naos-shuffle --naos-random-seed 99999 --naos-repeat 2 --naos-filter a.*)

# Iteration 1 leaves first_ran set, so second passes in iteration 2 whatever the order.
naos_expect(repeat_shuffle_reports_seed ${exe} ARGS --naos-shuffle --naos-random-seed 2 --naos-repeat 2 --naos-filter dependent.*)
naos_expect(fail_fast_with_shuffle ${exe} ARGS --naos-fail-fast --naos-shuffle --naos-random-seed 2 --naos-filter dependent.*)

# Without a seed, or with 0, one is chosen from the time.
naos_expect_match(shuffle_without_seed ${exe} EXIT_CODE 0 ARGS --naos-shuffle --naos-filter a.*
    EXPECT "^RANDOM SEED: [1-9][0-9]*.RUN: a\\.t[1-3].PASS: a\\.")
naos_expect_match(shuffle_seed_0 ${exe} EXIT_CODE 0 ARGS --naos-shuffle --naos-random-seed 0 --naos-filter a.*
    EXPECT "^RANDOM SEED: [1-9][0-9]*.RUN: a\\.t[1-3].PASS: a\\.")

# failing.c: tests that fail, in a test or in teardown, and tests that skip.
set(exe naos_selftest_failing)
naos_selftest_program(${exe} failing.c)
naos_expect(fail_fast ${exe} ARGS --naos-fail-fast --naos-filter mixed.*)
naos_expect(fail_fast_after_teardown ${exe} ARGS --naos-fail-fast --naos-filter teardown_fails.*:mixed.passes)
naos_expect(fail_fast_at_last_test ${exe} ARGS --naos-fail-fast --naos-filter mixed.passes:mixed.fails)
naos_expect(fail_fast_not_on_skip ${exe} ARGS --naos-fail-fast --naos-filter skips.*)
# The rest of iteration 1 and the 2 iterations after it are not run.
naos_expect(fail_fast_with_repeat ${exe} ARGS --naos-fail-fast --naos-repeat 3 --naos-filter mixed.fails:mixed.passes_too)
naos_expect(repeat_failures ${exe} ARGS --naos-repeat 2 --naos-filter mixed.fails)
naos_expect(repeat_all_skipped ${exe} ARGS --naos-repeat 2 --naos-filter skips.skip_*)
naos_expect(repeat_skip_among_passes ${exe} ARGS --naos-repeat 2 --naos-filter skips.*)

# The largest --naos-repeat: the tests not run (2 left in iteration 1, then 3 per iteration) are
# more than size_t holds where long is as wide as size_t, and the count stops at SIZE_MAX.
include(CheckTypeSize)
check_type_size(long NAOS_SIZEOF_LONG)
if(NAOS_SIZEOF_LONG EQUAL 4)
    set(long_max 2147483647)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(not_run 6442450940)
    else()
        set(not_run 4294967295)
    endif()
else()
    set(long_max 9223372036854775807)
    set(not_run 18446744073709551615)
endif()
naos_expect_match(fail_fast_with_largest_repeat ${exe} EXIT_CODE 1
    ARGS --naos-fail-fast --naos-repeat ${long_max} --naos-filter mixed.fails:mixed.fails_too:mixed.passes_too
    EXPECT "FAIL FAST: stopped at the first failure; ${not_run} not run"
           "REPEAT: 1 of ${long_max} iterations failed, [0-9]+ not run"
    REJECT "fails_too" "passes_too")
