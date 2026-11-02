# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# Naos added to a project with add_subdirectory(), as FetchContent adds it too: the project of
# tests/consumer/ configured with NAOS_SOURCE_DIR and BUILD_SHARED_LIBS on, built and tested. Its
# configure checks that Naos keeps to itself (see tests/consumer/CMakeLists.txt); Subproject.cmake
# checks that the tests of the project are registered, and those of Naos are not.

if(CMAKE_CROSSCOMPILING)
    return()
endif()

naos_selftest_add_project_case(add_subdirectory "${CMAKE_CURRENT_SOURCE_DIR}/Subproject.cmake")
