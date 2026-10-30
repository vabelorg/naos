# Command line

Every executable built with Naos (`prog` below) takes these options:

```
prog [--naos-filter <patterns>] [--naos-shuffle] [--naos-random-seed <n>]
     [--naos-repeat <n>] [--naos-fail-fast] [args...]
prog --naos-list
prog --help
```

Without options, it runs every test once, in one process: the suites in the order they are registered, and the tests of a suite in the order they are listed.

## Options

- `--naos-filter <patterns>`: runs only the tests whose whole name `<suite>.<test>` matches the patterns. `*` matches any string (empty, or holding `.`), `?` any one character. Patterns are separated by `:`. The patterns after the first `-` exclude; with nothing before the `-`, every test not excluded is selected. Quote the patterns so that the shell does not expand `*`.
- `--naos-list`: prints the names of the tests, one per line, in registration order, and runs nothing. Whatever the other options, the order is not shuffled and the list is printed once.
- `--naos-shuffle`: shuffles the order of the suites, and the order of the tests within each suite. Tests are never mixed across suites. Each iteration starts by printing its seed as `RANDOM SEED: <n>`; giving that seed to `--naos-random-seed` runs the same order again. The same seed gives the same order on every platform.
- `--naos-random-seed <n>`: the seed of `--naos-shuffle`, from 1 to 99999. 0, or no seed, picks one from the time. A value out of range or not a number is a usage error. Alone, it changes no order.
- `--naos-repeat <n>`: runs the selected tests n times (1 or more), to find flaky tests. The exit code is the result of all the iterations together. With `--naos-shuffle`, each iteration adds 1 to the seed (after 99999 comes 1) for a new order.
- `--naos-fail-fast`: stops after the first failing test, once its teardown has run, and runs no remaining test (with `--naos-repeat`, no remaining iteration either). A skip does not stop it.
- `--help`: prints the usage and exits with 0 without running any test.

An option taking a value can also be written `--naos-filter=<value>`. An empty value (`--naos-filter=`) and `=` after an option that takes no value (`--naos-list=1`) are usage errors.

The options of Naos may go anywhere. The other arguments stay in `argv` in their order, for a [custom `main()`](tests.md#custom-main) to read. An unknown `--naos-*` option and a filter matching no test are usage errors.

```
$ prog --naos-filter 'parser.*-*.slow_*'
$ prog --naos-shuffle --naos-random-seed 4242 --naos-repeat 20
```

## Exit codes

```c
#define NAOS_EXIT_PASS   0
#define NAOS_EXIT_FAIL   1
#define NAOS_EXIT_USAGE  2
#define NAOS_EXIT_SKIP   77
```

| Code | Meaning |
|---|---|
| 0 | No test failed, and not every test run was skipped (a run with no test at all included). Or `--naos-list` or `--help` was given (and the suites hold no mistake) |
| 1 | At least one test failed, or memory could not be allocated |
| 2 | Usage error: a mistake in the arguments or the suites, or a call out of order |
| 77 | Every test run was skipped |

`naos_run()` returns one of these; return it from `main()` as it is. 77 is the code CTest and Automake treat as a skip, and the CMake functions of Naos register the tests with `SKIP_RETURN_CODE 77`.
