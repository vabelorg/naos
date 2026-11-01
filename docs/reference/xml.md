# JUnit XML

With `--naos-output xml:<path>`, an executable built with Naos also writes the results of its run to the file `<path>` as JUnit XML, which CI services and test report tools read. The human-readable output on standard output does not change.

This file is the output meant for tools: its format is kept compatible ([version](version.md)). A tool that reads the results should read this file, not standard output ([output](output.md)).

```
prog --naos-output xml:results.xml
prog --naos-filter 'parser.*' --naos-output=xml:build/parser.xml
```

## When the file is written

- The file is opened before the first test runs, and replaced if it exists. When it cannot be opened, the executable prints `naos: cannot open the file of --naos-output: <path>` and exits with 2 without running any test.
- The results are kept in memory while the tests run, and the whole file is written when the run ends, after the last iteration of `--naos-repeat` or after a stop by `--naos-fail-fast`. When it cannot be written, or memory runs out for it, the executable prints a message on standard error and exits with 1.
- A process that ends during the run, by a crash or by `exit()`, leaves the file empty. The `RUN:` line on standard output then names the test that was running ([troubleshooting](../guides/troubleshooting.md)).
- `--naos-list` and `--help` run no test, and write no file.

The path is relative to the working directory. Under CTest, every test of an executable runs in a process of its own with the same arguments, so they would all write the same file: give `--naos-output` when running the executable directly, or collect CTest's own report with `--output-junit` ([CTest and CI](../guides/ctest-and-ci.md#in-ci)).

## Elements

```xml
<?xml version="1.0" encoding="UTF-8"?>
<testsuites name="<program>" tests="<n>" failures="<n>" errors="0" skipped="<n>">
  <testsuite name="<suite>" tests="<n>" failures="<n>" errors="0" skipped="<n>">
    <properties>
      <property name="iteration" value="<i>"/>
      <property name="random_seed" value="<seed>"/>
    </properties>
    <testcase classname="<suite>" name="<test>" file="<file>" line="<line>">
      <failure type="error" message="<text>">...</failure>
      <skipped message="<message>"/>
    </testcase>
  </testsuite>
</testsuites>
```

| Element | Written | Attributes |
|---|---|---|
| `<testsuites>` | Once, around everything | `name`: the name of the program, `argv[0]` without its directory and without `.exe`. `tests`, `failures`, `skipped`: the test cases of the whole file. `errors`: always `0` |
| `<testsuite>` | For each suite, in each iteration, in the order run | `name`: the suite. `tests`, `failures`, `skipped`, `errors`: as for `<testsuites>`, for its test cases |
| `<properties>` | In a `<testsuite>`, when `--naos-repeat` is 2 or more, or with `--naos-shuffle` | Holds `<property name="iteration">` (from 1, with `--naos-repeat` 2 or more) and `<property name="random_seed">` (with `--naos-shuffle`) |
| `<testcase>` | For each test run | `classname`: the suite. `name`: the test. `file`, `line`: where its first failure or skip happened, left out when there is none |
| `<failure>` | For each failure of a failed test, in the order they happened | `type`: always `error`. `message`: the text of the `error` diagnostic line ([output](output.md#diagnostic-lines)): the expression of the assertion, the message of `NAOS_FAIL`, or the mistake with traces |
| `<skipped>` | In a skipped test | `message`: the message of `NAOS_SKIP` |

A test case holds the `<failure>` elements when the test failed, one `<skipped>` when it was skipped, and nothing when it passed. A test that fails and then skips has its failures only. The tests that `--naos-fail-fast` left out have no test case. Naos writes no `time`, `timestamp` or `<system-out>`, since it measures no time and does not capture what the tests print.

## Failures

The text of a `<failure>` is lines of `<label>: <value>`, in this order:

1. `file: <file>` and `line: <line>`: where the failure happened
2. The [values](#values) of the assertion, such as `left: <value>` and `right: <value>`; none for `NAOS_FAIL`
3. `note: <text>`, if a note of `NAOS_NOTE` is there
4. `traces: <n> innermost not kept`, if more than 16 levels of traces are pushed
5. `trace: <file>:<line>: <text>` for each trace pushed, the innermost first

The labels are written as they are, with no space added to line up the values. More lines may be added in a later version: a tool should skip the labels it does not know.

## Values

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

## Text

The file is UTF-8. Every text is escaped for XML (`&amp;`, `&lt;`, `&gt;`, and `&quot;` and `&apos;` in attributes; line breaks and tabs in attributes as `&#10;` and `&#9;`; carriage returns as `&#13;`).

- **The values of strings** are written as the [value formats](#value-formats) say, which leaves only printable ASCII; the bytes from 0x80 are escaped as `\xNN`.
- **Other texts** (expressions, the messages of `NAOS_FAIL` and `NAOS_SKIP`, notes, traces and file names) keep valid UTF-8 as it is, so that a message written in any language reads as written. A byte that is not part of valid UTF-8, and a control character that XML 1.0 does not allow (below 0x20, except line breaks, tabs and carriage returns), is written as `\xNN`. Such a text cannot always be told apart from one that holds `\xNN` itself.

## Value formats

Strings and doubles are written the same way on every platform and in every locale, here and on standard output. Integers, pointers, the values of memory comparisons, notes, traces, the messages of `NAOS_FAIL` and `NAOS_SKIP`, and the expressions of diagnostic lines are written as they are, outside these rules.

A **string** is written in double quotes on one line and never cut short. `NULL` is `(null)`, without quotes. Bytes from 0x80 are escaped, whether they are valid UTF-8 or not.

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

## Compatibility

The elements, the attributes and the lines of a failure stated on this page are kept compatible. A later version may add attributes, properties, elements and lines of a failure, and formats besides `xml`; a tool should skip what it does not know.

## Example

Four tests of the self tests of Naos (`tests/report/report.c`), run with `--naos-output xml:results.xml --naos-filter 'results.passes:results.fails_twice:results.skips:results.with_context'`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<testsuites name="naos_selftest_report" tests="4" failures="2" errors="0" skipped="1">
  <testsuite name="results" tests="4" failures="2" errors="0" skipped="1">
    <testcase classname="results" name="passes"/>
    <testcase classname="results" name="fails_twice" file="report.c" line="26">
      <failure type="error" message="1 == 2">file: report.c
line: 26
left: 1
right: 2</failure>
      <failure type="error" message="&quot;hello\n&quot; == &quot;hello&quot;">file: report.c
line: 27
left: "hello\n"
right: "hello"
first difference: offset 5, end of right</failure>
    </testcase>
    <testcase classname="results" name="skips" file="report.c" line="32">
      <skipped message="not here"/>
    </testcase>
    <testcase classname="results" name="with_context" file="report.c" line="47">
      <failure type="error" message="0.1 ~= 0.25">file: report.c
line: 47
left: 0.1
right: 0.25
tolerance: 0.001
note: locale C
trace: report.c:45: inner 2
trace: report.c:44: outer</failure>
      <failure type="error" message="&quot;ab&quot; == &quot;ac&quot;">file: report.c
line: 48
left size: 2
right size: 2
first difference: offset 1, left 0x62, right 0x63
trace: report.c:45: inner 2
trace: report.c:44: outer</failure>
    </testcase>
  </testsuite>
</testsuites>
```

`tests/report/expected/all.txt` has the whole suite, and `text.txt` every kind of escape.
