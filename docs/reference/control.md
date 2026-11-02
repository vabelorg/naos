# Failing, skipping and notes

What stops a test, what makes it fail or skip, and the notes and traces that go with its failures.

## How a test stops

A failure of `NAOS_REQUIRE*`, `NAOS_FAIL` and `NAOS_SKIP` stop the test with `longjmp`. They work from a function deep inside helpers, or from a function that returns a value, with nothing to write at the callers.

From setup and the body they jump to teardown; from teardown, to the end of teardown.

A failure of `NAOS_REQUIRE*` and `NAOS_FAIL` are **fatal failures**; a failure of `NAOS_CHECK*` and `NAOS_SKIP` are not.

Because they leave every function in between at once:

- The rest of the functions left does not run. Put clean-up such as `free` in teardown, and state in `static` variables of the file.
- In C++, no function they leave may hold an automatic variable with a destructor (skipping destructors with `longjmp` is undefined behavior). They cannot be used in constructors and destructors either.
- They cannot be used outside a running test (before `naos_run()`, or in a suite function): they print a message and exit with code 2.
- They cannot leave from a thread other than the one running the test.

## Fail

```c
NAOS_FAIL(message)
```

Records a fatal failure and stops the test; it does not return. `message` is a `const char *` or `NULL`, printed as `(no message)` when `NULL`.

```c
static int find_index(const int *values, int count, int value)
{
    int i;

    for (i = 0; i < count; i++)
    {
        if (values[i] == value)
        {
            return i;
        }
    }

    NAOS_FAIL("not found");   /* does not return: no return statement needed after it */
}
```

```
search_test.c:14: error: not found
```

GCC, Clang and MSVC do not warn about a missing `return` in a function that ends with `NAOS_FAIL`. Other compilers may, with no change in behavior.

## Skip

```c
NAOS_SKIP(message)
```

Records the test as skipped and stops it, recording no failure; it does not return. `message` is a `const char *` or `NULL`, printed as `(no message)` when `NULL`.

- Called in setup, it keeps the body of the test from running. Teardown still runs.
- A failure wins: a skipped test that recorded a failure is a `FAIL`.
- When every test run was skipped, the exit code is 77, which CTest counts as skipped. `--naos-fail-fast` does not stop on a skip.

An example is in [writing tests](../guides/writing-tests.md#skip-a-test-whose-conditions-are-not-met).

## A test expected to fail

```c
NAOS_ADD_TEST_XFAIL(test, reason)
```

Listed among the entries of `NAOS_SUITE` in place of `NAOS_ADD_TEST`, it registers a test that is expected to fail for now, for `reason`, a string that is not empty: a test written before the code, a known bug not fixed yet, a known problem of one platform. Naos checks the mark: the test passes while it fails, and fails as soon as it passes, so that removing the mark is a step of its own.

| The test… | Result line | Counts as |
|---|---|---|
| Records one or more failures: an assertion, `NAOS_FAIL`, in setup, the body or teardown | `XFAIL: <suite>.<test> (<t> ms)` | A pass |
| Ends with no failure and no skip | `XPASS: <suite>.<test> (<t> ms)`, after a failure at the entry `NAOS_ADD_TEST_XFAIL` | A failure |
| Skips without failing | `SKIP: <suite>.<test> (<t> ms)` | A skip |

```
RUN: parser.parses_negative_numbers
parser_test.c:42: error: passed, but is expected to fail
  reason: "sign handling not implemented yet"
XPASS: parser.parses_negative_numbers (0 ms)
```

- The failures of an `XFAIL` are written as for any test, so that you can see the test fails for the reason you expect, and not for a typo.
- The failure of an `XPASS` is at the entry `NAOS_ADD_TEST_XFAIL` of the suite, the line to change once the test passes, and its value `reason` is the reason given. It is a failure like the others: the exit code is 1, `--naos-fail-fast` stops at it, and `--naos-quiet` writes it.
- An `XFAIL` is not a failure: the exit code is 0 when no test failed, even if every other test was skipped, `--naos-fail-fast` goes on, and `--naos-quiet` leaves it out with its failures. `SUMMARY:` counts it as an expected failure ([output](output.md#flow-lines)), and the [JUnit XML](xml.md) file as skipped.
- A test that runs no assertion did not fail: it is an `XPASS`, whether `--naos-allow-no-assertions` is given or not.
- A crash is never an expected failure. Run in one process, the run ends as for any test; under CTest, the test fails.
- `--naos-list` does not mark these tests, so that its names stay as they are; the JUnit XML file tells them apart.
- An empty or `NULL` reason is reported before any test runs, with exit code 2, as the other mistakes in the suites ([tests and suites](tests.md#suites)).
- For a failure on one platform only, choose the entry with `#ifdef` outside the suite, since `#ifdef` cannot go between its entries:

```c
#if defined(_WIN32)
#define PARSES_PATHS NAOS_ADD_TEST_XFAIL(parses_paths, "drive letters not handled yet")
#else
#define PARSES_PATHS NAOS_ADD_TEST(parses_paths)
#endif

NAOS_SUITE(parser,
    PARSES_PATHS
)
```

An example is in [writing tests](../guides/writing-tests.md#mark-a-test-that-must-fail-for-now).

## A test that runs no assertion

A test that runs no assertion fails, so that a test that checks nothing does not read as a pass: an empty or unfinished test, a loop over a table with no case, an early `return` before the assertions, or a helper holding them that is never called.

```
RUN: parser.parses_negative_numbers
naos: error: no assertion ran
  hint: check something, or call NAOS_PASS() when the check is that the code runs
FAIL: parser.parses_negative_numbers (1 failure, 0 ms)
```

- What counts: every `NAOS_CHECK*` and `NAOS_REQUIRE*` that runs, whether it passes or fails, `NAOS_PASS`, `NAOS_FAIL` and `NAOS_SKIP`. The count is made as the test runs, so an assertion in a helper counts, and one written but never reached does not.
- What does not count: `NAOS_NOTE`, `NAOS_TRACE_PUSH` and `NAOS_TRACE_POP`, and what the test prints.
- Setup, the body and teardown count together: a suite whose setup or teardown checks something for every test has no test reported.
- Only a test that would pass is reported. A test that failed or skipped already says what happened, and is left as it is.
- The failure has no location, since a test has none: its diagnostic line starts with `naos: ` ([output](output.md#diagnostic-lines)), and in the [JUnit XML](xml.md) file its test case has no `file` and `line`.
- It is a failure like the others: the exit code is 1, `--naos-fail-fast` stops at it, and `--naos-quiet` writes it.

`--naos-allow-no-assertions` lets such a test pass, as before Naos 0.4 ([command line](command-line.md#options)); `ALLOW_NO_ASSERTIONS` of the [CMake functions](cmake.md#functions) gives it to every test they register.

### Pass

```c
NAOS_PASS()
```

An assertion that always passes, for a test whose check is that the code runs: one that only checks that a call returns without crashing, for example. It counts as an assertion and does nothing else; the test goes on.

```c
NAOS_TEST(close_twice_is_harmless)
{
    file_t *file = file_open("data.bin");

    file_close(file);
    file_close(file);   /* the check: this returns */
    NAOS_PASS();
}
```

Like any assertion that passes, it uses up the [note](#notes).

## Notes

```c
NAOS_NOTE(format, ...)
```

Makes a note with a `printf` format. When the next assertion fails, the note is printed after its values. The next assertion uses the note up, whether it passes or fails. An example is in [diagnostics](../guides/diagnostics.md#show-the-iteration-of-a-loop).

- `NAOS_FAIL` and `NAOS_SKIP` print the note too, and use it up.
- A note made before the previous one is used replaces it. Each test and each teardown start without a note.
- A format alone, `NAOS_NOTE("text")`, is fine.
- A note longer than 511 bytes is cut short and ends with `...`.
- GCC and Clang warn when the arguments do not match the format.

## Traces

```c
NAOS_TRACE_PUSH(format, ...)
NAOS_TRACE_POP()
```

`NAOS_TRACE_PUSH` pushes a note made with a `printf` format; until the matching `NAOS_TRACE_POP` pops it, every failure and skip carries it. Unlike a [note](#notes), a trace goes with any number of assertions, those in helpers included.

```c
for (i = 0; i < count; i++)
{
    NAOS_TRACE_PUSH("item %d", i);
    check_item(&items[i]);      /* every failure inside carries the trace */
    NAOS_TRACE_POP();
}
```

```
item_test.c:12: error: item->size > 0
  left:  0
  right: 0
item_test.c:40: note: item 3
```

Each trace is printed after the detail lines as a `note` line pointing at the place it was pushed, the innermost first.

- Pair each `PUSH` with a `POP`. Setup and the body share one stack (pushing in setup and popping in the body is fine); teardown starts with an empty stack of its own.
- After leaving through a fatal failure or a skip, the `POP`s skipped need not be made up for.
- A trace longer than 255 bytes is cut short and ends with `...`.

These mistakes are recorded as failures of the test (not fatal ones):

- `error: NAOS_TRACE_PUSH without NAOS_TRACE_POP: <innermost trace>`: setup and the body, or teardown, ended with traces pushed. It is printed at the place the innermost trace was pushed, followed by the outer traces as `note` lines.
- `error: NAOS_TRACE_POP without NAOS_TRACE_PUSH`: a pop with nothing pushed.
- `error: NAOS_TRACE_PUSH beyond 16 levels`: more than 16 levels pushed. The extra levels are counted, so the matching `POP`s can stay as they are, but their text is not kept. A failure while they are pushed ends its detail lines with `traces: <n> innermost not kept`.

`NAOS_TRACE_POP without NAOS_TRACE_PUSH` and `NAOS_TRACE_PUSH beyond 16 levels` print one line only and leave the [note](#notes) for the next assertion.
