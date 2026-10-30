# Stubs

A function B called by the function A under test can be replaced by a fake written for the tests, a stub: write the stub of B and link it in place of the real B. The tests then decide what B returns and check what A passed to it. The complete example is [examples/stubs](../../examples/stubs/).

## The code under test

```c
/* module_a.c, under test */
#include "module_a.h"

#include "module_b.h"

int func_a(int x)
{
    if (func_b(x) != 0) // the real func_b is in module_b.c
    {
        return A_ERROR;
    }

    return A_OK;
}
```

## The stub

The stub records its calls in variables the tests can read and set:

```c
/* fake_module_b.h */
extern int fake_b_return;   /* returned by func_b */
extern int fake_b_calls;    /* number of calls */
extern int fake_b_last_arg; /* argument of the last call */
```

```c
/* fake_module_b.c */
#include "fake_module_b.h"

#include "module_b.h"

int fake_b_return;
int fake_b_calls;
int fake_b_last_arg;

int func_b(int x)
{
    fake_b_calls++;
    fake_b_last_arg = x;

    return fake_b_return;
}
```

## The tests

```c
/* module_a_test.c */
#include <naos/naos.h>

#include "fake_module_b.h"
#include "module_a.h"

/* The stub keeps its values from the previous test. */
NAOS_SETUP(setup)
{
    fake_b_return = 0;
    fake_b_calls = 0;
    fake_b_last_arg = 0;
}

NAOS_TEST(passes_x_to_b)
{
    NAOS_CHECK_INT_EQ(A_OK, func_a(3));
    NAOS_CHECK_INT_EQ(1, fake_b_calls);
    NAOS_CHECK_INT_EQ(3, fake_b_last_arg);
}

NAOS_TEST(propagates_b_failure)
{
    fake_b_return = -1;

    NAOS_CHECK_INT_EQ(A_ERROR, func_a(3));
}

NAOS_SUITE(module_a,
    NAOS_SET_SETUP(setup),
    NAOS_ADD_TEST(passes_x_to_b),
    NAOS_ADD_TEST(propagates_b_failure)
)

NAOS_MAIN(
    NAOS_ADD_SUITE(module_a)
)
```

```cmake
# fake_module_b.c in place of the real module_b.c
naos_add_executable(module_a_tests SOURCES module_a_test.c module_a.c fake_module_b.c)
```

## Limits

- The variables of the stub keep their values from the previous test: reset them in setup.
- Linking a library that holds the real B may clash with the stub's symbol. List the sources under test directly instead.
- Only a B defined in another file than A can be replaced. A function of the same file, or a `static` function, cannot.
