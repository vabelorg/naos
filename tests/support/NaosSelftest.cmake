# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# Helpers for the self tests of Naos. Each subject is a directory of tests/ holding its programs, a
# CMakeLists.txt registering its cases and, for golden cases, expected/<case>.txt. The CTest name of
# a case is naos.<subject>.<case>, <subject> being the name of the directory.
#
# naos_selftest_program(<target> <source>...)
#   An executable linked to naos::naos, built with the strict warnings of the self tests.
#
# naos_selftest_strict(<target>...)
#   Gives existing targets the strict warnings, when Naos is the top-level project: an enclosing
#   project that turns NAOS_SELFTEST on keeps its own compilers and flags.
#
# naos_expect(<case> <target> [ARGS <arg>...])
#   Golden case: runs <target> with the arguments and compares them, its exit code, its stdout and
#   its stderr with expected/<case>.txt, after making the output independent of the platform (see
#   support/check-output.cmake). Run the case with NAOS_UPDATE_EXPECTED=1 to write the file.
#
# naos_expect_tests(<target> <suite>.<test>...)
#   One golden case per test, named after it and run with "--naos-filter <suite>.<test>": the case
#   for a test whose whole output is the point.
#
# naos_expect_match(<case> <target> EXIT_CODE <n> [ARGS <arg>...] [EXPECT <regex>...] [REJECT <regex>...])
#   Regex case, for output that differs between runs or platforms (pointers, a seed from the time,
#   the width of long) or that grows with other tests: every EXPECT regex must match stdout followed
#   by stderr, and no REJECT regex may. '.' also matches a newline; the regexes hold no newline.
#
# naos_expect_command_match(<case> <executable path> EXIT_CODE <n> ...)
#   naos_expect_match for an executable that is not a target, such as cmake itself.
#
# The arguments are a CMake list; none may contain '|'.

include_guard(GLOBAL)

set(NAOS_SELFTEST_CHECK_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/check-output.cmake")

function(naos_selftest_strict)
    if(NOT PROJECT_IS_TOP_LEVEL)
        return()
    endif()
    if(MSVC)
        set(options /W4 /WX)
    else()
        set(options -Wall -Wextra -Wpedantic -Werror)
    endif()
    foreach(target IN LISTS ARGN)
        target_compile_options(${target} PRIVATE ${options})
    endforeach()
endfunction()

function(naos_selftest_program target)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target} PRIVATE naos::naos)
    naos_selftest_strict(${target})
endfunction()

# Registers the CTest test of a case, running check-output.cmake with the defines given.
function(naos_selftest_add_case case)
    get_filename_component(subject "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
    set(name "naos.${subject}.${case}")
    add_test(NAME ${name} COMMAND "${CMAKE_COMMAND}" ${ARGN} -P "${NAOS_SELFTEST_CHECK_SCRIPT}")
    set_tests_properties(${name} PROPERTIES LABELS naos)
endfunction()

# Sets out to the -D defines of the executable and its arguments.
function(naos_selftest_run_defines out exe)
    list(JOIN ARGN "|" joined)
    set(${out} -D "EXE=${exe}" -D "ARGS=${joined}" PARENT_SCOPE)
endfunction()

function(naos_expect case target)
    cmake_parse_arguments(PARSE_ARGV 2 arg "" "" "ARGS")
    if(arg_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "naos_expect(${case}): unknown arguments: ${arg_UNPARSED_ARGUMENTS}")
    endif()
    naos_selftest_run_defines(defines "$<TARGET_FILE:${target}>" ${arg_ARGS})
    naos_selftest_add_case(${case} ${defines} -D "EXPECTED=${CMAKE_CURRENT_SOURCE_DIR}/expected/${case}.txt")
endfunction()

function(naos_expect_tests target)
    foreach(test IN LISTS ARGN)
        naos_expect(${test} ${target} ARGS --naos-filter ${test})
    endforeach()
endfunction()

# Appends "-D <prefix>_<i>=<regex>" to the list variable out for each regex of the list variable
# regexes_var.
function(naos_selftest_regex_defines out prefix regexes_var)
    set(index 0)
    foreach(regex IN LISTS ${regexes_var})
        list(APPEND ${out} -D "${prefix}_${index}=${regex}")
        math(EXPR index "${index} + 1")
    endforeach()
    set(${out} "${${out}}" PARENT_SCOPE)
endfunction()

# naos_expect_command_match and naos_expect_match, for caller.
function(naos_selftest_add_match_case caller case exe)
    cmake_parse_arguments(PARSE_ARGV 3 arg "" "EXIT_CODE" "ARGS;EXPECT;REJECT")
    if(arg_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "${caller}(${case}): unknown arguments: ${arg_UNPARSED_ARGUMENTS}")
    endif()
    if(NOT DEFINED arg_EXIT_CODE)
        message(FATAL_ERROR "${caller}(${case}): EXIT_CODE is required")
    endif()
    if((NOT arg_EXPECT) AND (NOT arg_REJECT))
        message(FATAL_ERROR "${caller}(${case}): give EXPECT or REJECT, or use naos_expect")
    endif()
    naos_selftest_run_defines(defines "${exe}" ${arg_ARGS})
    list(APPEND defines -D "EXIT_CODE=${arg_EXIT_CODE}")
    naos_selftest_regex_defines(defines EXPECT arg_EXPECT)
    naos_selftest_regex_defines(defines REJECT arg_REJECT)
    naos_selftest_add_case(${case} ${defines})
endfunction()

function(naos_expect_command_match case exe)
    naos_selftest_add_match_case(naos_expect_command_match ${case} "${exe}" ${ARGN})
endfunction()

function(naos_expect_match case target)
    naos_selftest_add_match_case(naos_expect_match ${case} "$<TARGET_FILE:${target}>" ${ARGN})
endfunction()
