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
# Dependencies go one way ("a -> b" means a uses b; docs/development/architecture.md draws the main
# ones). All of them:
#   session    -> options, suite, runner, report
#   suite      -> report
#   check      -> diagnostic, value
#   control    -> context, diagnostic, test
#   options    -> order, report
#   runner     -> options, suite, filter, order, test, report
#   filter     -> suite
#   test       -> suite, context, diagnostic, report
#   diagnostic -> context, report
#   report     -> value
# version, order, context and value use no other module.
add_library(naos ${naos_library_type}
    # Public API
    session.c
    suite.c
    check.c
    control.c
    version.c
    # Running the tests
    options.c
    runner.c
    filter.c
    order.c
    test.c
    # Writing what happened
    diagnostic.c
    context.c
    report.c
    value.c
)
add_library(naos::naos ALIAS naos)

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
