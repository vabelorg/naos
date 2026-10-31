# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# Programs that must not compile: the macros refuse these mistakes at build time. Each case lists
# the identifiers the compiler must report. Only GCC-compatible drivers are asked, as -fsyntax-only
# is theirs.

function(naos_misuse_case name)
    add_test(NAME naos.misuse.${name}
        COMMAND "${CMAKE_COMMAND}"
            -D "CC=${CMAKE_C_COMPILER}"
            -D "INCLUDE_DIR=${PROJECT_SOURCE_DIR}/include"
            -D "SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/${name}.c"
            -D "EXPECT=${ARGN}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/check-misuse.cmake"
    )
    set_tests_properties(naos.misuse.${name} PROPERTIES LABELS naos)
endfunction()

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang" AND NOT CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    # A setup named where a teardown goes, and the other way round.
    naos_misuse_case(role_mix naos_gen_setup_cleanup naos_gen_teardown_prepare)
    # A test named where a setup goes.
    naos_misuse_case(test_as_setup naos_gen_setup_only)
    # A setup named where a test goes.
    naos_misuse_case(setup_as_test naos_gen_test_prepare)
endif()
