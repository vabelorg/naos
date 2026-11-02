# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# Checks that every test and every suite defined in the sources of a test executable is added to
# it. A test that NAOS_TEST defines and no NAOS_ADD_TEST adds never runs, nor does a suite that
# NAOS_SUITE defines and no NAOS_ADD_SUITE adds; and the run still passes. What is defined is found
# in the sources, what is added in the names <suite>.<test> that "<exe> --naos-list" prints.
#
# Included by NaosDiscover.cmake, for naos_add_executable() and naos_discover_tests(). Also a script
# of its own, for a build that uses neither:
#
#   cmake -D NAOS_EXECUTABLE=<exe> -D "NAOS_SOURCES=<file>;..." [-D "NAOS_IGNORE_SUITES=<name>;..."]
#         -P NaosCheckRegistration.cmake
#
# Relative paths of NAOS_SOURCES are taken from the current directory. The script exits with 0 when
# everything defined is added, and fails with one "<file>:<line>: error: " line for each test or
# suite that is not.
#
# The sources are read as C or C++ text: comments, preprocessor directives and what #if, #ifdef or
# #ifndef enclose are left out, and every other "NAOS_SUITE(<name>" and "NAOS_TEST(<name>" counts.
# Whether a definition inside a conditional is built depends on its condition, which only the
# compiler knows, so it is not checked; nor is one made through a macro of the project's own, which
# is not found.
#
# A suite is added when the list holds one of its tests. A test is static, so only a suite of its
# own file can add it: it is added when the list holds <suite>.<test> for a suite of that file, or
# for a suite whose definition was not found (behind #if, or through a macro), which may be of that
# file. The tests of a file that defines a suite not added, or one ignored, are not checked: they
# may be in that suite. Setup and teardown are not listed, so not checked.

cmake_minimum_required(VERSION 3.21)

include_guard(GLOBAL)

# Sets out to content with its comments, its preprocessor directives and what its conditionals
# enclose replaced by spaces, the line breaks kept, so that a position in it has the line it has in
# content. ';', '[' and ']', which CMake lists treat apart, become spaces too: no name holds them. A backslash, which would
# escape the ';' between two lines, becomes the character 0x01.
function(naos_source_code out content)
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

# Appends to the list variables names_var and places_var what macro (NAOS_SUITE or NAOS_TEST)
# defines in code, the text of file that naos_source_code() keeps: the names, and "<file>:<line>".
function(naos_find_definitions names_var places_var file code macro)
    set(names "${${names_var}}")
    set(places "${${places_var}}")
    string(LENGTH "${macro}" macro_length)
    set(line 1)
    while(TRUE)
        string(FIND "${code}" "${macro}" at)
        if(at EQUAL -1)
            break()
        endif()
        string(SUBSTRING "${code}" 0 ${at} before)
        math(EXPR at "${at} + ${macro_length}")
        string(SUBSTRING "${code}" ${at} -1 code)
        string(REGEX MATCHALL "\n" breaks "${before}")
        list(LENGTH breaks break_count)
        math(EXPR line "${line} + ${break_count}")
        # The macro as a word, followed by "(" and a name: not NAOS_SUITE_FUNC_ or MY_NAOS_TEST.
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

# Appends to the list variables names_var and places_var the suites that file defines, and where:
# their names, and "<file>:<line>".
function(naos_find_suites names_var places_var file)
    file(READ "${file}" content)
    naos_source_code(code "${content}")
    naos_find_definitions(${names_var} ${places_var} "${file}" "${code}" NAOS_SUITE)
    set(${names_var} "${${names_var}}" PARENT_SCOPE)
    set(${places_var} "${${places_var}}" PARENT_SCOPE)
endfunction()

# Appends to the list variables names_var and places_var the tests that file defines, and where.
function(naos_find_tests names_var places_var file)
    file(READ "${file}" content)
    naos_source_code(code "${content}")
    naos_find_definitions(${names_var} ${places_var} "${file}" "${code}" NAOS_TEST)
    set(${names_var} "${${names_var}}" PARENT_SCOPE)
    set(${places_var} "${${places_var}}" PARENT_SCOPE)
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

# Fails, naming each, when a test or a suite defined in sources is not added to the executable
# named target, whose tests are the names <suite>.<test> of tests; a suite named in ignored, and the
# tests of its file, are left out. Relative sources are taken from base_dir; sources that are not C
# or C++, or not found, are left out. ignore_how says how to name a suite to ignore, for the
# messages.
function(naos_check_registration target tests sources ignored base_dir ignore_how)
    set(added "")
    foreach(test IN LISTS tests)
        string(REGEX REPLACE "\\..*" "" suite "${test}")
        list(APPEND added "${suite}")
    endforeach()

    # The files, and the suites found in all of them.
    set(files "")
    set(all_suites "")
    foreach(source IN LISTS sources)
        get_filename_component(path "${source}" ABSOLUTE BASE_DIR "${base_dir}")
        naos_is_source(is_source "${path}")
        if((NOT is_source) OR (NOT EXISTS "${path}") OR (IS_DIRECTORY "${path}") OR ("${path}" IN_LIST files))
            continue()
        endif()
        list(APPEND files "${path}")
        file(READ "${path}" content)
        naos_source_code(code "${content}")
        set(code_${path} "${code}")
        set(suite_places "")
        naos_find_definitions(all_suites suite_places "${path}" "${code}" NAOS_SUITE)
    endforeach()

    set(missing 0)
    foreach(path IN LISTS files)
        set(suites "")
        set(suite_places "")
        naos_find_definitions(suites suite_places "${path}" "${code_${path}}" NAOS_SUITE)
        set(check_tests TRUE)
        set(index 0)
        foreach(suite IN LISTS suites)
            list(GET suite_places ${index} place)
            math(EXPR index "${index} + 1")
            if("${suite}" IN_LIST ignored)
                set(check_tests FALSE)
            elseif(NOT "${suite}" IN_LIST added)
                message("${place}: error: suite ${suite} is defined but never added, so its tests never run: "
                    "add it with NAOS_ADD_SUITE(${suite}), or ${ignore_how} when it is left out on purpose")
                math(EXPR missing "${missing} + 1")
                set(check_tests FALSE)
            endif()
        endforeach()
        if(NOT check_tests)
            continue()
        endif()

        set(defined "")
        set(test_places "")
        naos_find_definitions(defined test_places "${path}" "${code_${path}}" NAOS_TEST)
        set(index 0)
        foreach(name IN LISTS defined)
            list(GET test_places ${index} place)
            math(EXPR index "${index} + 1")
            # Added by a suite of this file, or by one that may be of it.
            set(found FALSE)
            foreach(test IN LISTS tests)
                if(NOT test MATCHES "^([A-Za-z0-9_]+)\\.${name}$")
                    continue()
                endif()
                set(suite "${CMAKE_MATCH_1}")
                if(("${suite}" IN_LIST suites) OR (NOT "${suite}" IN_LIST all_suites))
                    set(found TRUE)
                    break()
                endif()
            endforeach()
            if(NOT found)
                message("${place}: error: test ${name} is defined but never added, so it never runs: "
                    "add it with NAOS_ADD_TEST(${name}) to a suite of this file")
                math(EXPR missing "${missing} + 1")
            endif()
        endforeach()
    endforeach()
    if(missing GREATER 0)
        message(FATAL_ERROR "${missing} test(s) or suite(s) defined in the sources of ${target} are not added to it; see above.")
    endif()
endfunction()

# Run as a script: lists the tests of NAOS_EXECUTABLE and checks NAOS_SOURCES against them.
if(CMAKE_SCRIPT_MODE_FILE STREQUAL CMAKE_CURRENT_LIST_FILE)
    foreach(name IN ITEMS NAOS_EXECUTABLE NAOS_SOURCES)
        if(NOT DEFINED ${name})
            message(FATAL_ERROR "NaosCheckRegistration.cmake: ${name} is not set")
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
    naos_check_registration("${NAOS_EXECUTABLE}" "${list_tests}" "${NAOS_SOURCES}" "${NAOS_IGNORE_SUITES}"
        "${CMAKE_CURRENT_SOURCE_DIR}" "name it in NAOS_IGNORE_SUITES")
endif()
