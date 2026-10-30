# Tutorial

This tutorial tests a small string buffer, step by step, with the things most tests need: several files, `CHECK` and `REQUIRE`, setup and teardown, reading failures, notes, and running only some of the tests. It assumes the [quickstart](quickstart.md) is done. The finished project is [examples/tutorial](../../examples/tutorial/).

## The code to test

`strbuf` is a string buffer that grows as text is appended. Its content is always NUL-terminated:

```c
/* strbuf.h */
struct strbuf
{
    char *data;
    size_t length;   /* bytes in use, not counting the NUL */
    size_t capacity; /* bytes allocated */
};

/* Makes sb an empty buffer. Returns 0, or -1 when out of memory (sb is then zeroed). */
int strbuf_init(struct strbuf *sb);

/* Frees the memory of sb and zeroes it. Safe on a zeroed or already freed buffer. */
void strbuf_free(struct strbuf *sb);

/* Appends text to sb. Returns 0, or -1 when out of memory (sb is then unchanged). */
int strbuf_append(struct strbuf *sb, const char *text);

/* Returns the content of sb as a C string, valid until sb changes. */
const char *strbuf_cstr(const struct strbuf *sb);
```

The implementation is in [strbuf.c](../../examples/tutorial/strbuf.c).

## The project

The tests are in two files: `strbuf_test.c` defines the suite, and `main.c` lists the suites to run. As the project grows, add one file per suite and list each in `main.c`.

```cmake
# CMakeLists.txt, after FetchContent as in the quickstart
add_library(strbuf strbuf.c)
target_include_directories(strbuf PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

naos_add_executable(strbuf_tests
    SOURCES main.c strbuf_test.c
    LIBRARIES strbuf
)
```

```c
/* main.c */
#include <naos/naos.h>

NAOS_DECLARE_SUITE(strbuf);

NAOS_MAIN(
    NAOS_ADD_SUITE(strbuf)
)
```

`NAOS_DECLARE_SUITE(strbuf);` declares the suite defined in another file. Suite names must be unique within an executable; test functions are `static`, so two files may use the same test names.

## Setup and teardown

Suppose every test should start with an empty buffer. Creating and freeing it in each test is tedious. Instead, put the creation in a setup and the freeing in a teardown. Setup runs before each test, and teardown after it:

```c
/* strbuf_test.c */
#include <naos/naos.h>

#include "strbuf.h"

/* Shared by setup, the tests and teardown. */
static struct strbuf sb;

NAOS_SETUP(setup)
{
    NAOS_REQUIRE_INT_EQ(0, strbuf_init(&sb));
}

NAOS_TEARDOWN(teardown)
{
    strbuf_free(&sb);
}
```

- Setup and teardown take no arguments, so keep the state they share with the tests in a file-scope `static` variable.
- If `strbuf_init()` fails, `NAOS_REQUIRE_INT_EQ` stops the test right there, and the test body does not run.
- Teardown runs even when setup or the test stopped halfway.

The suite lists them with the tests, at the end of the file:

```c
NAOS_SUITE(strbuf,
    NAOS_SET_SETUP(setup),
    NAOS_SET_TEARDOWN(teardown),
    NAOS_ADD_TEST(starts_empty),
    NAOS_ADD_TEST(append_joins),
    NAOS_ADD_TEST(append_grows)
)
```

## CHECK and REQUIRE

```c
NAOS_TEST(starts_empty)
{
    NAOS_CHECK_UINT_EQ(0, sb.length);
    NAOS_CHECK_STR_EQ("", strbuf_cstr(&sb));
}

NAOS_TEST(append_joins)
{
    NAOS_REQUIRE_INT_EQ(0, strbuf_append(&sb, "hello, "));
    NAOS_REQUIRE_INT_EQ(0, strbuf_append(&sb, "world"));

    NAOS_CHECK_STR_EQ("hello, world", strbuf_cstr(&sb));
    NAOS_CHECK_UINT_EQ(12, sb.length);
}
```

Each assertion has two forms:

- `NAOS_CHECK_*` records a failure and goes on, so one run shows every value that is wrong.
- `NAOS_REQUIRE_*` records a failure and stops the test. Use it when nothing after the failure would make sense.

Pick the assertion by the type of the values (`*_UINT_*` for `size_t`, `*_STR_*` for strings), so a failure prints them in the right form.

## Reading a failure

Suppose `strbuf_append()` had a bug and forgot to update `length`. Each call would then write at the start of the buffer, so `append_joins` would print:

```
RUN: strbuf.append_joins
strbuf_test.c:32: error: "hello, world" == strbuf_cstr(&sb)
  left:  "hello, world"
  right: "world"
  first difference: offset 0, left 0x68, right 0x77
strbuf_test.c:33: error: 12 == sb.length
  left:  12
  right: 0
FAIL: strbuf.append_joins (2 failures)
```

Both checks failed and both are reported, because `CHECK` went on after the first. For strings, `first difference` points at the first byte that differs. Every line of this output is described in [output](../reference/output.md).

## Notes in loops

When an assertion inside a loop fails, the line number alone does not tell which iteration failed. Add a note with `NAOS_NOTE`, and it is printed with the failure:

```c
NAOS_TEST(append_grows)
{
    int i;

    for (i = 0; i < 100; i++)
    {
        NAOS_NOTE("append %d", i);
        NAOS_REQUIRE_INT_EQ(0, strbuf_append(&sb, "x"));
    }

    NAOS_CHECK_UINT_EQ(100, sb.length);
    NAOS_CHECK_UINT_GT(sb.capacity, sb.length);
}
```

The note is printed only when the next assertion fails, as `note: append 37` for instance; a passing assertion prints nothing. To show where a helper function was called from, through nested calls, use traces instead ([diagnostics](../guides/diagnostics.md)).

## Running only some of the tests

Running only the tests you are working on makes the fix-and-check loop faster. `--naos-list` prints the name of each test:

```
$ ./build/strbuf_tests --naos-list
strbuf.starts_empty
strbuf.append_joins
strbuf.append_grows
```

`--naos-filter` runs only the tests that match a pattern. `*` matches any string:

```
$ ./build/strbuf_tests --naos-filter 'strbuf.append_*'
RUN: strbuf.append_joins
PASS: strbuf.append_joins
RUN: strbuf.append_grows
PASS: strbuf.append_grows
SUMMARY: 2 passed, 0 failed, 0 skipped
```

## Hidden dependencies

All the tests run in one process, so `static` variables keep their values from one test to the next. A test can then come to depend on state an earlier test left behind. `--naos-shuffle` runs the tests in a shuffled order and prints the seed it used. Combined with `--naos-repeat`, each repetition uses a different order:

```sh
./build/strbuf_tests --naos-shuffle --naos-repeat 20
```

Here the setup makes a fresh buffer for each test, so every order passes. [Running tests](../guides/running-tests.md) shows how to replay a failing order.
