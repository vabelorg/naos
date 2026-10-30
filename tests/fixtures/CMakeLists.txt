# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# Setup and teardown: what runs, what is left out and what counts, one case per arrangement of
# fixtures.c.

set(exe naos_selftest_fixtures)
naos_selftest_program(${exe} fixtures.c)

naos_expect_tests(${exe}
    setup_fails.body setup_skips.body setup_check_continues.body teardown_fails.body
    body_fails.fatal_body teardown_requires.body body_and_teardown_require.fatal_body
)
