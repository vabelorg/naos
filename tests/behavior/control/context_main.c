/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * The context of a failure, one file per kind: the note of NAOS_NOTE and the traces of
 * NAOS_TRACE_PUSH.
 */

#include <naos/naos.h>

NAOS_DECLARE_SUITE(note);
NAOS_DECLARE_SUITE(note_reset);
NAOS_DECLARE_SUITE(note_before_teardown);
NAOS_DECLARE_SUITE(trace);
NAOS_DECLARE_SUITE(trace_phases);
NAOS_DECLARE_SUITE(trace_teardown);

NAOS_MAIN(
    NAOS_ADD_SUITE(note),
    NAOS_ADD_SUITE(note_reset),
    NAOS_ADD_SUITE(note_before_teardown),
    NAOS_ADD_SUITE(trace),
    NAOS_ADD_SUITE(trace_phases),
    NAOS_ADD_SUITE(trace_teardown)
)
