# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# Naos: project-independent test harness. Usable through add_subdirectory / FetchContent /
# find_package, or built standalone to run its self tests. Nothing here may refer to the
# enclosing project.
#
#   cmake/Naos.cmake          naos_add_executable() and naos_discover_tests(), for the users
#   cmake/NaosDiscover.cmake  the build-time script those run to register the tests with CTest
#   cmake/naos-config.cmake.in, cmake/NaosInstall.cmake  the package for find_package(naos)
#   cmake/NaosVersion.cmake   the version and what follows from it
#   src/                      the library, naos::naos
#   tests/                    the self tests

cmake_minimum_required(VERSION 3.21)

include("${CMAKE_CURRENT_LIST_DIR}/cmake/NaosVersion.cmake")
naos_read_version("${CMAKE_CURRENT_LIST_DIR}/include/naos/version.h")

project(naos
    VERSION ${naos_version}
    LANGUAGES C
)

# Standalone builds own testing, install rules and BUILD_SHARED_LIBS; as a subproject Naos stays
# quiet unless asked.
if(PROJECT_IS_TOP_LEVEL)
    option(BUILD_SHARED_LIBS "Build naos as a shared library" OFF)
    include(CTest)
    # The self tests find naos.dll next to them.
    if(NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
    endif()
endif()
option(NAOS_SELFTEST "Build and register the Naos self tests" ${PROJECT_IS_TOP_LEVEL})
option(NAOS_INSTALL "Generate Naos install rules" ${PROJECT_IS_TOP_LEVEL})

include(GNUInstallDirs)

add_subdirectory(src)

include("${CMAKE_CURRENT_LIST_DIR}/cmake/Naos.cmake")

if(NAOS_SELFTEST)
    if(BUILD_TESTING)
        add_subdirectory(tests)
    else()
        # An enclosing project that has not included CTest would otherwise lose them silently.
        message(STATUS "Naos: NAOS_SELFTEST is ON but BUILD_TESTING is not, so the self tests are not built")
    endif()
endif()

if(NAOS_INSTALL)
    include("${CMAKE_CURRENT_LIST_DIR}/cmake/NaosInstall.cmake")
endif()
