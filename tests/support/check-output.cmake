# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# Runs one self-test case: an executable with its arguments, then checks what it did (see
# NaosSelftest.cmake for the two kinds of case).
#
# -D EXE=<path>           executable to run
# -D ARGS=<a|b|...>       its arguments, separated by '|' (optional)
# Golden case:
# -D EXPECTED=<file>      the expected arguments, exit code, stdout and stderr, as written below
# Regex case:
# -D EXIT_CODE=<n>        the exit code required
# -D EXPECT_<i>=<regex>   must match stdout followed by stderr, for i = 0, 1, ... (optional)
# -D REJECT_<i>=<regex>   must not match it (optional)
#
# With the environment variable NAOS_UPDATE_EXPECTED set to 1, a golden case writes what the
# executable did into its file instead of comparing, and passes.

cmake_minimum_required(VERSION 3.21)

# Makes text the same wherever it was produced: no CR; the path of the executable, which the usage
# line prints, as PROG, and the usage on one line, since its wrapping depends on that path; and a
# location as <file name>:LINE: in either compiler form, without its directory, so that editing a
# test program moves no expected output.
function(normalize var)
    set(text "${${var}}")
    string(REPLACE "\r" "" text "${text}")
    file(TO_NATIVE_PATH "${EXE}" native_exe)
    string(REPLACE "${EXE}" "PROG" text "${text}")
    string(REPLACE "${native_exe}" "PROG" text "${text}")
    string(REPLACE "\n       [" " [" text "${text}")
    string(REGEX REPLACE "([^ \n]*[/\\\\])?([A-Za-z0-9_]+\\.(c|cpp))(:[0-9]+|\\([0-9]+\\)): " "\\2:LINE: " text "${text}")
    set(${var} "${text}" PARENT_SCOPE)
endfunction()

# Ends a stream that does not end with a newline so that the next header starts a line of its own.
function(end_stream var)
    if((NOT "${${var}}" STREQUAL "") AND (NOT "${${var}}" MATCHES "\n$"))
        set(${var} "${${var}}\n[no newline at the end]\n" PARENT_SCOPE)
    endif()
endfunction()

# Golden case: compares what the executable did with EXPECTED, or writes it there.
function(check_golden)
    normalize(stdout)
    normalize(stderr)
    end_stream(stdout)
    end_stream(stderr)
    string(REPLACE ";" " " shown_args "${args}")
    set(actual "args: ${shown_args}\nexit code: ${code}\n--- stdout\n${stdout}--- stderr\n${stderr}")

    if("$ENV{NAOS_UPDATE_EXPECTED}" STREQUAL "1")
        file(WRITE "${EXPECTED}" "${actual}")
        message(STATUS "updated ${EXPECTED}")
        return()
    endif()

    if(NOT EXISTS "${EXPECTED}")
        message(NOTICE "Run the case with NAOS_UPDATE_EXPECTED=1 to write it, then review it.\n--- actual ---\n${actual}")
        message(FATAL_ERROR "no expected output: ${EXPECTED}")
    endif()

    file(READ "${EXPECTED}" expected)
    string(REPLACE "\r" "" expected "${expected}")
    if(expected STREQUAL actual)
        return()
    endif()

    # Points at the first line that differs, then shows the whole of what the executable did.
    string(REPLACE ";" "\;" expected_lines "${expected}")
    string(REPLACE ";" "\;" actual_lines "${actual}")
    string(REPLACE "\n" ";" expected_lines "${expected_lines}")
    string(REPLACE "\n" ";" actual_lines "${actual_lines}")
    list(LENGTH expected_lines expected_count)
    list(LENGTH actual_lines actual_count)
    set(index 0)
    while((index LESS expected_count) AND (index LESS actual_count))
        list(GET expected_lines ${index} expected_line)
        list(GET actual_lines ${index} actual_line)
        if(NOT expected_line STREQUAL actual_line)
            break()
        endif()
        math(EXPR index "${index} + 1")
    endwhile()
    math(EXPR line "${index} + 1")
    set(expected_line "(end)")
    set(actual_line "(end)")
    if(index LESS expected_count)
        list(GET expected_lines ${index} expected_line)
    endif()
    if(index LESS actual_count)
        list(GET actual_lines ${index} actual_line)
    endif()
    # message(NOTICE) writes the text as it is; FATAL_ERROR would reflow it and merge spaces.
    message(NOTICE "output differs from ${EXPECTED} at line ${line}:\n"
        "  expected: [${expected_line}]\n"
        "  actual:   [${actual_line}]\n"
        "If the change is intended, run the case with NAOS_UPDATE_EXPECTED=1 and review the diff.\n"
        "--- actual ---\n${actual}")
    message(FATAL_ERROR "output differs from the expected output at line ${line}")
endfunction()

# Checks the output against <prefix>_0, <prefix>_1, ...: each must match when must_match is true,
# and must not otherwise. Every regex that does not hold adds a line to problems.
function(check_regexes output prefix must_match)
    set(index 0)
    while(DEFINED ${prefix}_${index})
        set(regex "${${prefix}_${index}}")
        if(output MATCHES "${regex}")
            set(matches TRUE)
        else()
            set(matches FALSE)
        endif()
        if(must_match AND NOT matches)
            string(APPEND problems "output does not match: ${regex}\n")
        elseif(NOT must_match AND matches)
            string(APPEND problems "output must not match: ${regex}\n")
        endif()
        math(EXPR index "${index} + 1")
    endwhile()
    set(problems "${problems}" PARENT_SCOPE)
endfunction()

# Regex case: the exit code and the regexes, against stdout followed by stderr with CR removed.
function(check_match)
    string(REPLACE "\r" "" output "${stdout}${stderr}")
    set(problems "")
    if(NOT code STREQUAL EXIT_CODE)
        string(APPEND problems "exit code: expected ${EXIT_CODE}, got ${code}\n")
    endif()
    check_regexes("${output}" EXPECT TRUE)
    check_regexes("${output}" REJECT FALSE)
    if(NOT problems STREQUAL "")
        message(NOTICE "${problems}--- output ---\n${output}")
        message(FATAL_ERROR "the output does not match the regexes of the case")
    endif()
endfunction()

if("${ARGS}" STREQUAL "")
    set(args "")
else()
    string(REPLACE "|" ";" args "${ARGS}")
endif()

execute_process(
    COMMAND "${EXE}" ${args}
    OUTPUT_VARIABLE stdout
    ERROR_VARIABLE stderr
    RESULT_VARIABLE code
    TIMEOUT 60
)

if(DEFINED EXPECTED)
    check_golden()
else()
    check_match()
endif()
