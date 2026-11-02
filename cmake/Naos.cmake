# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# CMake helpers for the Naos test harness.
#
# naos_add_executable(<target>
#     SOURCES <src>...
#     [LIBRARIES <lib>...]
#     [PREFIX <prefix>]               # CTest name prefix, default "<target>."
#     [LABELS <label>...]
#     [PROPERTIES <name> <value>...]
#     [DISCOVERY_TIMEOUT <seconds>]   # limit for "<exe> --naos-list", default 60
#     [ALLOW_NO_ASSERTIONS]           # run each test with --naos-allow-no-assertions
#     [IGNORE_SUITES <name>...]       # suites defined in the sources but not added, on purpose
#     [NO_REGISTRATION_CHECK])        # check neither the tests nor the suites added
#   Creates a test executable linked to Naos::naos and registers each of its tests with CTest. With
#   GCC and Clang, a test or a fixture defined and never added fails to compile: its static
#   function is unused, which -Werror=unused-function makes an error.
#
# naos_discover_tests(<target> [PREFIX ...] [LABELS ...] [PROPERTIES ...] [DISCOVERY_TIMEOUT ...]
#     [ALLOW_NO_ASSERTIONS] [IGNORE_SUITES ...] [NO_REGISTRATION_CHECK])
#   Registers the tests of an existing executable target that calls naos_init() and naos_run().
#   The target must be made in the same directory, and registered only once. Its compile options
#   are left as they are.
#
# Tests are discovered after each build by running "<exe> --naos-list", so adding or removing a test
# needs no reconfigure. Every test becomes one CTest test running
# "<exe> --naos-filter <suite>.<test>", with SKIP_RETURN_CODE 77; with ALLOW_NO_ASSERTIONS,
# "<exe> --naos-filter <suite>.<test> --naos-allow-no-assertions". Until the executable is built,
# or when discovery fails, CTest has one failing test in their place, <prefix>NOT_BUILT, with the
# same labels.
#
# Discovery also fails when a suite defined in the sources of the target (NAOS_SUITE) is not among
# those listed, unless IGNORE_SUITES names it or NO_REGISTRATION_CHECK is given
# (NaosCheckSuites.cmake).
#
# The functions whose names start with _naos_ are internal.

include_guard(GLOBAL)

# Functions keep the policies in effect where they are defined, so these hold whatever the
# cmake_minimum_required() of the project using Naos. CMP0174 (CMake 3.31): _naos_discovery_params()
# passes empty values on purpose, which an unset CMP0174 warns about. Empty and unset arguments are
# treated alike here, so the setting changes nothing else.
cmake_policy(PUSH)
if(POLICY CMP0174)
    cmake_policy(SET CMP0174 NEW)
endif()

# Quotes value as a bracket argument for a generated CMake file.
function(_naos_quote out value)
    set(${out} "[==[${value}]==]" PARENT_SCOPE)
endfunction()

# Sets out to the content of the parameter file read by NaosDiscover.cmake. Every keyword is
# required and takes one value, a list as one argument; the values may hold generator expressions.
function(_naos_discovery_params out)
    set(keywords TARGET EXECUTABLE EMULATOR PREFIX LABELS PROPERTIES DISCOVERY_TIMEOUT ARGS OUTPUT
        SOURCES SOURCE_DIR IGNORE_SUITES)
    cmake_parse_arguments(PARSE_ARGV 1 arg "" "${keywords}" "")
    set(content "")
    foreach(keyword IN LISTS keywords)
        _naos_quote(quoted "${arg_${keyword}}")
        string(APPEND content "set(NAOS_${keyword} ${quoted})\n")
    endforeach()
    set(${out} "${content}" PARENT_SCOPE)
endfunction()

# Fails the configure step when the arguments of caller (the public function) are wrong.
function(_naos_check_discovery_args caller target timeout properties ignore_suites)
    if(NOT TARGET ${target})
        message(FATAL_ERROR "${caller}: no target named '${target}'")
    endif()
    get_target_property(type ${target} TYPE)
    get_target_property(imported ${target} IMPORTED)
    if((NOT type STREQUAL "EXECUTABLE") OR imported)
        message(FATAL_ERROR "${caller}: '${target}' is not an executable built here")
    endif()
    get_target_property(discovered ${target} _NAOS_DISCOVERED)
    if(discovered)
        message(FATAL_ERROR "${caller}: the tests of '${target}' are already registered")
    endif()

    if(NOT timeout MATCHES "^[1-9][0-9]*$")
        message(FATAL_ERROR "${caller}: DISCOVERY_TIMEOUT must be a positive integer (seconds): '${timeout}'")
    endif()
    list(LENGTH properties property_count)
    math(EXPR odd "${property_count} % 2")
    if(odd)
        message(FATAL_ERROR "${caller}: PROPERTIES must be pairs of <name> <value>: '${properties}'")
    endif()
    foreach(suite IN LISTS ignore_suites)
        if(NOT suite MATCHES "^[A-Za-z0-9_]+$")
            message(FATAL_ERROR "${caller}: IGNORE_SUITES takes suite names (only letters, digits and '_'): '${suite}'")
        endif()
    endforeach()
endfunction()

# Makes a test or a fixture that no suite adds an error for the compiler, in the C and C++ sources
# of target: their functions are static (NAOS_TEST), so the compiler finds those never used. Only
# GCC and Clang with its own command line get the option. MSVC is left as it is: /we4505 did not
# report an unused static function in a Debug build of C, so it would promise a check it does not
# make.
function(_naos_registration_check_flags target)
    foreach(lang IN ITEMS C CXX)
        set(id "${CMAKE_${lang}_COMPILER_ID}")
        if((id MATCHES "^(GNU|Clang|AppleClang)$") AND (NOT CMAKE_${lang}_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC"))
            target_compile_options(${target} PRIVATE "$<$<COMPILE_LANGUAGE:${lang}>:-Werror=unused-function>")
        endif()
    endforeach()
endfunction()

# Registers the tests of target, for caller. The arguments are those of naos_discover_tests(),
# empty or false when not given.
function(_naos_discover caller target prefix labels properties timeout allow_no_assertions ignore_suites no_check)
    if(prefix STREQUAL "")
        set(prefix "${target}.")
    endif()
    if(timeout STREQUAL "")
        set(timeout 60)
    endif()
    if(allow_no_assertions)
        set(args "--naos-allow-no-assertions")
    else()
        set(args "")
    endif()
    _naos_check_discovery_args(${caller} ${target} "${timeout}" "${properties}" "${ignore_suites}")
    set_property(TARGET ${target} PROPERTY _NAOS_DISCOVERED TRUE)

    get_property(multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
    if(multi_config)
        set(config_suffix "-$<CONFIG>")
    else()
        set(config_suffix "")
    endif()

    set(file_base "${CMAKE_CURRENT_BINARY_DIR}/${target}")
    set(params_file "${file_base}_params${config_suffix}.cmake")
    set(tests_file "${file_base}_tests${config_suffix}.cmake")
    set(include_file "${file_base}_include.cmake")

    # The sources whose suites are checked, as the build sees them; none without the check.
    if(no_check)
        set(sources "")
    else()
        set(sources "$<TARGET_PROPERTY:${target},SOURCES>")
    endif()
    get_property(source_dir TARGET ${target} PROPERTY SOURCE_DIR)

    # Parameters go through a generated file so that lists survive without command-line escaping.
    get_property(emulator TARGET ${target} PROPERTY CROSSCOMPILING_EMULATOR)
    _naos_discovery_params(params
        TARGET "${target}"
        EXECUTABLE "$<TARGET_FILE:${target}>"
        EMULATOR "${emulator}"
        PREFIX "${prefix}"
        LABELS "${labels}"
        PROPERTIES "${properties}"
        DISCOVERY_TIMEOUT "${timeout}"
        ARGS "${args}"
        OUTPUT "${tests_file}"
        SOURCES "${sources}"
        SOURCE_DIR "${source_dir}"
        IGNORE_SUITES "${ignore_suites}"
    )
    file(GENERATE OUTPUT "${params_file}" CONTENT "${params}")
    # A change to the arguments rewrites the parameter file, which then links the executable again,
    # and so lists its tests again: without it, the tests kept their old names, labels and
    # properties until the executable changed. file(GENERATE) leaves an unchanged file as it is.
    # Only the Ninja and Makefile generators honor LINK_DEPENDS; with a multi-config one, the path
    # holds $<CONFIG>, which LINK_DEPENDS is not known to expand from CMake 3.21 on, so it is left out.
    if(NOT multi_config)
        set_property(TARGET ${target} APPEND PROPERTY LINK_DEPENDS "${params_file}")
    endif()

    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -D "NAOS_PARAMS=${params_file}" -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/NaosDiscover.cmake"
        COMMENT "Discovering Naos tests in ${target}"
        VERBATIM
    )

    # Read by CTest, which picks the file of the configuration it runs.
    if(multi_config)
        set(tests_file_at_ctest "${file_base}_tests-\${CTEST_CONFIGURATION_TYPE}.cmake")
    else()
        set(tests_file_at_ctest "${tests_file}")
    endif()
    _naos_quote(not_built_name "${prefix}NOT_BUILT")
    _naos_quote(not_built_command "${target}_NOT_BUILT")
    set(content "if(EXISTS \"${tests_file_at_ctest}\")\n")
    string(APPEND content "    include(\"${tests_file_at_ctest}\")\n")
    string(APPEND content "else()\n")
    string(APPEND content "    add_test(${not_built_name} ${not_built_command})\n")
    if(NOT labels STREQUAL "")
        _naos_quote(quoted_labels "${labels}")
        string(APPEND content "    set_tests_properties(${not_built_name} PROPERTIES LABELS ${quoted_labels})\n")
    endif()
    string(APPEND content "endif()\n")
    file(WRITE "${include_file}" "${content}")
    set_property(DIRECTORY APPEND PROPERTY TEST_INCLUDE_FILES "${include_file}")
endfunction()

function(naos_discover_tests target)
    cmake_parse_arguments(PARSE_ARGV 1 arg "ALLOW_NO_ASSERTIONS;NO_REGISTRATION_CHECK" "PREFIX;DISCOVERY_TIMEOUT"
        "LABELS;PROPERTIES;IGNORE_SUITES")
    if(arg_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "naos_discover_tests: unknown arguments: ${arg_UNPARSED_ARGUMENTS}")
    endif()

    _naos_discover(naos_discover_tests ${target}
        "${arg_PREFIX}" "${arg_LABELS}" "${arg_PROPERTIES}" "${arg_DISCOVERY_TIMEOUT}" "${arg_ALLOW_NO_ASSERTIONS}"
        "${arg_IGNORE_SUITES}" "${arg_NO_REGISTRATION_CHECK}")
endfunction()

function(naos_add_executable target)
    # The keywords of naos_discover_tests(), and SOURCES and LIBRARIES.
    cmake_parse_arguments(PARSE_ARGV 1 arg "ALLOW_NO_ASSERTIONS;NO_REGISTRATION_CHECK" "PREFIX;DISCOVERY_TIMEOUT"
        "SOURCES;LIBRARIES;LABELS;PROPERTIES;IGNORE_SUITES")
    if(arg_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "naos_add_executable: unknown arguments: ${arg_UNPARSED_ARGUMENTS}")
    endif()
    if(NOT arg_SOURCES)
        message(FATAL_ERROR "naos_add_executable: SOURCES is required")
    endif()

    add_executable(${target} ${arg_SOURCES})
    target_link_libraries(${target} PRIVATE Naos::naos ${arg_LIBRARIES})
    if(NOT arg_NO_REGISTRATION_CHECK)
        _naos_registration_check_flags(${target})
    endif()

    _naos_discover(naos_add_executable ${target}
        "${arg_PREFIX}" "${arg_LABELS}" "${arg_PROPERTIES}" "${arg_DISCOVERY_TIMEOUT}" "${arg_ALLOW_NO_ASSERTIONS}"
        "${arg_IGNORE_SUITES}" "${arg_NO_REGISTRATION_CHECK}")
endfunction()

cmake_policy(POP)
