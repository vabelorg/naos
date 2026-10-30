/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * Single source of the Naos version: cmake/NaosVersion.cmake reads these three macros for
 * project(VERSION), and derives the package compatibility and the SOVERSION from them.
 * Versions follow Semantic Versioning (MAJOR.MINOR.PATCH), each component < 256. Before 1.0 a
 * minor version may break compatibility.
 */

#ifndef NAOS_VERSION_H
#define NAOS_VERSION_H

#include <naos/export.h>

NAOS_BEGIN_DECLS

#define NAOS_VERSION_MAJOR 0
#define NAOS_VERSION_MINOR 0
#define NAOS_VERSION_PATCH 0

/* Formats a version as "MAJOR.MINOR.PATCH"; each argument is macro-expanded first. */
#define NAOS_VERSION_STRINGIFY_(x) #x
#define NAOS_VERSION_STRINGIFY(major, minor, patch) NAOS_VERSION_STRINGIFY_(major) "." NAOS_VERSION_STRINGIFY_(minor) "." NAOS_VERSION_STRINGIFY_(patch)
#define NAOS_VERSION_STRING NAOS_VERSION_STRINGIFY(NAOS_VERSION_MAJOR, NAOS_VERSION_MINOR, NAOS_VERSION_PATCH)

/* Packs a version into one comparable number, 0xMMNNPP; usable in #if, so no casts. */
#define NAOS_VERSION_ENCODE(major, minor, patch) (((major) * 0x10000UL) + ((minor) * 0x100UL) + (patch))
#define NAOS_VERSION_NUMBER NAOS_VERSION_ENCODE(NAOS_VERSION_MAJOR, NAOS_VERSION_MINOR, NAOS_VERSION_PATCH)

/*
 * Version of the Naos library actually linked at run time, as NAOS_VERSION_STRING and
 * NAOS_VERSION_NUMBER give it. With a shared library this can differ from the macros the caller
 * was compiled against.
 */
NAOS_API const char *naos_version_string(void);
NAOS_API unsigned long naos_version_number(void);

NAOS_END_DECLS

#endif
