# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# The example of docs/guides/stubs.md: module_a is tested with a stub of module_b linked in place of
# the real one.

cmake_minimum_required(VERSION 3.21)
project(naos_stubs LANGUAGES C)

enable_testing()

if(NOT TARGET naos::naos)
    include(FetchContent)
    FetchContent_Declare(naos
        GIT_REPOSITORY https://github.com/vabelorg/naos.git
        GIT_TAG v0.0.0
    )
    FetchContent_MakeAvailable(naos)
endif()

# The program links the real module_b.c; the tests link fake_module_b.c in its place, and list the
# sources of module_a directly instead of linking a library that holds module_b too.
naos_add_executable(module_a_tests SOURCES module_a_test.c module_a.c fake_module_b.c)
