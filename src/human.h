/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * The human-readable output: the lines a run writes on stdout.
 */

#ifndef NAOS_HUMAN_H
#define NAOS_HUMAN_H

#include "writer.h"

/* The writer of the human-readable output; it is always on. */
const naos_writer_t *naos_human_writer(void);

#endif
