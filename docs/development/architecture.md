# Architecture

How the sources of Naos are laid out and how the modules depend on each other, for those who maintain and extend it. This page is the reference for the module layout: when a module is added or a dependency changes, update it, together with the dependency comment of `src/CMakeLists.txt`.

## Principles

These decide what goes into Naos, before any feature:

- **Easy to adopt, first**: no dependency besides the standard C library, and no compiler extension. A feature that would cost either of them stays out. A dependency is a library to install or link: a function of the operating system that needs neither, on every supported platform, is not one. Such a function is used only when standard C has nothing that does the job, and only in one file, with a fallback on standard C elsewhere. The one so far is the monotonic clock of `clock.c`, since `clock()` measures processor time on POSIX and wall-clock time on Windows.
- **One output for people, one for tools**: standard output is written for people, and may change in any version to read better. Tools read the JUnit XML file of `--naos-output`: what the common tools of JUnit XML process in it is a contract, and the rest, such as the text of a failure, is for people as standard output is ([JUnit XML](../reference/xml.md#compatibility)). Of standard output, only what tools cannot do without stays stable: the start of a diagnostic line, which editors read, the names of `--naos-list`, which CTest discovery reads, and the exit codes ([output](../reference/output.md#what-is-kept-compatible)).
- **A documented public API**: only what the [reference](../reference/README.md) documents is public and kept compatible.
- **A narrow scope**: process isolation, parallel runs and time limits are left to the caller (CTest, scripts, CI). Naos writes one report format, for the results only it knows (where each failure is and what it says); others are added only when a clear need for them shows.

## Directories

| Path | Content |
|---|---|
| `include/naos/` | The public headers. Users are pointed to `naos/naos.h` only |
| `src/` | The implementation and the internal headers |
| `cmake/` | Installed for users: `Naos.cmake` (`naos_add_executable()`, `naos_discover_tests()`), `NaosDiscover.cmake` (the script that lists the tests after a build and registers them with CTest) and the config of `find_package(Naos)`. Used by the build of Naos only, not installed: `NaosVersion.cmake` (the version and what follows from it: compatibility, SOVERSION) and `NaosInstall.cmake` (the install rules) |
| `tests/` | The self tests, in layers by who judges them (`contract/`, `api/`, `unit/`, `behavior/`, `build/`, `examples/`), each with its programs, the `CMakeLists.txt` registering its cases and the expected outputs (`expected/`); `consumer/`, a project using Naos; and their helpers (`harness/`). See [testing](testing.md) |
| `examples/` | Complete projects quoted by the documentation, built and run by the self tests |
| `docs/` | The documentation in English, and its Japanese translation in `docs/ja/` |
| `scripts/` | Scripts for maintaining Naos, not used by its build: `PrepareRelease.cmake` and `ReleaseNotes.cmake` prepare a release and write its notes, see [releasing](release.md); `Lint.cmake` runs the checks that need no build (`CheckMarkdownLinks.cmake`, `CheckPrintfCalls.cmake`, `CheckReleaseScripts.cmake`), see [lint](testing.md#lint); `TestPackage.cmake` tests the package a build installs, see [building](building.md#installing) |
| `.github/` | The workflows (CI and the release) and the templates of issues and pull requests |

## Modules

`src/` is split into one file per concern. A file that implements a public header has its name (`naos/control.h` ↔ `control.c`); the others declare what they share in an internal header of the same name (`diagnostic.c` ↔ `diagnostic.h`). Only an internal header that has a public header of the same name gets `_internal.h` (`suite_internal.h`).

The files form three groups, listed in this order in `src/CMakeLists.txt` and in the table below: the public API (`session.c` to `version.c`), running the tests (`options.c` to `test.c`) and writing what happened (`diagnostic.c` to `value.c`). A new file goes into its group.

| File | Role |
|---|---|
| `session.c` | `naos/session.h`: the order of `naos_init` → `NAOS_ADD_SUITE` → `naos_run` |
| `suite.c` | `naos/suite.h`: registering suites, and finding their mistakes before the run |
| `check.c` | `naos/check.h`: the comparisons of the assertions, and the values printed when they fail |
| `control.c` | `naos/control.h`: `NAOS_NOTE`, `NAOS_TRACE_*`, and the stops of `NAOS_FAIL`, `NAOS_SKIP` and `NAOS_REQUIRE_*` |
| `version.c` | `naos/version.h`: the version at run time |
| `options.c` | The command line (`--naos-*`): parsing, help and usage |
| `runner.c` | The whole run, with repeat, shuffle and fail fast, and the summary |
| `filter.c` | Matching the patterns of `--naos-filter` |
| `order.c` | The order and the seed of `--naos-shuffle` (its own xorshift generator) |
| `test.c` | Running one test: setup, body and teardown, with `setjmp` / `longjmp`, and its time |
| `clock.c` | The clock that times the tests: the monotonic clock of the operating system, or `clock()`. The only file that depends on the platform |
| `diagnostic.c` | One diagnostic (begin → gather details → end), and the results of the test |
| `context.c` | The note (`NAOS_NOTE`) and the stack of traces that go with diagnostics. Prints nothing |
| `report.c` | Tells the events of the run (a test starts, a diagnostic, a test ends, ...) to every writer on, and writes the names of `--naos-list` and the messages of Naos itself (standard error). `writer.h` declares the events and the functions of a writer |
| `human.c` | The writer of the human-readable output: the form of every line on standard output, and which lines a run writes |
| `xml.c` | The writer of the JUnit XML file of `--naos-output xml:<path>`: keeps the results while the tests run and writes the whole file at the end |
| `text.c` | A text that grows as it is added to, for the writers that keep what they write until later |
| `value.c` | How values are written (escaped strings, shortest doubles) |

Dependencies go one way and never form a cycle. An arrow points at the module used. `report` is used by every module that prints something (`session`, `suite`, `options`, `runner` and others), and the suites as registered (`suite_internal.h`) by every module that reads them (`runner`, `filter`, `test`), so the diagram leaves those arrows out. The dependency comment of `src/CMakeLists.txt` lists them all.

```mermaid
flowchart LR
  session --> options --> order
  session --> suite
  session --> runner
  runner --> filter & order
  runner --> test --> diagnostic
  test --> context & clock
  diagnostic --> context
  diagnostic --> report --> xml --> human --> value
  xml & human --> text
  report --> human
  control --> test & diagnostic & context
  check --> diagnostic & value
```

## Key mechanisms

- **Registration**: `NAOS_TEST`, `NAOS_SETUP` and `NAOS_TEARDOWN` define a function whose name holds its role, `naos_gen_<role>_<name>`. `NAOS_ADD_TEST`, `NAOS_SET_SETUP` and `NAOS_SET_TEARDOWN` accept only a function of their own role, so mixing them up does not compile. `NAOS_SUITE` defines a function `naos_gen_suite_<name>` that registers the tests on a builder (`naos_begin_suite`, `naos_add_test`, `naos_set_*`). `NAOS_ADD_SUITE` hands that function to `naos_add_suite`; `naos_run()` calls them to collect the suites, then checks them all before running any test.
- **Stopping**: `test.c` runs setup, body and teardown under `setjmp`. A stop (`naos_stop`) jumps straight back to that `setjmp`, from whichever function the macro was used in.
- **Diagnostics**: every failure goes through `naos_diagnostic_begin()` → `naos_diagnostic_add*()` → `naos_diagnostic_end()`, which adds the note and the traces of `context.c` and tells the whole diagnostic to the writers through `report.c` at its end.
- **Writers**: what the run writes goes through `report.c`, which tells each event to every writer on, in the same order. The modules that run the tests decide what happened and when; only a writer decides how it looks and which of it is written, so each output can change without the others.
- **Times**: `test.c` reads the clock (`clock.c`) once just before setup and once just after teardown, and tells the time with the result of the test; `runner.c` adds the times of a round to its tally, which ends the round. The writers only write them, so standard output and the JUnit XML file always show the same times ([output](../reference/output.md#times)). `clock.c` holds every `#if` of `src/` on the operating system: `QueryPerformanceCounter` on Windows, `clock_gettime(CLOCK_MONOTONIC)` on POSIX (it defines `_POSIX_C_SOURCE` before any include, for `-std=c99`, except on macOS, whose headers declare everything without it), and `clock()` elsewhere; `--help` says which one measures.
- **Formatted output**: every formatted output of `src/` goes through `format.h`, which picks C99 conforming functions of the C library (the `__mingw_` ones on MinGW). `NAOS_PRINTF_FORMAT` of `naos/export.h` declares the printf-like functions of Naos with the same formats, so the check of the compiler matches what Naos prints.
- **State**: shared mutable state lives in `static` variables of each module (`g_` prefix), never in `extern` variables. The one exception is the options, read only after `naos_init`, through `naos_options_get()`.
- **Discovery**: `naos_add_executable()` adds a `POST_BUILD` command that runs `NaosDiscover.cmake` after each build. The script runs `<exe> --naos-list` and writes a CTest file that registers each line of the output as one test. Parameters go through a file, so nothing needs escaping. When listing fails, the build fails, with the cause and what to do in its message, instead of going on with no test ([troubleshooting](../guides/troubleshooting.md#cmake-and-ctest)).
- **Version**: `include/naos/version.h` is the only source of the version. `NaosVersion.cmake` reads it and derives `project(VERSION)`, the package compatibility and the `SOVERSION`.

[Extending](extending.md) tells where each kind of change goes.
