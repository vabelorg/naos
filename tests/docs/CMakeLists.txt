# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# The documentation: every relative link and anchor of the Markdown files leads somewhere.

add_test(NAME naos.docs.links
    COMMAND "${CMAKE_COMMAND}" -D "ROOT=${PROJECT_SOURCE_DIR}" -P "${CMAKE_CURRENT_SOURCE_DIR}/check-links.cmake"
)
set_tests_properties(naos.docs.links PROPERTIES LABELS naos)
