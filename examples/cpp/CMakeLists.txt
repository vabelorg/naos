# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# The example of docs/guides/cpp.md: tests in C++, with the same headers and macros.

cmake_minimum_required(VERSION 3.21)
project(naos_cpp LANGUAGES C CXX)

enable_testing()

if(NOT TARGET naos::naos)
    include(FetchContent)
    FetchContent_Declare(naos
        GIT_REPOSITORY https://github.com/vabelorg/naos.git
        GIT_TAG v0.1.0
    )
    FetchContent_MakeAvailable(naos)
endif()

naos_add_executable(cpp_tests SOURCES words_test.cpp)
# The macros are variadic: C++11 or later.
set_target_properties(cpp_tests PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
