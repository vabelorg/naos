# Testing

The self tests of Naos: what each layer checks and who judges it, how outputs are compared, and how to add a case. Running them is in [building](building.md).

## Layers

Naos is a test harness, so most of its self tests cannot let Naos judge them: a Naos that judged wrongly would pass them all.

The first directory of `tests/` is the **layer**, which says who judges its cases. The directories under it hold:

- the programs of one area
- the `CMakeLists.txt` registering its cases
- for golden cases, the expected outputs (`expected/<case>.txt`)

| Layer | Judged by | Checks |
|---|---|---|
| `contract/` | Exit codes, regexes and an XML schema, never a golden file | What Naos keeps compatible ([output](../reference/output.md#what-is-kept-compatible), [JUnit XML](../reference/xml.md#compatibility)): the exit codes, the start of a diagnostic line, the names of `--naos-list`, and the JUnit XML file |
| `api/` | Naos itself | The public API, used as a project uses it, in C and C++. One file per public header |
| `unit/` | Naos itself | The internal modules, one file per module: those whose results depend only on their arguments (`value`, `filter`, `shuffle`, `text`, and `plan`, whose order the seed of each round decides), and the writers driven through their events (the JUnit XML file of `xml`, the diagnostics of `human`) |
| `behavior/` | Golden files ([below](#cases-compared-with-an-output)) | Everything the [reference](../reference/README.md) states about running tests, one directory per page: `tests/`, `assertions/`, `control/`, `command-line/`, `output/`, `xml/` |
| `build/` | The compiler and CMake | Programs that must not compile (`misuse/`), the messages of test discovery (`discovery/`), the [CMake functions](../reference/cmake.md#functions) (`functions/`), and Naos added with `add_subdirectory()` (`subproject/`) |
| `examples/` | Both | The projects of `examples/`, and the outputs the documentation quotes ([below](#examples-and-documentation)) |

`tests/consumer/` is not a layer: it is a project using Naos, which gets Naos in two ways:

- `build/subproject/` adds it with `add_subdirectory()`
- `scripts/TestPackage.cmake` adds it with `find_package(Naos <version>)` after installing a build, then builds `consumer_test.c` again with the flags of `pkg-config` alone, without CMake ([building](building.md#installing))

The helpers are in `tests/harness/`:

- `NaosSelftest.cmake`: the functions of the self tests
- `RunCase.cmake`: runs one case
- `NestedProject.cmake`: for the cases that build a project of their own

### Names and labels

The CTest name of a case is `naos.<path>.<case>`, `<path>` being its directory under `tests/` with `/` made `.` (`naos.behavior.assertions.integer.eq`, `naos.contract.exit_code.fail`).

Every case has the label `naos` and the label of its layer; the cases that build a project of their own have the label `slow` too.

Only the tests of the projects of `examples/` keep the names of their executables (`clamp_tests.clamp.inside_range`), since users copy their `CMakeLists.txt`.

```sh
ctest --preset default -L contract     # one layer
ctest --preset default -LE slow        # all but the cases that build a project
ctest --preset default -R '^naos\.behavior\.xml\.'
```

### Trusting Naos

The cases of `contract/` that check the exit codes of a pass, a failure and a skip set up the CTest fixture `naos_trust`, which the tests of `api/` and `unit/` require.

Until a failure is shown to be judged a failure, CTest runs none of the tests that Naos judges itself, and shows them as not run rather than passed. CTest adds the setup to any run that selects one of them, with `-R` or `-L` as well.

## When each check runs

| When | Runs | For |
|---|---|---|
| Before any build | `cmake -P scripts/CheckMarkdownLinks.cmake` ([lint](#lint)) | What the sources alone tell, in seconds |
| While changing Naos | `ctest --preset default`, or a layer of it | Nothing broken, in a few seconds |
| Before sending a change | The presets `default` and `shared`; `release` and `sanitize` for a change to `src/` ([building](building.md#with-the-presets)) | What the type of the library, the optimizer or the sanitizers find |
| Every pull request | CI: lint, then every preset on every platform, and the package installed ([building](building.md#continuous-integration)) | What the platform, the compiler or the oldest CMake supported finds |

## Contract cases

A case of `contract/` is judged by its exit code, regexes or the schema `contract/junit.xsd`, never by a golden file, so that rewriting the golden files with `NAOS_UPDATE_EXPECTED=1` cannot change what it checks. A change to a case there is a change to what Naos keeps compatible, which the [changelog](changelog.md) records.

- **Exit codes**: one case per row of the [exit codes](../reference/command-line.md#exit-codes).
- **The start of a diagnostic line**: the location in the form of the compiler that built the library, with the file as the compiler gave it, then the kind and `: `.
- **`--naos-list`**: `ListRoundTrip.cmake` checks that standard output holds the names only, one per line, and that each name given to `--naos-filter` runs that test and no other, as the JUnit XML file shows. CTest discovery depends on both.
- **The JUnit XML file**: where `xmllint` is found, the files of every outcome, of repeat with shuffle and of fail fast are valid against `junit.xsd`, which states what [compatibility](../reference/xml.md#compatibility) keeps; and the file is written, left empty or not written [when the reference says](../reference/xml.md#when-the-file-is-written). CI installs `xmllint` on Linux; macOS has it.

## Cases compared with an output

A case of `behavior/` usually compares the **whole expected output**, a golden file. `expected/<case>.txt` holds the arguments, the exit code, standard output and standard error, as they are:

```
args: --naos-filter integer.eq
exit code: 1
--- stdout
RUN: integer.eq
integer.c:LINE: error: -1 == 2
  left:  -1
  right: 2
FAIL: integer.eq (1 failure, N ms)
SUMMARY: 0 passed, 1 failed, 0 skipped (N ms)
--- stderr
```

So that the content is the same on every platform, only this is replaced before comparing:

- The location of a diagnostic: `<directory>/<file>:<line>: ` (`(<line>)` with MSVC) becomes `<file>:LINE: `, so that editing a test does not change the expected outputs. `contract/` checks the location as it is.
- The time that ends a result line or `SUMMARY:`, `(<n> ms)` or `, <n> ms)`, becomes `N ms`, since it changes from run to run. `behavior/output/` checks with regexes that the times are measured.
- The clock that the last line of `--help` names becomes `<clock>`, since it depends on the system: `wall-clock time, from a monotonic clock` or `processor time, from clock(), not wall-clock time`, and only those.
- The path of the executable becomes `PROG`. Usage lines, which wrap at a position that depends on the length of the path, are joined into one line.
- CRs are removed.

A line missing from the expected output is a line that must not appear: no list of `REJECT` regexes is needed. To check that a place is never reached, print `reached-...` there: its absence from the expected output checks it.

Only output that changes between runs or platforms (pointer values, a seed taken from the time, numbers whose width depends on `long`, output that grows when tests are added) is matched with **regexes** instead (`naos_expect_match`).

A case of a file the executable writes, such as the XML of `--naos-output`, gives `OUTPUT_FILE <name>` to `naos_expect`:

- `{output}` in its arguments stands for that file in the build directory.
- The file is removed before the run and compared after it, in a third part `--- output` (`[no file]` when none is written).
- The locations are made the same in the same way (`file="<file>" line="LINE"`) and the times become `time="N.NNN"`; a time in another form stays as it is, and fails the comparison.
- Where `xmllint` is found, the file must also be well-formed XML.

A test that prints bytes beyond ASCII on standard output gives `FILE_ONLY` as well, so that only the file is compared: CMake reads standard output in the code page of the platform, which makes those bytes differ.

## Cases that build a project

A case of `build/functions/` or `build/subproject/` configures, builds and tests a project of its own, with the generator, the compiler and the configuration of the build of Naos (`naos_selftest_add_project_case()` and `NestedProject.cmake`).

- `functions/`: checks each mistake in the arguments of the CMake functions with a configure alone, and how the tests are registered with `Registration.cmake`.
- `subproject/`: builds `tests/consumer/` with Naos added by `add_subdirectory()`, whose configure fails when Naos does not keep to itself.

These cases take seconds each, which the label `slow` lets `-LE slow` leave out.

## Adding a case

Add the case to the layer that judges what changed:

| A change to | Gets a case in |
|---|---|
| What is kept compatible: an exit code, the start of a diagnostic line, `--naos-list`, the JUnit XML file | `contract/`, and the reference and the changelog in the same change |
| Behavior the reference states: an assertion, an option, a line of the output, an element of the XML file | `behavior/<page>/`, the page of the [reference](../reference/README.md) that states it |
| An internal function with many edge cases, such as an escape | `unit/<module>.c` |
| The public API as a project uses it, in C and C++ | `api/<header>.c` and `api/cpp.cpp` |
| The CMake functions, the package or the build | `build/` |
| An example, or an output the documentation quotes | `examples/` |

For a golden case:

1. Write the test in the program of the area. A test meant to fail is written like any other. Test function names must be unique within a file, so a suite may get a file of its own.
2. Register it in `CMakeLists.txt`: `naos_expect_tests(<target> <suite>.<test>...)` to see the output of single tests, or `naos_expect(<case> <target> ARGS ...)` with arguments.
3. Write the expected output with `NAOS_UPDATE_EXPECTED=1 ctest --preset default -R <case>`, **read it and make sure it is right**, then commit it.

When the output format changes, rewrite with `NAOS_UPDATE_EXPECTED=1 ctest --preset default`, and check with `git diff` that every line changed is meant to.

A case that does not match prints the first line that differs (in `[...]`, so that spaces show) and the whole actual output. The cases of `contract/` are left as they are by the update, and fail when the change touches what Naos keeps compatible.

A new area is a new directory, added to the list of the `CMakeLists.txt` of its layer.

## Examples and documentation

The projects of `examples/` are complete projects that users can copy.

- Built alone, each fetches Naos with `FetchContent`.
- Added by `tests/examples/`, each uses the Naos being tested.

Their tests run as CTest tests named after their executables (`clamp_tests.clamp.inside_range`), with the strict warnings of the self tests.

The outputs that the documentation quotes are golden cases of `tests/examples/` (`naos.examples.quickstart`, for example). When an example or its output changes, update the documentation that quotes it (the Japanese translation may follow later, as in [documentation style](docs-style.md#languages)).

The links of the documentation are checked by a [lint](#lint) script, not by a self test.

## Lint

Checks that read the sources and need no build are scripts of `scripts/`, not self tests: they would otherwise wait for the whole build, and run again with every preset and compiler of CI for the same result.

There is one: `scripts/CheckMarkdownLinks.cmake` checks that every relative link of the Markdown files leads to a file that exists, and every `#anchor` to a heading of that file. The `lint` job of CI is that script; it runs from any directory:

```sh
cmake -P scripts/CheckMarkdownLinks.cmake
```

The other rule the sources alone could tell, that `src/` formats its output through `format.h`, is enforced by that header at compile time ([coding style](coding-style.md#formatted-output)).
