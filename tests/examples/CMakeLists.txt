# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# The projects of examples/, built as the documentation shows them: their tests must pass, one
# CTest test each, and the outputs the documentation quotes are golden cases here.

set(examples_dir "${PROJECT_SOURCE_DIR}/examples")

# The libraries of the examples stand for code of the user's project and export nothing, so they stay
# static; a shared build of Naos still links them to naos.dll.
set(BUILD_SHARED_LIBS OFF)

foreach(example IN ITEMS quickstart tutorial stubs custom-main)
    add_subdirectory("${examples_dir}/${example}" "${CMAKE_CURRENT_BINARY_DIR}/${example}")
endforeach()
naos_selftest_strict(clamp clamp_tests strbuf strbuf_tests module_a_tests
    custom_main_tests)

include(CheckLanguage)
check_language(CXX)
if(CMAKE_CXX_COMPILER)
    add_subdirectory("${examples_dir}/cpp" "${CMAKE_CURRENT_BINARY_DIR}/cpp")
    naos_selftest_strict(cpp_tests)
else()
    message(STATUS "Naos: no C++ compiler found, examples/cpp is not built")
endif()

# docs/getting-started/quickstart.md and tutorial.md
naos_expect(quickstart clamp_tests)
naos_expect(tutorial strbuf_tests)
naos_expect(tutorial_list strbuf_tests ARGS --naos-list)
naos_expect(tutorial_filter strbuf_tests ARGS --naos-filter "strbuf.append_*")
# docs/guides/custom-main.md
naos_expect(custom_main_slow custom_main_tests ARGS --slow)
