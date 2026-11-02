# Installation

How to bring Naos into a project. Naos is built with CMake: a CMake project adds it in one of three ways, and a project with another build system links the library that CMake built and installed.

## Requirements

| What | Version |
|---|---|
| C compiler | C99: GCC, Clang or MSVC |
| C++ compiler | C++11, only for tests written in C++ |
| CMake | 3.21 or later, to build Naos, also for a project with [another build system](#other-build-systems) |

Every change is built and tested on Linux (GCC and Clang), macOS (Clang) and Windows (MSVC, and GCC of MinGW-w64 in the UCRT64 and MINGW64 environments of MSYS2), as a static and as a shared library. Other builds with MinGW-w64, such as cross-compiling for Windows from Linux, are expected to work but are not tested.

## With CMake

Each way below gives the target `Naos::naos` and the functions `naos_add_executable()` and `naos_discover_tests()` ([CMake reference](../reference/cmake.md)). Then make the test executable and let CTest run it:

```cmake
enable_testing()   # or include(CTest)

naos_add_executable(my_tests SOURCES my_tests.c LIBRARIES my_lib)
```

Without `enable_testing()` (or `include(CTest)`) in the top-level `CMakeLists.txt`, CTest finds no test.

### FetchContent

Naos is downloaded at configure time. This is the simplest way, and the one the [quickstart](quickstart.md) uses.

```cmake
include(FetchContent)
FetchContent_Declare(Naos
    GIT_REPOSITORY https://github.com/vabelorg/naos.git
    GIT_TAG v0.2.0
)
FetchContent_MakeAvailable(Naos)
```

Pin `GIT_TAG` to a release tag.

### Subdirectory

With a copy of Naos in the project (a Git submodule, or the sources copied):

```cmake
add_subdirectory(third_party/naos)
```

Inside another project, unless asked, Naos builds as a static library, registers no self test and generates no install rule ([options](../reference/cmake.md#options)).

### Installed package

Build and install Naos once:

```sh
cmake -S naos -B naos/build -DCMAKE_BUILD_TYPE=Release
cmake --build naos/build --config Release
cmake --install naos/build --config Release --prefix /opt/naos
```

Then find it, with the prefix in `CMAKE_PREFIX_PATH` (`-DCMAKE_PREFIX_PATH=/opt/naos`):

```cmake
find_package(Naos 0.2 REQUIRED)
```

Which versions it accepts is in the [CMake reference](../reference/cmake.md#package).

## Other build systems

Build and install Naos with CMake once, as for an [installed package](#installed-package), then give the compiler the headers and the library under the install prefix:

```sh
cc -std=c99 -I/opt/naos/include -c my_tests.c
cc -o my_tests my_tests.o -L/opt/naos/lib -lnaos
```

- A Debug build names the library with the suffix `d`: `-lnaosd`. With MSVC, the library is `naos.lib` (`naosd.lib`).
- A shared library needs `NAOS_SHARED` defined when compiling the tests (`-DNAOS_SHARED`), and must be found when they run ([shared library](../guides/shared-library.md)).
- The directories follow `GNUInstallDirs`: the library may be in `lib64/` rather than `lib/`. The files installed are in the [CMake reference](../reference/cmake.md#package).

Building the sources of `src/` into your own project, without the build of Naos, is not supported.

To run the tests one by one in separate processes without CTest, see [CTest and CI](../guides/ctest-and-ci.md#without-ctest).

## License

Naos is distributed under the [MIT License](../../LICENSE). Whether you copy its sources or distribute a library built from them, include the copyright and permission notice of `LICENSE`. `cmake --install` puts it in `<prefix>/share/licenses/naos/`.
