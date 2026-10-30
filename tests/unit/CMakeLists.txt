# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# Unit tests of the internal modules whose results depend only on their arguments (value, filter,
# order), run by Naos itself, one CTest test each.
#
# The executable is built from the sources of the library rather than linked to naos::naos: a
# static Naos would also do, but a shared one exports only NAOS_API. Built this way the unit tests
# need no NAOS_SHARED and no DLL.

get_target_property(naos_library_sources naos SOURCES)
list(TRANSFORM naos_library_sources PREPEND "${PROJECT_SOURCE_DIR}/src/")
add_executable(naos_selftest_unit main.c value.c filter.c order.c ${naos_library_sources})
target_include_directories(naos_selftest_unit PRIVATE
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/src"
)
set_target_properties(naos_selftest_unit PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS NO
)
naos_selftest_strict(naos_selftest_unit)
naos_discover_tests(naos_selftest_unit PREFIX "naos.unit." LABELS naos)
