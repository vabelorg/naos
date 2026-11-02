# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# Checks that every suite defined in the sources of a test executable is added to it: a suite that
# NAOS_SUITE defines and no NAOS_ADD_SUITE adds never runs, and nothing else reports it, since its
# function has external linkage. The suites defined are found in the sources, those added in the
# names "<exe> --naos-list" prints.
#
# Included by NaosDiscover.cmake, for naos_add_executable() and naos_discover_tests(). Also a script
# of its own, for a build that uses neither:
#
#   cmake -D NAOS_EXECUTABLE=<exe> -D "NAOS_SOURCES=<file>;..." [-D "NAOS_IGNORE_SUITES=<name>;..."]
#         -P NaosCheckSuites.cmake
#
# Relative paths of NAOS_SOURCES are taken from the current directory. The script exits with 0 when
# every suite is added, and fails with one "<file>:<line>: error: " line per suite that is not.
#
# The sources are read as C or C++ text: comments, preprocessor directives and what #if, #ifdef or
# #ifndef enclose are left out, and every other "NAOS_SUITE(<name>" counts. Whether a suite inside a
# conditional is built depends on its condition, which only the compiler knows, so it is not
# checked; nor is a suite defined through a macro of the project's own, which is not found.

cmake_minimum_required(VERSION 3.21)

include_guard(GLOBAL)

# Sets out to content with its comments, its preprocessor directives and what its conditionals
# enclose replaced by spaces, the line breaks kept, so that a position in it has the line it has in content. ';', '[' and ']', which
# CMake lists treat apart, become spaces too: no suite name holds them. A backslash, which would
# escape the ';' between two lines, becomes the character 0x01.
function(naos_suite_code out content)
    string(ASCII 1 backslash)
    string(REPLACE "\r" "" content "${content}")
    string(REPLACE "\\" "${backslash}" content "${content}")
    string(REPLACE ";" " " content "${content}")
    string(REPLACE "[" " " content "${content}")
    string(REPLACE "]" " " content "${content}")
    # Each line starts with '_', so that an empty line is still an item of the list.
    string(REPLACE "\n" ";_" lines "_${content}")

    set(code "")
    set(in_comment FALSE)
    set(in_directive FALSE)
    set(depth 0)
    foreach(line IN LISTS lines)
        string(SUBSTRING "${line}" 1 -1 line)
        set(rest "${line}")
        set(kept "")
        while(NOT rest STREQUAL "")
            if(in_comment)
                string(FIND "${rest}" "*/" end)
                if(end EQUAL -1)
                    set(rest "")
                else()
                    math(EXPR end "${end} + 2")
                    string(SUBSTRING "${rest}" ${end} -1 rest)
                    string(APPEND kept " ")
                    set(in_comment FALSE)
                endif()
            else()
                string(FIND "${rest}" "/*" block)
                string(FIND "${rest}" "//" to_end)
                if((to_end GREATER -1) AND ((block EQUAL -1) OR (to_end LESS block)))
                    string(SUBSTRING "${rest}" 0 ${to_end} before)
                    string(APPEND kept "${before}")
                    set(rest "")
                elseif(block GREATER -1)
                    string(SUBSTRING "${rest}" 0 ${block} before)
                    string(APPEND kept "${before} ")
                    math(EXPR block "${block} + 2")
                    string(SUBSTRING "${rest}" ${block} -1 rest)
                    set(in_comment TRUE)
                else()
                    string(APPEND kept "${rest}")
                    set(rest "")
                endif()
            endif()
        endwhile()

        # A directive goes on to the next line while its line ends with a backslash.
        if(in_directive OR (kept MATCHES "^[ \t]*#"))
            if(NOT in_directive)
                if(kept MATCHES "^[ \t]*#[ \t]*if")
                    math(EXPR depth "${depth} + 1")
                elseif((kept MATCHES "^[ \t]*#[ \t]*endif") AND (depth GREATER 0))
                    math(EXPR depth "${depth} - 1")
                endif()
            endif()
            set(kept "")
            if(line MATCHES "${backslash}[ \t]*$")
                set(in_directive TRUE)
            else()
                set(in_directive FALSE)
            endif()
        elseif(depth GREATER 0)
            set(kept "")
        endif()
        string(APPEND code "${kept}\n")
    endforeach()
    set(${out} "${code}" PARENT_SCOPE)
endfunction()

# Appends to the list variables names_var and places_var the suites that file defines, and where:
# their names, and "<file>:<line>".
function(naos_find_suites names_var places_var file)
    file(READ "${file}" content)
    naos_suite_code(code "${content}")
    set(names "${${names_var}}")
    set(places "${${places_var}}")
    set(line 1)
    while(TRUE)
        string(FIND "${code}" "NAOS_SUITE" at)
        if(at EQUAL -1)
            break()
        endif()
        string(SUBSTRING "${code}" 0 ${at} before)
        math(EXPR at "${at} + 10")
        string(SUBSTRING "${code}" ${at} -1 code)
        string(REGEX MATCHALL "\n" breaks "${before}")
        list(LENGTH breaks break_count)
        math(EXPR line "${line} + ${break_count}")
        # NAOS_SUITE as a word, followed by "(" and a name: not NAOS_SUITE_FUNC_ or MY_NAOS_SUITE.
        if(before MATCHES "[A-Za-z0-9_]$")
            continue()
        endif()
        if(code MATCHES "^[ \t\n]*\\([ \t\n]*([A-Za-z0-9_]+)")
            list(APPEND names "${CMAKE_MATCH_1}")
            list(APPEND places "${file}:${line}")
        endif()
    endwhile()
    set(${names_var} "${names}" PARENT_SCOPE)
    set(${places_var} "${places}" PARENT_SCOPE)
endfunction()

# Whether path is a C or C++ source or header, by its extension; the others, such as objects, are
# not read.
function(naos_is_source out path)
    get_filename_component(extension "${path}" LAST_EXT)
    string(TOLOWER "${extension}" extension)
    if(extension MATCHES "^\\.(c|cc|cp|cpp|cxx|c\\+\\+|h|hh|hpp|hxx|h\\+\\+|inl|ipp|tpp|m|mm)$")
        set(${out} TRUE PARENT_SCOPE)
    else()
        set(${out} FALSE PARENT_SCOPE)
    endif()
endfunction()

# Fails, naming each, when a suite defined in sources is added neither to the executable named
# target, whose tests are the names <suite>.<test> of tests, nor named in ignored. Relative sources
# are taken from base_dir; sources that are not C or C++, or not found, are left out. ignore_how
# says how to name a suite to ignore, for the messages.
function(naos_check_suites target tests sources ignored base_dir ignore_how)
    set(added "")
    foreach(test IN LISTS tests)
        string(REGEX REPLACE "\\..*" "" suite "${test}")
        list(APPEND added "${suite}")
    endforeach()

    set(names "")
    set(places "")
    set(read "")
    foreach(source IN LISTS sources)
        get_filename_component(path "${source}" ABSOLUTE BASE_DIR "${base_dir}")
        naos_is_source(is_source "${path}")
        if((NOT is_source) OR (NOT EXISTS "${path}") OR (IS_DIRECTORY "${path}") OR ("${path}" IN_LIST read))
            continue()
        endif()
        list(APPEND read "${path}")
        naos_find_suites(names places "${path}")
    endforeach()

    set(missing 0)
    set(index 0)
    foreach(name IN LISTS names)
        list(GET places ${index} place)
        math(EXPR index "${index} + 1")
        if(("${name}" IN_LIST added) OR ("${name}" IN_LIST ignored))
            continue()
        endif()
        message("${place}: error: suite ${name} is defined but never added, so its tests never run: "
            "add it with NAOS_ADD_SUITE(${name}), or ${ignore_how} when it is left out on purpose")
        math(EXPR missing "${missing} + 1")
    endforeach()
    if(missing GREATER 0)
        message(FATAL_ERROR "${missing} suite(s) defined in the sources of ${target} are not added to it; see above.")
    endif()
endfunction()

# Run as a script: lists the tests of NAOS_EXECUTABLE and checks NAOS_SOURCES against them.
if(CMAKE_SCRIPT_MODE_FILE STREQUAL CMAKE_CURRENT_LIST_FILE)
    foreach(name IN ITEMS NAOS_EXECUTABLE NAOS_SOURCES)
        if(NOT DEFINED ${name})
            message(FATAL_ERROR "NaosCheckSuites.cmake: ${name} is not set")
        endif()
    endforeach()
    execute_process(
        COMMAND "${NAOS_EXECUTABLE}" --naos-list
        OUTPUT_VARIABLE list_output
        ERROR_VARIABLE list_error
        RESULT_VARIABLE list_result
    )
    if(NOT list_result EQUAL 0)
        message(FATAL_ERROR "Failed to list the tests of ${NAOS_EXECUTABLE} (result: ${list_result})\n${list_output}${list_error}")
    endif()
    string(REPLACE "\r" "" list_output "${list_output}")
    string(REPLACE "\n" ";" list_tests "${list_output}")
    list(REMOVE_ITEM list_tests "")
    naos_check_suites("${NAOS_EXECUTABLE}" "${list_tests}" "${NAOS_SOURCES}" "${NAOS_IGNORE_SUITES}"
        "${CMAKE_CURRENT_SOURCE_DIR}" "name it in NAOS_IGNORE_SUITES")
endif()
