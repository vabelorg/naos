# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT

# Build as SHARED only when Naos is the top-level project and BUILD_SHARED_LIBS is set.
# The type is always passed explicitly so that an enclosing project's BUILD_SHARED_LIBS
# cannot change it.
if(PROJECT_IS_TOP_LEVEL AND BUILD_SHARED_LIBS)
    set(naos_library_type SHARED)
else()
    set(naos_library_type STATIC)
endif()

# One file per concern. The ones implementing a public header are named after it (session.c for
# naos/session.h); the others declare their functions in an internal header of the same name.
#
# The files form four layers, listed below from the top. A file uses only files of its own layer or
# of a layer below it, never one above, so the layers never form a cycle
# (docs/development/architecture.md tells what each layer is for):
#   1. The run:        session, options, plan, runner, filter, shuffle, version
#   2. The tests:      suite, test, control, check, diagnostic, context
#   3. The output:     report, writer.h, human, xml, text
#   4. Shared parts:   message, clock, value, format.h
#
# Dependencies go one way ("a -> b" means a uses b; docs/development/architecture.md draws the main
# ones). All of them:
#   session    -> options, plan, runner, suite, message
#   options    -> shuffle, report, message, clock
#   plan       -> options, filter, shuffle, suite, message
#   runner     -> options, plan, shuffle, test, report
#   suite      -> message
#   test       -> suite, diagnostic, context, report, message, clock
#   control    -> test, diagnostic, context
#   check      -> diagnostic, value
#   diagnostic -> context, report
#   context    -> writer
#   report     -> writer, human, xml, message
#   writer     -> clock
#   human      -> writer, text, value
#   xml        -> writer, human, text, value
# filter, shuffle, version, text, message, clock and value use no other module. writer.h has no .c:
# it is the events report.c tells and the functions a writer has.
add_library(naos ${naos_library_type}
    # 1. The run
    session.c
    options.c
    plan.c
    runner.c
    filter.c
    shuffle.c
    version.c
    # 2. The tests
    suite.c
    test.c
    control.c
    check.c
    diagnostic.c
    context.c
    # 3. The output
    report.c
    human.c
    xml.c
    text.c
    # 4. Shared parts
    message.c
    clock.c
    value.c
)
add_library(Naos::naos ALIAS naos)

target_include_directories(naos PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_features(naos PUBLIC c_std_99)
set_target_properties(naos PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS NO
    # Only NAOS_API symbols are exported from a shared build.
    C_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    # Debug builds produce naosd.* so they can coexist with release artifacts.
    DEBUG_POSTFIX d
)

# A static Naos needs no definition (see include/naos/export.h).
if(naos_library_type STREQUAL "SHARED")
    target_compile_definitions(naos
        PRIVATE   NAOS_BUILDING
        INTERFACE NAOS_SHARED
    )
    set_target_properties(naos PROPERTIES
        VERSION ${PROJECT_VERSION}
        # From naos_read_version(): libnaos.so.0.0 before 1.0, libnaos.so.1 from 1.0 on.
        SOVERSION ${naos_soversion}
    )
endif()
