# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
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

# The location takes the form of the compiler the library is built with, file(line) for MSVC and
# file:line otherwise, and keeps the path as the compiler gave it; the golden cases hide both.
if(MSVC)
    set(location_kept "integer\\.c\\([0-9]+\\): error: ")
    set(location_other "integer\\.c:[0-9]+: ")
else()
    set(location_kept "integer\\.c:[0-9]+: error: ")
    set(location_other "integer\\.c\\([0-9]+\\): ")
endif()
naos_expect_match(location_form ${exe} EXIT_CODE 1 ARGS --naos-filter integer.eq
    EXPECT "${location_kept}" "assertions[/\\\\]integer\\.c" REJECT "${location_other}")
