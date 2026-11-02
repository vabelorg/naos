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
nested_ctest(output "${binary}")
if(NOT output MATCHES "100% tests passed, 0 tests failed out of 3")
    message(FATAL_ERROR "the project does not have its 3 tests, all passing or skipping:\n${output}")
endif()
if(output MATCHES "naos\\.")
    message(FATAL_ERROR "the self tests of Naos are registered in the project:\n${output}")
endif()

message(STATUS "Naos added with add_subdirectory() keeps to itself")
