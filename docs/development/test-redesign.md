# Test redesign

The plan for redesigning the self tests, the build presets, the scripts and CI of Naos, and the documentation that describes them. The current layout may be broken: names of directories, files and CTest tests may change, and CI may grow. This page goes away when the plan is done; [testing](testing.md) and [building](building.md) then describe the result.

## Principles

- **Split by who judges.** A test is judged by Naos itself, by a script outside it, by the compiler or by CMake. The directory, the CTest name and the label of a test follow from it.
- **Separate the contract from the presentation.** What is kept compatible (the exit codes, the start of a diagnostic line, `--naos-list`, the structure of the JUnit XML file) is checked by cases that `NAOS_UPDATE_EXPECTED=1` cannot rewrite. The golden files check the rest, which may change in any version ([output](../reference/output.md#what-is-kept-compatible), [JUnit XML](../reference/xml.md#compatibility)).
- **Every documented promise has a test.** The behavior tests have one directory per page of the [reference](../reference/README.md), so that a rule of a page leads to its cases.
- **No new build option.** Optimized and sanitized builds are presets that set the usual CMake variables.
- **No logic in the YAML of CI.** Every step is a script of `scripts/`, which runs the same locally and in CI.

## When each check runs

| When | Runs | Purpose |
|---|---|---|
| Before any build (seconds) | `cmake -D ROOT=. -P scripts/Lint.cmake` | The checks that read the sources only: the links, the printf calls, and the tests of the release scripts (new) |
| While developing | `ctest --preset default`, narrowed with `-L contract`, `-LE build` and the like | Nothing is broken |
| Every pull request (CI) | The whole matrix below, and the installed package | Problems that depend on the platform or the build |
| Release pull request | The same CI | Nothing more |

## Layout

```
tests/
  CMakeLists.txt          the layers, and the labels they give
  harness/                was support/: NaosSelftest.cmake, RunCase.cmake (was CheckOutput.cmake)
  contract/               new; judged by scripts (regexes, XSD): what is kept compatible
    contract.c  junit.xsd  ListRoundTrip.cmake
  unit/                   judged by Naos: internal functions and the writers
  api/                    judged by Naos: the public API, in C and C++
  behavior/               judged by scripts (golden files): one directory per page of the reference
    tests/                was session/ and fixtures/       (reference/tests.md)
    assertions/           was assertions/                  (reference/assertions.md)
    control/              was control/ and context/        (reference/control.md)
    command-line/         was cli/ and runs/               (reference/command-line.md)
    output/               was quiet/ and timing/           (reference/output.md)
    xml/                  was report/                      (reference/xml.md)
  build/                  judged by the compiler and CMake
    misuse/  no_cmake/  discovery/
    functions/            new: naos_add_executable() and naos_discover_tests()
    subproject/           new: Naos added with add_subdirectory()
  examples/               the projects of examples/, and the outputs the documentation quotes
  consumer/               was package/: one project, used through find_package() or add_subdirectory()
scripts/
  Lint.cmake              new: runs the three checks below (the lint job of CI is this one line)
  CheckMarkdownLinks.cmake  CheckPrintfCalls.cmake
  CheckReleaseScripts.cmake new, with scripts/fixtures/release/
  TestPackage.cmake       new: install, then configure, build and test consumer/; takes in WerrorOption.cmake
  PrepareRelease.cmake  ReleaseNotes.cmake
  fixtures/author-warning/  was tests/package/author-warning/
```

The CTest name of a case is `naos.<layer>.<page>.<case>`, such as `naos.behavior.assertions.integer.eq`: the path of its directory under `tests/`, with `/` made `.`. Every case has the labels `naos` and its layer: `contract`, `unit`, `api`, `behavior`, `build` or `examples`.

## The layers

### contract/ (new)

Every case is judged by its exit code and regexes, so updating the golden files leaves it as it is.

| Contract | Cases |
|---|---|
| Exit codes | A pass gives 0, a failure 1, a mistake in the arguments 2, a mistake in the suites 2, a call out of order 2, every test skipped 77, skips among passes 0, no test selected 0, `--naos-list` and `--help` 0, a file of `--naos-output` that cannot be opened 2 |
| The start of a diagnostic line | `error`, `skip` and `note`, each with the location in the form of the compiler (`f.c:12:` or `f.c(12):`). `assertions/location_form` moves here |
| `--naos-list` | Standard output holds the names only, one per line. Each name given to `--naos-filter` runs that one test and no other (`ListRoundTrip.cmake`): CTest discovery depends on it |
| The structure of the XML file | `junit.xsd` states what [compatibility](../reference/xml.md#compatibility) keeps; `xmllint --schema` checks the files of every outcome, of repeat with shuffle and of fail fast. Where xmllint is found only (Linux in CI) |
| When the XML file is written | A crash leaves it empty, `--naos-list` and `--help` write none, a file that cannot be opened gives 2. `report/list`, `help`, `crash` and `cannot_open` move here |

**Bootstrap**: the cases of the exit codes are `FIXTURES_SETUP naos_trust`, and the tests of `unit/` and `api/` are registered with `PROPERTIES FIXTURES_REQUIRED naos_trust`. Until a failure is shown to be judged a failure, CTest runs none of the tests that Naos judges itself.

### unit/

- Kept: `value`, `filter`, `order`.
- **Added: `text.c`, `xml.c` and `human.c`**, by calling the events of `writer.h` on a writer and comparing the text it makes. The edge cases of XML escaping (invalid UTF-8, control characters XML does not allow, line breaks and tabs in attributes, `]]>`) are easier to cover this way than through a test program. The golden case `xml/text` then keeps one representative test.
- To check first: that a writer can be called on its own, without global state. If it cannot, this addition is dropped.

### api/

- C and C++ stay.
- `run_all` goes; `run_in_any_order` (shuffled, repeated 50 times) covers it.

### behavior/

- **The golden files do not change**: they move only, and the number of cases and a full pass show that nothing was lost.
- One test program per page, still made with `naos_selftest_program()`.
- The `run_all` cases of cli, control and context grow with every test added. Check what each of them is for (the run goes on after failures, each test starts clean, the summary counts them); they are candidates to merge into `control/run_all` alone.

### build/

The CMake functions are the part least tested today.

| Case | Checks |
|---|---|
| `functions/errors` | The errors at configure time: an unknown argument, no such target, an imported target, a second registration, a bad `DISCOVERY_TIMEOUT`, `PROPERTIES` with an odd number of values, no `SOURCES`. None is tested today |
| `functions/registration` | Before the build, `ctest -N` shows `<prefix>NOT_BUILT` with the labels. After it, `ctest --show-only=json-v1` shows `PREFIX`, `LABELS`, `PROPERTIES` and `SKIP_RETURN_CODE 77` applied |
| `subproject` | `consumer/` adds Naos with `add_subdirectory()`: with `BUILD_SHARED_LIBS=ON` outside, Naos stays static; its self tests are not registered; `-Werror` does not reach the enclosing project; no install rules are made; the project builds and its tests pass |

A nested configure uses the generator, compilers, make program and configuration of the outer build. These cases are slower, which the label `build` lets `-LE build` leave out.

### Left out

- Compiling each public header alone: users are pointed to `naos/naos.h` only.
- Coverage as a gate, fuzzing, a nightly job: more upkeep than they are worth at this size.
- `FetchContent` from git: it adds Naos the way `add_subdirectory()` does, which `subproject` checks.
- New CMake options.

## Presets

| Preset | Builds | Why |
|---|---|---|
| `default` | Static, Debug | As now |
| `shared` | Shared, Debug | As now |
| **`release`** (new) | Static, Release | `test.c` uses `setjmp` / `longjmp`: local variables that only optimization breaks, and `-Wclobbered` of GCC (part of `-Wextra`, reported only when optimizing), never show in Debug |
| **`sanitize`** (new) | Static, Debug, `-fsanitize=address,undefined -fno-sanitize-recover=all` | Memory mistakes in escaping and in growing texts. Its test preset sets `ASAN_OPTIONS`. GCC and Clang only |

`CMakePresets.json` stays at version 3: workflow presets need CMake 3.25, which would leave the minimum, 3.21, untested.

## CI

| Job | Builds |
|---|---|
| `lint` | `cmake -D ROOT=. -P scripts/Lint.cmake`, alone |
| `test` | linux-gcc {default, shared, **release**}, linux-clang {default, **sanitize**}, macos-clang {default, shared}, msvc {default, shared, **release**}, **cmake-3.21** (linux-gcc, default) |
| `mingw` | UCRT64 {default, shared}, MINGW64 {default}: MINGW64 is there for the printf of msvcrt, which static checks |

- The installed package is tested with one line, `cmake -D PRESET=<preset> -P scripts/TestPackage.cmake`, in every job but `sanitize`. With CMake 3.21 it checks `find_package(Naos)` with the oldest CMake users may have.
- **The CMake 3.21 job is new**: `cmake_minimum_required(VERSION 3.21)` is never checked today.
- 15 jobs instead of 12 (`lint` 1, `test` 11, `mingw` 3).

## Documentation to update

| File | Change |
|---|---|
| `docs/development/testing.md` | Rewritten: the layers and what each checks, who judges, the labels, adding a case, contract cases that the update of golden files does not touch |
| `docs/development/building.md` | The presets, the examples of `-R` and `-L`, the section on CI |
| `docs/development/extending.md` | The layer that gets a test for each kind of change (it names `tests/cli/` and other old paths) |
| `docs/development/architecture.md` | The rows of `tests/` and `scripts/` in the table of directories |
| `docs/development/release.md` | Step 4 (`tests/package/`), and the tests of the release scripts |
| `docs/CONTRIBUTING.md`, `.github/PULL_REQUEST_TEMPLATE.md` | What to run before sending: `default`, `shared` and `Lint.cmake` |
| `docs/reference/xml.md` | The two references to `tests/report/` |
| `tests/CMakeLists.txt` and the head comments of each `CMakeLists.txt` | The new layout |
| `docs/ja/` | The same pages, in a later pull request as [documentation style](docs-style.md#languages) allows |

`CHANGELOG.md` needs no entry, since users see no change, unless `release` or `sanitize` finds a bug: its fix gets one.

## Steps

Each step is one pull request, with the English documentation it changes.

- [ ] 1. **Harness**: `support/` becomes `harness/`; names made from the path; labels; cases judged by the exit code alone. Every existing case still passes.
- [ ] 2. **Move to `behavior/` and `build/`**: `git mv` only, the golden files unchanged. The number of cases is the same before and after.
- [ ] 3. **`contract/`**, with `junit.xsd` and `FIXTURES_REQUIRED naos_trust`.
- [ ] 4. **`unit/`**: `text`, `xml` and `human`.
- [ ] 5. **`build/functions` and `build/subproject`**; `package/` becomes `consumer/`; `scripts/TestPackage.cmake`.
- [ ] 6. **The presets `release` and `sanitize`**; each bug they find is fixed in a commit of its own.
- [ ] 7. **CI**: `Lint.cmake`, `CheckReleaseScripts.cmake`, the CMake 3.21 job.
- [ ] 8. **The Japanese translation** of the pages changed.

Step 6 will likely find `-Wclobbered` or sanitizer reports of unknown size, so it stays apart from steps 1 to 5.
