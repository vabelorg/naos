# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# Self tests of the Naos harness, one directory per subject. Each holds its programs and a
# CMakeLists.txt registering its cases; a case checked against golden output keeps it in
# <subject>/expected/<case>.txt. support/NaosSelftest.cmake has the helpers and tells how a case is
# written; docs/development/testing.md tells how to add one.
#
# Tests that check themselves, one CTest test per Naos test:
#   api/        the public API used as a project uses it, in C and C++
#   unit/       unit tests of the internal modules whose results depend only on their arguments
# Cases that run a program and check its exit code and output, one CTest test each:
#   cli/        the command line: options, filter patterns, --naos-list, --help, usage errors
#   session/    naos_init(), NAOS_ADD_SUITE and naos_run(), and the mistakes in the suites
#   assertions/ the failure message of each kind of assertion
#   control/    going on and stopping: CHECK, REQUIRE, FAIL, SKIP, results and exit codes
#   fixtures/   setup and teardown
#   context/    notes and traces
#   runs/       --naos-shuffle, --naos-repeat and --naos-fail-fast
#   discovery/  the messages of NaosDiscover.cmake when listing the tests fails
#   no_cmake/   building without CMake, as docs/getting-started/installation.md describes
#   examples/   the projects of examples/, and the outputs the documentation quotes from them
#   docs/       the relative links and anchors of the Markdown files
#   misuse/     programs that must not compile
# Not a self test, but configured on its own by CI after installing Naos:
#   package/    a project using the installed package with find_package(naos <version>)

include(${CMAKE_CURRENT_SOURCE_DIR}/support/NaosSelftest.cmake)

# The library gets the strict warnings of its self tests too.
naos_selftest_strict(naos)

foreach(subject IN ITEMS
        api unit cli session assertions control fixtures context runs discovery no_cmake examples docs misuse)
    add_subdirectory(${subject})
endforeach()
