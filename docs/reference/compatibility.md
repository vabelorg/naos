# Compatibility

What stays the same when you update Naos: what you write, run, build and read that gives the same result in the next version. The rest of the reference states exactly what the current version does; you can rely on it within that version, and a later one may change it, with an entry in the changelog.

## Which versions

The promises below hold from one version to the next:

- Before 1.0, across patch versions: from 0.4.0 to any 0.4.x.
- From 1.0 on, across minor and patch versions: from 1.2.0 to any 1.x.y.

Before 1.0, a new minor version may break any of them ([version](version.md)). The changelog marks each such change `**BREAKING**` and says what to change in your tests or build.

## Principles

- **Only what this page lists is promised.** A detail the reference describes but this page does not list may change in a later version.
- **Naos may get stricter.** A test harness is there to find mistakes, so a later version may report one that an earlier version missed: a test that passed may then fail, or a suite be refused before any test runs. Such a change comes in a minor version, never in a patch version. The changelog says what Naos now reports and how to fix the test, and, where one exists, the option that brings back the earlier judgment, as `--naos-allow-no-assertions` does for a test that runs no assertion. The reverse, letting pass what used to fail, breaks compatibility.
- **Ask for what you need.** If your use relies on something this page does not promise, open an issue: a later version may promise it, or add a way that does.

## Writing tests

The tests you write compile and behave the same:

- The macros and functions of [tests and suites](tests.md), [assertions](assertions.md), [failing, skipping and notes](control.md) and [version](version.md), with their arguments, through `#include <naos/naos.h>`.
- When each assertion passes or fails, and that each of its arguments is evaluated exactly once.
- What a failure does: `NAOS_CHECK*` goes on with the test; `NAOS_REQUIRE*`, `NAOS_FAIL` and `NAOS_SKIP` stop it.
- The order of a run: the suites in the order registered, the tests of a suite in the order listed, setup before each test and teardown after it, teardown even when setup or the body stopped.
- What a test passes, fails or skips with: a failure in setup, the body or teardown fails the test; a skip without a failure skips it; a test [expected to fail](control.md#a-test-expected-to-fail) passes while it fails and fails once it passes.
- The names the macros define, `naos_gen_<role>_<name>`, which debuggers and stack traces show.
- A [custom `main()`](tests.md#custom-main): `naos_init()`, `NAOS_ADD_SUITE` and `naos_run()`, and their order.

## Running tests

A command that works keeps working, with the same meaning:

- The options `--naos-*` and `--help`, their values, and the form `--naos-<option>=<value>` ([command line](command-line.md)).
- The patterns of `--naos-filter`: `*`, `?`, `:` and `-`.
- The [exit codes](command-line.md#exit-codes) `0`, `1`, `2` and `77`, and what each means.
- `--naos-list`: the names `<suite>.<test>`, one per line, in registration order.
- A seed given to `--naos-random-seed` with `--naos-shuffle` runs the same order again, on every platform, with the same version of Naos and the same tests selected.

## Reading the results

A tool that reads the results keeps working:

- The [JUnit XML](xml.md) file of `--naos-output xml:<path>`: when it is written, its elements and how they nest, which test cases and which `<failure>` and `<skipped>` elements it holds, and the attributes `name`, `classname`, `tests`, `failures`, `errors`, `skipped`, `time`, `file`, `line` and `type`. The attribute `message` is there wherever the reference says, but its text is not promised.
- A run that ends before its results are written, by a crash, by `exit()` or by a time limit, leaves a file with an `<error>`, which tools count as an error ([placeholder](xml.md#placeholder)). The name of its test case and its text are not promised.
- The start of a diagnostic line on standard output, which editors and problem matchers read: the [location](output.md#diagnostic-lines) in its form, or `naos: `, then `error`, `skip` or `note`, and `: ` ([open failures in your editor](../guides/ctest-and-ci.md#open-failures-in-your-editor)).
- The exit codes, as above.

## Building

A build that works keeps working:

- The package `Naos`, found with `find_package(Naos <version>)` as [CMake](cmake.md#package) says, or added with `FetchContent` or `add_subdirectory()`, and its target `Naos::naos`.
- `naos_add_executable()` and `naos_discover_tests()`, with their arguments ([CMake](cmake.md#functions)).
- The tests they register with CTest: named `<prefix><suite>.<test>`, `<target>.` being the default prefix, counted as skipped by CTest when they skip, and with the labels and properties given. Until the executable is built, a failing test stands in for them.
- The options `NAOS_SELFTEST`, `NAOS_INSTALL` and `BUILD_SHARED_LIBS` ([CMake](cmake.md#options)).
- Without CMake: the `pkg-config` module `naos`, whose flags build and link the tests, and the names of the library, `naos`, and `naosd` for a Debug build ([installation](../getting-started/installation.md#other-build-systems)).

## Shared library

- A test executable built with one version runs with the shared library of any later version that has the same `SOVERSION`: `0.<minor>` before 1.0, `<major>` from 1.0 on ([CMake](cmake.md#target)).
- The functions, types and helper macros that the macros use, such as `naos_check_int()`, are kept for that alone. Do not use them in your code: how the macros use them may change.

## Not kept compatible

These may change in any version. Knowing it saves you from relying on them:

- **Standard output, apart from the start of a diagnostic line.** The flow lines (`RUN:`, `PASS:`, `SUMMARY:` and the others), the detail lines and their order are written for people, and change to read better. A tool should read the JUnit XML file instead.
- **The wording of what Naos writes.** The text after the kind of a diagnostic line, the hints, the `naos: ` messages on standard error, the output of `--help`, and the text and `message` of `<failure>`. Do not match on them.
- **The order a seed gives in another version.** To run an order again, use the version of Naos that printed its seed.
