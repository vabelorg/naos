# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# Building without CMake, as docs/getting-started/installation.md describes, with GCC-compatible
# drivers only.

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang" AND NOT CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC"
   AND NOT CMAKE_CROSSCOMPILING AND CMAKE_AR)
    add_test(NAME naos.no_cmake.build
        COMMAND "${CMAKE_COMMAND}"
            -D "CC=${CMAKE_C_COMPILER}"
            -D "AR=${CMAKE_AR}"
            -D "NAOS_DIR=${PROJECT_SOURCE_DIR}"
            -D "WORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/no_cmake"
            -D "EXE_SUFFIX=${CMAKE_EXECUTABLE_SUFFIX}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/no-cmake.cmake"
    )
    set_tests_properties(naos.no_cmake.build PROPERTIES LABELS naos)
endif()
