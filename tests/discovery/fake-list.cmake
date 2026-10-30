# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# Stands in for a test executable run as "<emulator> <exe> --naos-list" by NaosDiscover.cmake, with
# the emulator "cmake -D MODE=<mode> -P" and this file as the executable.
#
# MODE=hang            : runs until killed by the timeout of the listing
# MODE=missing_library : fails with the message of the Linux loader for a missing shared library
# MODE=fail            : fails with another message

cmake_minimum_required(VERSION 3.21)

if(MODE STREQUAL "hang")
    string(TIMESTAMP start "%s")
    while(TRUE)
        string(TIMESTAMP now "%s")
        math(EXPR elapsed "${now} - ${start}")
        if(elapsed GREATER 60)
            break()
        endif()
    endwhile()
elseif(MODE STREQUAL "missing_library")
    message(FATAL_ERROR "fake-list: error while loading shared libraries: libfake.so: cannot open shared object file")
elseif(MODE STREQUAL "fail")
    message(FATAL_ERROR "fake-list-failed")
else()
    message(FATAL_ERROR "unknown MODE: ${MODE}")
endif()
