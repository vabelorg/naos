# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# Builds and runs the C tests of api/ without CMake, as docs/getting-started/installation.md describes
# (include/ on the include path, the .c files of src/ in the build): once with the sources given
# directly, once through a static library.
# GCC-compatible drivers only.
#
# -D CC=<path>          C compiler
# -D AR=<path>          archiver
# -D NAOS_DIR=<path>    root of Naos
# -D WORK_DIR=<path>    directory for the outputs, emptied first
# -D EXE_SUFFIX=<s>     suffix of executables (optional)

cmake_minimum_required(VERSION 3.21)

set(flags -std=c99 -Wall -Wextra -Wpedantic -Werror "-I${NAOS_DIR}/include")
file(GLOB tests "${NAOS_DIR}/tests/api/*.c")
# Every .c file of src/, as installation.md says.
file(GLOB sources "${NAOS_DIR}/src/*.c")

file(REMOVE_RECURSE "${WORK_DIR}")
file(MAKE_DIRECTORY "${WORK_DIR}/naos-obj")

# Runs a command in dir.
function(run_in dir)
    execute_process(
        COMMAND ${ARGN}
        WORKING_DIRECTORY "${dir}"
        OUTPUT_VARIABLE output
        ERROR_VARIABLE output
        RESULT_VARIABLE code
        TIMEOUT 120
    )
    if(NOT code STREQUAL "0")
        string(REPLACE ";" " " command "${ARGN}")
        message(FATAL_ERROR "failed (${code}): ${command}\n--- output ---\n${output}")
    endif()
endfunction()

function(run)
    run_in("${WORK_DIR}" ${ARGN})
endfunction()

# The sources added to the build directly.
run("${CC}" ${flags} -o "direct${EXE_SUFFIX}" ${tests} ${sources} -lm)
run("${WORK_DIR}/direct${EXE_SUFFIX}")

# A static library linked to the tests, from objects kept in a directory of their own.
run_in("${WORK_DIR}/naos-obj" "${CC}" ${flags} -c ${sources})
file(GLOB objects "${WORK_DIR}/naos-obj/*.o")
run("${AR}" rcs libnaos.a ${objects})
run("${CC}" ${flags} -o "static${EXE_SUFFIX}" ${tests} -L. -lnaos -lm)
run("${WORK_DIR}/static${EXE_SUFFIX}")
