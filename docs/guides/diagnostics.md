# Diagnostics

How to make a failure say where and why it happened, and how to stop a test from a helper. The rules are in [failing, skipping and notes](../reference/control.md).

## Show the iteration of a loop

When an assertion fails inside a loop, its output alone does not tell which iteration failed. Write the loop variable in a `NAOS_NOTE` just before the assertion: the note is printed only if the assertion fails.

```c
for (i = 0; i < count; i++)
{
    NAOS_NOTE("i=%d", i);
    NAOS_CHECK_INT_EQ(expected[i], actual[i]);
}
```

```
parser_test.c:57: error: expected[i] == actual[i]
  left:  4
  right: 5
  note: i=3
```

A note goes with the next assertion only. For several assertions, write a note before each, or use a trace.

## Show which call of a helper failed

When assertions are gathered in a helper function, a failure points inside the helper. To know which call failed, put the call between `NAOS_TRACE_PUSH` and `NAOS_TRACE_POP`. A failure in between is followed by the line of the `NAOS_TRACE_PUSH` and its text. Helpers may push traces of their own; all of them are printed, the innermost first.

```c
static void check_order(const struct order *order)
{
    int i;

    for (i = 0; i < order->item_count; i++)
    {
        NAOS_TRACE_PUSH("item %d", i);
        check_item(&order->items[i]);
        NAOS_TRACE_POP();
    }
}

NAOS_TEST(orders)
{
    int i;

    for (i = 0; i < count; i++)
    {
        NAOS_TRACE_PUSH("order %d", i);
        check_order(&orders[i]);
        NAOS_TRACE_POP();
    }
}
```

A failure of an assertion in `check_item` then tells which item of which order failed:

```
order_test.c:13: error: item->size > 0
  left:  0
  right: 0
order_test.c:22: note: item 3
order_test.c:34: note: order 1
```

Always pair `NAOS_TRACE_PUSH` with `NAOS_TRACE_POP`. When the test stops in between and the `NAOS_TRACE_POP` is never reached, there is nothing to make up for. What happens when they are not paired is in [traces](../reference/control.md#traces).

## Stop the test from a helper

`NAOS_REQUIRE*`, `NAOS_FAIL` and `NAOS_SKIP` stop the test from any depth of helpers. The callers need not check a return value again.

```c
static const char *require_header(const char *data, size_t size)
{
    NAOS_REQUIRE_TRUE(size >= 4);
    NAOS_REQUIRE_MEM_EQ("NAOS", 4, data, 4);

    return data + 4;
}

NAOS_TEST(reads_header)
{
    const char *body = require_header(input, input_size); // nothing after this runs if it fails

    NAOS_CHECK_INT_EQ('v', body[0]);
}
```

When the test stops, the rest of the helper and of the test are skipped together, and teardown runs. Hence:

- Put clean-up, such as freeing what was allocated, in teardown rather than at the end of the test or the helper.
- In C++, put no local variable with a destructor in a function where the test may stop, directly or in a function it calls ([C++](cpp.md)).

`NAOS_FAIL(message)` stops the test with a failure of your own wording, for a case no assertion describes. Where a test jumps and where these macros cannot be used is in [how a test stops](../reference/control.md#how-a-test-stops).
