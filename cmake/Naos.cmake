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
#     [ALLOW_NO_ASSERTIONS])          # run each test with --naos-allow-no-assertions
#   Creates a test executable linked to Naos::naos and registers each of its tests with CTest.
#
# naos_discover_tests(<target> [PREFIX ...] [LABELS ...] [PROPERTIES ...] [DISCOVERY_TIMEOUT ...]
#     [ALLOW_NO_ASSERTIONS])
#   Registers the tests of an existing executable target that calls naos_init() and naos_run().
#   The target must be made in the same directory, and registered only once.
#
# Tests are discovered after each build by running "<exe> --naos-list", so adding or removing a test
# needs no reconfigure. Every test becomes one CTest test running
# "<exe> --naos-filter <suite>.<test>", with SKIP_RETURN_CODE 77; with ALLOW_NO_ASSERTIONS,
# "<exe> --naos-filter <suite>.<test> --naos-allow-no-assertions". Until the executable is built,
# CTest has one failing test in their place, <prefix>NOT_BUILT, with the same labels. A change to
# the arguments takes effect at the next build, through a target <target>_naos_tests that each
# function adds.
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
# PROPERTIES, <name> <value> pairs, is written one variable per name and per value, so that a value
# keeps its own semicolons, also those a generator expression gives when the file is generated:
# NAOS_PROPERTY_COUNT, then NAOS_PROPERTY_NAME_<i> and NAOS_PROPERTY_VALUE_<i> for i from 1.
function(_naos_discovery_params out)
    set(keywords TARGET EXECUTABLE EMULATOR PREFIX LABELS DISCOVERY_TIMEOUT ARGS OUTPUT)
    cmake_parse_arguments(PARSE_ARGV 1 arg "" "${keywords};PROPERTIES" "")
    set(content "")
    foreach(keyword IN LISTS keywords)
        _naos_quote(quoted "${arg_${keyword}}")
        string(APPEND content "set(NAOS_${keyword} ${quoted})\n")
    endforeach()
    set(count 0)
    set(is_value FALSE)
    foreach(item IN LISTS arg_PROPERTIES)
        _naos_quote(quoted "${item}")
        if(is_value)
            string(APPEND content "set(NAOS_PROPERTY_VALUE_${count} ${quoted})\n")
            set(is_value FALSE)
        else()
            math(EXPR count "${count} + 1")
            string(APPEND content "set(NAOS_PROPERTY_NAME_${count} ${quoted})\n")
            set(is_value TRUE)
        endif()
    endforeach()
    string(APPEND content "set(NAOS_PROPERTY_COUNT ${count})\n")
    set(${out} "${content}" PARENT_SCOPE)
endfunction()

# Fails the configure step when the arguments of caller (the public function) are wrong.
function(_naos_check_discovery_args caller target timeout properties)
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
endfunction()

# Registers the tests of target, for caller. The arguments are those of naos_discover_tests(),
# empty or false when not given.
function(_naos_discover caller target prefix labels properties timeout allow_no_assertions)
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
    _naos_check_discovery_args(${caller} ${target} "${timeout}" "${properties}")
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
    )
    file(GENERATE OUTPUT "${params_file}" CONTENT "${params}")

    # A change to the arguments must list the tests again, or they keep their old names, labels
    # and properties until the executable changes. The arguments change only at the configure, so
    # they are compared here, by content, with those of the previous configure: when they differ,
    # the CTest files go, and the rule below writes them again at the next build because they are
    # missing. Nothing compares times, which the make of macOS keeps to the second, and it works
    # the same with every generator. The content compared still holds the generator expressions.
    set(stored_file "${file_base}_params.stored")
    set(stored "")
    if(EXISTS "${stored_file}")
        file(READ "${stored_file}" stored)
    endif()
    if(NOT stored STREQUAL params)
        if(multi_config)
            set(tests_files "")
            foreach(config IN LISTS CMAKE_CONFIGURATION_TYPES)
                list(APPEND tests_files "${file_base}_tests-${config}.cmake")
            endforeach()
        else()
            set(tests_files "${tests_file}")
        endif()
        file(REMOVE ${tests_files})
        file(WRITE "${stored_file}" "${params}")
    endif()

    set(discover_command "${CMAKE_COMMAND}" -D "NAOS_PARAMS=${params_file}"
        -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/NaosDiscover.cmake")
    set(discover_comment "Discovering Naos tests in ${target}")

    # Lists the tests after each link, so that building the executable alone, with --target, lists
    # its tests too.
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${discover_command}
        COMMENT "${discover_comment}"
        VERBATIM
    )
    # Writes the CTest file when it is missing: after the configure removed it above, or after a
    # clean. The rule has no DEPENDS, so it runs only then. A target-level dependency alone orders
    # it after the link; a file-level one, from DEPENDS ${target}, would run it after each link
    # too, like the POST_BUILD command, and would compare times again. Ninja decides what is
    # missing when the build starts, so on a build that links the executable and lacks the file it
    # lists the tests twice, once for each command, which is harmless. The target carries the
    # EXCLUDE_FROM_ALL of the executable, so that it does not pull an excluded one into all.
    add_custom_command(OUTPUT "${tests_file}"
        COMMAND ${discover_command}
        COMMENT "${discover_comment}"
        VERBATIM
    )
    set(discovery_target "${target}_naos_tests")
    get_target_property(excluded ${target} EXCLUDE_FROM_ALL)
    if(excluded)
        add_custom_target(${discovery_target} DEPENDS "${tests_file}")
    else()
        add_custom_target(${discovery_target} ALL DEPENDS "${tests_file}")
    endif()
    add_dependencies(${discovery_target} ${target})
    set_property(TARGET ${discovery_target} PROPERTY FOLDER "Naos")

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
    cmake_parse_arguments(PARSE_ARGV 1 arg "ALLOW_NO_ASSERTIONS" "PREFIX;DISCOVERY_TIMEOUT" "LABELS;PROPERTIES")
    if(arg_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "naos_discover_tests: unknown arguments: ${arg_UNPARSED_ARGUMENTS}")
    endif()

    _naos_discover(naos_discover_tests ${target}
        "${arg_PREFIX}" "${arg_LABELS}" "${arg_PROPERTIES}" "${arg_DISCOVERY_TIMEOUT}" "${arg_ALLOW_NO_ASSERTIONS}")
endfunction()

function(naos_add_executable target)
    # The keywords of naos_discover_tests(), and SOURCES and LIBRARIES.
    cmake_parse_arguments(PARSE_ARGV 1 arg "ALLOW_NO_ASSERTIONS" "PREFIX;DISCOVERY_TIMEOUT" "SOURCES;LIBRARIES;LABELS;PROPERTIES")
    if(arg_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "naos_add_executable: unknown arguments: ${arg_UNPARSED_ARGUMENTS}")
    endif()
    if(NOT arg_SOURCES)
        message(FATAL_ERROR "naos_add_executable: SOURCES is required")
    endif()

    add_executable(${target} ${arg_SOURCES})
    target_link_libraries(${target} PRIVATE Naos::naos ${arg_LIBRARIES})

    _naos_discover(naos_add_executable ${target}
        "${arg_PREFIX}" "${arg_LABELS}" "${arg_PROPERTIES}" "${arg_DISCOVERY_TIMEOUT}" "${arg_ALLOW_NO_ASSERTIONS}")
endfunction()

cmake_policy(POP)
