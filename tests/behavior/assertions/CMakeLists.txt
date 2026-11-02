# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# The behavior that docs/reference/assertions.md states, checked against golden
# files: a program per subject, and the cases that run it.

# assertions
# Failure messages: the error line and the detail lines of each kind of assertion. Every label is
# followed by its colon and one space, except that left and left size get one more to line up with
# right and right size.

set(exe naos_selftest_assertions)
naos_selftest_program(${exe}
    main.c boolean.c integer.c unsigned_integer.c floating.c string.c memory.c pointer.c comparison.c)

naos_expect_tests(${exe}
    boolean.true_fails boolean.false_fails
    integer.eq integer.ne integer.lt integer.le integer.gt integer.ge
    integer.macro_argument_as_written
    unsigned_integer.eq unsigned_integer.compare
    floating.near floating.nan_never_matches floating.shortest floating.exponent floating.special
    floating.locale
    string.eq string.eq_null string.ne string.ne_null string.escapes string.non_ascii
    string.trailing_space string.left_ends string.long_value
    memory.byte memory.right_shorter memory.left_shorter memory.null_with_size
    pointer.not_null
    comparison.unknown
)

# Pointers are written with %p, whose form depends on the platform (letters and digits, with or
# without 0x); NULL is (null) everywhere.
set(at "(:[0-9]+|\\([0-9]+\\)): ")
naos_expect_match(pointer.eq ${exe} EXIT_CODE 1 ARGS --naos-filter pointer.eq
    EXPECT "pointer\\.c${at}error: &values\\[0\\] == &values\\[1\\].  left:  [0-9A-Za-z]+.  right: [0-9A-Za-z]+.FAIL: ")
naos_expect_match(pointer.eq_null ${exe} EXIT_CODE 1 ARGS --naos-filter pointer.eq_null
    EXPECT "pointer\\.c${at}error: NULL == &value.  left:  \\(null\\).  right: [0-9A-Za-z]+.FAIL: ")
naos_expect_match(pointer.null ${exe} EXIT_CODE 1 ARGS --naos-filter pointer.null
    EXPECT "pointer\\.c${at}error: &value == NULL.  value: [0-9A-Za-z]+.FAIL: ")
