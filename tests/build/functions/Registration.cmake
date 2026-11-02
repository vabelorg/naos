# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# How naos_add_executable() registers the tests of an executable with CTest
# (docs/reference/cmake.md#discovery), in a project of its own (project/, CASE=registration):
#
# 1. Before the build, a single failing test <prefix>NOT_BUILT stands in for them, with the labels.
# 2. After it, each test is <prefix><suite>.<test>, running --naos-filter <suite>.<test>, with
#    SKIP_RETURN_CODE 77, the LABELS and the PROPERTIES given; CTest passes, fails and skips them as
#    the tests do.
# 3. With the single-config Ninja and Makefile generators, new arguments register the tests again at the next
#    build, without a change to the executable.
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

# Sets out to the property of the test at index of json, a list for an array, or "(none)". A whole
# number is written without decimals.
function(test_property out json index property)
    set(${out} "(none)" PARENT_SCOPE)
    string(JSON count LENGTH "${json}" tests ${index} properties)
    math(EXPR last "${count} - 1")
    foreach(i RANGE ${last})
        string(JSON name GET "${json}" tests ${index} properties ${i} name)
        if(name STREQUAL property)
            string(JSON type TYPE "${json}" tests ${index} properties ${i} value)
            if(type STREQUAL "ARRAY")
                string(JSON length LENGTH "${json}" tests ${index} properties ${i} value)
                math(EXPR value_last "${length} - 1")
                set(values "")
                foreach(j RANGE ${value_last})
                    string(JSON item GET "${json}" tests ${index} properties ${i} value ${j})
                    list(APPEND values "${item}")
                endforeach()
                set(${out} "${values}" PARENT_SCOPE)
            else()
                string(JSON value GET "${json}" tests ${index} properties ${i} value)
                # CTest writes a number of seconds as 7.0, whole or not.
                if(type STREQUAL "NUMBER")
                    string(REGEX REPLACE "\\.0+$" "" value "${value}")
                endif()
                set(${out} "${value}" PARENT_SCOPE)
            endif()
            return()
        endif()
    endforeach()
endfunction()

# Fails unless the property of the test at index of json is expected.
function(expect_property json index property expected)
    test_property(actual "${json}" ${index} ${property})
    string(JSON name GET "${json}" tests ${index} name)
    if(NOT actual STREQUAL expected)
        message(FATAL_ERROR "${name}: ${property} is '${actual}', not '${expected}'")
    endif()
endfunction()

# Fails unless the tests of json are named prefix + suffixes, each with the labels and its command.
function(expect_tests json prefix labels)
    test_names(names "${json}")
    set(expected "")
    foreach(test IN ITEMS listed.fails listed.passes listed.skips)
        list(APPEND expected "${prefix}${test}")
    endforeach()
    if(NOT names STREQUAL expected)
        message(FATAL_ERROR "the tests are '${names}', not '${expected}'")
    endif()

    foreach(i RANGE 2)
        string(JSON name GET "${json}" tests ${i} name)
        string(LENGTH "${prefix}" prefix_length)
        string(SUBSTRING "${name}" ${prefix_length} -1 test)
        string(JSON command GET "${json}" tests ${i} command)
        if(NOT command MATCHES "\"--naos-filter\",[ \n]*\"${test}\"")
            message(FATAL_ERROR "${name} does not run --naos-filter ${test}: ${command}")
        endif()
        expect_property("${json}" ${i} SKIP_RETURN_CODE 77)
        expect_property("${json}" ${i} LABELS "${labels}")
        expect_property("${json}" ${i} TIMEOUT 7)
        expect_property("${json}" ${i} ENVIRONMENT "A=1;B=2")
    endforeach()
endfunction()

# 1. Configured, not built.
nested_configure("${CMAKE_CURRENT_LIST_DIR}/project" "${binary}"
    -D CASE=registration -D "NAOS_SOURCE_DIR=${NAOS_SOURCE_DIR}" -D LISTED_PREFIX=first. -D LISTED_LABELS=l1,l2)
nested_ctest_json(json "${binary}")
test_names(names "${json}")
if(NOT names STREQUAL "first.NOT_BUILT")
    message(FATAL_ERROR "before the build, the tests are '${names}', not first.NOT_BUILT")
endif()
expect_property("${json}" 0 LABELS "l1;l2")

# 2. Built.
nested_build("${binary}")
nested_ctest_json(json "${binary}")
expect_tests("${json}" first. "l1;l2")
nested_ctest(output "${binary}" -R "passes|skips")
if(NOT output MATCHES "first\\.listed\\.passes[ .]*Passed" OR NOT output MATCHES "first\\.listed\\.skips[ .]*\\*\\*\\*Skipped")
    message(FATAL_ERROR "CTest did not pass and skip the tests:\n${output}")
endif()
execute_process(COMMAND "${CMAKE_CTEST_COMMAND}" --test-dir "${binary}" -C "${CONFIG}" -R fails
    RESULT_VARIABLE code OUTPUT_VARIABLE output ERROR_VARIABLE output)
if(code EQUAL 0 OR NOT output MATCHES "first\\.listed\\.fails[ .]*\\*\\*\\*Failed")
    message(FATAL_ERROR "CTest did not fail the failing test:\n${output}")
endif()

# 3. New arguments, the executable unchanged.
if(GENERATOR MATCHES "Ninja|Makefiles" AND NOT MULTI_CONFIG)
    # The make of macOS (GNU make 3.81) compares times to the second: a parameter file rewritten in
    # the second the executable was linked is not newer than it, and the executable is not linked
    # again. Waiting a second keeps the test from depending on when within a second the build ended.
    execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep 1)
    nested_reconfigure("${binary}" -D LISTED_PREFIX=second. -D LISTED_LABELS=l3)
    nested_build("${binary}")
    nested_ctest_json(json "${binary}")
    expect_tests("${json}" second. "l3")
endif()

message(STATUS "the tests are registered as docs/reference/cmake.md states")
