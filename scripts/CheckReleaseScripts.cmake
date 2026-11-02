# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# Tests the scripts that prepare a release (docs/development/release.md): PrepareRelease.cmake and
# ReleaseNotes.cmake, run on a small copy of the files they read and change, made in a directory of
# the build directory. Run from the root of Naos, before any build; scripts/Lint.cmake runs it:
#
#   cmake -D ROOT=. -P scripts/CheckReleaseScripts.cmake
#
# Each case writes the files, runs a script with a version, and compares what it changed, or the
# message with which it refused the version.

cmake_minimum_required(VERSION 3.21)

if(NOT DEFINED ROOT)
    message(FATAL_ERROR "CheckReleaseScripts.cmake: -D ROOT=<root of Naos> is required")
endif()
get_filename_component(ROOT "${ROOT}" ABSOLUTE)
set(work "${ROOT}/build/check-release-scripts")
set(url "https://github.com/example/naos")
set(failures 0)

# The pages PrepareRelease.cmake updates for a new minor or major version.
set(versioned_pages README.md README.ja.md examples/quickstart/CMakeLists.txt)
foreach(lang IN ITEMS "" "ja/")
    foreach(page IN ITEMS getting-started/installation getting-started/quickstart reference/cmake
            guides/troubleshooting)
        list(APPEND versioned_pages "docs/${lang}${page}.md")
    endforeach()
endforeach()

# Writes the files of a repository at version, whose [Unreleased] holds unreleased.
function(write_repository version unreleased)
    file(REMOVE_RECURSE "${work}")
    file(COPY "${ROOT}/cmake/NaosVersion.cmake" DESTINATION "${work}/cmake")
    string(REPLACE "." ";" parts "${version}")
    list(GET parts 0 major)
    list(GET parts 1 minor)
    list(GET parts 2 patch)
    file(WRITE "${work}/include/naos/version.h"
        "#ifndef NAOS_VERSION_H\n#define NAOS_VERSION_MAJOR ${major}\n#define NAOS_VERSION_MINOR ${minor}\n"
        "#define NAOS_VERSION_PATCH ${patch}\n#endif\n")
    foreach(page IN LISTS versioned_pages)
        file(WRITE "${work}/${page}"
            "GIT_REPOSITORY ${url}.git\nGIT_TAG v${version}\nfind_package(Naos ${major}.${minor} REQUIRED)\n"
            "accepts ${major}.${minor}.x only\n")
    endforeach()
    file(WRITE "${work}/CHANGELOG.md"
        "# Changelog\n\n## [Unreleased]\n\n${unreleased}\n## [${version}] - 2026-01-01\n\n### Added\n\n- The start.\n\n"
        "[Unreleased]: ${url}/compare/v${version}...HEAD\n[${version}]: ${url}/releases/tag/v${version}\n")
endfunction()

# Runs a script of scripts/ on the copy with the defines, setting code and output.
function(run_script script)
    set(defines "")
    foreach(define IN LISTS ARGN)
        list(APPEND defines -D "${define}")
    endforeach()
    execute_process(COMMAND "${CMAKE_COMMAND}" -D "ROOT=${work}" ${defines} -P "${ROOT}/scripts/${script}"
        RESULT_VARIABLE run_code OUTPUT_VARIABLE run_output ERROR_VARIABLE run_output)
    set(code "${run_code}" PARENT_SCOPE)
    set(output "${run_output}" PARENT_SCOPE)
endfunction()

function(fail case message)
    message(NOTICE "FAIL ${case}: ${message}")
    math(EXPR count "${failures} + 1")
    set(failures ${count} PARENT_SCOPE)
endfunction()

# Fails case unless the file of the copy holds the rest of the arguments, joined.
function(expect_file case file)
    string(CONCAT expected ${ARGN})
    file(READ "${work}/${file}" actual)
    if(NOT actual STREQUAL expected)
        fail(${case} "${file} is\n${actual}\nnot\n${expected}")
        set(failures ${failures} PARENT_SCOPE)
    endif()
endfunction()

# Fails case unless the last script run refused, with a message matching regex.
macro(expect_refused case regex)
    if(code EQUAL 0 OR NOT output MATCHES "${regex}")
        fail(${case} "the version was not refused with '${regex}' (${code}):\n${output}")
    endif()
endmacro()

macro(expect_success case)
    if(NOT code EQUAL 0)
        fail(${case} "the script failed (${code}):\n${output}")
    endif()
endmacro()

set(fixed "### Fixed\n\n- A fix.\n")
set(added "### Added\n\n- A feature.\n")
set(breaking "### Changed\n\n- **BREAKING**: A change.\n")

# A patch version: the changelog closed and linked, the version set, the pages left alone.
write_repository(0.2.0 "${fixed}")
run_script(PrepareRelease.cmake VERSION=0.2.1 DATE=2026-02-03)
expect_success(patch)
expect_file(patch CHANGELOG.md
    "# Changelog\n\n## [Unreleased]\n\n## [0.2.1] - 2026-02-03\n\n### Fixed\n\n- A fix.\n\n## [0.2.0] - 2026-01-01\n\n"
    "### Added\n\n- The start.\n\n[Unreleased]: ${url}/compare/v0.2.1...HEAD\n[0.2.1]: ${url}/compare/v0.2.0...v0.2.1\n"
    "[0.2.0]: ${url}/releases/tag/v0.2.0\n")
expect_file(patch include/naos/version.h
    "#ifndef NAOS_VERSION_H\n#define NAOS_VERSION_MAJOR 0\n#define NAOS_VERSION_MINOR 2\n#define NAOS_VERSION_PATCH 1\n#endif\n")
expect_file(patch README.md
    "GIT_REPOSITORY ${url}.git\nGIT_TAG v0.2.0\nfind_package(Naos 0.2 REQUIRED)\naccepts 0.2.x only\n")

# The notes of that release: its section, without the heading.
run_script(ReleaseNotes.cmake VERSION=0.2.1 "OUTPUT=${work}/notes.md")
expect_success(notes)
expect_file(notes notes.md "### Fixed\n\n- A fix.\n")
run_script(ReleaseNotes.cmake VERSION=0.9.0 "OUTPUT=${work}/notes.md")
expect_refused(notes_of_no_version "CHANGELOG.md has no section for 0\\.9\\.0")

# A minor version: the tag and the minor version of every page, a leading zero dropped.
write_repository(0.2.1 "${added}")
run_script(PrepareRelease.cmake VERSION=0.03.0 DATE=2026-02-03)
expect_success(minor)
foreach(page IN LISTS versioned_pages)
    expect_file(minor "${page}"
        "GIT_REPOSITORY ${url}.git\nGIT_TAG v0.3.0\nfind_package(Naos 0.3 REQUIRED)\naccepts 0.3.x only\n")
endforeach()

# 1.0 changes what find_package() accepts, which the pages tell in words: a warning says so.
write_repository(0.3.0 "${breaking}")
run_script(PrepareRelease.cmake VERSION=1.0.0 DATE=2026-02-03)
expect_success(major)
if(NOT output MATCHES "first version from 1\\.0 on")
    fail(major "no warning about the text on compatibility:\n${output}")
endif()

# The versions refused, and why.
write_repository(0.2.0 "${added}")
run_script(PrepareRelease.cmake VERSION=0.2.1)
expect_refused(patch_with_added "0\\.2\\.1 is a patch version")
write_repository(0.2.0 "${fixed}")
run_script(PrepareRelease.cmake VERSION=0.2.0)
expect_refused(not_greater "is not greater than 0\\.2\\.0")
run_script(PrepareRelease.cmake VERSION=0.3)
expect_refused(not_three_numbers "is not MAJOR\\.MINOR\\.PATCH")
run_script(PrepareRelease.cmake VERSION=0.256.0)
expect_refused(component_too_large "each component must be < 256")
write_repository(1.0.0 "${breaking}")
run_script(PrepareRelease.cmake VERSION=1.1.0)
expect_refused(breaking_needs_major "from 1\\.0 on it needs a major version")
write_repository(0.2.0 "")
run_script(PrepareRelease.cmake VERSION=0.3.0)
expect_refused(nothing_to_release "is empty: record the changes first")
# A refused version changes no file.
expect_file(nothing_to_release include/naos/version.h
    "#ifndef NAOS_VERSION_H\n#define NAOS_VERSION_MAJOR 0\n#define NAOS_VERSION_MINOR 2\n#define NAOS_VERSION_PATCH 0\n#endif\n")

file(REMOVE_RECURSE "${work}")
if(failures GREATER 0)
    message(FATAL_ERROR "${failures} case(s) of the release scripts failed")
endif()
message(STATUS "The release scripts prepare and refuse versions as docs/development/release.md says")
