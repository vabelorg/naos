# Setup and teardown

When the tests of a suite start and end the same way, give the suite a setup and a teardown with `NAOS_SET_SETUP` and `NAOS_SET_TEARDOWN`. Setup and teardown take no argument: the state they share with the tests goes in `static` variables of the file, like `buf` here.

```c
static struct buffer buf;

NAOS_SETUP(setup)
{
    NAOS_REQUIRE_TRUE(buffer_init(&buf) == 0);
}

NAOS_TEARDOWN(teardown)
{
    buffer_free(&buf);
}

NAOS_TEST(append_grows) { ... }

NAOS_SUITE(buffer,
    NAOS_SET_SETUP(setup),
    NAOS_SET_TEARDOWN(teardown),
    NAOS_ADD_TEST(append_grows)
)
```

- When setup stops halfway, the body of the test does not run.
- Teardown always runs, even when setup or the test stopped halfway. Write it so that it can clean up after a setup that stopped: here, `buffer_free` should be safe on a buffer that is not initialized or already freed.
- Failures in setup and teardown count as failures of the test.

The tests run one after the other in one process, and `static` variables keep their values from one test to the next. To give each test a fresh state, reset it in setup or teardown too.

The [tutorial](../getting-started/tutorial.md#setup-and-teardown) builds a suite this way, and the rules are in [setup and teardown](../reference/tests.md#setup-and-teardown). To skip every test of a suite when its conditions are not met, call `NAOS_SKIP` in setup ([skip a test](writing-tests.md#skip-a-test-whose-conditions-are-not-met)).
