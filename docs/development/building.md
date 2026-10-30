# Building Naos

How to build Naos from its sources and run its self tests. To use Naos in a project instead, see [installation](../getting-started/installation.md).

## Requirements

- A C99 compiler: GCC, Clang or MSVC
- CMake 3.21 or later
- A C++ compiler, for the C++ self tests and example (optional: they are left out without one)
- Git

## With the presets

`CMakePresets.json` has two presets, each building in `build/<preset>/`:

| Preset | Builds |
|---|---|
| `default` | A static library, Debug, with the self tests |
| `shared` | A shared library (DLL), Debug, with the self tests |

```sh
cmake --preset default
cmake --build --preset default
ctest --preset default
```

The same with `shared` checks the shared build. Run both before sending a change that touches the library or the CMake files.

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

Every self test has a CTest name starting with `naos.` and the label `naos`; the [tests of the examples](testing.md#examples-and-documentation) are named after their executables.

```sh
ctest --preset default -R '^naos\.cli\.'          # one subject
ctest --preset default -R '^naos\.docs\.links$'   # the links of the documentation
ctest --preset default -j8                        # in parallel
```

[Testing](testing.md) explains the self tests and how to add one.

## Installing

```sh
cmake --install build/default --prefix /tmp/naos-install
```

The layout installed is in the [CMake reference](../reference/cmake.md#package). A project can then use it with `find_package(naos)` and `-DCMAKE_PREFIX_PATH=/tmp/naos-install`. `tests/package/` is such a project; CI builds and tests it against each package installed.

## Without CMake

The library must also build with nothing but a compiler; the self test `naos.no_cmake.build` checks it as [installation](../getting-started/installation.md#without-cmake) describes, on GCC-compatible compilers.

## Continuous integration

`.github/workflows/ci.yml` builds and tests every push and pull request with the presets, on Linux (GCC and Clang), macOS (Clang) and Windows (MSVC), each as a static and a shared library. A change is merged only when every job passes.
