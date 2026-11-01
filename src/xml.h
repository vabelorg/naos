/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * The JUnit XML file of --naos-output xml:<path>.
 */

#ifndef NAOS_XML_H
#define NAOS_XML_H

#include "writer.h"

/* The writer of the JUnit XML file; on with --naos-output xml:<path>, the path given to its begin. */
const naos_writer_t *naos_xml_writer(void);

#endif
