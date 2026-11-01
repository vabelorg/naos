# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# The sources of src/: every formatted output goes through src/format.h.

add_test(NAME naos.sources.format
    COMMAND "${CMAKE_COMMAND}" -D "ROOT=${PROJECT_SOURCE_DIR}" -P "${CMAKE_CURRENT_SOURCE_DIR}/CheckFormat.cmake"
)
set_tests_properties(naos.sources.format PROPERTIES LABELS naos)
