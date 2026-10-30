# Installation

How to bring Naos into a project: with CMake, in one of three ways, or without CMake, by compiling its sources.

## Requirements

| What | Version |
|---|---|
| C compiler | C99: GCC, Clang or MSVC |
| C++ compiler | C++11, only for tests written in C++ |
| CMake | 3.21 or later; optional ([without CMake](#without-cmake)) |

Every change is built and tested on Linux (GCC and Clang), macOS (Clang) and Windows (MSVC), as a static and as a shared library.

## With CMake

Each way below gives the target `naos::naos` and the functions `naos_add_executable()` and `naos_discover_tests()` ([CMake reference](../reference/cmake.md)). Then make the test executable and let CTest run it:

```cmake
enable_testing()   # or include(CTest)

naos_add_executable(my_tests SOURCES my_tests.c LIBRARIES my_lib)
```

Without `enable_testing()` (or `include(CTest)`) in the top-level `CMakeLists.txt`, CTest finds no test.

### FetchContent

Naos is downloaded at configure time. This is the simplest way, and the one the [quickstart](quickstart.md) uses.

```cmake
include(FetchContent)
FetchContent_Declare(naos
    GIT_REPOSITORY https://github.com/vabelorg/naos.git
    GIT_TAG v0.0.0
)
FetchContent_MakeAvailable(naos)
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
find_package(naos 0.0 REQUIRED)
```

Which versions it accepts is in the [CMake reference](../reference/cmake.md#package).

## Without CMake

Add `include/` to the include path and the `.c` files of `src/` to the build.

To run the tests one by one in separate processes without CTest, see [CTest and CI](../guides/ctest-and-ci.md#without-cmake).

## License

Naos is distributed under the [MIT License](../../LICENSE). Whether you copy its sources or distribute a library built from them, include the copyright and permission notice of `LICENSE`. `cmake --install` puts it in `<prefix>/share/licenses/naos/`.
