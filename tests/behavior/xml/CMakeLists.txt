# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# The behavior that docs/reference/xml.md states, checked against golden
# files: a program per subject, and the cases that run it.

# report
# The JUnit XML file of --naos-output xml:<path>: each case compares the file as well as stdout and
# stderr, which stay as they are without the option.

set(exe naos_selftest_report)
naos_selftest_program(${exe} report.c)

# Runs <exe> with --naos-output xml:<case>.xml and the arguments, and compares the file too.
function(report_case case)
    naos_expect(${case} ${exe} ARGS --naos-output xml:{output} ${ARGN} OUTPUT_FILE ${case}.xml)
endfunction()

# Every outcome, value, note and trace.
report_case(all --naos-filter results.*)
# The escapes. The tests print bytes beyond ASCII on stdout, which CMake reads in the code page of
# the platform, so only the file is compared.
naos_expect(text ${exe} ARGS --naos-output xml:{output} --naos-filter text.* OUTPUT_FILE text.xml FILE_ONLY)
report_case(one_pass --naos-filter results.passes)
# Each round has its suites, with the iteration and the seed as properties.
report_case(repeat_shuffle --naos-repeat 2 --naos-shuffle --naos-random-seed 3 --naos-filter results.passes:results.skips:text.markup)
report_case(fail_fast --naos-fail-fast --naos-filter results.*)
# When the file is written is a contract (contract/); the message of a file that cannot be opened is
# written for people.
naos_expect(cannot_open ${exe} ARGS --naos-output xml:{output} OUTPUT_FILE no_such_directory/out.xml)
