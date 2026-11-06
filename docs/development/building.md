# Building Naos

How to build Naos from its sources and run its self tests. To use Naos in a project instead, see [installation](../getting-started/installation.md).

## Requirements

- A C99 compiler: GCC, Clang or MSVC
- CMake 3.21 or later
- A C++ compiler, for the C++ self tests and example (optional: they are left out without one)
- Git

## With the presets

`CMakePresets.json` has four presets, each building in `build/<preset>/`:

| Preset | Builds |
|---|---|
| `default` | A static library, Debug, with the self tests |
| `shared` | A shared library (DLL), Debug, with the self tests |
| `release` | A static library, Release (optimized), with the self tests |
| `sanitize` | A static library, Debug, with the self tests, built and run with AddressSanitizer and UndefinedBehaviorSanitizer. GCC and Clang only |

```sh
cmake --preset default
cmake --build --preset default
ctest --preset default
```

The same with `shared` checks the shared build. Run both before sending a change that touches the library or the CMake files. `release` shows what only the optimizer finds: `test.c` stops a test with `longjmp`, which an optimized build must survive, and some warnings of GCC, such as `-Wclobbered`, need it. `sanitize` finds memory mistakes and undefined behavior in the library and the tests; CI runs all four.

With MinGW-w64 on Windows, run them in the shell of MSYS2 (UCRT64 or MINGW64), with its `gcc`, `cmake` and `ninja` packages and `CMAKE_GENERATOR=Ninja`, as CI does.

## By hand

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

With a multi-config generator (Visual Studio, Ninja Multi-Config), add `--config Debug` to the build and `-C Debug` to `ctest`. Other compilers are chosen as usual, with `CC` and `CXX` or `-DCMAKE_C_COMPILER=...` at the first configure.

As the top-level project, Naos:

- builds the self tests (`NAOS_SELFTEST`) and treats every warning as an error: `-Wall -Wextra -Wpedantic -Werror` for GCC and Clang, `/W4 /WX` for MSVC;
- generates the install rules (`NAOS_INSTALL`);
- puts every executable and DLL in `build/bin/`, so that the self tests find a shared Naos next to them.

The options are in the [CMake reference](../reference/cmake.md#options).

## Running some self tests

Every self test has a CTest name starting with `naos.`, the label `naos` and the label of its layer; the [tests of the examples](testing.md#examples-and-documentation) are named after their executables ([names and labels](testing.md#names-and-labels)).

```sh
ctest --preset default -L contract                             # one layer
ctest --preset default -R '^naos\.behavior\.command-line\.'    # one area
ctest --preset default -LE slow                                # all but the cases that build a project
ctest --preset default -j8                                     # in parallel
```

The checks that need no build, such as the links of the documentation, are not self tests: see [lint](testing.md#lint).

[Testing](testing.md) explains the self tests and how to add one.

## Installing

```sh
cmake --install build/default --prefix /tmp/naos-install
```

The layout installed is in the [CMake reference](../reference/cmake.md#package). A project can then use it with `find_package(Naos)` and `-DCMAKE_PREFIX_PATH=/tmp/naos-install`. `tests/consumer/` is such a project; `cmake -D PRESET=<preset> -P scripts/TestPackage.cmake` installs a preset built and tests the project against it, as CI does for each build but `sanitize`. Where `pkg-config` is found, but with MSVC, the script then checks the flags of the `naos.pc` installed and builds and runs `tests/consumer/consumer_test.c` with them, without CMake.

## Continuous integration

`.github/workflows/ci.yml` checks every push and pull request. Each of its steps is a command that runs the same locally:

| Job | Runs |
|---|---|
| `lint` | `cmake -P scripts/CheckMarkdownLinks.cmake` ([lint](testing.md#lint)) |
| `test` | Configure, build and `ctest` with a preset, then `cmake -D PRESET=<preset> -P scripts/TestPackage.cmake` ([installing](#installing)), but with `sanitize`. GCC on Linux with `default`, `shared`, `release` and `sanitize`, and with `default` and CMake 3.21, the oldest CMake supported; Clang on Linux with `default` and `release`; Clang on macOS with `default` and `shared`; MSVC with `default`, `shared` and `release` |
| `mingw` | The same with GCC of MinGW-w64 in MSYS2: UCRT64 with `default` and `shared`, MINGW64 with `default`. MINGW64 is there for the old C library of Windows, msvcrt, whose printf does not follow C99 ([coding style](coding-style.md#formatted-output)) |
