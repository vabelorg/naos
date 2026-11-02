# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# The CMake functions of Naos, naos_add_executable() and naos_discover_tests()
# (docs/reference/cmake.md#functions), called by project/: the mistakes in their arguments, which
# the configure reports, how they register the tests with CTest (Registration.cmake), and how they
# report a test or a suite defined and never added (Unregistered.cmake).

if(CMAKE_CROSSCOMPILING)
    return()
endif()

# Configures project/ with CASE=<case> and matches what the configure reports. CMake reflows the
# message, so each regex holds words that stay on one line.
function(functions_error case)
    naos_expect_command_match(${case} "${CMAKE_COMMAND}" EXIT_CODE 1
        ARGS -S "${CMAKE_CURRENT_SOURCE_DIR}/project" -B "${CMAKE_CURRENT_BINARY_DIR}/${case}"
             -G "${CMAKE_GENERATOR}" -D CASE=${case} -D "NAOS_SOURCE_DIR=${PROJECT_SOURCE_DIR}"
        ${ARGN})
endfunction()

functions_error(add_unknown_argument EXPECT "naos_add_executable: unknown arguments: BOGUS")
functions_error(add_without_sources EXPECT "naos_add_executable: SOURCES is required")
functions_error(discover_unknown_argument EXPECT "naos_discover_tests: unknown arguments: BOGUS")
functions_error(no_such_target EXPECT "naos_discover_tests: no target named 'nothing'")
functions_error(not_an_executable EXPECT "naos_discover_tests: 't' is not an executable built here")
functions_error(imported_executable EXPECT "naos_discover_tests: 't' is not an executable built here")
functions_error(registered_twice EXPECT "naos_discover_tests: the tests of 't' are already registered")
functions_error(timeout_zero EXPECT "DISCOVERY_TIMEOUT must be a positive integer")
functions_error(timeout_not_a_number EXPECT "DISCOVERY_TIMEOUT must be a positive integer")
functions_error(properties_not_pairs EXPECT "PROPERTIES must be pairs of <name> <value>")
functions_error(ignore_invalid_name EXPECT "IGNORE_SUITES takes suite names" "'a\\.b'")

naos_selftest_add_project_case(registration "${CMAKE_CURRENT_SOURCE_DIR}/Registration.cmake")
naos_selftest_add_project_case(unregistered "${CMAKE_CURRENT_SOURCE_DIR}/Unregistered.cmake")
