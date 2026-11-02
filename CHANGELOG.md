# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- `NAOS_PASS()`, an assertion that always passes, for a test whose check is that the code runs, such as one that only checks that a call returns without crashing ([#26](https://github.com/vabelorg/naos/issues/26)).
- `--naos-allow-no-assertions` lets a test that runs no assertion pass, as before; `ALLOW_NO_ASSERTIONS` of `naos_add_executable()` and `naos_discover_tests()` gives it to every test they register ([#26](https://github.com/vabelorg/naos/issues/26)).
- `naos_add_executable()` and `naos_discover_tests()` fail the build when a suite is defined with `NAOS_SUITE` in the sources of the target and never added with `NAOS_ADD_SUITE`, with `<file>:<line>: error: suite <name> is defined but never added` for each; `IGNORE_SUITES <suite>...` names those left out on purpose, such as a suite that a custom `main()` adds on an argument. A suite inside `#if`, `#ifdef` or `#ifndef` is not checked. `NaosCheckSuites.cmake`, installed beside the functions, runs the same check with `cmake -P` for a build without them ([#7](https://github.com/vabelorg/naos/issues/7)).
- `NO_REGISTRATION_CHECK` of `naos_add_executable()` and `naos_discover_tests()` turns off the checks of tests and suites never added ([#7](https://github.com/vabelorg/naos/issues/7)).
- `NAOS_ADD_TEST_XFAIL(test, reason)` registers a test expected to fail, such as one written before the code or one for a known bug. It passes as `XFAIL` while it records a failure, in setup, the body or teardown, and fails as `XPASS` once it ends without one, with `error: passed, but is expected to fail` at the entry of the suite, so that removing the mark is a step of its own. `--naos-fail-fast` stops at an `XPASS` only, `--naos-quiet` leaves out an `XFAIL` with its failures, `SUMMARY:` counts the expected failures when there are some, and the JUnit XML file has an `XFAIL` as `<skipped message="expected failure: <reason>"/>` and an `XPASS` as a `<failure>`. An empty or `NULL` reason is a mistake in the suites ([#6](https://github.com/vabelorg/naos/issues/6)).

### Changed

- **BREAKING**: A test that runs no assertion fails, with `naos: error: no assertion ran`, so that a test that checks nothing no longer reads as a pass: an empty or unfinished test, a loop over a table with no case, an early `return` before the assertions. Every `NAOS_CHECK_*` and `NAOS_REQUIRE_*` that runs counts, whether it passes or fails, as do `NAOS_PASS`, `NAOS_FAIL` and `NAOS_SKIP`, in setup, the body, teardown and the helpers they call; notes, traces and printing do not. Add an assertion to such a test, or end it with `NAOS_PASS()` when its check is that the code runs; to keep a suite passing while you do, run it with `--naos-allow-no-assertions`, or register it with `ALLOW_NO_ASSERTIONS` ([#26](https://github.com/vabelorg/naos/issues/26)).
- **BREAKING**: `naos_add_executable()` compiles the C and C++ sources of its target with `-Werror=unused-function` (GCC, and Clang with its own command line) or `/we4505` (MSVC), so that a test, a setup or a teardown that no suite adds does not compile. Any other `static` function of those sources that nothing uses is an error too: remove it, or give `NO_REGISTRATION_CHECK` ([#7](https://github.com/vabelorg/naos/issues/7)).
- **BREAKING**: A suite with no test is a mistake in the suites, reported with `naos: suite <name> has no test` before any test runs, with exit code 2: it never showed in `--naos-list`. Add a test to it, or remove it ([#7](https://github.com/vabelorg/naos/issues/7)).
- When test discovery fails after a build, the tests it registered before are removed, so that CTest has `<prefix>NOT_BUILT` in their place and fails rather than running the tests of an older build ([#7](https://github.com/vabelorg/naos/issues/7)).
- A diagnostic line without a location, that of a test that ran no assertion, starts with `naos: ` in place of `<file>:<line>: `, and its test case in the JUnit XML file has no `file` and `line` ([#26](https://github.com/vabelorg/naos/issues/26)).

## [0.3.0] - 2026-11-02

### Changed

- **BREAKING**: The package is `Naos` and its target `Naos::naos`, as the project is named: `find_package(Naos)`, `FetchContent_Declare(Naos ...)` and `target_link_libraries(... Naos::naos)`. The target `naos::naos` is no longer there, and the variables of `find_package()` are `Naos_VERSION` and `Naos_DIR` rather than `naos_VERSION` and `naos_DIR`. The package is installed as `<libdir>/cmake/Naos/NaosConfig.cmake`, which `find_package(naos)` does not find on a file system that tells the case of letters apart.

### Removed

- **BREAKING**: Building the `.c` files of `src/` directly into a project, without the build of Naos, is no longer supported: the object files of Naos, such as `session.o` and `text.o`, could take the place of those of the project with the same names. A project using CMake adds Naos with `FetchContent`, `add_subdirectory()` or `find_package(Naos)`; a project with another build system builds and installs Naos with CMake, then links the installed library ([installation](docs/getting-started/installation.md#other-build-systems)).

### Fixed

- Changing the arguments of `naos_add_executable()` or `naos_discover_tests()` (`PREFIX`, `LABELS`, `PROPERTIES`, `DISCOVERY_TIMEOUT`) now registers the tests again at the next build, with the single-config Ninja and Makefile generators. The tests kept their old names, labels and properties until the executable itself was built again.

## [0.2.0] - 2026-11-01

### Added

- Building and testing with GCC of MinGW-w64 on Windows (MSYS2 UCRT64 and MINGW64). Naos formats its output as C99 does there too, and `NAOS_NOTE` and `NAOS_TRACE_PUSH` accept the formats of C99, such as `%zu` ([#3](https://github.com/vabelorg/naos/issues/3)).
- `--naos-output xml:<path>` writes the results of the run to a JUnit XML file as well, for CI services and other tools: each test with its result, and each failure with its message and its diagnostic as standard output shows it ([#4](https://github.com/vabelorg/naos/issues/4)).
- `--naos-quiet` leaves out of standard output the tests that pass or skip, so that a failure is not buried among them, in a terminal or for an agent reading the output. A failing test is written as without it, and the run ends with its summary ([#8](https://github.com/vabelorg/naos/issues/8)).
- The time each test took, setup and teardown included, ends its result line (`PASS: storage.rebuild_index (3120 ms)`), and the total of the iteration ends `SUMMARY:`, so that a slow test, or a test made slower by a change, shows without asking for it. The JUnit XML file of `--naos-output` has the same times as `time`, in seconds. The clock is the monotonic clock of the system, which needs no library to install or link; `--help` names it ([#9](https://github.com/vabelorg/naos/issues/9)).

### Changed

- **BREAKING**: The lines Naos writes on standard output are no longer kept compatible, so that they can change to read better. Only the start of a diagnostic line (`file:line: error: `, which editors read), the names of `--naos-list` and the exit codes stay compatible. A tool that parses standard output to collect results, in CI for example, should read the file of `--naos-output xml:<path>` instead, whose tests, results, counts, and the message and location of each failure are kept compatible ([#4](https://github.com/vabelorg/naos/issues/4)).
- `SUMMARY:` ends every iteration, also when a single test is selected, so that a run always ends with a line saying what happened ([#8](https://github.com/vabelorg/naos/issues/8)).

## [0.1.0] - 2026-10-31

### Added

- Tests in plain C99 or C++11, with no dependency besides the standard C library.
- A single executable that runs every test, prints a line per test and a summary, and returns the result as its exit code.
- Failures as `file:line: error: ...`, which editors can jump to.
- `--naos-filter` to run a single test or a group, for example under a debugger.
- `--naos-shuffle` and `--naos-repeat` to find tests that depend on their order or fail only sometimes.
- `naos_add_executable()` to register each test with CTest, which runs each one in its own process, in parallel and with a time limit.
- Installation with `FetchContent`, `add_subdirectory()`, `find_package()`, or the sources alone.

[Unreleased]: https://github.com/vabelorg/naos/compare/v0.3.0...HEAD
[0.3.0]: https://github.com/vabelorg/naos/compare/v0.2.0...v0.3.0
[0.2.0]: https://github.com/vabelorg/naos/compare/v0.1.0...v0.2.0
[0.1.0]: https://github.com/vabelorg/naos/releases/tag/v0.1.0
