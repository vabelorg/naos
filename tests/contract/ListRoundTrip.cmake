# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# The contract of --naos-list that CTest discovery depends on: standard output holds the names of
# the tests only, <suite>.<test>, one per line, and nothing goes to standard error; and each name
# given to --naos-filter runs that test and no other. What runs is read from the JUnit XML file,
# whose test cases are kept compatible, rather than from standard output, which is not.
#
# -D EXE=<path>          the executable
# -D FILTER=<patterns>   given to --naos-list, to leave out tests that end the process (optional)
# -D WORK_DIR=<path>     where the XML files are written

cmake_minimum_required(VERSION 3.21)

set(list_args --naos-list)
if(DEFINED FILTER)
    list(APPEND list_args --naos-filter "${FILTER}")
endif()
execute_process(COMMAND "${EXE}" ${list_args}
    RESULT_VARIABLE code OUTPUT_VARIABLE stdout ERROR_VARIABLE stderr TIMEOUT 60)
string(REPLACE "\r" "" stdout "${stdout}")
if(NOT code EQUAL 0)
    message(FATAL_ERROR "--naos-list exited with ${code}:\n${stdout}${stderr}")
endif()
if(NOT stderr STREQUAL "")
    message(FATAL_ERROR "--naos-list wrote to standard error:\n${stderr}")
endif()
if((stdout STREQUAL "") OR (NOT stdout MATCHES "\n$"))
    message(FATAL_ERROR "--naos-list wrote no name, or a last line without a newline:\n[${stdout}]")
endif()

string(REGEX REPLACE "\n$" "" names "${stdout}")
string(REPLACE "\n" ";" names "${names}")
file(MAKE_DIRECTORY "${WORK_DIR}")
set(output "${WORK_DIR}/round_trip.xml")
foreach(name IN LISTS names)
    if(NOT name MATCHES "^([A-Za-z_][A-Za-z0-9_]*)\\.([A-Za-z_][A-Za-z0-9_]*)$")
        message(FATAL_ERROR "--naos-list wrote a line that is not <suite>.<test>: [${name}]")
    endif()
    set(suite "${CMAKE_MATCH_1}")
    set(test "${CMAKE_MATCH_2}")

    file(REMOVE "${output}")
    execute_process(COMMAND "${EXE}" --naos-filter "${name}" --naos-output "xml:${output}"
        RESULT_VARIABLE code OUTPUT_VARIABLE run_output ERROR_VARIABLE run_output TIMEOUT 60)
    if(NOT EXISTS "${output}")
        message(FATAL_ERROR "--naos-filter ${name} wrote no XML file (exit code ${code}):\n${run_output}")
    endif()
    file(READ "${output}" xml)
    string(REGEX MATCHALL "<testcase [^>]*>" cases "${xml}")
    list(LENGTH cases count)
    if(NOT count EQUAL 1)
        message(FATAL_ERROR "--naos-filter ${name} ran ${count} tests, not 1:\n${xml}")
    endif()
    if((NOT cases MATCHES " classname=\"${suite}\"") OR (NOT cases MATCHES " name=\"${test}\""))
        message(FATAL_ERROR "--naos-filter ${name} ran another test: ${cases}")
    endif()
endforeach()

list(LENGTH names count)
message(STATUS "${count} names listed, each running its test alone")
