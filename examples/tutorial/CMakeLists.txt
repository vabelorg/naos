# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# The project of docs/getting-started/tutorial.md: a string buffer and its tests, in several files.

cmake_minimum_required(VERSION 3.21)
project(naos_tutorial LANGUAGES C)

enable_testing()

if(NOT TARGET naos::naos)
    include(FetchContent)
    FetchContent_Declare(naos
        GIT_REPOSITORY https://github.com/vabelorg/naos.git
        GIT_TAG v0.0.0
    )
    FetchContent_MakeAvailable(naos)
endif()

add_library(strbuf strbuf.c)
target_include_directories(strbuf PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

naos_add_executable(strbuf_tests
    SOURCES main.c strbuf_test.c
    LIBRARIES strbuf
)
