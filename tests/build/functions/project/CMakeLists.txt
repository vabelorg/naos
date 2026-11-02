# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# A project calling the CMake functions of Naos (cmake/Naos.cmake) as -D CASE=<case> says:
#
#   registration      Naos added with add_subdirectory(NAOS_SOURCE_DIR), and an executable whose
#                     tests are registered with PREFIX, LABELS and PROPERTIES, from the variables
#                     LISTED_PREFIX and LISTED_LABELS (labels separated by ','); and three
#                     executables of a test that runs no assertion, registered with
#                     ALLOW_NO_ASSERTIONS by each function (PREFIX added. and discovered.) and
#                     without it (PREFIX strict.)
#   unregistered      Naos added with add_subdirectory(NAOS_SOURCE_DIR), and executables of a suite
#                     (left_out_suite.c) or of a test and a setup (left_out_test.c) defined and never
#                     added: built by default, those registered with IGNORE_SUITES or
#                     NO_REGISTRATION_CHECK; left out of the default build, those checked, whose build
#                     fails (checked_suite with IGNORE_SUITES from the variable CHECKED_IGNORE)
#   any other case    a mistake in the arguments, which the configure must report. No language is
#                     enabled, since the mistakes are found before anything is built
#
# Used by the self tests of tests/build/functions/.

cmake_minimum_required(VERSION 3.21)

if(CASE STREQUAL "registration")
    project(naos_functions LANGUAGES C)
    enable_testing()
    add_subdirectory("${NAOS_SOURCE_DIR}" naos)
    string(REPLACE "," ";" labels "${LISTED_LABELS}")
    naos_add_executable(listed
        SOURCES listed.c
        PREFIX "${LISTED_PREFIX}"
        LABELS ${labels}
        PROPERTIES TIMEOUT 7 ENVIRONMENT "A=1;B=2"
    )
    naos_add_executable(unchecked_added SOURCES unchecked.c PREFIX added. ALLOW_NO_ASSERTIONS)
    add_executable(unchecked_discovered unchecked.c)
    target_link_libraries(unchecked_discovered PRIVATE Naos::naos)
    naos_discover_tests(unchecked_discovered PREFIX discovered. ALLOW_NO_ASSERTIONS)
    naos_add_executable(unchecked_strict SOURCES unchecked.c PREFIX strict.)
    return()
endif()

if(CASE STREQUAL "unregistered")
    project(naos_functions LANGUAGES C)
    enable_testing()
    add_subdirectory("${NAOS_SOURCE_DIR}" naos)

    naos_add_executable(ignored_added SOURCES left_out_suite.c IGNORE_SUITES left_out)
    add_executable(ignored_discovered left_out_suite.c)
    target_link_libraries(ignored_discovered PRIVATE Naos::naos)
    naos_discover_tests(ignored_discovered IGNORE_SUITES left_out)
    naos_add_executable(unchecked_suite SOURCES left_out_suite.c NO_REGISTRATION_CHECK)
    naos_add_executable(unchecked_test SOURCES left_out_test.c NO_REGISTRATION_CHECK)

    naos_add_executable(checked_suite SOURCES left_out_suite.c IGNORE_SUITES ${CHECKED_IGNORE})
    add_executable(checked_discovered left_out_suite.c)
    target_link_libraries(checked_discovered PRIVATE Naos::naos)
    naos_discover_tests(checked_discovered)
    naos_add_executable(checked_test SOURCES left_out_test.c)
    set_target_properties(checked_suite checked_discovered checked_test PROPERTIES EXCLUDE_FROM_ALL TRUE)
    return()
endif()

project(naos_functions LANGUAGES NONE)
include("${NAOS_SOURCE_DIR}/cmake/Naos.cmake")

if(CASE STREQUAL "add_unknown_argument")
    naos_add_executable(t BOGUS SOURCES listed.c)
elseif(CASE STREQUAL "add_without_sources")
    naos_add_executable(t LIBRARIES m)
elseif(CASE STREQUAL "discover_unknown_argument")
    add_executable(t listed.c)
    naos_discover_tests(t BOGUS x)
elseif(CASE STREQUAL "no_such_target")
    naos_discover_tests(nothing)
elseif(CASE STREQUAL "not_an_executable")
    add_library(t INTERFACE)
    naos_discover_tests(t)
elseif(CASE STREQUAL "imported_executable")
    add_executable(t IMPORTED)
    naos_discover_tests(t)
elseif(CASE STREQUAL "registered_twice")
    add_executable(t listed.c)
    naos_discover_tests(t)
    naos_discover_tests(t PREFIX other.)
elseif(CASE STREQUAL "timeout_zero")
    add_executable(t listed.c)
    naos_discover_tests(t DISCOVERY_TIMEOUT 0)
elseif(CASE STREQUAL "timeout_not_a_number")
    add_executable(t listed.c)
    naos_discover_tests(t DISCOVERY_TIMEOUT 1.5)
elseif(CASE STREQUAL "properties_not_pairs")
    add_executable(t listed.c)
    naos_discover_tests(t PROPERTIES TIMEOUT 7 WILL_FAIL)
elseif(CASE STREQUAL "ignore_invalid_name")
    add_executable(t listed.c)
    naos_discover_tests(t IGNORE_SUITES kept a.b)
else()
    message(FATAL_ERROR "unknown CASE '${CASE}'")
endif()
