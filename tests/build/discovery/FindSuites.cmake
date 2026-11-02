# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# Checks which suites NaosCheckSuites.cmake finds in SOURCE, and on which lines: those, and only
# those, that a comment "found: <name>" names on the line.
#
# Input: -D NAOS_SOURCE_DIR=<root of Naos> -D SOURCE=<file>

cmake_minimum_required(VERSION 3.21)

include("${NAOS_SOURCE_DIR}/cmake/NaosCheckSuites.cmake")

# One item per line, empty lines included: each starts with '_', and the characters that CMake
# lists treat apart are left out.
file(READ "${SOURCE}" content)
foreach(char IN ITEMS "[" "]" "\\" "\r")
    string(REPLACE "${char}" " " content "${content}")
endforeach()
string(REPLACE ";" " " content "${content}")
string(REPLACE "\n" ";_" lines "_${content}")
set(expected "")
set(number 0)
foreach(line IN LISTS lines)
    math(EXPR number "${number} + 1")
    string(REGEX MATCHALL "found: [A-Za-z0-9_]+" marks "${line}")
    foreach(mark IN LISTS marks)
        string(REGEX REPLACE "^found: " "" name "${mark}")
        list(APPEND expected "${name}@${number}")
    endforeach()
endforeach()

set(names "")
set(places "")
naos_find_suites(names places "${SOURCE}")
set(found "")
set(index 0)
foreach(name IN LISTS names)
    list(GET places ${index} place)
    math(EXPR index "${index} + 1")
    string(REGEX REPLACE "^.*:" "" number "${place}")
    list(APPEND found "${name}@${number}")
endforeach()

if(NOT found STREQUAL expected)
    message(FATAL_ERROR "found '${found}', not '${expected}'")
endif()
message(STATUS "found ${found}")
