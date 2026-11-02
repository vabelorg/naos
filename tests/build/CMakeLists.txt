# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# The build layer: what the compiler and CMake judge. Programs that must not compile (misuse/), the
# build without CMake (no_cmake/), the messages of test discovery (discovery/), the CMake functions
# (functions/), and Naos added to a project with add_subdirectory() (subproject/).

foreach(area IN ITEMS misuse no_cmake discovery functions subproject)
    add_subdirectory(${area})
endforeach()
