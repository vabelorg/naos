# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# Test discovery: when listing the tests fails after a build, the message of NaosDiscover.cmake says
# why and what to do. fake-list.cmake stands in for the executable, run as its emulator.

# Runs NaosDiscover.cmake on fake-list.cmake in mode with the timeout given, and matches its output.
# The parameter file is the one Naos.cmake writes, with fake-list.cmake as the executable.
function(naos_expect_discovery case mode timeout)
    set(params_file "${CMAKE_CURRENT_BINARY_DIR}/${case}_params.cmake")
    _naos_discovery_params(params
        TARGET "fake_${case}"
        EXECUTABLE "${CMAKE_CURRENT_SOURCE_DIR}/fake-list.cmake"
        EMULATOR "${CMAKE_COMMAND};-D;MODE=${mode};-P"
        PREFIX ""
        LABELS ""
        PROPERTIES ""
        DISCOVERY_TIMEOUT "${timeout}"
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${case}_tests.cmake"
    )
    file(WRITE "${params_file}" "${params}")
    naos_expect_command_match(${case} "${CMAKE_COMMAND}" EXIT_CODE 1
        ARGS -D NAOS_PARAMS=${params_file} -P ${PROJECT_SOURCE_DIR}/cmake/NaosDiscover.cmake ${ARGN})
endfunction()

# CMake reflows the message, so each regex matches only words that start a paragraph or stay on one line.
naos_expect_discovery(timeout hang 1 EXPECT "Failed to list the tests of fake_timeout \\(result: [^)]*timeout"
    "Listing the tests did not finish within" "DISCOVERY_TIMEOUT" REJECT "shared library")
naos_expect_discovery(missing_library missing_library 30 EXPECT "libfake\\.so"
    "The executable could not start because a shared library" "LD_LIBRARY_PATH" REJECT "DISCOVERY_TIMEOUT")
naos_expect_discovery(other_failure fail 30 EXPECT "fake-list-failed"
    "If the executable could not start because a shared" "LD_LIBRARY_PATH" REJECT "DISCOVERY_TIMEOUT")
