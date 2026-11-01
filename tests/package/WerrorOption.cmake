# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# Prints the option of this CMake that turns the warnings for the authors of a project
# (CMake Warning (dev)) into errors, after checking that it does: CI configures tests/package with
# it, since such a warning from the installed package would show in every project using it.
#
#   cmake -DBINARY_DIR=<dir> -P tests/package/WerrorOption.cmake
#
# CMake 4.4 renamed -Werror=dev to -Werror=author, and prints that -Werror=dev is deprecated. An
# older CMake ignores -Werror=author without a message, so the option depends on the version.
# BINARY_DIR is where the check configures tests/package/author-warning.

cmake_minimum_required(VERSION 3.21)

if(NOT BINARY_DIR)
    message(FATAL_ERROR "BINARY_DIR is not set")
endif()

if(CMAKE_VERSION VERSION_LESS 4.4)
    set(option -Werror=dev)
else()
    set(option -Werror=author)
endif()

# The check: the configure of a project with a single AUTHOR_WARNING must fail on that warning,
# and on nothing else.
file(REMOVE_RECURSE "${BINARY_DIR}")
execute_process(
    COMMAND "${CMAKE_COMMAND}" -S "${CMAKE_CURRENT_LIST_DIR}/author-warning" -B "${BINARY_DIR}" ${option}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE output
)
if(result EQUAL 0 OR NOT output MATCHES "CMake Error \\((dev|author)\\)")
    message(FATAL_ERROR "${option} does not turn a warning for the authors into an error with CMake ${CMAKE_VERSION}:\n${output}")
endif()

execute_process(COMMAND "${CMAKE_COMMAND}" -E echo "${option}")
