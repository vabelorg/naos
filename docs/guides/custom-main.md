# Custom main()

To give the test executable arguments of its own (to run slow tests only when asked, for example), write `main()` yourself instead of `NAOS_MAIN`. `naos_init()` removes the options of Naos, so what is left in `argv` belongs to the program. The complete example is [examples/custom-main](../../examples/custom-main/).

```c
#include <naos/naos.h>

#include <stdio.h>
#include <string.h>

NAOS_DECLARE_SUITE(quick);
NAOS_DECLARE_SUITE(slow);

int main(int argc, char **argv)
{
    int i;

    /* Removes the --naos-* options: only the arguments of the program are left. */
    naos_init(&argc, argv);

    /* The shared library loaded may be older than the headers the tests were built with. */
    if (naos_version_number() != NAOS_VERSION_NUMBER)
    {
        fprintf(stderr, "naos %s is loaded, but the tests were built with %s\n",
            naos_version_string(), NAOS_VERSION_STRING);
        return 1;
    }

    NAOS_ADD_SUITE(quick);

    for (i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--slow") == 0)
        {
            NAOS_ADD_SUITE(slow);
        }
    }

    return naos_run();
}
```

```
$ ./custom_main_tests --slow
RUN: quick.adds
PASS: quick.adds
RUN: slow.sums_a_million
PASS: slow.sums_a_million
SUMMARY: 2 passed, 0 failed, 0 skipped
```

The options of Naos still work, anywhere among the arguments:

```sh
./custom_main_tests --slow --naos-filter 'slow.*'
```

- The order is `naos_init()` once, `NAOS_ADD_SUITE` any number of times, then `naos_run()` once; return what `naos_run()` returns.
- `--help` stays in `argv`, so the program can add its own help after that of Naos.
- The example checks, before adding the suites, that the library loaded is the version the tests were built with ([shared library](shared-library.md#check-the-version-loaded)).
- CTest registers only the tests listed when the executable runs without arguments. Suites added by an argument are run by starting the executable directly.

The rules are in [custom main()](../reference/tests.md#custom-main).
