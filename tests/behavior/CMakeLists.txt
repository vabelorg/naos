# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# The behavior layer: one directory per page of docs/reference/, whose cases compare what a program
# does with a golden file (see harness/NaosSelftest.cmake).

foreach(page IN ITEMS tests assertions control command-line output xml)
    add_subdirectory(${page})
endforeach()
