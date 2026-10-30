# Testing

The self tests of Naos: what they check, how they compare outputs, and how to add one. Running them is in [building](building.md).

## Subjects

`tests/` has one directory per subject. A directory holds the programs of its subject, the `CMakeLists.txt` registering its cases and the expected outputs (`expected/<case>.txt`). The CTest name of a case is `naos.<subject>.<case>`.

| Directory | Checks | How |
|---|---|---|
| `api/` | The public API, used as a project uses it (C and C++). One file per public header | The tests judge themselves (one Naos test is one CTest test) |
| `unit/` | The internal functions whose results depend only on their arguments (`value.c`, `filter.c`, `order.c` given its seed). One file per module | The same |
| `cli/` | Options, filters, `--naos-list`, `--help`, usage errors | Output compared |
| `session/` | The order of `naos_init` → `NAOS_ADD_SUITE` → `naos_run`, and mistakes in suites | The same |
| `assertions/` | The failure output of each kind of assertion. One file per kind | The same |
| `control/` | Going on and stopping (CHECK, REQUIRE, FAIL, SKIP), results, exit codes, processes that end halfway | The same |
| `fixtures/` | Setup and teardown | The same |
| `context/` | Notes and traces | The same |
| `runs/` | `--naos-shuffle`, `--naos-repeat`, `--naos-fail-fast` | The same |
| `discovery/` | The messages of `NaosDiscover.cmake` when listing the tests fails | The same |
| `no_cmake/` | Building without CMake, as [installation](../getting-started/installation.md#without-cmake) describes | Built and run |
| `examples/` | The projects of `examples/`, and the outputs the documentation quotes from them | Their tests pass; outputs compared |
| `docs/` | The relative links and anchors of every Markdown file | `check-links.cmake` |
| `misuse/` | Programs that must not compile, such as a teardown named in `NAOS_SET_SETUP` or a setup named in `NAOS_ADD_TEST` | The compiler refuses them and names the mistake |

The helpers are in `tests/support/`: the functions of `NaosSelftest.cmake`, and `check-output.cmake`, which runs one case.

`tests/package/` is not a self test: it is a project that uses the installed package with `find_package(naos <version>)`. CI configures it on its own after installing each build ([building](building.md#installing)).

## Cases compared with an output

A case usually compares the **whole expected output**, a golden file. `expected/<case>.txt` holds the arguments, the exit code, standard output and standard error, as they are:

```
args: --naos-filter integer.eq
exit code: 1
--- stdout
RUN: integer.eq
integer.c:LINE: error: -1 == 2
  left:  -1
  right: 2
FAIL: integer.eq (1 failure)
--- stderr
```

So that the content is the same on every platform, only this is replaced before comparing:

- The location of a diagnostic: `<directory>/<file>:<line>: ` (`(<line>)` with MSVC) becomes `<file>:LINE: `, so that editing a test does not change the expected outputs.
- The path of the executable becomes `PROG`. Usage lines, which wrap at a position that depends on the length of the path, are joined into one line.
- CRs are removed.

A line missing from the expected output is a line that must not appear: no list of `REJECT` regexes is needed. To check that a place is never reached, print `reached-...` there: its absence from the expected output checks it.

Only output that changes between runs or platforms (pointer values, a seed taken from the time, numbers whose width depends on `long`, output that grows when tests are added) is matched with **regexes** instead (`naos_expect_match`).

## Adding a case

1. Write the test in the program of the subject. A test meant to fail is written like any other. Test function names must be unique within a file, so a suite may get a file of its own.
2. Register it in `CMakeLists.txt`: `naos_expect_tests(<target> <suite>.<test>...)` to see the output of single tests, or `naos_expect(<case> <target> ARGS ...)` with arguments.
3. Write the expected output with `NAOS_UPDATE_EXPECTED=1 ctest --preset default -R <case>`, **read it and make sure it is right**, then commit it.

When the output format changes, do the same: rewrite with `NAOS_UPDATE_EXPECTED=1 ctest --preset default`, and check with `git diff` that every line changed is meant to. A case that does not match prints the first line that differs (in `[...]`, so that spaces show) and the whole actual output.

A new subject is a new directory, added to the list of `tests/CMakeLists.txt`.

## Examples and documentation

The projects of `examples/` are complete projects that users can copy. Each fetches Naos with `FetchContent` when built alone, and uses the Naos being tested when `tests/examples/` adds it. Their tests run as CTest tests named after their executables (`clamp_tests.clamp.inside_range`), with the strict warnings of the self tests.

The outputs that the documentation quotes are golden cases of `tests/examples/` (`naos.examples.quickstart`, for example). When an example or its output changes, update the documentation that quotes it (the Japanese translation may follow later, as in [documentation style](docs-style.md#languages)).

`naos.docs.links` checks that every relative link of the Markdown files leads to a file that exists, and every `#anchor` to a heading of that file. It runs with the other self tests.
