# Running tests

The options of every test executable, for choosing tests and changing how they run. The rules are in [command line](../reference/command-line.md).

## Run some tests only

`--naos-filter` with `<suite>.<test>` runs that test only: to run again a test that failed, or to follow one test in a debugger.

```sh
./my_tests --naos-filter parser.empty_object
```

To stop in a test under a debugger, put a breakpoint on `naos_gen_test_<name>`, the name of the function that `NAOS_TEST` defines. See [generated names](../reference/tests.md#generated-names).

In a pattern, `*` matches any string and `?` any one character. Separate several patterns with `:`, and put the patterns to exclude after a `-`. Quote the patterns, such as `'parser.*'`, so that the shell does not expand `*` and `?`.

| Filter | Tests run |
|---|---|
| `parser.*` | Every test of the suite `parser` |
| `*.empty_*` | The tests of every suite whose names start with `empty_` |
| `parser.*:writer.*` | Every test of the suites `parser` and `writer` |
| `parser.*-*.slow_*` | The tests of the suite `parser`, except those whose names start with `slow_` |
| `-*.slow_*` | Every test except those whose names start with `slow_` |

A filter that selects no test is taken for a typo: a usage error, with exit code `2`. `--naos-list` prints the names of every test registered.

## Find hidden dependencies

The tests run in registration order, in one process. A test that unknowingly depends on the state an earlier test left behind may pass only thanks to that order. `--naos-shuffle` shuffles the order of the suites and of the tests within each suite, so that such dependencies show up as failures.

```
$ ./my_tests --naos-shuffle
RANDOM SEED: 4242
...
$ ./my_tests --naos-shuffle --naos-random-seed 4242   # the same order again
```

After a failure, give the seed of the `RANDOM SEED` line to `--naos-random-seed` to run the same order again. Filtering with `--naos-filter` keeps the remaining tests in the same order as before, so you can narrow the run down to the suspects.

## Find flaky tests

A test that fails only once in a while is found by running it many times with `--naos-repeat`. With `--naos-shuffle`, each iteration runs in another order, and the `ITERATION FAILED` line of a failed iteration gives its seed. `--naos-fail-fast` stops at the first failure.

This runs the tests 50 times in random orders and stops at the first failure:

```sh
./my_tests --naos-shuffle --naos-repeat 50 --naos-fail-fast
```

## Run each test in its own process

Running every test in one process is fast, but a crash ends the run and a hang never ends it. To isolate tests, run them in parallel, or give them a time limit, see [CTest and CI](ctest-and-ci.md).
