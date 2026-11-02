# Writing tests

## Split the tests into files

As tests grow, give each suite a file of its own. The file that holds `main()` declares the suites of the other files with `NAOS_DECLARE_SUITE` and lists them in `NAOS_MAIN`.

```c
/* parser_test.c */
#include <naos/naos.h>

NAOS_TEST(empty_object) { ... }
NAOS_TEST(nested) { ... }

NAOS_SUITE(parser,
    NAOS_ADD_TEST(empty_object),
    NAOS_ADD_TEST(nested)
)
```

```c
/* main.c */
#include <naos/naos.h>

NAOS_DECLARE_SUITE(parser);
NAOS_DECLARE_SUITE(writer);

NAOS_MAIN(
    NAOS_ADD_SUITE(parser),
    NAOS_ADD_SUITE(writer)
)
```

```cmake
naos_add_executable(my_tests SOURCES main.c parser_test.c writer_test.c LIBRARIES my_lib)
```

- Test functions are `static`: files may reuse test names, but the tests of one suite must all be in one file.
- Suite names must be unique within an executable (a duplicate is a link error).
- When a suite gets large, split it into suites with a common prefix, such as `parser_object` and `parser_array`: `--naos-filter 'parser_*'` then selects them all.

## Catch a test or a suite never added

A test is written twice: `NAOS_TEST` defines it and `NAOS_ADD_TEST` adds it to its suite. A suite is too: `NAOS_SUITE`, then `NAOS_ADD_SUITE`. When the second half is missing, the test never runs, and the run still passes.

`naos_add_executable()` and `naos_discover_tests()` make both a build error, with every compiler ([registration check](../reference/cmake.md#registration-check)). After the link, they compare the tests the executable lists with those its sources define, and name the line of each test or suite not added:

```
left_out_test.c:15: error: test left_out is defined but never added, so it never runs: add it with NAOS_ADD_TEST(left_out) to a suite of this file
```

Add what is missing, or remove what is no longer needed. A suite left out on purpose, such as one that a [custom main()](custom-main.md) adds only on an argument, goes in `IGNORE_SUITES`. Without the CMake functions, [a script](../reference/cmake.md#without-the-cmake-functions) runs the same check.

## Choose the assertion

Each type of value has its assertions: integers, unsigned integers, doubles, strings, memory and pointers ([assertions](../reference/assertions.md)). Use the one that fits: when it fails, it prints both values in a form that fits the type.

```c
NAOS_CHECK_STR_EQ("hello", greeting); // greeting = "hello\n"
```

```
greet_test.c:12: error: "hello" == greeting
  left:  "hello"
  right: "hello\n"
  first difference: offset 5, end of left
```

A condition that fits no typed assertion goes in `NAOS_CHECK_TRUE(expr)` or `NAOS_CHECK_FALSE(expr)`; they print no value.

`NAOS_CHECK*` goes on with the test after a failure. When going on makes no sense (reading through a pointer that may be `NULL`, for example), use the matching `NAOS_REQUIRE*`, which stops the test at the failure:

```c
NAOS_REQUIRE_NOT_NULL(node); // stops the test here if node is NULL
NAOS_CHECK_STR_EQ("root", node->name);
```

## Compare a type that has no assertion

For a type with no assertion of its own, such as a structure, write a small macro that compares with `NAOS_CHECK_TRUE` and prints the values with `NAOS_NOTE`.

Make it a macro rather than a function, so that the failure points at the line that uses it. Each argument is evaluated more than once, so do not pass expressions with side effects such as `points[i++]`.

```c
#define CHECK_POINT_EQ(a, b) \
    do \
    { \
        NAOS_NOTE("left (%d, %d), right (%d, %d)", (a).x, (a).y, (b).x, (b).y); \
        NAOS_CHECK_TRUE(point_eq(&(a), &(b))); \
    } while (0)
```

```
geom_test.c:20: error: point_eq(&(expected), &(actual))
  note: left (2, 1), right (1, 1)
```

## Check that the code runs

A test that runs no assertion fails: Naos takes it for a test that checks nothing. When the check is only that the code runs, without crashing or stopping on an `assert`, say so with `NAOS_PASS()`, an assertion that always passes.

```c
NAOS_TEST(free_null_is_harmless)
{
    buffer_free(NULL);   /* the check: this returns */
    NAOS_PASS();
}
```

Write a test first and leave its body for later, and it fails until it checks something: the name alone is not a pass ([a test that runs no assertion](../reference/control.md#a-test-that-runs-no-assertion)).

## Mark a test that must fail for now

A test written before the code fails at first; so does a test for a known bug not fixed yet. Register it with `NAOS_ADD_TEST_XFAIL` and the reason: the run passes while the test fails, and fails as soon as it passes, so that you remove the mark when the code is done, not long after.

```c
NAOS_TEST(parses_negative_numbers)
{
    NAOS_CHECK_INT_EQ(-42, parse_int("-42"));
}

NAOS_SUITE(parser,
    NAOS_ADD_TEST(parses_positive_numbers),
    NAOS_ADD_TEST_XFAIL(parses_negative_numbers, "sign handling not implemented yet")
)
```

```
RUN: parser.parses_negative_numbers
parser_test.c:3: error: -42 == parse_int("-42")
  left:  -42
  right: 42
XFAIL: parser.parses_negative_numbers (0 ms)
```

Read the failure: it should be the one you expect, not a typo. Once `parse_int()` handles the sign, the run fails at the mark, the line to change into `NAOS_ADD_TEST`:

```
parser_test.c:8: error: passed, but is expected to fail
  reason: "sign handling not implemented yet"
XPASS: parser.parses_negative_numbers (0 ms)
```

Prefer it to `NAOS_SKIP` or to leaving the test out of the suite: a skipped test never tells you when it starts passing. Setup and teardown count with the body, and a crash is never an expected failure ([a test expected to fail](../reference/control.md#a-test-expected-to-fail)).

## Skip a test whose conditions are not met

When running a test makes no sense (a file or an environment variable it needs is missing, for example), skip it with `NAOS_SKIP`. A skipped test is not a failure; its result is `SKIP`.

```c
if (getenv("LARGE_FILE") == NULL)
{
    NAOS_SKIP("LARGE_FILE is not set");
}
```

```
io_test.c:8: skip: LARGE_FILE is not set
SKIP: io.reads_large_file (0 ms)
```

- `NAOS_SKIP` in setup skips every test of the suite at once ([setup and teardown](fixtures.md)).
- A failure wins over a skip: a test that failed before skipping is a `FAIL`.
- When every test run is skipped, the exit code is `77`, which CTest counts as a skip.
- `--naos-fail-fast` does not stop on a skip.
