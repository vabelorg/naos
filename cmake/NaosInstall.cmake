# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# Used by the build of Naos only; not installed. Included by the top-level CMakeLists.txt when
# NAOS_INSTALL is on, after naos_read_version().
#
# Installs the library (with its PDB for an MSVC DLL), its headers and LICENSE, and the package
# found by find_package(naos):
#   <libdir>/cmake/naos/naos-config.cmake          imports naos::naos and includes Naos.cmake
#   <libdir>/cmake/naos/naos-config-version.cmake  accepts the versions naos_compatibility allows
#   <libdir>/cmake/naos/Naos.cmake, NaosDiscover.cmake

include_guard(GLOBAL)

include(CMakePackageConfigHelpers)

set(naos_config_dir "${CMAKE_INSTALL_LIBDIR}/cmake/naos")

install(TARGETS naos
    EXPORT naos-targets
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)
# The PDB of a DLL, so the installed DLL can be debugged once the build tree is gone. Only Debug and
# RelWithDebInfo make one, hence OPTIONAL.
get_target_property(naos_type naos TYPE)
if(MSVC AND naos_type STREQUAL "SHARED_LIBRARY")
    install(FILES "$<TARGET_PDB_FILE:naos>" DESTINATION "${CMAKE_INSTALL_BINDIR}" OPTIONAL)
endif()
install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/naos" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
# The MIT License asks for its notice in every copy, binaries included.
install(FILES "${PROJECT_SOURCE_DIR}/LICENSE" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/licenses/naos")
install(EXPORT naos-targets
    NAMESPACE naos::
    DESTINATION "${naos_config_dir}"
)

configure_package_config_file("${PROJECT_SOURCE_DIR}/cmake/naos-config.cmake.in"
    "${PROJECT_BINARY_DIR}/naos-config.cmake"
    INSTALL_DESTINATION "${naos_config_dir}"
)
write_basic_package_version_file("${PROJECT_BINARY_DIR}/naos-config-version.cmake"
    COMPATIBILITY ${naos_compatibility}
)
install(FILES
    "${PROJECT_BINARY_DIR}/naos-config.cmake"
    "${PROJECT_BINARY_DIR}/naos-config-version.cmake"
    "${PROJECT_SOURCE_DIR}/cmake/Naos.cmake"
    "${PROJECT_SOURCE_DIR}/cmake/NaosDiscover.cmake"
    DESTINATION "${naos_config_dir}"
)
