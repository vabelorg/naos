# Extending

Where each kind of change goes, and what must change with it. Every change to the public API or the output updates the English reference in the same change; the Japanese translation may follow later ([documentation style](docs-style.md#languages)).

## Add or change an output line

The form of every line is written in `report.c`, and only there. The output format is part of the public API, so:

1. Update [output](../reference/output.md).
2. Rewrite the expected outputs of the self tests with `NAOS_UPDATE_EXPECTED=1` and check the diff ([testing](testing.md#adding-a-case)).

## Add an assertion

1. Add the public macros to `naos/check.h`, a `CHECK` and a `REQUIRE` form, and the function that records the result to `check.c`.
2. Comparisons that differ only by their kind go to one function taking a `naos_cmp_t`.
3. Stringify the arguments in the public macro itself. Passing them to a helper macro first and stringifying there with `#` would expand the macros in the arguments.
4. Write the failure with `naos_diagnostic_begin()` → `naos_diagnostic_add*()` → `naos_diagnostic_end()`.
5. Add a file to `tests/assertions/` for its failure output, and a test to `tests/api/check.c`.
6. Update [assertions](../reference/assertions.md) and the table of detail lines in [output](../reference/output.md).

## Add a command-line option

Add one line to the option table of `options.c` (`g_option_table`): parsing, the usage line and the help are all made from it. Add cases to `tests/cli/` and update [command line](../reference/command-line.md).

## Add an argument to the CTest registration

1. Add the keyword to `cmake_parse_arguments` of both public functions of `Naos.cmake`, and pass it to `_naos_discover()`.
2. If `NaosDiscover.cmake` needs the value, add the keyword to `_naos_discovery_params()`, and to the list and the checks at the start of `NaosDiscover.cmake`. The self tests of `discovery/` write their parameters through the same function, so they stay in step.
3. Update the [CMake reference](../reference/cmake.md).

Functions whose names start with `_naos_` are internal.

## Add a source file

Start it with the copyright notice and `SPDX-License-Identifier: MIT`, then a short comment on what the file is responsible for, and add it to its group in `src/CMakeLists.txt`. A new module, or a new dependency between modules, updates [architecture](architecture.md) and the dependency comment of `src/CMakeLists.txt`.

## Change the version

Edit `include/naos/version.h` only. `project(VERSION)`, the package compatibility (`SameMinorVersion` before 1.0, `SameMajorVersion` from 1.0 on) and the `SOVERSION` of the shared library (`0.<minor>` before 1.0, `<major>` from 1.0 on) follow from it through `NaosVersion.cmake`. A release has its own steps: see [releasing](release.md).
