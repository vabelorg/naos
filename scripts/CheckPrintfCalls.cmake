# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# Checks that the sources of src/ call the formatted output functions through src/format.h
# (naos_printf, naos_snprintf, ...), never printf, snprintf and the others directly: those are not
# C99 on every platform (src/format.h tells why). Comments are not checked. Needs no build: run by
# the lint job of CI, and by hand with  cmake -D ROOT=. -P scripts/CheckPrintfCalls.cmake
#
# -D ROOT=<path>   root of Naos

cmake_minimum_required(VERSION 3.21)

if(NOT DEFINED ROOT)
    message(FATAL_ERROR "CheckPrintfCalls.cmake: -D ROOT=<path> is required")
endif()
# A relative ROOT, such as ".", is taken from the current directory.
get_filename_component(ROOT "${ROOT}" ABSOLUTE)

file(GLOB files LIST_DIRECTORIES false "${ROOT}/src/*.c" "${ROOT}/src/*.h")
list(REMOVE_ITEM files "${ROOT}/src/format.h")
list(SORT files)

set(call_regex "(^|[^A-Za-z0-9_])(printf|fprintf|vprintf|vfprintf|sprintf|vsprintf|snprintf|vsnprintf)[ \t]*\\(")

set(errors "")
foreach(file IN LISTS files)
    # One element per line: ';' and the brackets, which lists treat apart, made ',' and '(' ')'.
    file(READ "${file}" text)
    string(REPLACE "\r" "" text "${text}")
    string(REPLACE ";" "," text "${text}")
    string(REPLACE "[" "(" text "${text}")
    string(REPLACE "]" ")" text "${text}")
    string(REPLACE "\n" ";" lines "${text}")
    file(RELATIVE_PATH shown "${ROOT}" "${file}")
    set(in_comment FALSE)
    set(number 0)
    foreach(line IN LISTS lines)
        math(EXPR number "${number} + 1")
        # The code of the line, without its comments: /* ... */ may span lines, // ends the line.
        set(code "")
        set(rest "${line}")
        while(NOT rest STREQUAL "")
            if(in_comment)
                string(FIND "${rest}" "*/" end)
                if(end EQUAL -1)
                    set(rest "")
                else()
                    math(EXPR end "${end} + 2")
                    string(SUBSTRING "${rest}" ${end} -1 rest)
                    set(in_comment FALSE)
                endif()
            else()
                string(FIND "${rest}" "/*" begin)
                string(FIND "${rest}" "//" line_comment)
                if(NOT line_comment EQUAL -1 AND (begin EQUAL -1 OR line_comment LESS begin))
                    string(SUBSTRING "${rest}" 0 ${line_comment} part)
                    string(APPEND code "${part}")
                    set(rest "")
                elseif(begin EQUAL -1)
                    string(APPEND code "${rest}")
                    set(rest "")
                else()
                    string(SUBSTRING "${rest}" 0 ${begin} part)
                    string(APPEND code "${part} ")
                    math(EXPR begin "${begin} + 2")
                    string(SUBSTRING "${rest}" ${begin} -1 rest)
                    set(in_comment TRUE)
                endif()
            endif()
        endwhile()
        if(code MATCHES "${call_regex}")
            string(APPEND errors "${shown}:${number}: calls ${CMAKE_MATCH_2} directly; use naos_${CMAKE_MATCH_2} of format.h\n")
        endif()
    endforeach()
endforeach()

if(NOT errors STREQUAL "")
    message(FATAL_ERROR "formatted output must go through src/format.h:\n${errors}")
endif()
