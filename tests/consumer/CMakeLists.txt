# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# A project using Naos as docs/getting-started/installation.md describes, in one of two ways:
#
#   find_package(Naos <version>)   the package installed; scripts/TestPackage.cmake configures this
#                                  project so, after installing a build of Naos (CI does it for
#                                  each build)
#   add_subdirectory(<naos>)       with -D NAOS_SOURCE_DIR=<naos>; the self test
#                                  naos.build.subproject.add_subdirectory configures this project so
#
# Added with add_subdirectory(), Naos must keep to itself: it stays a static library whatever
# BUILD_SHARED_LIBS of the project, registers none of its self tests, adds no install rules and
# gives no flag of its own to the targets of the project. The configure fails when it does not.

cmake_minimum_required(VERSION 3.21)
project(naos_consumer LANGUAGES C)

enable_testing()

# The version of the sources, which the Naos found must be.
include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/NaosVersion.cmake")
naos_read_version("${CMAKE_CURRENT_LIST_DIR}/../../include/naos/version.h")
set(expected_version "${naos_version}")

if(DEFINED NAOS_SOURCE_DIR)
    add_subdirectory("${NAOS_SOURCE_DIR}" naos)
else()
    find_package(Naos ${expected_version} REQUIRED)
    if(NOT Naos_VERSION VERSION_EQUAL expected_version)
        message(FATAL_ERROR "found Naos ${Naos_VERSION} in ${Naos_DIR}, not ${expected_version}")
    endif()
endif()

add_executable(consumer_tests consumer_test.c)
target_link_libraries(consumer_tests PRIVATE Naos::naos)
target_compile_definitions(consumer_tests PRIVATE "EXPECTED_VERSION=\"${expected_version}\"")
# docs/guides/shared-library.md: the listing at build time must find naos.dll.
get_target_property(naos_type Naos::naos TYPE)
if(WIN32 AND naos_type STREQUAL "SHARED_LIBRARY")
    add_custom_command(TARGET consumer_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:consumer_tests> $<TARGET_FILE_DIR:consumer_tests>
        COMMAND_EXPAND_LISTS
    )
endif()
naos_discover_tests(consumer_tests)

if(DEFINED NAOS_SOURCE_DIR)
    if(BUILD_SHARED_LIBS AND (NOT naos_type STREQUAL "STATIC_LIBRARY"))
        message(FATAL_ERROR "Naos is a ${naos_type} with BUILD_SHARED_LIBS of the project; it must stay static")
    endif()
    if(NAOS_SELFTEST OR NAOS_INSTALL)
        message(FATAL_ERROR "NAOS_SELFTEST or NAOS_INSTALL is on in a project that did not ask for it")
    endif()
    foreach(target IN ITEMS naos consumer_tests)
        foreach(property IN ITEMS COMPILE_OPTIONS INTERFACE_COMPILE_OPTIONS)
            get_target_property(options ${target} ${property})
            if(options)
                message(FATAL_ERROR "Naos gives ${target} the ${property} '${options}'")
            endif()
        endforeach()
    endforeach()
endif()
