# Quickstart

From an empty directory to tests run by CTest, in five minutes. You need a C compiler, CMake 3.21 or later and Git. The four files built below, `clamp.h`, `clamp.c`, `clamp_test.c` and `CMakeLists.txt`, go in the same directory. The finished project is [examples/quickstart](../../examples/quickstart/).

## 1. The code to test

A small library whose only function, `clamp()`, limits a value to the range from `low` to `high`. The next step tests it.

```c
/* clamp.h */
#ifndef CLAMP_H
#define CLAMP_H

/* Returns value limited to the range [low, high]. */
int clamp(int value, int low, int high);

#endif
```

```c
/* clamp.c */
#include "clamp.h"

int clamp(int value, int low, int high)
{
    if (value < low)
    {
        return low;
    }

    if (value > high)
    {
        return high;
    }

    return value;
}
```

## 2. The tests

The tests check `clamp()` in three cases: a value inside the range comes back unchanged (`inside_range`), a value under the lower bound becomes `low` (`below_range`), and a value over the upper bound becomes `high` (`above_range`).

```c
/* clamp_test.c */
#include <naos/naos.h>

#include "clamp.h"

NAOS_TEST(inside_range)
{
    NAOS_CHECK_INT_EQ(5, clamp(5, 0, 10));
}

NAOS_TEST(below_range)
{
    NAOS_CHECK_INT_EQ(0, clamp(-3, 0, 10));
}

NAOS_TEST(above_range)
{
    NAOS_CHECK_INT_EQ(10, clamp(42, 0, 10));
}

NAOS_SUITE(clamp,
    NAOS_ADD_TEST(inside_range),
    NAOS_ADD_TEST(below_range),
    NAOS_ADD_TEST(above_range)
)

NAOS_MAIN(
    NAOS_ADD_SUITE(clamp)
)
```

Read from the top, the code does this:

- `#include <naos/naos.h>` makes all the macros below available.
- `NAOS_TEST` defines a test, a function.
- `NAOS_SUITE` groups tests into a suite. Tests are not registered automatically: a test listed in no suite does not run.
- `NAOS_MAIN` defines `main()`, running the suites listed.
- `NAOS_CHECK_INT_EQ(expected, actual)` compares two integers and prints both when they differ.

## 3. The build

Write `CMakeLists.txt` as follows.

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.21)
project(quickstart LANGUAGES C)

enable_testing()

include(FetchContent)
FetchContent_Declare(naos
    GIT_REPOSITORY https://github.com/vabelorg/naos.git
    GIT_TAG v0.0.0
)
FetchContent_MakeAvailable(naos)

add_library(clamp clamp.c)
target_include_directories(clamp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

naos_add_executable(clamp_tests SOURCES clamp_test.c LIBRARIES clamp)
```

- `enable_testing()` turns on CTest. Without it, tests cannot be registered.
- `FetchContent` downloads Naos at the given tag from Git and brings it into the build. Nothing needs to be installed beforehand.
- `add_library(clamp ...)` and `target_include_directories(...)` turn the code under test into a library and make `clamp.h` includable.
- `naos_add_executable(clamp_tests SOURCES clamp_test.c LIBRARIES clamp)` makes the executable `clamp_tests` from `clamp_test.c` linked with the `clamp` library, and registers each of its tests with CTest after every build.

## 4. Build and run

Three commands in order: configure, build, and run the tests.

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

CTest runs each test in a process of its own and names it `<target>.<suite>.<test>`:

```
    Start 1: clamp_tests.clamp.inside_range
1/3 Test #1: clamp_tests.clamp.inside_range ...   Passed
    Start 2: clamp_tests.clamp.below_range
2/3 Test #2: clamp_tests.clamp.below_range ....   Passed
    Start 3: clamp_tests.clamp.above_range
3/3 Test #3: clamp_tests.clamp.above_range ....   Passed

100% tests passed, 0 tests failed out of 3
```

This generated the test executable `clamp_tests`, and its three tests ran as separate CTest tests and all passed. That is how easy testing C is with Naos: write the tests, add a few lines to CMake, and run them.

The executable also runs on its own, all its tests in one process. The following is an example with a single-config generator such as Makefiles or Ninja:

```
$ ./build/clamp_tests
RUN: clamp.inside_range
PASS: clamp.inside_range
RUN: clamp.below_range
PASS: clamp.below_range
RUN: clamp.above_range
PASS: clamp.above_range
SUMMARY: 3 passed, 0 failed, 0 skipped
```

With a multi-config generator such as Visual Studio, add `--config Debug` to the build and `-C Debug` to `ctest`. The executable is then `build\Debug\clamp_tests.exe`.

## 5. A failure

To see what a failing test looks like, write a wrong expected value on purpose.

Change the expected value of `inside_range` from `5` to `6`, build again and run:

```
$ ./build/clamp_tests
RUN: clamp.inside_range
clamp_test.c:10: error: 6 == clamp(5, 0, 10)
  left:  6
  right: 5
FAIL: clamp.inside_range (1 failure)
RUN: clamp.below_range
PASS: clamp.below_range
RUN: clamp.above_range
PASS: clamp.above_range
SUMMARY: 2 passed, 1 failed, 0 skipped
```

Only `inside_range` fails; the other two still pass. A failing test does not stop the rest from running. The failure output reads like this:

- The `error` line has the form of a compiler diagnostic, `file:line: error: ...`, so editors can jump to it.
- The expression is the assertion's arguments as written; `left` and `right` are their values.
- The exit code is 1, and `ctest` reports the test as failed.

Run that test alone with its name:

```sh
./build/clamp_tests --naos-filter clamp.inside_range
```

Finally, change the value back to `5`.
