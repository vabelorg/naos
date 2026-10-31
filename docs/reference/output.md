# Output

While the tests run, Naos writes only three kinds of lines to standard output, and their format is kept compatible. The output of `--naos-list` and `--help`, and the `naos: ...` messages on standard error, are not part of this format. Whatever the tests print themselves comes in between, as it is.

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
