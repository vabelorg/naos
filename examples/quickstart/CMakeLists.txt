# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# The project of docs/getting-started/quickstart.md: a small library and its tests.

cmake_minimum_required(VERSION 3.21)
project(naos_quickstart LANGUAGES C)

enable_testing()

# Inside the self tests of Naos, naos::naos is already there; on its own, the example fetches it.
if(NOT TARGET naos::naos)
    include(FetchContent)
    FetchContent_Declare(naos
        GIT_REPOSITORY https://github.com/vabelorg/naos.git
        GIT_TAG v0.0.0
    )
    FetchContent_MakeAvailable(naos)
endif()

add_library(clamp clamp.c)
target_include_directories(clamp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

naos_add_executable(clamp_tests SOURCES clamp_test.c LIBRARIES clamp)
