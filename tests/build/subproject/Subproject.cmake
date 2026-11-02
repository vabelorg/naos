# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# Configures tests/consumer/ with Naos added by add_subdirectory() and BUILD_SHARED_LIBS on, builds
# it, and runs its tests: those of the project only, every one passing or skipping.
#
# Defines: those of naos_selftest_add_project_case().

include("${CMAKE_CURRENT_LIST_DIR}/../../harness/NestedProject.cmake")

set(binary "${WORK_DIR}/build")

nested_configure("${CMAKE_CURRENT_LIST_DIR}/../../consumer" "${binary}"
    -D "NAOS_SOURCE_DIR=${NAOS_SOURCE_DIR}" -D BUILD_SHARED_LIBS=ON)
nested_build("${binary}")
# The tests registered, by name: those of the project, and none of Naos. ctest itself fails the
# script when one of them fails; its summary line, which differs between versions, is not read.
nested_ctest_json(json "${binary}")
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
set(expected consumer_tests.consumer.headers consumer_tests.consumer.library consumer_tests.consumer.skips)
if(NOT names STREQUAL expected)
    message(FATAL_ERROR "the project has the tests '${names}', not '${expected}'")
endif()
nested_ctest(output "${binary}")

message(STATUS "Naos added with add_subdirectory() keeps to itself")
