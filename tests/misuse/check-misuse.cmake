# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# Compiles a program that must not compile and expects the compiler to name each mistake.
# Defines: CC, INCLUDE_DIR, SOURCE, EXPECT (a list of identifiers the diagnostics must contain).

execute_process(
    COMMAND "${CC}" -fsyntax-only "-I${INCLUDE_DIR}" "${SOURCE}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE output
)
if(result EQUAL 0)
    message(FATAL_ERROR "${SOURCE} compiled, but it must not\n${output}")
endif()
foreach(name IN LISTS EXPECT)
    if(NOT output MATCHES "${name}")
        message(FATAL_ERROR "the compiler did not report '${name}':\n${output}")
    endif()
endforeach()
