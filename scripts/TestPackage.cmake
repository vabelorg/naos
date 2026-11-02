# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# Tests the package that a build of Naos installs: installs the build, then configures, builds and
# tests tests/consumer/, which finds it with find_package(Naos <version>). CI runs it after the self
# tests of each build; run it from the root of Naos, after building a preset:
#
#   cmake -D PRESET=default -P scripts/TestPackage.cmake
#
# -D PRESET=<name>   the preset built, whose build directory is build/<name>
# -D CONFIG=<name>   the configuration built (optional): Debug, or Release for the preset release
#
# The package goes to build/<name>-install and the project to build/<name>-consumer. The compiler is
# the one of CC and the generator the one of CMAKE_GENERATOR, as for any configure.
#
# The warnings for the authors of a project (CMake Warning (dev)) are errors in the configure of the
# project, since such a warning from the installed package would show in every project using it.
# CMake 4.4 renamed -Werror=dev to -Werror=author, and prints that -Werror=dev is deprecated, while an
# older CMake ignores -Werror=author without a message; so the option depends on the version, and
# the script checks first that it does turn such a warning into an error (fixtures/author-warning).

cmake_minimum_required(VERSION 3.21)

if(NOT PRESET)
    message(FATAL_ERROR "PRESET is not set: cmake -D PRESET=<name> -P scripts/TestPackage.cmake")
endif()
if(NOT CONFIG)
    if(PRESET STREQUAL "release")
        set(CONFIG Release)
    else()
        set(CONFIG Debug)
    endif()
endif()

get_filename_component(root "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
set(build "${root}/build/${PRESET}")
set(prefix "${root}/build/${PRESET}-install")
set(consumer "${root}/build/${PRESET}-consumer")
if(NOT EXISTS "${build}/CMakeCache.txt")
    message(FATAL_ERROR "${build} is not a build directory: build the preset ${PRESET} first")
endif()

# Runs a command, failing with its output unless it exits with 0.
function(run what)
    message(STATUS "${what}")
    execute_process(COMMAND ${ARGN} RESULT_VARIABLE code)
    if(NOT code EQUAL 0)
        message(FATAL_ERROR "${what} failed (${code})")
    endif()
endfunction()

# The option that turns the warnings for the authors into errors with this CMake, checked.
if(CMAKE_VERSION VERSION_LESS 4.4)
    set(werror -Werror=dev)
else()
    set(werror -Werror=author)
endif()
set(check "${root}/build/${PRESET}-werror")
file(REMOVE_RECURSE "${check}")
execute_process(
    COMMAND "${CMAKE_COMMAND}" -S "${CMAKE_CURRENT_LIST_DIR}/fixtures/author-warning" -B "${check}" ${werror}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE output
)
if(result EQUAL 0 OR NOT output MATCHES "CMake Error \\((dev|author)\\)")
    message(FATAL_ERROR "${werror} does not turn a warning for the authors into an error with CMake ${CMAKE_VERSION}:\n${output}")
endif()

file(REMOVE_RECURSE "${prefix}" "${consumer}")
run("Installing ${build} to ${prefix}"
    "${CMAKE_COMMAND}" --install "${build}" --config "${CONFIG}" --prefix "${prefix}")
run("Configuring tests/consumer with the package"
    "${CMAKE_COMMAND}" -S "${root}/tests/consumer" -B "${consumer}" ${werror}
    "-DCMAKE_PREFIX_PATH=${prefix}" "-DCMAKE_BUILD_TYPE=${CONFIG}")
run("Building tests/consumer"
    "${CMAKE_COMMAND}" --build "${consumer}" --config "${CONFIG}")
run("Testing tests/consumer"
    "${CMAKE_CTEST_COMMAND}" --test-dir "${consumer}" -C "${CONFIG}" --output-on-failure)
