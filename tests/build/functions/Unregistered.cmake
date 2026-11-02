# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# How the CMake functions report a suite, a test or a fixture defined and never added
# (docs/reference/cmake.md#registration-check), in a project of its own (project/,
# CASE=unregistered):
#
# 1. A suite left out on purpose, named in IGNORE_SUITES of either function, and both kinds of
#    mistake under NO_REGISTRATION_CHECK: the executables build, and their tests are registered.
# 2. Without them, the build fails: discovery names the file, the line and the suite left out, for
#    either function; naos_add_executable() makes the compiler reject the test and the setup.
# 3. With the single-config Ninja and Makefile generators, a suite named in IGNORE_SUITES, then no
#    longer: the build that fails removes the tests it had registered, and CTest has
#    <prefix>NOT_BUILT in their place.
#
# Defines: those of naos_selftest_add_project_case().

include("${CMAKE_CURRENT_LIST_DIR}/../../harness/NestedProject.cmake")

set(binary "${WORK_DIR}/build")

# Sets out to the names of the tests in json, sorted.
function(test_names out json)
    string(JSON count LENGTH "${json}" tests)
    set(names "")
    if(count GREATER 0)
        math(EXPR last "${count} - 1")
        foreach(i RANGE ${last})
            string(JSON name GET "${json}" tests ${i} name)
            list(APPEND names "${name}")
        endforeach()
    endif()
    list(SORT names)
    set(${out} "${names}" PARENT_SCOPE)
endfunction()

# Fails unless the tests whose names start with prefix are expected.
function(expect_tests prefix expected)
    nested_ctest_json(json "${binary}")
    test_names(names "${json}")
    set(actual "")
    foreach(name IN LISTS names)
        string(FIND "${name}" "${prefix}" at)
        if(at EQUAL 0)
            list(APPEND actual "${name}")
        endif()
    endforeach()
    if(NOT actual STREQUAL expected)
        message(FATAL_ERROR "the tests of ${prefix} are '${actual}', not '${expected}'")
    endif()
endfunction()

# Fails unless building target fails with an output that every regex given matches.
function(expect_build_failure target)
    execute_process(COMMAND "${CMAKE_COMMAND}" --build "${binary}" --config "${CONFIG}" --target ${target}
        RESULT_VARIABLE code OUTPUT_VARIABLE output ERROR_VARIABLE output)
    if(code EQUAL 0)
        message(FATAL_ERROR "the build of ${target} did not fail:\n${output}")
    endif()
    foreach(regex IN LISTS ARGN)
        if(NOT output MATCHES "${regex}")
            message(FATAL_ERROR "the build of ${target} failed without '${regex}':\n${output}")
        endif()
    endforeach()
endfunction()

set(suite_error "left_out_suite\\.c:19: error: suite left_out is defined but never added")

# 1. Left out on purpose, or not checked.
nested_configure("${CMAKE_CURRENT_LIST_DIR}/project" "${binary}"
    -D CASE=unregistered -D "NAOS_SOURCE_DIR=${NAOS_SOURCE_DIR}" -D CHECKED_IGNORE=)
nested_build("${binary}")
foreach(target IN ITEMS ignored_added ignored_discovered unchecked_suite unchecked_test)
    expect_tests("${target}." "${target}.kept.passes")
endforeach()
nested_ctest(output "${binary}" -R "^(ignored|unchecked)_")

# 2. Checked.
expect_build_failure(checked_suite "${suite_error}" "IGNORE_SUITES")
expect_tests("checked_suite." "checked_suite.NOT_BUILT")
expect_build_failure(checked_discovered "${suite_error}")
expect_build_failure(checked_test "naos_gen_test_left_out" "naos_gen_setup_left_out")

# 3. Ignored, then no longer.
if(GENERATOR MATCHES "Ninja|Makefiles" AND NOT MULTI_CONFIG)
    nested_reconfigure("${binary}" -D CHECKED_IGNORE=left_out)
    nested_run("build" "${CMAKE_COMMAND}" --build "${binary}" --config "${CONFIG}" --target checked_suite)
    expect_tests("checked_suite." "checked_suite.kept.passes")
    # A parameter file rewritten in the second the executable was linked is not newer than it for
    # the make of macOS (see Registration.cmake).
    execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep 1)
    nested_reconfigure("${binary}" -D CHECKED_IGNORE=)
    expect_build_failure(checked_suite "${suite_error}")
    expect_tests("checked_suite." "checked_suite.NOT_BUILT")
endif()

message(STATUS "the suites, tests and fixtures never added are reported as docs/reference/cmake.md states")
