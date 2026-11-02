# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# Helpers for the self tests of Naos. A directory of tests/ holds the programs of one area, a
# CMakeLists.txt registering its cases and, for golden cases, expected/<case>.txt. The first
# directory under tests/ is the layer, which tells who judges the cases (see tests/CMakeLists.txt).
# The CTest name of a case is naos.<path>.<case>, <path> being the directory under tests/ with '/'
# made '.' (naos.behavior.assertions.integer.eq); every case has the labels naos and <layer>.
#
# naos_selftest_program(<target> <source>...)
#   An executable linked to Naos::naos, built with the strict warnings of the self tests.
#
# naos_selftest_strict(<target>...)
#   Gives existing targets the strict warnings, when Naos is the top-level project: an enclosing
#   project that turns NAOS_SELFTEST on keeps its own compilers and flags.
#
# naos_selftest_prefix(<out>)
# naos_selftest_labels(<out>)
#   The start of the CTest names (naos.<path>.) and the labels (naos;<layer>) of the current
#   directory, for the tests registered by naos_discover_tests().
#
# naos_selftest_add_test(<case> <command>...)
#   A case that runs a command and passes when it exits with 0.
#
# naos_selftest_add_project_case(<case> <script> [<define>...])
#   A case that runs <script> with cmake -P, which configures, builds and tests a project of its own
#   through harness/NestedProject.cmake: the script gets the generator, the compiler and the
#   configuration of this build, and NAOS_SOURCE_DIR, WORK_DIR (a directory of its own in the
#   build directory) and the defines given, each as "NAME=value". Such a case builds a project, so it
#   is slow; it gets the label slow as well.
#
# naos_expect(<case> <target> [ARGS <arg>...] [OUTPUT_FILE <name> [FILE_ONLY]])
#   Golden case: runs <target> with the arguments and compares them, its exit code, its stdout and
#   its stderr with expected/<case>.txt, after making the output independent of the platform (see
#   harness/RunCase.cmake). Run the case with NAOS_UPDATE_EXPECTED=1 to write the file.
#   With OUTPUT_FILE, {output} in the arguments stands for the file <name> of the build directory,
#   which the case removes before the run and compares after it as well; where xmllint is found, a
#   file the case writes must also be well-formed XML. With FILE_ONLY as well, stdout and stderr
#   are not compared: for tests that print bytes beyond ASCII there, which CMake reads in the code
#   page of the platform.
#
# naos_expect_tests(<target> <suite>.<test>...)
#   One golden case per test, named after it and run with "--naos-filter <suite>.<test>": the case
#   for a test whose whole output is the point.
#
# naos_expect_match(<case> <target> EXIT_CODE <n> [ARGS <arg>...] [EXPECT <regex>...] [REJECT <regex>...]
#                   [OUTPUT_FILE <name> [FILE absent|empty|written] [SCHEMA <xsd>]])
#   Regex case, for output that differs between runs or platforms (pointers, a seed from the time,
#   the width of long), that grows with other tests, or that a contract fixes whatever the golden
#   files say: the exit code must be <n>, every EXPECT regex must match stdout followed by stderr,
#   and no REJECT regex may. '.' also matches a newline; the regexes hold no newline. With
#   OUTPUT_FILE, {output} is a file as for naos_expect: with FILE, the run must leave it absent,
#   empty or written (not empty); with SCHEMA, where xmllint is found, a file written must be valid
#   against <xsd>.
#
# naos_expect_command_match(<case> <executable path> EXIT_CODE <n> ...)
#   naos_expect_match for an executable that is not a target, such as cmake itself.
#
# The arguments are a CMake list; none may contain '|'.

include_guard(GLOBAL)

set(NAOS_SELFTEST_RUN_CASE "${CMAKE_CURRENT_LIST_DIR}/RunCase.cmake")
get_filename_component(NAOS_SELFTEST_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

# Checks the files of OUTPUT_FILE, where it is installed.
find_program(NAOS_SELFTEST_XMLLINT xmllint)

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
    target_link_libraries(${target} PRIVATE Naos::naos)
    naos_selftest_strict(${target})
endfunction()

# Sets path to the directory of the caller under tests/ with '/' made '.', and layer to its first part.
function(naos_selftest_where path_out layer_out)
    file(RELATIVE_PATH relative "${NAOS_SELFTEST_ROOT}" "${CMAKE_CURRENT_SOURCE_DIR}")
    string(REGEX REPLACE "/.*" "" layer "${relative}")
    string(REPLACE "/" "." path "${relative}")
    set(${path_out} "${path}" PARENT_SCOPE)
    set(${layer_out} "${layer}" PARENT_SCOPE)
endfunction()

function(naos_selftest_prefix out)
    naos_selftest_where(path layer)
    set(${out} "naos.${path}." PARENT_SCOPE)
endfunction()

function(naos_selftest_labels out)
    naos_selftest_where(path layer)
    set(${out} "naos;${layer}" PARENT_SCOPE)
endfunction()

function(naos_selftest_add_test case)
    naos_selftest_where(path layer)
    set(name "naos.${path}.${case}")
    add_test(NAME ${name} COMMAND ${ARGN})
    set_tests_properties(${name} PROPERTIES LABELS "naos;${layer}")
endfunction()

function(naos_selftest_add_project_case case script)
    get_property(multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
    if(multi_config)
        set(config Debug)
    elseif(CMAKE_BUILD_TYPE)
        set(config "${CMAKE_BUILD_TYPE}")
    else()
        set(config Debug)
    endif()
    set(defines
        -D "GENERATOR=${CMAKE_GENERATOR}"
        -D "GENERATOR_PLATFORM=${CMAKE_GENERATOR_PLATFORM}"
        -D "GENERATOR_TOOLSET=${CMAKE_GENERATOR_TOOLSET}"
        -D "MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}"
        -D "C_COMPILER=${CMAKE_C_COMPILER}"
        -D "CONFIG=${config}"
        -D "MULTI_CONFIG=$<BOOL:${multi_config}>"
        -D "NAOS_SOURCE_DIR=${PROJECT_SOURCE_DIR}"
        -D "WORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/${case}")
    foreach(define IN LISTS ARGN)
        list(APPEND defines -D "${define}")
    endforeach()
    naos_selftest_add_test(${case} "${CMAKE_COMMAND}" ${defines} -P "${script}")
    naos_selftest_where(path layer)
    set_property(TEST "naos.${path}.${case}" APPEND PROPERTY LABELS slow)
endfunction()

# Registers a case that runs RunCase.cmake with the defines given.
function(naos_selftest_add_case case)
    naos_selftest_add_test(${case} "${CMAKE_COMMAND}" ${ARGN} -P "${NAOS_SELFTEST_RUN_CASE}")
endfunction()

# Sets out to the -D defines of the executable and its arguments.
function(naos_selftest_run_defines out exe)
    list(JOIN ARGN "|" joined)
    set(${out} -D "EXE=${exe}" -D "ARGS=${joined}" PARENT_SCOPE)
endfunction()

# Appends to the list variable out the defines of OUTPUT_FILE <name>.
function(naos_selftest_output_defines out name)
    list(APPEND ${out} -D "OUTPUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/${name}")
    if(NAOS_SELFTEST_XMLLINT)
        list(APPEND ${out} -D "XMLLINT=${NAOS_SELFTEST_XMLLINT}")
    endif()
    set(${out} "${${out}}" PARENT_SCOPE)
endfunction()

function(naos_expect case target)
    cmake_parse_arguments(PARSE_ARGV 2 arg "FILE_ONLY" "OUTPUT_FILE" "ARGS")
    if(arg_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "naos_expect(${case}): unknown arguments: ${arg_UNPARSED_ARGUMENTS}")
    endif()
    if(arg_FILE_ONLY AND (NOT DEFINED arg_OUTPUT_FILE))
        message(FATAL_ERROR "naos_expect(${case}): FILE_ONLY needs OUTPUT_FILE")
    endif()
    naos_selftest_run_defines(defines "$<TARGET_FILE:${target}>" ${arg_ARGS})
    if(DEFINED arg_OUTPUT_FILE)
        naos_selftest_output_defines(defines "${arg_OUTPUT_FILE}")
        if(arg_FILE_ONLY)
            list(APPEND defines -D "FILE_ONLY=1")
        endif()
    endif()
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
    cmake_parse_arguments(PARSE_ARGV 3 arg "" "EXIT_CODE;OUTPUT_FILE;FILE;SCHEMA" "ARGS;EXPECT;REJECT")
    if(arg_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "${caller}(${case}): unknown arguments: ${arg_UNPARSED_ARGUMENTS}")
    endif()
    if(NOT DEFINED arg_EXIT_CODE)
        message(FATAL_ERROR "${caller}(${case}): EXIT_CODE is required")
    endif()
    if((DEFINED arg_SCHEMA OR DEFINED arg_FILE) AND (NOT DEFINED arg_OUTPUT_FILE))
        message(FATAL_ERROR "${caller}(${case}): FILE and SCHEMA need OUTPUT_FILE")
    endif()
    if(DEFINED arg_FILE AND (NOT arg_FILE MATCHES "^(absent|empty|written)$"))
        message(FATAL_ERROR "${caller}(${case}): FILE is absent, empty or written: '${arg_FILE}'")
    endif()
    naos_selftest_run_defines(defines "${exe}" ${arg_ARGS})
    list(APPEND defines -D "EXIT_CODE=${arg_EXIT_CODE}")
    if(DEFINED arg_OUTPUT_FILE)
        naos_selftest_output_defines(defines "${arg_OUTPUT_FILE}")
        if(DEFINED arg_FILE)
            list(APPEND defines -D "FILE_STATE=${arg_FILE}")
        endif()
        if(DEFINED arg_SCHEMA)
            list(APPEND defines -D "SCHEMA=${arg_SCHEMA}")
        endif()
    endif()
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
