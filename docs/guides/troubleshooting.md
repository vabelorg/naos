# Troubleshooting

Problems by symptom, with their usual causes.

## Compiling and linking

**Errors at `NAOS_SUITE` or `NAOS_MAIN`, or a warning about an extra `;`**

- A comma after the last entry of `NAOS_SUITE` or `NAOS_MAIN` does not compile. Remove it.
- No `;` goes after `NAOS_TEST(...) { }`, `NAOS_SUITE(...)` or `NAOS_MAIN(...)`; `-Wpedantic` warns about one.
- `NAOS_DECLARE_SUITE(name);` does need its `;`.
- `#ifdef` between the entries of `NAOS_SUITE` is not allowed: put the platform-specific tests in a suite of their own and wrap the whole suite ([suites](../reference/tests.md#suites)).

**A link error about a suite defined twice**

Two suites of one executable have the same name. Suite names must be unique within an executable; test names need only be unique within a file.

**A link error about a function defined twice, with a stub**

The real function is linked along with its stub, usually through a library that holds it. List the sources under test directly instead of linking that library ([stubs](stubs.md#limits)).

**A warning about a missing `return` after `NAOS_FAIL`**

GCC, Clang and MSVC know that `NAOS_FAIL` does not return; other compilers may warn. The behavior is the same; add a `return` to silence it.

## Running

**A test never runs**

It is listed in no suite, or its suite is not registered in `main()`. Check with `--naos-list`. The compiler's unused-function warning points at tests that no suite lists.

**Exit code 2, with a message on standard error**

A usage error: an unknown `--naos-*` option, a value that is missing or out of range, a filter that matches no test, or a mistake in the suites (a name used twice, setup given twice). Every mistake is reported before any test runs ([exit codes](../reference/command-line.md#exit-codes)).

**Exit code 77**

Every test run was skipped. CTest counts it as a skip, not a failure.

**Tests pass alone but fail together, or the other way round**

A test depends on state left by another: `static` variables keep their values between tests. Find the pair with `--naos-shuffle` and a fixed seed, then reset the state in setup ([running tests](running-tests.md#find-hidden-dependencies)).

**A crash or a hang ends the whole run**

All the tests share one process. Run them through CTest, which gives each its own process and can set `--timeout` ([CTest and CI](ctest-and-ci.md)). The last `RUN:` line printed names the test that was running.

**Objects are not destroyed in C++ tests**

The stopping macros leave by `longjmp`, which runs no destructor. Keep objects with destructors out of functions that may stop ([C++](cpp.md)).

## CMake and CTest

**`ctest` says no tests were found**

The top-level `CMakeLists.txt` lacks `enable_testing()` or `include(CTest)`, or `ctest` is run in the wrong directory (use `--test-dir build`).

**CTest shows `<target>.NOT_BUILT`, which fails**

The executable is not built yet, or not for the configuration chosen with `-C` (multi-config generators). Build it, for that configuration, then run `ctest` again.

**The build fails at "Discovering Naos tests"**

Listing the tests after the build failed. The message tells why:

- The time limit was reached: raise `DISCOVERY_TIMEOUT`, for example under an emulator ([CMake](../reference/cmake.md#naos_discover_tests)).
- The shared library was not found. On Windows, this shows as `Exit code 0xc0000135` with no output. Put the DLL next to the executable or on the `PATH` before the listing runs ([shared library](shared-library.md#make-the-dll-findable-at-build-time)).

**`find_package(naos 0.0)` does not accept the installed version**

Before 1.0, it accepts 0.0.x only. Install the minor version the project asks for, or update the project.

**Tests added by a command-line argument are not in CTest**

CTest registers the tests listed without arguments. Run the executable directly with the argument ([custom main()](custom-main.md)).

## Editors

**VS Code does not pick up failures**

`$gcc` reads the GCC form `file:line: error:`; a Naos built with MSVC prints `file(line): error:`, which `$msCompile` does not read. Use the problem matcher of [CTest and CI](ctest-and-ci.md#open-failures-in-your-editor), which reads both.
