# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# The public API, used the way a project uses it: test executables made with naos_add_executable(),
# whose tests must all pass and become one CTest test each.

naos_add_executable(naos_selftest_api
    SOURCES main.c check.c control.c fixture.c version.c
    PREFIX "naos.api."
    LABELS naos
    DISCOVERY_TIMEOUT 30
)
naos_selftest_strict(naos_selftest_api)

# The tests share the process and must pass together, in any order and any number of times. The
# output grows with every test added, so only its end is matched.
naos_expect_match(run_all naos_selftest_api EXIT_CODE 0 EXPECT "SUMMARY: [0-9]+ passed, 0 failed, 0 skipped.$")
naos_expect_match(run_in_any_order naos_selftest_api EXIT_CODE 0
    ARGS --naos-shuffle --naos-random-seed 54865 --naos-repeat 50
    EXPECT "REPEAT: 0 of 50 iterations failed.$" REJECT "FAIL")

# C++ uses the same headers; the tests are built only when a C++ compiler exists.
include(CheckLanguage)
check_language(CXX)
if(CMAKE_CXX_COMPILER)
    enable_language(CXX)
    naos_add_executable(naos_selftest_api_cpp
        SOURCES cpp.cpp
        PREFIX "naos.api.cpp."
        LABELS naos
    )
    # Variadic macros need C++11.
    set_target_properties(naos_selftest_api_cpp PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
    )
    naos_selftest_strict(naos_selftest_api_cpp)
else()
    message(STATUS "Naos: no C++ compiler found, the C++ self tests are not built")
endif()
