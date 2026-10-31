# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# Writes the notes of a release: the section of its version in CHANGELOG.md, without the heading.
# Used by .github/workflows/release-pr.yml and release.yml.
#
# -D ROOT=<path>       root of Naos
# -D VERSION=<x.y.z>   the version released
# -D OUTPUT=<file>     the file to write

cmake_minimum_required(VERSION 3.21)

foreach(var IN ITEMS ROOT VERSION OUTPUT)
    if(NOT DEFINED ${var})
        message(FATAL_ERROR "release-notes.cmake: -D ${var}=... is required")
    endif()
endforeach()

file(READ "${ROOT}/CHANGELOG.md" log)
string(REPLACE "\r" "" log "${log}")

string(REPLACE "." "\\." version_pattern "${VERSION}")
if(NOT log MATCHES "\n## \\[${version_pattern}\\][^\n]*\n")
    message(FATAL_ERROR "CHANGELOG.md has no section for ${VERSION}")
endif()
string(FIND "${log}" "${CMAKE_MATCH_0}" heading_at)
string(LENGTH "${CMAKE_MATCH_0}" heading_length)
math(EXPR body_at "${heading_at} + ${heading_length}")
string(SUBSTRING "${log}" ${body_at} -1 rest)

# The section ends at the next version heading, or at the links at the end of the file.
string(LENGTH "${rest}" body_length)
foreach(end_marker IN ITEMS "\n## [" "\n[Unreleased]: ")
    string(FIND "${rest}" "${end_marker}" end_at)
    if(NOT end_at EQUAL -1 AND end_at LESS body_length)
        set(body_length ${end_at})
    endif()
endforeach()
string(SUBSTRING "${rest}" 0 ${body_length} notes)
string(STRIP "${notes}" notes)
if(notes STREQUAL "")
    message(FATAL_ERROR "The section of ${VERSION} in CHANGELOG.md is empty")
endif()

file(WRITE "${OUTPUT}" "${notes}\n")
