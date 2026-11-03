# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# Tests the package that a build of Naos installs: installs the build, then configures, builds and
# tests tests/consumer/, which finds it with find_package(Naos <version>), then builds and runs
# tests/consumer/consumer_test.c again with the flags of naos.pc from pkg-config, where it is found.
# CI runs it after the self tests of each build; run it from the root of Naos, after building a preset:
#
#   cmake -D PRESET=default -P scripts/TestPackage.cmake
#
# -D PRESET=<name>   the preset built, whose build directory is build/<name>
# -D CONFIG=<name>   the configuration built (optional): Debug, or Release for the preset release
#
# The package goes to build/<name>-install, the project to build/<name>-consumer and the program
# built with pkg-config to build/<name>-pkgconfig. The compiler is the one of CC and the generator
# the one of CMAKE_GENERATOR, as for any configure; the program is built by the compiler of the build.
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

# The same package through pkg-config, as a project with another build system uses it
# (docs/getting-started/installation.md#other-build-systems): <libdir>/pkgconfig/naos.pc is
# generated, so its flags are checked against the build, then tests/consumer/consumer_test.c is built
# with them by the compiler of the build, without CMake, and run. Skipped where pkg-config is not
# found, and with MSVC, whose cl takes no -L or -l.
function(cache_value name variable)
    file(STRINGS "${build}/CMakeCache.txt" line REGEX "^${name}:[A-Z]+=")
    string(REGEX REPLACE "^${name}:[A-Z]+=" "" value "${line}")
    set(${variable} "${value}" PARENT_SCOPE)
endfunction()

# Runs pkg-config with the arguments given, setting <variable> to its output.
function(pkg_config variable)
    execute_process(COMMAND "${pkg_config}" ${ARGN} naos
        RESULT_VARIABLE code OUTPUT_VARIABLE output ERROR_VARIABLE output OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(NOT code EQUAL 0)
        message(FATAL_ERROR "pkg-config ${ARGN} naos failed (${code}):\n${output}")
    endif()
    set(${variable} "${output}" PARENT_SCOPE)
endfunction()

# The compiler, and the kind of command line it takes (MSVC for cl and clang-cl), are not in the cache
# but in CMakeFiles/<version>/CMakeCCompiler.cmake. An older CMake leaves the kind empty for GCC.
cache_value(CMAKE_C_COMPILER cc)
file(GLOB compiler_files "${build}/CMakeFiles/*/CMakeCCompiler.cmake")
set(cc_id "")
set(frontend "")
foreach(compiler_file IN LISTS compiler_files)
    file(STRINGS "${compiler_file}" lines REGEX "^set\\(CMAKE_C_COMPILER_(ID|FRONTEND_VARIANT) \"[A-Za-z]*\"\\)$")
    foreach(line IN LISTS lines)
        if(line MATCHES "_ID \"([A-Za-z]*)\"")
            set(cc_id "${CMAKE_MATCH_1}")
        elseif(line MATCHES "_FRONTEND_VARIANT \"([A-Za-z]*)\"")
            set(frontend "${CMAKE_MATCH_1}")
        endif()
    endforeach()
endforeach()
if(cc_id STREQUAL "")
    message(FATAL_ERROR "The compiler of ${build} is not found in ${build}/CMakeFiles")
endif()
if(cc_id STREQUAL "MSVC" OR frontend STREQUAL "MSVC")
    message(STATUS "Testing naos.pc: skipped with a compiler taking the options of MSVC")
    return()
endif()
find_program(pkg_config NAMES pkgconf pkg-config)
if(NOT pkg_config)
    message(STATUS "Testing naos.pc: skipped, pkg-config is not found")
    return()
endif()

cache_value(CMAKE_INSTALL_LIBDIR libdir)
cache_value(CMAKE_INSTALL_BINDIR bindir)
cache_value(BUILD_SHARED_LIBS shared)
include("${root}/cmake/NaosVersion.cmake")
naos_read_version("${root}/include/naos/version.h")
set(pkgconfig "${root}/build/${PRESET}-pkgconfig")
file(REMOVE_RECURSE "${pkgconfig}")
file(MAKE_DIRECTORY "${pkgconfig}")
set(ENV{PKG_CONFIG_PATH} "${prefix}/${libdir}/pkgconfig")

message(STATUS "Testing naos.pc with ${pkg_config}")
pkg_config(version --modversion)
pkg_config(cflags --cflags)
pkg_config(libs --libs)
pkg_config(static_libs --static --libs)
if(NOT version STREQUAL naos_version)
    message(FATAL_ERROR "naos.pc has the version ${version}, not ${naos_version}")
endif()
if(CONFIG STREQUAL "Debug")
    set(library naosd)
else()
    set(library naos)
endif()
if(NOT libs MATCHES "(^| )-l${library}( |$)")
    message(FATAL_ERROR "naos.pc links '${libs}', without -l${library} for ${CONFIG}")
endif()
if(shared AND NOT cflags MATCHES "(^| )-DNAOS_SHARED( |$)")
    message(FATAL_ERROR "naos.pc gives '${cflags}', without -DNAOS_SHARED for the shared library")
elseif(NOT shared AND cflags MATCHES "NAOS_SHARED")
    message(FATAL_ERROR "naos.pc gives '${cflags}', with NAOS_SHARED for the static library")
endif()
if(NOT shared AND CMAKE_HOST_UNIX AND NOT static_libs MATCHES "(^| )-lm( |$)")
    message(FATAL_ERROR "naos.pc links '${static_libs}' with --static, without -lm")
endif()

separate_arguments(cflags UNIX_COMMAND "${cflags}")
separate_arguments(libs UNIX_COMMAND "${libs}")
set(program "${pkgconfig}/consumer_tests")
if(CMAKE_HOST_WIN32)
    string(APPEND program ".exe")
endif()
run("Building tests/consumer/consumer_test.c with the flags of pkg-config"
    "${cc}" -std=c99 ${cflags} "-DEXPECTED_VERSION=\"${naos_version}\""
    "${root}/tests/consumer/consumer_test.c" ${libs} -o "${program}")
# The shared library is found where it is installed.
if(CMAKE_HOST_WIN32)
    set(ENV{PATH} "${prefix}/${bindir};$ENV{PATH}")
elseif(CMAKE_HOST_APPLE)
    set(ENV{DYLD_LIBRARY_PATH} "${prefix}/${libdir}")
else()
    set(ENV{LD_LIBRARY_PATH} "${prefix}/${libdir}")
endif()
run("Running the tests built with the flags of pkg-config" "${program}")
