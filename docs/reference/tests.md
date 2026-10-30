# Tests and suites

A test is a function, grouped with others into a suite. `main()` registers the suites and runs them; nothing is registered automatically. The name of a test is `<suite>.<test>`: it is what `--naos-filter` takes and what the `RUN:` and result lines print.

```c
#include <naos/naos.h>

NAOS_TEST(addition)
{
    NAOS_CHECK_INT_EQ(4, 2 + 2);
}

NAOS_SUITE(math,
    NAOS_ADD_TEST(addition)
)

NAOS_MAIN(
    NAOS_ADD_SUITE(math)
)
```

The tests run in one process: the suites in the order they are registered, and the tests of a suite in the order they are listed. Only the record of failures is reset between tests; `static` variables keep their values from one test to the next.

The functions the macros define are named `naos_gen_<role>_<name>` (see [generated names](#generated-names)), so the names of your tests and suites cannot clash with the API of Naos.

## Tests

```c
NAOS_TEST(name)
{
    /* assertions */
}
```

`NAOS_TEST(name)` defines a test function. It defines `static void naos_gen_test_<name>(void)`, the name a debugger shows ([generated names](#generated-names)). It runs only when a suite lists it with `NAOS_ADD_TEST`.

- Put no `;` after the closing brace. `NAOS_TEST` defines a function, and `-Wpedantic` warns about the extra `;`.
- A test that is defined but listed in no suite is not detected. The compiler's unused-function warning points it out.

## Suites

```c
NAOS_SUITE(name, entry, ...)
NAOS_ADD_TEST(test)
NAOS_DECLARE_SUITE(name);
```

`NAOS_SUITE(name, ...)` defines the suite `name` from entries separated by commas: one `NAOS_ADD_TEST(test)` per test (they run in the order listed) and, if needed, [setup and teardown](#setup-and-teardown). Write it after the tests it lists, usually at the end of the file.

`NAOS_DECLARE_SUITE(name);` declares a suite defined in another file, so that this file can register it.

```c
/* parser_test.c */
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

NAOS_MAIN(
    NAOS_ADD_SUITE(parser)
)
```

- Put no comma after the last entry (it does not compile), and no `;` after `NAOS_SUITE(...)`. `NAOS_DECLARE_SUITE` is a declaration and needs its `;`.
- A suite is defined within one file. A file may define several. Test functions are `static`, so different files can use the same test names.
- Two suites of the same name in one executable are a link error.
- `#ifdef` cannot go between the arguments (undefined behavior in C99). Put the tests that depend on the platform in a suite of their own and wrap the whole suite in `#ifdef`.
- Naos reports the following mistakes on standard error before running any test, all of them at once, and exits with code 2: a name that is empty or holds a character other than a letter, a digit or `_` (names written through the macros are always valid), a test listed twice in one suite, and a suite registered twice.

## Setup and teardown

```c
NAOS_SET_SETUP(name)
NAOS_SET_TEARDOWN(name)
```

Listed among the entries of `NAOS_SUITE`, `NAOS_SET_SETUP(name)` calls the function defined by `NAOS_SETUP(name)` before each test of the suite and `NAOS_SET_TEARDOWN(name)` calls the one defined by `NAOS_TEARDOWN(name)` after each. Both are optional and apply to the whole suite wherever they are written (first is recommended). Each may appear once per suite; a second one is reported before any test runs, with exit code 2.

- `NAOS_SETUP(name)` and `NAOS_TEARDOWN(name)` define `naos_gen_setup_<name>` and `naos_gen_teardown_<name>`. Naming a teardown in `NAOS_SET_SETUP` (or a setup in `NAOS_SET_TEARDOWN`) does not compile ([generated names](#generated-names)).
- When setup ends with a fatal failure or a skip, the body of the test does not run.
- Teardown always runs, even when setup or the body stopped halfway. Write it so that it can clean up what was only partly set up.
- Failures in setup and teardown count as failures of the test.
- There is no setup or teardown run once per suite.

State shared by setup, the tests and teardown lives in `static` variables of the file. An example is in the [setup and teardown guide](../guides/fixtures.md).

## Generated names

```
NAOS_TEST(x)         →  static void naos_gen_test_x(void)
NAOS_SETUP(x)        →  static void naos_gen_setup_x(void)
NAOS_TEARDOWN(x)     →  static void naos_gen_teardown_x(void)
NAOS_SUITE(x, ...)   →  void naos_gen_suite_x(naos_builder_t *builder)
```

The functions the macros define are named `naos_gen_<role>_<name>`. Write the macros with the plain name `x`, as in the examples; `naos_gen_...` is not written in your code.

- The role is part of the name, so `NAOS_SET_SETUP`, `NAOS_SET_TEARDOWN` and `NAOS_ADD_TEST` accept only a function defined for their own role. Naming a teardown in `NAOS_SET_SETUP`, or a setup in `NAOS_ADD_TEST`, is a compile error ("`naos_gen_setup_x` undeclared"). The same name `x` can therefore be used for a test, a setup and a teardown without a clash.
- Tests, setup and teardown are `static`, so different files can use the same names. A suite function has external linkage: it is what `NAOS_DECLARE_SUITE` declares.
- Debuggers, stack traces and profilers show these names, so a breakpoint on a test is a breakpoint on `naos_gen_test_<name>`.
- Names that start with `naos_gen_` are reserved for Naos. Do not define functions of that form yourself, and do not call the generated functions: a test is run by the suite that lists it.

## main()

```c
NAOS_MAIN(entry, ...)
```

`NAOS_MAIN` defines `main()`. It parses the options of Naos, registers the suites of its `NAOS_ADD_SUITE(name)` entries, separated by commas, in the order written, runs them, and returns the exit code. Use it once per executable, with no `;` after it.

## Custom main()

```c
void naos_init(int *argc, char **argv);
NAOS_ADD_SUITE(name);
int  naos_run(void);
```

Call these instead of `NAOS_MAIN` to parse arguments of your own or to choose suites at run time. The order is `naos_init()` once, `NAOS_ADD_SUITE` any number of times, then `naos_run()` once.

- `naos_init()` removes the options of Naos (`--naos-*`) wherever they are in `argv`, moves the remaining arguments up in their order, and decreases `*argc`. `--help` stays in `argv`, so that the program can print its own help too. Help and usage errors are printed here. `naos_run()` then runs no test: it still collects the suites and reports their mistakes, and returns 2 after a usage error or a mistake in the suites, 0 after `--help` otherwise.
- `NAOS_ADD_SUITE(name)` registers a suite to run after those registered before it.
- `naos_run()` collects the suites, reports every mistake in them, runs the selected tests and returns the exit code ([exit codes](command-line.md#exit-codes)). When the memory to collect the suites cannot be allocated, it reports it in one line, runs nothing and returns 1.

An example is in the [custom main() guide](../guides/custom-main.md).

- A call out of order prints a message: `naos_run()` returns 2, and `naos_init()` and `NAOS_ADD_SUITE` exit with code 2. Calling `NAOS_ADD_SUITE` from a test or a suite function is a call out of order too.
- CMake registers the tests with CTest from `--naos-list` run without arguments. Suites that are registered only for some arguments are not in CTest; run the executable directly to use them.
