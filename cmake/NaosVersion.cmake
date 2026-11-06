# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# Used by the build of Naos only; not installed. Also run as a script by scripts/release.py,
# so that the version is read one way only:
#
#   cmake -D HEADER=<header> -P cmake/NaosVersion.cmake   prints MAJOR.MINOR.PATCH
#
# naos_read_version(<header>)
#   Reads the version from <header> (include/naos/version.h, its only home) and sets, in the scope
#   of the caller, everything that follows from it:
#     naos_version        MAJOR.MINOR.PATCH, for project(VERSION)
#     naos_compatibility  COMPATIBILITY of the package version file
#     naos_soversion      SOVERSION of the shared library
#   Before 1.0 a minor version may break compatibility, so both the package and the shared library
#   stay compatible within one minor version; from 1.0 on, within one major version.

include_guard(GLOBAL)

function(naos_read_version header)
    file(STRINGS "${header}" lines REGEX "^#define NAOS_VERSION_(MAJOR|MINOR|PATCH) [0-9]+$")

    foreach(component IN ITEMS MAJOR MINOR PATCH)
        set(${component} "")
        foreach(line IN LISTS lines)
            if(line MATCHES "^#define NAOS_VERSION_${component} ([0-9]+)$")
                set(${component} "${CMAKE_MATCH_1}")
            endif()
        endforeach()
        if((${component} STREQUAL "") OR (${component} GREATER_EQUAL 256))
            message(FATAL_ERROR "${header}: NAOS_VERSION_${component} is missing or not < 256")
        endif()
    endforeach()

    if(MAJOR EQUAL 0)
        set(compatibility SameMinorVersion)
        set(soversion "${MAJOR}.${MINOR}")
    else()
        set(compatibility SameMajorVersion)
        set(soversion "${MAJOR}")
    endif()

    set(naos_version "${MAJOR}.${MINOR}.${PATCH}" PARENT_SCOPE)
    set(naos_compatibility "${compatibility}" PARENT_SCOPE)
    set(naos_soversion "${soversion}" PARENT_SCOPE)
endfunction()

if(CMAKE_SCRIPT_MODE_FILE STREQUAL CMAKE_CURRENT_LIST_FILE)
    naos_read_version("${HEADER}")
    execute_process(COMMAND "${CMAKE_COMMAND}" -E echo "${naos_version}")
endif()
