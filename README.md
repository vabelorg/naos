# Naos

English · [日本語](README.ja.md)

[![CI](https://github.com/vabelorg/naos/actions/workflows/ci.yml/badge.svg)](https://github.com/vabelorg/naos/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
![C99](https://img.shields.io/badge/C-99-blue.svg)

> [!NOTE]
> On the `main` branch, this README and the documentation may describe changes not released yet, breaking ones included. If you use a released version, read them at its tag, such as the [latest release](https://github.com/vabelorg/naos/releases/latest). The changes not released yet are under `[Unreleased]` in the [changelog](CHANGELOG.md).

A test harness for C and C++ that needs only a C99 compiler: no library to install, no compiler extension. Built with CMake and usable from any build system.

```c
#include <naos/naos.h>

#include "clamp.h"

NAOS_TEST(below_range)
{
    NAOS_CHECK_INT_EQ(0, clamp(-3, 0, 10));
}

NAOS_TEST(above_range)
{
    NAOS_CHECK_INT_EQ(10, clamp(42, 0, 10));
}

NAOS_SUITE(clamp,
    NAOS_ADD_TEST(below_range),
    NAOS_ADD_TEST(above_range)
)

NAOS_MAIN(
    NAOS_ADD_SUITE(clamp)
)
```

When `clamp()` forgets its upper bound:

```
$ ./clamp_tests
RUN: clamp.below_range
PASS: clamp.below_range (0 ms)
RUN: clamp.above_range
clamp_test.c:12: error: 10 == clamp(42, 0, 10)
  left:  10
  right: 42
FAIL: clamp.above_range (1 failure, 0 ms)
SUMMARY: 1 passed, 1 failed, 0 skipped (0 ms)
```

## Features

- **One executable runs every test**: a line per test with the time it took, a summary, and the result as the exit code.
- **One test at a time**: `--naos-filter clamp.above_range`, under a debugger for instance. Wildcards and exclusions select groups.
- **One process per test**: `naos_add_executable()` registers each test with CTest, which isolates crashes, runs in parallel and sets time limits.
- **No test that checks nothing**: a test that runs no assertion fails, so an empty or unfinished test, or a loop over no case, does not read as a pass.
- **No test left out**: with the CMake functions, a suite defined and never added fails the build, and so does such a test with GCC and Clang, instead of never running while the run passes.
- **Tests expected to fail**: `NAOS_ADD_TEST_XFAIL` marks a test written before the code, or one for a known bug, so the run passes while it fails and fails as soon as it passes.
- **Hidden dependencies shaken out**: `--naos-shuffle` (replayable seed), `--naos-repeat`, `--naos-fail-fast`.
- **Failures your editor understands**: `file:line: error: ...`, in a stable, documented format.
- **Easy to adopt**: `FetchContent`, `add_subdirectory`, `find_package`, or the installed library from any build system.

## Requirements

- A C99 compiler (GCC, Clang or MSVC); C++11 for tests written in C++.
- CMake 3.21 or later, to build Naos.

Every change is built and tested on Linux (GCC, Clang), macOS (Clang) and Windows (MSVC, and GCC of MinGW-w64 in MSYS2).

## Installation

With CMake, `FetchContent` is the simplest way:

```cmake
include(FetchContent)
FetchContent_Declare(Naos
    GIT_REPOSITORY https://github.com/vabelorg/naos.git
    GIT_TAG v0.3.0
)
FetchContent_MakeAvailable(Naos)
```

`add_subdirectory()` and `find_package(Naos)` work as well. Another build system links the library that CMake built and installed. Every way is in [Installation](docs/getting-started/installation.md).

## Quick start

```cmake
enable_testing()
naos_add_executable(clamp_tests SOURCES clamp_test.c LIBRARIES clamp)
```

```sh
cmake -S . -B build && cmake --build build && ctest --test-dir build
```

The [quickstart](docs/getting-started/quickstart.md) walks through it in five minutes.

## Documentation

| | |
|---|---|
| **Start** | [Installation](docs/getting-started/installation.md) · [Quickstart](docs/getting-started/quickstart.md) · [Tutorial](docs/getting-started/tutorial.md) |
| **Use** | [Guides](docs/guides/) · [Troubleshooting](docs/guides/troubleshooting.md) |
| **Look up** | [Tests](docs/reference/tests.md) · [Assertions](docs/reference/assertions.md) · [Failing, skipping and notes](docs/reference/control.md) · [Version](docs/reference/version.md) · [Command line](docs/reference/command-line.md) · [Output](docs/reference/output.md) · [JUnit XML](docs/reference/xml.md) · [CMake](docs/reference/cmake.md) |

The whole map is in [docs/](docs/README.md).

## Status

Naos is before 1.0: a new minor version may break compatibility, and `find_package(Naos 0.3)` accepts 0.3.x only. What changes in each version is in the [changelog](CHANGELOG.md).

## Contributing

Bug reports, ideas and pull requests are welcome. Start with [CONTRIBUTING](docs/CONTRIBUTING.md); [Building](docs/development/building.md), [Architecture](docs/development/architecture.md) and [Testing](docs/development/testing.md) cover the code. Everyone taking part follows the [Code of Conduct](docs/CODE_OF_CONDUCT.md).

## Security

Please report a vulnerability privately, as [SECURITY](docs/SECURITY.md) describes, not in a public issue.

## License

[MIT](LICENSE)
