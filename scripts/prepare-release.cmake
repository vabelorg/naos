# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# Prepares the release of a version, as docs/development/release.md describes: checks that the
# entries of [Unreleased] in CHANGELOG.md allow the version, sets it in include/naos/version.h,
# updates the tag and the version named in the documentation and the examples for a new minor or
# major version, and closes the changelog. Run by .github/workflows/release-pr.yml; changes the
# files only, commits nothing.
#
# -D ROOT=<path>            root of Naos
# -D VERSION=<x.y.z>        the version to release
# -D REPO_URL=<url>         optional, the repository the changelog links to; by default the one
#                           README.md fetches with FetchContent
# -D DATE=<YYYY-MM-DD>      optional, the date of the release; by default today (UTC)

cmake_minimum_required(VERSION 3.21)

foreach(var IN ITEMS ROOT VERSION)
    if(NOT DEFINED ${var})
        message(FATAL_ERROR "prepare-release.cmake: -D ${var}=... is required")
    endif()
endforeach()

include("${ROOT}/cmake/NaosVersion.cmake")

set(version_header "${ROOT}/include/naos/version.h")
set(changelog "${ROOT}/CHANGELOG.md")

# Files naming the tag fetched by FetchContent or the minor version asked of find_package(). The
# pages of docs/development/ are left alone: their versions are examples.
file(GLOB example_lists "${ROOT}/examples/*/CMakeLists.txt")
set(versioned_files
    "${ROOT}/README.md"
    "${ROOT}/README.ja.md"
    ${example_lists}
)
foreach(lang IN ITEMS "" "ja/")
    foreach(page IN ITEMS getting-started/installation getting-started/quickstart reference/cmake
            guides/troubleshooting)
        list(APPEND versioned_files "${ROOT}/docs/${lang}${page}.md")
    endforeach()
endforeach()

# Sets out to the content of file with LF line endings.
function(naos_read_text out file)
    file(READ "${file}" text)
    string(REPLACE "\r" "" text "${text}")
    set(${out} "${text}" PARENT_SCOPE)
endfunction()

# --- The version ---

if(NOT VERSION MATCHES "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$")
    message(FATAL_ERROR "VERSION '${VERSION}' is not MAJOR.MINOR.PATCH")
endif()
set(new_major "${CMAKE_MATCH_1}")
set(new_minor "${CMAKE_MATCH_2}")
set(new_patch "${CMAKE_MATCH_3}")
foreach(component IN ITEMS new_major new_minor new_patch)
    if(${component} GREATER_EQUAL 256)
        message(FATAL_ERROR "VERSION '${VERSION}': each component must be < 256")
    endif()
endforeach()
# Without leading zeros, as the tag and the macros have it.
set(VERSION "${new_major}.${new_minor}.${new_patch}")

naos_read_version("${version_header}")
set(old_version "${naos_version}")
if(NOT VERSION VERSION_GREATER old_version)
    message(FATAL_ERROR "VERSION ${VERSION} is not greater than ${old_version}, the version in "
        "include/naos/version.h")
endif()
string(REGEX MATCH "^([0-9]+)\\.[0-9]+" old_minor_version "${old_version}")
set(old_major "${CMAKE_MATCH_1}")
set(new_minor_version "${new_major}.${new_minor}")

if(NOT new_major EQUAL old_major)
    set(kind major)
elseif(NOT new_minor_version STREQUAL old_minor_version)
    set(kind minor)
else()
    set(kind patch)
endif()

# --- The changelog ---

naos_read_text(log "${changelog}")

set(unreleased_heading "## [Unreleased]\n")
string(FIND "${log}" "${unreleased_heading}" unreleased_at)
if(unreleased_at EQUAL -1)
    message(FATAL_ERROR "CHANGELOG.md has no '## [Unreleased]' heading")
endif()
string(LENGTH "${unreleased_heading}" heading_length)
math(EXPR body_at "${unreleased_at} + ${heading_length}")
string(SUBSTRING "${log}" ${body_at} -1 rest)

# The section ends at the next version heading, or at the links at the end of the file.
string(LENGTH "${rest}" body_length)
foreach(end_marker IN ITEMS "\n## [" "\n[Unreleased]: ")
    string(FIND "${rest}" "${end_marker}" end_at)
    if(NOT end_at EQUAL -1 AND end_at LESS body_length)
        set(body_length ${end_at})
    endif()
endforeach()
string(SUBSTRING "${rest}" 0 ${body_length} unreleased)
string(SUBSTRING "${rest}" ${body_length} -1 after_unreleased)
string(STRIP "${unreleased}" unreleased)
if(unreleased STREQUAL "")
    message(FATAL_ERROR "[Unreleased] in CHANGELOG.md is empty: record the changes first")
endif()

# docs/development/changelog.md: a patch version has Fixed and Security entries only, and a BREAKING
# entry needs a minor version before 1.0 and a major version from 1.0 on.
string(REGEX MATCHALL "(^|\n)### [^\n]*" sections "${unreleased}")
string(FIND "${unreleased}" "**BREAKING**" breaking_at)
if(kind STREQUAL "patch")
    foreach(section IN LISTS sections)
        string(STRIP "${section}" section)
        if(NOT section MATCHES "^### (Fixed|Security)$")
            message(FATAL_ERROR "[Unreleased] has '${section}': ${VERSION} is a patch version, "
                "which can have Fixed and Security entries only")
        endif()
    endforeach()
elseif(NOT breaking_at EQUAL -1 AND kind STREQUAL "minor" AND new_major GREATER 0)
    message(FATAL_ERROR "[Unreleased] has a BREAKING entry: from 1.0 on it needs a major version")
endif()

if(REPO_URL)
    string(REGEX REPLACE "/$" "" repo_url "${REPO_URL}")
else()
    naos_read_text(readme "${ROOT}/README.md")
    if(NOT readme MATCHES "GIT_REPOSITORY (https://[^ \n]+)\\.git")
        message(FATAL_ERROR "README.md has no GIT_REPOSITORY to link to; give -D REPO_URL=<url>")
    endif()
    set(repo_url "${CMAKE_MATCH_1}")
endif()

if(NOT DEFINED DATE)
    string(TIMESTAMP DATE "%Y-%m-%d" UTC)
endif()

# The version released before, if any: the first version heading after [Unreleased].
if(after_unreleased MATCHES "\n## \\[([0-9]+\\.[0-9]+\\.[0-9]+)\\]")
    set(previous_link "${repo_url}/compare/v${CMAKE_MATCH_1}...v${VERSION}")
else()
    set(previous_link "${repo_url}/releases/tag/v${VERSION}")
endif()

string(SUBSTRING "${log}" 0 ${unreleased_at} before)
set(log "${before}${unreleased_heading}\n## [${VERSION}] - ${DATE}\n\n${unreleased}\n${after_unreleased}")

set(links "[Unreleased]: ${repo_url}/compare/v${VERSION}...HEAD\n[${VERSION}]: ${previous_link}\n")
if(log MATCHES "\n\\[Unreleased\\]: [^\n]*\n")
    string(REPLACE "${CMAKE_MATCH_0}" "\n${links}" log "${log}")
else()
    string(REGEX REPLACE "\n+$" "" log "${log}")
    string(APPEND log "\n\n${links}")
endif()
file(WRITE "${changelog}" "${log}")

# --- include/naos/version.h ---

naos_read_text(header "${version_header}")
foreach(component IN ITEMS MAJOR MINOR PATCH)
    string(TOLOWER "${component}" lower)
    string(REGEX REPLACE "\n#define NAOS_VERSION_${component} [0-9]+\n"
        "\n#define NAOS_VERSION_${component} ${new_${lower}}\n" header "${header}")
endforeach()
file(WRITE "${version_header}" "${header}")

# --- The documentation and the examples ---

if(NOT kind STREQUAL "patch")
    foreach(file IN LISTS versioned_files)
        naos_read_text(text "${file}")
        set(old_text "${text}")
        string(REGEX REPLACE "GIT_TAG v[0-9]+\\.[0-9]+\\.[0-9]+" "GIT_TAG v${VERSION}" text "${text}")
        string(REPLACE "find_package(naos ${old_minor_version}" "find_package(naos ${new_minor_version}"
            text "${text}")
        string(REPLACE "${old_minor_version}.x" "${new_minor_version}.x" text "${text}")
        if(NOT text STREQUAL old_text)
            file(WRITE "${file}" "${text}")
        endif()
    endforeach()
endif()

if(kind STREQUAL "major" AND old_major EQUAL 0)
    message(WARNING "${VERSION} is the first version from 1.0 on: find_package() now accepts the "
        "same major version, not the same minor one. Update the text that says so in README.md, "
        "README.ja.md and docs/ by hand.")
endif()

message(STATUS "Prepared ${VERSION} (${kind}, from ${old_version})")
