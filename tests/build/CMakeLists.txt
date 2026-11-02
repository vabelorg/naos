# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# The build layer: what the compiler and CMake judge. Programs that must not compile (misuse/), the
# messages of test discovery and its check of the suites (discovery/), the CMake functions
# (functions/), and Naos added to a project with add_subdirectory() (subproject/).

foreach(area IN ITEMS misuse discovery functions subproject)
    add_subdirectory(${area})
endforeach()
