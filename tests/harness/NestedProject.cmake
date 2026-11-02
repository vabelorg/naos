# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# Included by the scripts of the self tests that configure, build and test a project of their own,
# with the generator and the compiler of the build of Naos. naos_selftest_project_defines() of
# NaosSelftest.cmake passes them:
#
# -D GENERATOR=<name>          -D GENERATOR_PLATFORM=<p>   -D GENERATOR_TOOLSET=<t>
# -D MAKE_PROGRAM=<path>       -D C_COMPILER=<path>        -D CONFIG=<Debug|Release|...>
# -D MULTI_CONFIG=<0|1>
#
# nested_configure(<source> <binary> <arg>...)  configures, the binary directory emptied first
# nested_reconfigure(<binary> <arg>...)         configures again, keeping the cache
# nested_build(<binary>)                        builds the configuration CONFIG
# nested_ctest(<out> <binary> <arg>...)         runs ctest, its output in <out>; fails when it fails
# nested_ctest_json(<out> <binary> <arg>...)    the tests ctest -N would run, as the JSON of
#                                               --show-only=json-v1
# Each fails the script with the output of the command when the command fails.

cmake_minimum_required(VERSION 3.21)

# Runs a command, failing the script with what it printed unless it exits with 0.
function(nested_run what)
    execute_process(COMMAND ${ARGN} RESULT_VARIABLE code OUTPUT_VARIABLE output ERROR_VARIABLE output)
    if(NOT code EQUAL 0)
        list(JOIN ARGN " " command)
        message(FATAL_ERROR "${what} failed (${code}): ${command}\n${output}")
    endif()
    set(nested_output "${output}" PARENT_SCOPE)
endfunction()

function(nested_configure_args out)
    set(args -G "${GENERATOR}")
    if(GENERATOR_PLATFORM)
        list(APPEND args -A "${GENERATOR_PLATFORM}")
    endif()
    if(GENERATOR_TOOLSET)
        list(APPEND args -T "${GENERATOR_TOOLSET}")
    endif()
    if(MAKE_PROGRAM)
        list(APPEND args "-DCMAKE_MAKE_PROGRAM=${MAKE_PROGRAM}")
    endif()
    # The generators of an IDE pick the compiler themselves.
    if(C_COMPILER AND (NOT GENERATOR MATCHES "^(Visual Studio|Xcode)"))
        list(APPEND args "-DCMAKE_C_COMPILER=${C_COMPILER}")
    endif()
    if(NOT MULTI_CONFIG)
        list(APPEND args "-DCMAKE_BUILD_TYPE=${CONFIG}")
    endif()
    set(${out} "${args}" PARENT_SCOPE)
endfunction()

function(nested_configure source binary)
    file(REMOVE_RECURSE "${binary}")
    nested_configure_args(args)
    nested_run("configure" "${CMAKE_COMMAND}" -S "${source}" -B "${binary}" ${args} ${ARGN})
endfunction()

function(nested_reconfigure binary)
    # A binary directory alone, without -S, configures again the source it was configured from.
    nested_run("configure" "${CMAKE_COMMAND}" ${ARGN} "${binary}")
endfunction()

function(nested_build binary)
    nested_run("build" "${CMAKE_COMMAND}" --build "${binary}" --config "${CONFIG}")
endfunction()

function(nested_ctest out binary)
    nested_run("ctest" "${CMAKE_CTEST_COMMAND}" --test-dir "${binary}" -C "${CONFIG}" ${ARGN})
    set(${out} "${nested_output}" PARENT_SCOPE)
endfunction()

function(nested_ctest_json out binary)
    nested_run("ctest" "${CMAKE_CTEST_COMMAND}" --test-dir "${binary}" -C "${CONFIG}" -N --show-only=json-v1 ${ARGN})
    set(${out} "${nested_output}" PARENT_SCOPE)
endfunction()
