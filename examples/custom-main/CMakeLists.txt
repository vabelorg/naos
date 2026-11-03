# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# The example of docs/guides/custom-main.md: a main() of its own, adding a suite on --slow.

cmake_minimum_required(VERSION 3.21)
project(naos_custom_main LANGUAGES C)

enable_testing()

if(NOT TARGET Naos::naos)
    include(FetchContent)
    FetchContent_Declare(Naos
        GIT_REPOSITORY https://github.com/vabelorg/naos.git
        GIT_TAG v0.4.0
    )
    FetchContent_MakeAvailable(Naos)
endif()

# CTest gets the tests listed without arguments: those of the suite quick.
naos_add_executable(custom_main_tests SOURCES main.c quick_test.c slow_test.c)
