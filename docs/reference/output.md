# Output

While the tests run, Naos writes only three kinds of lines to standard output. They are written for people to read, and this page describes them as the current version writes them: apart from what [is kept compatible](#what-is-kept-compatible), they may change in any version, to read better. A tool that reads the results should read the [JUnit XML](xml.md) file of `--naos-output` instead. The output of `--naos-list` and `--help`, and the `naos: ...` messages on standard error, are not part of this output. Whatever the tests print themselves comes in between, as it is.

| Kind | Form | Content |
|---|---|---|
| Flow line | `KEY: value` | The start and result of each test, the iterations and the summary |
| Diagnostic line | `<location> <kind>: <text>` | A failed assertion, a skip, and the traces that go with them |
| Detail line | `  <label>: <value>` | Two spaces first; belongs to the diagnostic line above it |

## Flow lines

| Line | When |
|---|---|
| `RUN: <suite>.<test>` | As a test starts (before setup). Flushed at once, so it stays even if the process dies |
| `PASS: <suite>.<test>` | The test ended with no failure and no skip |
| `FAIL: <suite>.<test> (<N> failure)` | The test recorded failures; `failures` when N is 2 or more |
| `SKIP: <suite>.<test>` | The test was skipped and recorded no failure |
| `SUMMARY: <p> passed, <f> failed, <s> skipped` | At the end of each iteration, when 2 or more tests are selected. After a stop by `--naos-fail-fast`, only the tests run are counted |
| `RANDOM SEED: <seed>` | At the start of each iteration, with `--naos-shuffle` |
| `ITERATION: <i> of <n>` | Before each iteration, when `--naos-repeat` is 2 or more |
| `ITERATION FAILED: <i> of <n>` | After an iteration that failed, when `--naos-repeat` is 2 or more; followed by ` (random seed <seed>)` with `--naos-shuffle` |
| `REPEAT: <f> of <n> iterations failed` | After all the iterations, when `--naos-repeat` is 2 or more; followed by `, <k> not run` after a stop |
| `FAIL FAST: stopped at the first failure; <n> not run` | When `--naos-fail-fast` stopped the run with tests not run |

Within an iteration, the order is `ITERATION:`, `RANDOM SEED:`, each test (from `RUN:` to its result line), `FAIL FAST:`, `SUMMARY:` and `ITERATION FAILED:`; `REPEAT:` comes last. Every diagnostic of the setup, body and teardown of a test comes between its `RUN:` and its result line.

## Diagnostic lines

| Kind | When | Text |
|---|---|---|
| `error` | A failed assertion | The expression of the assertion, from the macro arguments as written (`count < limit` for `NAOS_CHECK_INT_LT(count, limit)`) |
| | `NAOS_FAIL(msg)` | `msg`, or `(no message)` when `NULL` |
| | A mistake with traces | `NAOS_TRACE_PUSH without NAOS_TRACE_POP: <trace>`, `NAOS_TRACE_POP without NAOS_TRACE_PUSH`, `NAOS_TRACE_PUSH beyond 16 levels` |
| `skip` | `NAOS_SKIP(msg)` | `msg`, or `(no message)` when `NULL` |
| `note` | A trace that goes with a failure or a skip | The trace; the location is where it was pushed |

The text of a diagnostic line is never empty and holds no value: the values are in the detail lines that follow.

The location is `<file>:<line>: ` (the GCC form). When the library of Naos is built by a compiler that defines `_MSC_VER` (MSVC, clang-cl), it is `<file>(<line>): ` (the MSVC form). The form is decided when the library is built, not the tests: tests built with MinGW GCC using a Naos built with MSVC print the MSVC form. No option chooses it at run time. The file name is the `__FILE__` the compiler gave.

`error` lines in the GCC form are picked up as they are by problem matchers that read GCC diagnostics, such as `$gcc` of VS Code. Lines in the MSVC form are not picked up by `$msCompile` of VS Code. A problem matcher that picks up both forms is in [the CTest and CI guide](../guides/ctest-and-ci.md#open-failures-in-your-editor).

## Detail lines

A detail line is two spaces, a label, a colon, one space and the value. The labels and the values are those of a failure in [JUnit XML](xml.md#values): `left`, `right`, `first difference` and the others, with strings and doubles written in the [value formats](xml.md#value-formats). `left` and `left size` get one more space when the same block has `right` or `right size`, so that the two values start at the same column. Other labels are not aligned.

## Order

A failure or a skip is printed in this order, then standard output is flushed:

1. The `error` or `skip` line
2. The detail lines of the values
3. `note: <text>`, if a note of `NAOS_NOTE` is there
4. `traces: <n> innermost not kept`, if more than 16 levels of traces are pushed
5. A `note` diagnostic line for each trace pushed, the innermost first

## What is kept compatible

Of what an executable writes, only this is kept compatible ([version](version.md)), since tools cannot do without it:

- The start of a diagnostic line: the [location](#diagnostic-lines) in its form, then the kind, `error`, `skip` or `note`, and `: `, as editors read them ([open failures in your editor](../guides/ctest-and-ci.md#open-failures-in-your-editor)). The text after it may change.
- The names printed by `--naos-list`, one per line ([command line](command-line.md#options)), which the CMake functions of Naos read to register the tests with CTest.
- The [exit codes](command-line.md#exit-codes).

Everything else on this page, such as the flow lines, the detail lines, the order and the summary, may change in any version. The changelog says when it does.

## Example

```
RUN: math.addition
PASS: math.addition
RUN: text.greeting
greet_test.c:12: error: greeting == "hello"
  left:  "hello\n"
  right: "hello"
  first difference: offset 5, end of right
greet_test.c:30: note: locale C
FAIL: text.greeting (1 failure)
SUMMARY: 1 passed, 1 failed, 0 skipped
```
