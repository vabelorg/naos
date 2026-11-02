# Running tests

The options of every test executable, for choosing tests and changing how they run. The rules are in [command line](../reference/command-line.md).

## Run some tests only

`--naos-filter` with `<suite>.<test>` runs that test only: to run again a test that failed, or to follow one test in a debugger.

```sh
./my_tests --naos-filter parser.empty_object
```

To stop in a test under a debugger, put a breakpoint on `naos_gen_test_<name>`, the name of the function that `NAOS_TEST` defines. See [generated names](../reference/tests.md#generated-names).

In a pattern, `*` matches any string and `?` any one character.

- Separate several patterns with `:`.
- Put the patterns to exclude after a `-`.
- Quote the patterns, such as `'parser.*'`, so that the shell does not expand `*` and `?`.

| Filter | Tests run |
|---|---|
| `parser.*` | Every test of the suite `parser` |
| `*.empty_*` | The tests of every suite whose names start with `empty_` |
| `parser.*:writer.*` | Every test of the suites `parser` and `writer` |
| `parser.*-*.slow_*` | The tests of the suite `parser`, except those whose names start with `slow_` |
| `-*.slow_*` | Every test except those whose names start with `slow_` |

A filter that selects no test is taken for a typo: a usage error, with exit code `2`. `--naos-list` prints the names of every test registered.

## See only what failed

With many tests, the lines of the tests that pass hide the one that failed. `--naos-quiet` writes only the tests that fail, with their diagnostics, and the summary:

```
$ ./my_tests --naos-quiet
RUN: parser.nested
parser_test.c:41: error: depth == 3
  left:  2
  right: 3
FAIL: parser.nested (1 failure, 0 ms)
SUMMARY: 211 passed, 1 failed, 0 skipped (1834 ms)
```

- The tests that are skipped are only counted in the summary.
- A test that ends the process, such as by a crash, leaves no line naming it: run the same command again without `--naos-quiet` to see which one it was.

The lines that are left out are listed in [output](../reference/output.md#with---naos-quiet).

## Find hidden dependencies

The tests run in registration order, in one process. A test that unknowingly depends on the state an earlier test left behind may pass only thanks to that order.

`--naos-shuffle` shuffles the order of the suites and of the tests within each suite, so that such dependencies show up as failures.

```
$ ./my_tests --naos-shuffle
RANDOM SEED: 4242
...
$ ./my_tests --naos-shuffle --naos-random-seed 4242   # the same order again
```

After a failure, give the seed of the `RANDOM SEED` line to `--naos-random-seed` to run the same order again. Filtering with `--naos-filter` keeps the remaining tests in the same order as before, so you can narrow the run down to the suspects.

## Find flaky tests

A test that fails only once in a while is found by running it many times with `--naos-repeat`. With `--naos-shuffle`, each iteration runs in another order, and the `ITERATION FAILED` line of a failed iteration gives its seed.

`--naos-fail-fast` stops at the first failure. This runs the tests 50 times in random orders and stops at the first failure:

```sh
./my_tests --naos-shuffle --naos-repeat 50 --naos-fail-fast
```

## Find slow tests

Each result line ends with the time the test took, setup and teardown included, and `SUMMARY:` with the total:

```
$ ./my_tests
RUN: storage.open_empty
PASS: storage.open_empty (0 ms)
RUN: storage.rebuild_index
PASS: storage.rebuild_index (3120 ms)
SUMMARY: 2 passed, 0 failed, 0 skipped (3120 ms)
```

A test that a change made slower shows on its line. The times also help to choose the time limit of CTest (`--timeout`, [CTest and CI](ctest-and-ci.md)).

- To list the slowest tests, sort the result lines by their time, or the test cases of the [JUnit XML](../reference/xml.md) file of `--naos-output` by `time`.
- Slow tests named after a pattern, such as `slow_*`, can be left out of the edit-and-run loop with `--naos-filter '-*.slow_*'`.

The times are wall-clock time. On a system without a monotonic clock, they are processor time, which `--help` says ([times](../reference/output.md#times)).

## Run each test in its own process

Running every test in one process is fast, but a crash ends the run and a hang never ends it.

To isolate tests, run them in parallel, or give them a time limit, see [CTest and CI](ctest-and-ci.md).
