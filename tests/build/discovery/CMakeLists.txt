# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# Test discovery: when listing the tests fails after a build, the message of NaosDiscover.cmake says
# why and what to do. FakeList.cmake stands in for the executable, run as its emulator. Then the
# check that every test and suite defined in the sources is added, NaosCheckRegistration.cmake.

# Runs NaosDiscover.cmake on FakeList.cmake in mode with the timeout given, and matches its output.
# The parameter file is the one Naos.cmake writes, with FakeList.cmake as the executable.
function(naos_expect_discovery case mode timeout)
    set(params_file "${CMAKE_CURRENT_BINARY_DIR}/${case}_params.cmake")
    _naos_discovery_params(params
        TARGET "fake_${case}"
        EXECUTABLE "${CMAKE_CURRENT_SOURCE_DIR}/FakeList.cmake"
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

# The check that every test and suite defined is added (NaosCheckRegistration.cmake): which text
# counts as a definition, then the script run alone on executables with a suite (unadded.c) or a
# test (unadded_test.c) defined and not added.
naos_selftest_add_test(find_definitions "${CMAKE_COMMAND}" -D "NAOS_SOURCE_DIR=${PROJECT_SOURCE_DIR}"
    -D "SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/scan.c" -P "${CMAKE_CURRENT_SOURCE_DIR}/FindDefinitions.cmake")

if(NOT CMAKE_CROSSCOMPILING)
    set(check_script "${PROJECT_SOURCE_DIR}/cmake/NaosCheckRegistration.cmake")

    naos_selftest_program(naos_selftest_unadded unadded.c)
    set(check_args -D "NAOS_EXECUTABLE=$<TARGET_FILE:naos_selftest_unadded>"
        -D "NAOS_SOURCES=${CMAKE_CURRENT_SOURCE_DIR}/unadded.c")
    naos_expect_command_match(check_script_unadded "${CMAKE_COMMAND}" EXIT_CODE 1
        ARGS ${check_args} -P "${check_script}"
        EXPECT "unadded\\.c:19: error: suite left_out is defined but never added" "NAOS_IGNORE_SUITES"
        REJECT "suite added " "test runs ")
    naos_expect_command_match(check_script_ignored "${CMAKE_COMMAND}" EXIT_CODE 0
        ARGS ${check_args} -D NAOS_IGNORE_SUITES=left_out -P "${check_script}"
        REJECT "error")

    # Not with the strict warnings: the test never added is an unused static function.
    add_executable(naos_selftest_forgotten unadded_test.c)
    target_link_libraries(naos_selftest_forgotten PRIVATE Naos::naos)
    naos_expect_command_match(check_script_forgotten "${CMAKE_COMMAND}" EXIT_CODE 1
        ARGS -D "NAOS_EXECUTABLE=$<TARGET_FILE:naos_selftest_forgotten>"
             -D "NAOS_SOURCES=${CMAKE_CURRENT_SOURCE_DIR}/unadded_test.c" -P "${check_script}"
        EXPECT "unadded_test\\.c:10: error: test forgotten is defined but never added" "NAOS_ADD_TEST\\(forgotten\\)"
        REJECT "test runs " "suite kept ")
endif()
