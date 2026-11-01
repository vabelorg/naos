# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Building and testing with GCC of MinGW-w64 on Windows (MSYS2 UCRT64 and MINGW64). Naos formats its output as C99 does there too, and `NAOS_NOTE` and `NAOS_TRACE_PUSH` accept the formats of C99, such as `%zu` ([#3](https://github.com/vabelorg/naos/issues/3)).
- `--naos-output xml:<path>` writes the results of the run to a JUnit XML file as well, for CI services and other tools: each test with its result, and each failure with its message and its diagnostic as standard output shows it ([#4](https://github.com/vabelorg/naos/issues/4)).

### Changed

- **BREAKING**: The lines Naos writes on standard output are no longer kept compatible, so that they can change to read better. Only the start of a diagnostic line (`file:line: error: `, which editors read), the names of `--naos-list` and the exit codes stay compatible. A tool that parses standard output to collect results, in CI for example, should read the file of `--naos-output xml:<path>` instead, whose tests, results, counts, and the message and location of each failure are kept compatible ([#4](https://github.com/vabelorg/naos/issues/4)).

## [0.1.0] - 2026-10-31

### Added

- Tests in plain C99 or C++11, with no dependency besides the standard C library.
- A single executable that runs every test, prints a line per test and a summary, and returns the result as its exit code.
- Failures as `file:line: error: ...`, which editors can jump to.
- `--naos-filter` to run a single test or a group, for example under a debugger.
- `--naos-shuffle` and `--naos-repeat` to find tests that depend on their order or fail only sometimes.
- `naos_add_executable()` to register each test with CTest, which runs each one in its own process, in parallel and with a time limit.
- Installation with `FetchContent`, `add_subdirectory()`, `find_package()`, or the sources alone.

[Unreleased]: https://github.com/vabelorg/naos/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/vabelorg/naos/releases/tag/v0.1.0
