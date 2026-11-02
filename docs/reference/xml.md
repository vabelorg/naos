# JUnit XML

With `--naos-output xml:<path>`, an executable built with Naos also writes the results of its run to the file `<path>` as JUnit XML, which CI services and test report tools read. The human-readable output on standard output does not change.

This file is the output meant for tools. What the common tools that read JUnit XML process (the tests, their results and counts, the message and the location of each failure) is kept compatible ([version](version.md)); the rest is written for people ([compatibility](#compatibility)). A tool that reads the results should read this file, not standard output ([output](output.md)).

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
<testsuites name="<program>" tests="<n>" failures="<n>" errors="0" skipped="<n>" time="<s>">
  <testsuite name="<suite>" tests="<n>" failures="<n>" errors="0" skipped="<n>" time="<s>">
    <properties>
      <property name="iteration" value="<i>"/>
      <property name="random_seed" value="<seed>"/>
    </properties>
    <testcase classname="<suite>" name="<test>" time="<s>" file="<file>" line="<line>">
      <failure type="error" message="<text>">...</failure>
      <skipped message="<message>"/>
    </testcase>
  </testsuite>
</testsuites>
```

| Element | Written | Attributes |
|---|---|---|
| `<testsuites>` | Once, around everything | `name`: the name of the program, `argv[0]` without its directory and without `.exe`. `tests`, `failures`, `skipped`: the test cases of the whole file. `errors`: always `0`. `time`: the sum of the times of the test cases of the whole file |
| `<testsuite>` | For each suite, in each iteration, in the order run | `name`: the suite. `tests`, `failures`, `skipped`, `errors`, `time`: as for `<testsuites>`, for its test cases |
| `<properties>` | In a `<testsuite>`, when `--naos-repeat` is 2 or more, or with `--naos-shuffle` | Holds `<property name="iteration">` (from 1, with `--naos-repeat` 2 or more) and `<property name="random_seed">` (with `--naos-shuffle`) |
| `<testcase>` | For each test run | `classname`: the suite. `name`: the test. `time`: the time of the test. `file`, `line`: where its first failure or skip happened, left out when there is none |
| `<failure>` | For each failure of a failed test, in the order they happened | `type`: always `error`. `message`: the text of the `error` diagnostic line ([output](output.md#diagnostic-lines)): the expression of the assertion, the message of `NAOS_FAIL`, or the mistake with traces |
| `<skipped>` | In a skipped test | `message`: the message of `NAOS_SKIP` |

A test case holds the `<failure>` elements when the test failed, one `<skipped>` when it was skipped, and nothing when it passed. A test that fails and then skips has its failures only. The tests that `--naos-fail-fast` left out have no test case. Naos writes no `timestamp` and no `<system-out>`, since it does not capture what the tests print.

A `time` is in seconds, with three decimals and rounded down to the millisecond (`time="3.120"`), as the tools of JUnit XML read it. The time of a test case is the time its result line shows on standard output: setup, body and teardown, with the clock that `--help` names ([times](output.md#times)). The time of a suite or of the whole file is the sum of the exact times of its test cases, rounded down once, so it may be a little more than the sum of the times written. The times change from run to run, so the file does too.

The text of a `<failure>` is the diagnostic as standard output writes it: the `error` line, its detail lines with the values, the note and the traces ([output](output.md)). CI services show it as it is, so that a failure reads the same in CI as in a terminal. It is written for people, as standard output is.

## Text

The file is UTF-8. Every text is escaped for XML (`&amp;`, `&lt;`, `&gt;`, and `&quot;` and `&apos;` in attributes; line breaks and tabs in attributes as `&#10;` and `&#9;`; carriage returns as `&#13;`). Valid UTF-8 is kept as it is, so that a message written in any language reads as written. A byte that is not part of valid UTF-8, and a control character that XML 1.0 does not allow (below 0x20, except line breaks, tabs and carriage returns), is written as `\xNN`; such a text cannot always be told apart from one that holds `\xNN` itself. The values of strings are written as on standard output ([value formats](output.md#value-formats)), which leaves only printable ASCII.

## Compatibility

Kept compatible is what the common tools that read JUnit XML process:

- The elements `<testsuites>`, `<testsuite>`, `<testcase>`, `<failure>` and `<skipped>`, how they nest, and which test cases and which `<failure>` and `<skipped>` elements are written ([elements](#elements)).
- Their attributes stated above: `name`, `classname`, the counts `tests`, `failures`, `errors` and `skipped`, `time`, `file`, `line`, `type` and `message`, with what they hold and how their text is escaped ([text](#text)).
- When the file is written, and the exit codes ([when the file is written](#when-the-file-is-written)).

Not kept compatible, since the common tools only show it or leave it out: the text of `<failure>`, which follows standard output, the `<properties>` of a suite, and the layout of the file (indentation, line breaks between elements, the order of attributes). A later version may add attributes, elements and formats besides `xml`; a tool should skip what it does not know.

## Example

Four tests of the self tests of Naos (`tests/behavior/xml/report.c`), run with `--naos-output xml:results.xml --naos-filter 'results.passes:results.fails_twice:results.skips:results.with_context'`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<testsuites name="naos_selftest_report" tests="4" failures="2" errors="0" skipped="1" time="0.001">
  <testsuite name="results" tests="4" failures="2" errors="0" skipped="1" time="0.001">
    <testcase classname="results" name="passes" time="0.000"/>
    <testcase classname="results" name="fails_twice" time="0.000" file="report.c" line="26">
      <failure type="error" message="1 == 2">report.c:26: error: 1 == 2
  left:  1
  right: 2
</failure>
      <failure type="error" message="&quot;hello\n&quot; == &quot;hello&quot;">report.c:27: error: "hello\n" == "hello"
  left:  "hello\n"
  right: "hello"
  first difference: offset 5, end of right
</failure>
    </testcase>
    <testcase classname="results" name="skips" time="0.000" file="report.c" line="32">
      <skipped message="not here"/>
    </testcase>
    <testcase classname="results" name="with_context" time="0.000" file="report.c" line="47">
      <failure type="error" message="0.1 ~= 0.25">report.c:47: error: 0.1 ~= 0.25
  left:  0.1
  right: 0.25
  tolerance: 0.001
  note: locale C
report.c:45: note: inner 2
report.c:44: note: outer
</failure>
      <failure type="error" message="&quot;ab&quot; == &quot;ac&quot;">report.c:48: error: "ab" == "ac"
  left size:  2
  right size: 2
  first difference: offset 1, left 0x62, right 0x63
report.c:45: note: inner 2
report.c:44: note: outer
</failure>
    </testcase>
  </testsuite>
</testsuites>
```

`tests/behavior/xml/expected/all.txt` has the whole suite, and `text.txt` every kind of escape.
