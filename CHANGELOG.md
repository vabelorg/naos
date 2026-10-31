# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

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
