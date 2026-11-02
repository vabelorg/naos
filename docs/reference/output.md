# Output

While the tests run, Naos writes only three kinds of lines to standard output:

| Kind | Form | Content |
|---|---|---|
| Flow line | `KEY: value` | The start and result of each test, the iterations and the summary |
| Diagnostic line | `<location> <kind>: <text>` | A failed assertion, a skip, and the traces that go with them |
| Detail line | `  <label>: <value>` | Two spaces first; belongs to the diagnostic line above it |

They are written for people to read, and this page describes them as the current version writes them: apart from what [is kept compatible](#what-is-kept-compatible), they may change in any version, to read better. A tool that reads the results should read the [JUnit XML](xml.md) file of `--naos-output` instead.

These are not part of this output:

- The output of `--naos-list` and `--help`.
- The `naos: ...` messages on standard error.

Whatever the tests print themselves comes in between, as it is.

## Flow lines

| Line | When |
|---|---|
| `RUN: <suite>.<test>` | As a test starts (before setup). Flushed at once, so it stays even if the process dies |
| `PASS: <suite>.<test> (<t> ms)` | The test ran an assertion and ended with no failure and no skip, or ran none with `--naos-allow-no-assertions` |
| `FAIL: <suite>.<test> (<N> failure, <t> ms)` | The test recorded failures; `failures` when N is 2 or more |
| `SKIP: <suite>.<test> (<t> ms)` | The test was skipped and recorded no failure |
| `SUMMARY: <p> passed, <f> failed, <s> skipped (<t> ms)` | At the end of each iteration, even when it ran a single test or none. After a stop by `--naos-fail-fast`, only the tests run are counted |
| `RANDOM SEED: <seed>` | At the start of each iteration, with `--naos-shuffle` |
| `ITERATION: <i> of <n>` | Before each iteration, when `--naos-repeat` is 2 or more |
| `ITERATION FAILED: <i> of <n>` | After an iteration that failed, when `--naos-repeat` is 2 or more; followed by ` (random seed <seed>)` with `--naos-shuffle` |
| `REPEAT: <f> of <n> iterations failed` | After all the iterations, when `--naos-repeat` is 2 or more; followed by `, <k> not run` after a stop |
| `FAIL FAST: stopped at the first failure; <n> not run` | When `--naos-fail-fast` stopped the run with tests not run |

<t> is a [time](#times) in milliseconds.

Within an iteration, the order is `ITERATION:`, `RANDOM SEED:`, each test (from `RUN:` to its result line), `FAIL FAST:`, `SUMMARY:` and `ITERATION FAILED:`; `REPEAT:` comes last. Every diagnostic of the setup, body and teardown of a test comes between its `RUN:` and its result line.

## With `--naos-quiet`

`--naos-quiet` leaves out the tests that need no attention. The lines that are written are the same as without it, in the same order.

| Line | Written with `--naos-quiet` |
|---|---|
| `RUN:` | Only for a test that fails, just before its first failure |
| `PASS:` | No |
| A test that is skipped: its `RUN:`, `skip` lines and `SKIP:` | No; `SUMMARY:` counts it |
| `FAIL:`, and every diagnostic of a test that fails | Yes |
| `SUMMARY:`, `RANDOM SEED:`, `FAIL FAST:`, `ITERATION FAILED:`, `REPEAT:` | Yes |
| `ITERATION:` | No |

- A test that skips, then fails in teardown, is a `FAIL`: its `skip` line is kept until the test ends, and written before the failure. So a failing test reads the same as without `--naos-quiet`.
- What a test prints itself is written as it is, but with no `RUN:` line before it to tell which test printed it.
- `RUN:` is no longer written as each test starts, so a test that ends the process before any failure leaves no line naming it. The file of `--naos-output` is written only when the run ends, so it does not name it either. Under CTest, each test runs in its own process and CTest names the one that ended; otherwise, run the same command again without `--naos-quiet`.

```
$ prog --naos-quiet
RUN: text.greeting
greet_test.c:12: error: greeting == "hello"
  left:  "hello\n"
  right: "hello"
  first difference: offset 5, end of right
FAIL: text.greeting (1 failure, 1 ms)
SUMMARY: 497 passed, 1 failed, 2 skipped (2318 ms)
```

## Times

The result lines and `SUMMARY:` end with the time the tests took, so that a slow test shows without asking for it.

- The time of a test covers its setup, body and teardown: from just before setup to just after teardown, as the test starts and ends. Writing its lines is not counted.
- The time of `SUMMARY:` is the sum of the times of the tests of its iteration, of those run only after a stop by `--naos-fail-fast`. With `--naos-repeat`, each iteration has its own.
- Both are whole milliseconds, rounded down: a test shorter than a millisecond shows `0 ms`. `SUMMARY:` rounds the sum down once, so it may be a little more than the sum of the times shown.
- The clock is wall-clock time from the monotonic clock of the system, which does not jump when the time of the system is changed.
  - Windows: `QueryPerformanceCounter`
  - macOS and other POSIX systems: `clock_gettime(CLOCK_MONOTONIC)`
  - Elsewhere: Naos falls back on `clock()` of standard C, which is processor time: a test that sleeps or waits shows less than it took.
- The last line of `--help` says which clock is used ([command line](command-line.md)).
- The times change from run to run, so standard output does too. To compare the results of two runs, compare the [JUnit XML](xml.md) files without their `time`, or the output without the times.

The [JUnit XML](xml.md) file has the same times, in seconds.

## Diagnostic lines

| Kind | When | Text |
|---|---|---|
| `error` | A failed assertion | The expression of the assertion, from the macro arguments as written (`count < limit` for `NAOS_CHECK_INT_LT(count, limit)`) |
| | `NAOS_FAIL(msg)` | `msg`, or `(no message)` when `NULL` |
| | A mistake with traces | `NAOS_TRACE_PUSH without NAOS_TRACE_POP: <trace>`, `NAOS_TRACE_POP without NAOS_TRACE_PUSH`, `NAOS_TRACE_PUSH beyond 16 levels` |
| | A test that [ran no assertion](control.md#a-test-that-runs-no-assertion) | `no assertion ran`, without a location |
| `skip` | `NAOS_SKIP(msg)` | `msg`, or `(no message)` when `NULL` |
| `note` | A trace that goes with a failure or a skip | The trace; the location is where it was pushed |

The text of a diagnostic line is never empty and holds no value: the values are in the detail lines that follow.

The location is `<file>:<line>: ` (the GCC form). When the library of Naos is built by a compiler that defines `_MSC_VER` (MSVC, clang-cl), it is `<file>(<line>): ` (the MSVC form). The file name is the `__FILE__` the compiler gave.

A failure without a location, that of a test that ran no assertion, has `naos: ` in place of the location, as compilers name themselves in a diagnostic of no file: `naos: error: no assertion ran`.

The form is decided when the library is built, not the tests: tests built with MinGW GCC using a Naos built with MSVC print the MSVC form. No option chooses it at run time.

`error` lines in the GCC form are picked up as they are by problem matchers that read GCC diagnostics, such as `$gcc` of VS Code. Lines in the MSVC form are not picked up by `$msCompile` of VS Code. A problem matcher that picks up both forms is in [the CTest and CI guide](../guides/ctest-and-ci.md#open-failures-in-your-editor).

## Detail lines

A detail line is two spaces, a label, a colon, one space and the value. `left` and `left size` get one more space when the same block has `right` or `right size`, so that the two values start at the same column. Other labels are not aligned.

| Assertion | Labels, in order |
|---|---|
| `*_INT_*`, `*_UINT_*` | `left`, `right` (decimal) |
| `*_DOUBLE_NEAR` | `left`, `right`, `tolerance` |
| `*_STR_EQ` (neither `NULL`) | `left`, `right`, `first difference` |
| `*_STR_EQ` (one `NULL`), `*_STR_NE` | `left`, `right` |
| `*_PTR_EQ` | `left`, `right` (`%p`, or `(null)`) |
| `*_NULL` | `value` (`%p`) |
| `*_MEM_EQ` | `left size`, `right size`, then `first difference` or `invalid` |
| `*_TRUE`, `*_FALSE`, `*_NOT_NULL` | none |
| A test that ran no assertion | `hint` |

`first difference` is `offset N, left 0xXX, right 0xYY`. When one side is the beginning of the other, it is `offset N, end of left` or `end of right`, N being the shorter length.

## Value formats

Strings and doubles are printed the same way on every platform and in every locale. Integers, pointers, the values of memory comparisons, notes, traces, the messages of `NAOS_FAIL` and `NAOS_SKIP`, and the expressions of diagnostic lines are printed as they are, outside these rules.

A **string** is printed in double quotes on one line and never cut short. `NULL` is `(null)`, without quotes. Bytes from 0x80 are escaped, whether they are valid UTF-8 or not.

| Byte | Written as |
|---|---|
| 0x0A, 0x09, 0x0D | `\n`, `\t`, `\r` |
| 0x22, 0x5C | `\"`, `\\` |
| Other bytes below 0x20, 0x7F, 0x80 to 0xFF | `\x` and two uppercase hexadecimal digits |
| Other bytes from 0x20 to 0x7E | As they are |

A **double** is written in the shortest decimal form that reads back as the same value. With a decimal exponent from -4 to below 17, no exponent is used; otherwise the form is `<digits>e<sign><exponent>`, with at least two exponent digits. No trailing `0` is written, nor a decimal point with no fraction after it. Negative zero is `-0`, the infinities `inf` and `-inf`, every NaN `nan`. The decimal point is always `.`.

| Value | Written as |
|---|---|
| `0.1` | `0.1` |
| `1.0 / 3.0` | `0.3333333333333333` |
| `100.0` | `100` |
| `1e-5` | `1e-05` |
| `1.5e20` | `1.5e+20` |

## Order

A failure or a skip is printed in this order, then standard output is flushed:

1. The `error` or `skip` line
2. The detail lines of the values
3. `note: <text>`, if a note of `NAOS_NOTE` is there
4. `traces: <n> innermost not kept`, if more than 16 levels of traces are pushed
5. A `note` diagnostic line for each trace pushed, the innermost first

## What is kept compatible

Of standard output, only this is kept compatible ([version](version.md)), since tools cannot do without it; the [JUnit XML](xml.md#compatibility) file has its own rules:

- The start of a diagnostic line: the [location](#diagnostic-lines) in its form, or `naos: ` for a failure without a location, then the kind, `error`, `skip` or `note`, and `: `, as editors read them ([open failures in your editor](../guides/ctest-and-ci.md#open-failures-in-your-editor)). The text after it may change.
- The names printed by `--naos-list`, one per line ([command line](command-line.md#options)), which the CMake functions of Naos read to register the tests with CTest.
- The [exit codes](command-line.md#exit-codes).

Everything else on this page, such as the flow lines, the detail lines, the order and the summary, may change in any version. The changelog says when it does.

## Example

```
RUN: math.addition
PASS: math.addition (0 ms)
RUN: text.greeting
greet_test.c:12: error: greeting == "hello"
  left:  "hello\n"
  right: "hello"
  first difference: offset 5, end of right
greet_test.c:30: note: locale C
FAIL: text.greeting (1 failure, 1 ms)
SUMMARY: 1 passed, 1 failed, 0 skipped (1 ms)
```
