# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# Checks which suites and tests NaosCheckRegistration.cmake finds in SOURCE, and on which lines:
# those, and only those, that a comment "found: <name>" (a suite) or "test: <name>" (a test) names
# on the line.
#
# Input: -D NAOS_SOURCE_DIR=<root of Naos> -D SOURCE=<file>

cmake_minimum_required(VERSION 3.21)

include("${NAOS_SOURCE_DIR}/cmake/NaosCheckRegistration.cmake")

# One item per line, empty lines included: each starts with '_', and the characters that CMake
# lists treat apart are left out.
file(READ "${SOURCE}" content)
foreach(char IN ITEMS "[" "]" "\\" "\r")
    string(REPLACE "${char}" " " content "${content}")
endforeach()
string(REPLACE ";" " " content "${content}")
string(REPLACE "\n" ";_" lines "_${content}")

# Sets out to "<name>@<line>" for each comment "<mark>: <name>" of the lines.
function(expected_marks out mark)
    set(expected "")
    set(number 0)
    foreach(line IN LISTS lines)
        math(EXPR number "${number} + 1")
        string(REGEX MATCHALL "${mark}: [A-Za-z0-9_]+" marks "${line}")
        foreach(item IN LISTS marks)
            string(REGEX REPLACE "^${mark}: " "" name "${item}")
            list(APPEND expected "${name}@${number}")
        endforeach()
    endforeach()
    set(${out} "${expected}" PARENT_SCOPE)
endfunction()

# Sets out to "<name>@<line>" for each of names, at places.
function(found_at out names places)
    set(found "")
    set(index 0)
    foreach(name IN LISTS names)
        list(GET places ${index} place)
        math(EXPR index "${index} + 1")
        string(REGEX REPLACE "^.*:" "" number "${place}")
        list(APPEND found "${name}@${number}")
    endforeach()
    set(${out} "${found}" PARENT_SCOPE)
endfunction()

foreach(kind IN ITEMS suites tests)
    if(kind STREQUAL "suites")
        expected_marks(expected found)
    else()
        expected_marks(expected test)
    endif()
    set(names "")
    set(places "")
    if(kind STREQUAL "suites")
        naos_find_suites(names places "${SOURCE}")
    else()
        naos_find_tests(names places "${SOURCE}")
    endif()
    found_at(found "${names}" "${places}")
    if(NOT found STREQUAL expected)
        message(FATAL_ERROR "${kind}: found '${found}', not '${expected}'")
    endif()
    message(STATUS "${kind}: found ${found}")
endforeach()
