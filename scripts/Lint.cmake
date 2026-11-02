# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# Runs every check that reads the sources and needs no build, and reports each that fails: the lint
# job of CI is this script. Run it from the root of Naos, before sending a change:
#
#   cmake -D ROOT=. -P scripts/Lint.cmake
#
#   CheckMarkdownLinks.cmake    the relative links of the Markdown files
#   CheckPrintfCalls.cmake      the formatted output of src/ goes through src/format.h
#   CheckReleaseScripts.cmake   PrepareRelease.cmake and ReleaseNotes.cmake
#
# -D ROOT=<path>   root of Naos

cmake_minimum_required(VERSION 3.21)

if(NOT DEFINED ROOT)
    message(FATAL_ERROR "Lint.cmake: -D ROOT=<path> is required")
endif()
get_filename_component(ROOT "${ROOT}" ABSOLUTE)

set(failed "")
foreach(check IN ITEMS CheckMarkdownLinks CheckPrintfCalls CheckReleaseScripts)
    message(STATUS "${check}")
    execute_process(COMMAND "${CMAKE_COMMAND}" -D "ROOT=${ROOT}" -P "${CMAKE_CURRENT_LIST_DIR}/${check}.cmake"
        RESULT_VARIABLE code)
    if(NOT code EQUAL 0)
        list(APPEND failed ${check})
    endif()
endforeach()

if(failed)
    list(JOIN failed ", " failed)
    message(FATAL_ERROR "failed: ${failed}")
endif()
message(STATUS "Every check passed")
