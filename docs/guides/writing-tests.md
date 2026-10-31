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

For a type with no assertion of its own, such as a structure, write a small macro that compares with `NAOS_CHECK_TRUE` and prints the values with `NAOS_NOTE`. Make it a macro rather than a function, so that the failure points at the line that uses it. Each argument is evaluated more than once, so do not pass expressions with side effects such as `points[i++]`.

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
SKIP: io.reads_large_file
```

- `NAOS_SKIP` in setup skips every test of the suite at once ([setup and teardown](fixtures.md)).
- A failure wins over a skip: a test that failed before skipping is a `FAIL`.
- When every test run is skipped, the exit code is `77`, which CTest counts as a skip.
- `--naos-fail-fast` does not stop on a skip.
