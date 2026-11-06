# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# Self tests of the Naos harness. The first directory is the layer, which tells who judges its cases;
# the directories under it hold the programs of one area, the CMakeLists.txt registering its cases
# and, for golden cases, expected/<case>.txt. A case is named naos.<path>.<case> after its directory
# and has the labels naos and <layer>. harness/NaosSelftest.cmake has the helpers and tells how a
# case is written; docs/development/testing.md tells how to add one.
#
#   contract/   judged by exit codes, regexes and a schema, never by golden files: what Naos keeps
#               compatible. Its exit codes set up the fixture naos_trust that api/ and unit/ require
#   api/        judged by Naos: the public API used as a project uses it, in C and C++
#   unit/       judged by Naos: the internal modules whose results depend only on their arguments
#   behavior/   judged by RunCase.cmake against golden files: one directory per page of
#               docs/reference/ (tests, assertions, control, command-line, output, xml)
#   build/      judged by the compiler and CMake: programs that must not compile (misuse/), the
#               messages of test discovery (discovery/), the CMake functions (functions/), Naos
#               added with add_subdirectory() (subproject/).
#               The cases that build a project of their own have the label slow too
#   examples/   the projects of examples/, and the outputs the documentation quotes from them
# Not a layer: a project using Naos, which build/subproject/ adds Naos to with add_subdirectory(),
# and scripts/TestPackage.cmake with find_package(Naos <version>) after installing a build:
#   consumer/
#
# The checks that read the sources and need no build (the links of the Markdown files, the printf
# calls of src/) are not here: they are scripts/check_markdown_links.py and
# scripts/check_printf_calls.py, run once by the lint job of CI.

include(${CMAKE_CURRENT_SOURCE_DIR}/harness/NaosSelftest.cmake)

# The library gets the strict warnings of its self tests too.
naos_selftest_strict(naos)

foreach(layer IN ITEMS contract api unit behavior build examples)
    add_subdirectory(${layer})
endforeach()
