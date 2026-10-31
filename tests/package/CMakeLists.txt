# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# A project using an installed Naos with find_package(naos <version>), as
# docs/getting-started/installation.md describes. Not one of the self tests: CI installs each build
# of Naos, then configures, builds and tests this project on its own:
#
#   cmake --install build/default --prefix <prefix>
#   cmake -S tests/package -B <dir> -DCMAKE_PREFIX_PATH=<prefix>
#   cmake --build <dir> && ctest --test-dir <dir>

cmake_minimum_required(VERSION 3.21)
project(naos_package LANGUAGES C)

enable_testing()

# The version of the sources, which the installed package must accept.
include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/NaosVersion.cmake")
naos_read_version("${CMAKE_CURRENT_LIST_DIR}/../../include/naos/version.h")
set(expected_version "${naos_version}")

find_package(naos ${expected_version} REQUIRED)
if(NOT naos_VERSION VERSION_EQUAL expected_version)
    message(FATAL_ERROR "found Naos ${naos_VERSION} in ${naos_DIR}, not ${expected_version}")
endif()

add_executable(package_tests package_test.c)
target_link_libraries(package_tests PRIVATE naos::naos)
target_compile_definitions(package_tests PRIVATE "EXPECTED_VERSION=\"${expected_version}\"")
# docs/guides/shared-library.md: the listing at build time must find naos.dll.
get_target_property(naos_type naos::naos TYPE)
if(WIN32 AND naos_type STREQUAL "SHARED_LIBRARY")
    add_custom_command(TARGET package_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:package_tests> $<TARGET_FILE_DIR:package_tests>
        COMMAND_EXPAND_LISTS
    )
endif()
naos_discover_tests(package_tests)
