# Assertions

Every assertion comes in two forms:

- `NAOS_CHECK*` records the failure and goes on with the test.
- `NAOS_REQUIRE*` records the failure and stops the test. A failure of `NAOS_REQUIRE*` is a **fatal failure**; [how a test stops](control.md#how-a-test-stops) tells what that means.

Both are expressions of type `void`, so they can also go in a comma expression. Each argument is evaluated exactly once.

A failed assertion prints an `error` line with its location and its arguments as written, followed by its values as detail lines, then the [note](control.md#notes) and the [traces](control.md#traces). [Output](output.md) has the whole format.

```
parser_test.c:42: error: count < limit
  left:  8
  right: 8
```

## Booleans

```c
NAOS_CHECK_TRUE(expr)       NAOS_REQUIRE_TRUE(expr)
NAOS_CHECK_FALSE(expr)      NAOS_REQUIRE_FALSE(expr)
```

`*_TRUE` passes when `expr` is true (non-zero), `*_FALSE` when it is false (zero). `expr` may be a `bool`, an integer or a pointer. No value is printed. The `error` line shows `expr` as written for `*_TRUE`, and as `!(expr)` for `*_FALSE`.

```
parser_test.c:57: error: !(is_empty(list))
```

For a type without its own comparison, a macro combining `NAOS_CHECK_TRUE` with `NAOS_NOTE` prints both sides when it fails. Being a macro rather than a function, it makes the `error` line point at the place it is used.

```c
#define CHECK_POINT_EQ(a, b) \
    do \
    { \
        NAOS_NOTE("left (%d, %d), right (%d, %d)", (a).x, (a).y, (b).x, (b).y); \
        NAOS_CHECK_TRUE(point_eq(&(a), &(b))); \
    } while (0)
```

## Integers

```c
NAOS_CHECK_INT_EQ(a, b)     NAOS_REQUIRE_INT_EQ(a, b)
NAOS_CHECK_INT_NE(a, b)     NAOS_REQUIRE_INT_NE(a, b)
NAOS_CHECK_INT_LT(a, b)     NAOS_REQUIRE_INT_LT(a, b)
NAOS_CHECK_INT_LE(a, b)     NAOS_REQUIRE_INT_LE(a, b)
NAOS_CHECK_INT_GT(a, b)     NAOS_REQUIRE_INT_GT(a, b)
NAOS_CHECK_INT_GE(a, b)     NAOS_REQUIRE_INT_GE(a, b)

NAOS_CHECK_UINT_EQ(a, b)    NAOS_REQUIRE_UINT_EQ(a, b)
NAOS_CHECK_UINT_NE(a, b)    NAOS_REQUIRE_UINT_NE(a, b)
NAOS_CHECK_UINT_LT(a, b)    NAOS_REQUIRE_UINT_LT(a, b)
NAOS_CHECK_UINT_LE(a, b)    NAOS_REQUIRE_UINT_LE(a, b)
NAOS_CHECK_UINT_GT(a, b)    NAOS_REQUIRE_UINT_GT(a, b)
NAOS_CHECK_UINT_GE(a, b)    NAOS_REQUIRE_UINT_GE(a, b)
```

The suffixes `EQ`, `NE`, `LT`, `LE`, `GT` and `GE` pass when `a == b`, `a != b`, `a < b`, `a <= b`, `a > b` and `a >= b` respectively. `*_INT_*` converts both arguments to `intmax_t` and `*_UINT_*` to `uintmax_t` before comparing. `left` and `right` are printed in decimal.

Use `*_UINT_*` for unsigned values that may exceed `INTMAX_MAX` (such as `SIZE_MAX`), and `*_INT_*` for values that may be negative. A negative value converted to `uintmax_t` becomes a large positive number, and a large unsigned value converted to `intmax_t` gives an implementation-defined result.

## double

```c
NAOS_CHECK_DOUBLE_NEAR(a, b, tol)
NAOS_REQUIRE_DOUBLE_NEAR(a, b, tol)
```

Converts the arguments to `double` and passes when `|a - b| <= tol`. Equal values, infinities of the same sign included, always pass whatever `tol` is. A NaN on either side always fails.

```
geom_test.c:31: error: area ~= expected
  left:  0.3333333333333333
  right: 0.33
  tolerance: 0.001
```

Values are printed in the shortest decimal form that reads back as the same `double` (`0.1`, `100`, `1e-05`, `1.5e+20`), or as `-0`, `inf`, `-inf` or `nan`. The decimal point is `.` whatever the locale. See [value formats](output.md#value-formats).

## Strings

```c
NAOS_CHECK_STR_EQ(a, b)     NAOS_REQUIRE_STR_EQ(a, b)
NAOS_CHECK_STR_NE(a, b)     NAOS_REQUIRE_STR_NE(a, b)
```

`*_STR_EQ` passes when `a` and `b` are the same bytes up to their terminating NUL; `*_STR_NE` passes when they are not. Two `NULL`s are equal, and `NULL` never equals a string, even an empty one. The comparison is byte by byte, with no locale, case folding or Unicode normalization.

```
test.c:12: error: name == "alice"
  left:  "alicia"
  right: "alice"
  first difference: offset 4, left 0x69, right 0x65
```

- Values are printed in double quotes on one line, with control characters, `"`, `\` and bytes from 0x80 escaped. They are never cut short. `NULL` prints as `(null)`. See [value formats](output.md#value-formats).
- `first difference` gives the position of the first byte that differs. When one value is the beginning of the other, it is `offset N, end of left` or `end of right`. Only `*_STR_EQ` prints it, and only when neither value is `NULL`.
- To compare buffers that may hold a NUL, use the [memory](#memory) comparison.

## Memory

```c
NAOS_CHECK_MEM_EQ(a, a_size, b, b_size)
NAOS_REQUIRE_MEM_EQ(a, a_size, b, b_size)
```

Passes when the `a_size` bytes at `a` are the same as the `b_size` bytes at `b`: the sizes are equal and so is every byte. A pointer whose size is 0 may be `NULL`.

```
codec_test.c:58: error: expected == actual
  left size:  3
  right size: 3
  first difference: offset 1, left 0x02, right 0xFF
```

The bytes themselves are not printed. The third line is one of:

- `first difference: offset N, left 0xXX, right 0xYY`: the first byte that differs.
- `first difference: offset N, end of left` (or `end of right`): one side is the beginning of the other, and shorter. `N` is the shorter size.
- `invalid: left is NULL with non-zero size` (or `right is ...`, `left and right are ...`).

## Pointers

```c
NAOS_CHECK_PTR_EQ(a, b)
NAOS_REQUIRE_PTR_EQ(a, b)
```

Passes when `a` and `b`, converted to `const void *`, are equal. Use it for pointers to objects (standard C cannot convert function pointers to `const void *`).

```
list_test.c:77: error: list.head == &node
  left:  (null)
  right: 0x7ffd812c93a4
```

Values are printed with `%p`, whose form depends on the platform; `NULL` prints as `(null)`.

## Null pointers

```c
NAOS_CHECK_NULL(ptr)        NAOS_REQUIRE_NULL(ptr)
NAOS_CHECK_NOT_NULL(ptr)    NAOS_REQUIRE_NOT_NULL(ptr)
```

`*_NULL` passes when `ptr` is a null pointer, `*_NOT_NULL` when it is not. On failure, `*_NULL` prints the pointer as `value` with `%p`; `*_NOT_NULL` prints no value (it is always NULL when it fails).

`NAOS_REQUIRE_NOT_NULL` is the usual check before using a result:

```c
struct node *node = list_find(&list, 3);

NAOS_REQUIRE_NOT_NULL(node);
NAOS_CHECK_INT_EQ(3, node->value);
```
