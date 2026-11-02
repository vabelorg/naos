# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# What Naos keeps compatible (docs/reference/output.md#what-is-kept-compatible and
# docs/reference/xml.md#compatibility): the exit codes, the start of a diagnostic line, the names of
# --naos-list, and the JUnit XML file. Every case is judged by its exit code, regexes or a schema,
# never by a golden file, so that rewriting the golden files with NAOS_UPDATE_EXPECTED=1 cannot
# change what is checked here. A change to a case here is a change to what Naos keeps compatible.
#
# The cases of the exit codes of a pass, a failure and a skip set up the fixture naos_trust: the
# tests that Naos judges itself (unit/, api/) require it, so that CTest does not run them until a
# failure is shown to be judged a failure.

set(exe naos_selftest_contract)
naos_selftest_program(${exe} contract.c)

# The name of the case <case> of this directory.
function(contract_name out case)
    naos_selftest_prefix(prefix)
    set(${out} "${prefix}${case}" PARENT_SCOPE)
endfunction()

# Exit codes (docs/reference/command-line.md#exit-codes).
naos_expect_match(exit_code.pass ${exe} EXIT_CODE 0 ARGS --naos-filter outcome.passes)
naos_expect_match(exit_code.fail ${exe} EXIT_CODE 1 ARGS --naos-filter outcome.fails)
naos_expect_match(exit_code.all_skipped ${exe} EXIT_CODE 77 ARGS --naos-filter skipping.*)
foreach(case IN ITEMS pass fail all_skipped)
    contract_name(name exit_code.${case})
    set_tests_properties(${name} PROPERTIES FIXTURES_SETUP naos_trust)
endforeach()
naos_expect_match(exit_code.skip_among_passes ${exe} EXIT_CODE 0 ARGS --naos-filter outcome.passes:outcome.skips)
naos_expect_match(exit_code.fail_among_skips ${exe} EXIT_CODE 1 ARGS --naos-filter outcome.fails:skipping.*)
naos_expect_match(exit_code.no_assertion ${exe} EXIT_CODE 1 ARGS --naos-filter unchecked.runs)
naos_expect_match(exit_code.no_assertion_allowed ${exe} EXIT_CODE 0 ARGS --naos-filter unchecked.runs --naos-allow-no-assertions)
naos_expect_match(exit_code.no_test_registered ${exe} EXIT_CODE 0 ARGS no-suites)
naos_expect_match(exit_code.list ${exe} EXIT_CODE 0 ARGS --naos-list)
naos_expect_match(exit_code.help ${exe} EXIT_CODE 0 ARGS --help)
naos_expect_match(exit_code.unknown_option ${exe} EXIT_CODE 2 ARGS --naos-unknown)
naos_expect_match(exit_code.filter_matches_nothing ${exe} EXIT_CODE 2 ARGS --naos-filter nothing.*)
naos_expect_match(exit_code.mistake_in_suites ${exe} EXIT_CODE 2 ARGS mistake)
naos_expect_match(exit_code.mistake_wins_over_list ${exe} EXIT_CODE 2 ARGS mistake --naos-list)
naos_expect_match(exit_code.call_out_of_order ${exe} EXIT_CODE 2 ARGS out-of-order)

# The start of a diagnostic line: the location in the form of the compiler that built the library,
# then the kind and ": " (docs/reference/output.md#diagnostic-lines). Each line follows the line
# before it, matched here by '.'.
if(MSVC)
    set(at "contract\\.c\\([0-9]+\\): ")
    set(other_form "contract\\.c:[0-9]+: ")
else()
    set(at "contract\\.c:[0-9]+: ")
    set(other_form "contract\\.c\\([0-9]+\\): ")
endif()
set(path "([^ ]*[/\\\\])?")
naos_expect_match(diagnostic.error_and_note ${exe} EXIT_CODE 1 ARGS --naos-filter diagnostic.error_with_trace
    EXPECT "RUN: diagnostic\\.error_with_trace.${path}${at}error: 0.${path}${at}note: trace."
    REJECT "${other_form}")
# The file is the __FILE__ the compiler gave, with its directory.
naos_expect_match(diagnostic.path_kept ${exe} EXIT_CODE 1 ARGS --naos-filter outcome.fails
    EXPECT "contract[/\\\\]${at}error: ")
naos_expect_match(diagnostic.skip ${exe} EXIT_CODE 77 ARGS --naos-filter diagnostic.skip
    EXPECT "RUN: diagnostic\\.skip.${path}${at}skip: skip message."
    REJECT "${other_form}")
# A failure without a location, that of a test that ran no assertion, starts with "naos: ".
naos_expect_match(diagnostic.no_assertion ${exe} EXIT_CODE 1 ARGS --naos-filter unchecked.runs
    EXPECT "RUN: unchecked\\.runs.naos: error: ")

# --naos-list: the names only, each of which runs its test alone through --naos-filter.
naos_selftest_add_test(list.round_trip "${CMAKE_COMMAND}"
    -D "EXE=$<TARGET_FILE:${exe}>"
    -D "FILTER=-crash.*"
    -D "WORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/round_trip"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/ListRoundTrip.cmake")

# The JUnit XML file: its structure, wherever xmllint is found, and when it is written
# (docs/reference/xml.md#when-the-file-is-written).
set(schema "${CMAKE_CURRENT_SOURCE_DIR}/junit.xsd")
function(contract_xml case code)
    naos_expect_match(xml.${case} ${exe} EXIT_CODE ${code} OUTPUT_FILE xml_${case}.xml ${ARGN})
endfunction()
# OUTPUT_FILE is in a directory that does not exist.
naos_expect_match(xml.cannot_open ${exe} EXIT_CODE 2 OUTPUT_FILE no_such_directory/out.xml FILE absent
    ARGS --naos-output xml:{output})
contract_xml(every_outcome 1 FILE written SCHEMA "${schema}"
    ARGS --naos-output xml:{output} --naos-filter -crash.*)
contract_xml(repeat_shuffle 1 FILE written SCHEMA "${schema}"
    ARGS --naos-output xml:{output} --naos-repeat 2 --naos-shuffle --naos-random-seed 3 --naos-filter outcome.*)
contract_xml(fail_fast 1 FILE written SCHEMA "${schema}"
    ARGS --naos-output xml:{output} --naos-fail-fast --naos-filter outcome.*)
contract_xml(all_skipped 77 FILE written SCHEMA "${schema}"
    ARGS --naos-output xml:{output} --naos-filter skipping.*)
contract_xml(left_empty_by_exit 3 FILE empty ARGS --naos-output xml:{output} --naos-filter crash.*)
contract_xml(not_written_by_list 0 FILE absent ARGS --naos-output xml:{output} --naos-list)
contract_xml(not_written_by_help 0 FILE absent ARGS --naos-output xml:{output} --help)
