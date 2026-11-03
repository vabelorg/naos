# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# Used by the build of Naos only; not installed. Included by the top-level CMakeLists.txt when
# NAOS_INSTALL is on, after naos_read_version().
#
# Installs the library (with its PDB for an MSVC DLL), its headers and LICENSE, and the package
# found by find_package(Naos):
#   <libdir>/cmake/Naos/NaosConfig.cmake        imports Naos::naos and includes Naos.cmake
#   <libdir>/cmake/Naos/NaosConfigVersion.cmake accepts the versions naos_compatibility allows
#   <libdir>/cmake/Naos/Naos.cmake, NaosDiscover.cmake
# and, for a project with another build system, <libdir>/pkgconfig/naos.pc (from cmake/naos.pc.in).

include_guard(GLOBAL)

include(CMakePackageConfigHelpers)

set(naos_config_dir "${CMAKE_INSTALL_LIBDIR}/cmake/Naos")

install(TARGETS naos
    EXPORT NaosTargets
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
install(EXPORT NaosTargets
    NAMESPACE Naos::
    DESTINATION "${naos_config_dir}"
)

configure_package_config_file("${PROJECT_SOURCE_DIR}/cmake/NaosConfig.cmake.in"
    "${PROJECT_BINARY_DIR}/NaosConfig.cmake"
    INSTALL_DESTINATION "${naos_config_dir}"
)
write_basic_package_version_file("${PROJECT_BINARY_DIR}/NaosConfigVersion.cmake"
    COMPATIBILITY ${naos_compatibility}
)
install(FILES
    "${PROJECT_BINARY_DIR}/NaosConfig.cmake"
    "${PROJECT_BINARY_DIR}/NaosConfigVersion.cmake"
    "${PROJECT_SOURCE_DIR}/cmake/Naos.cmake"
    "${PROJECT_SOURCE_DIR}/cmake/NaosDiscover.cmake"
    DESTINATION "${naos_config_dir}"
)

# naos.pc, for pkg-config and Meson. Its flags follow the library built: -lnaosd for Debug
# (DEBUG_POSTFIX, through $<TARGET_FILE_BASE_NAME>, since $<TARGET_LINKER_FILE_BASE_NAME> needs
# CMake 3.27), -DNAOS_SHARED for a shared library, and libm for a static library on Unix, which C
# does not promise to leave out. The paths are relative to ${pcfiledir}, so the installed tree can
# move; a directory given as an absolute path stays absolute.
set(naos_pc_dir "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
if(IS_ABSOLUTE "${CMAKE_INSTALL_LIBDIR}")
    set(naos_pc_prefix "${CMAKE_INSTALL_PREFIX}")
else()
    file(RELATIVE_PATH naos_pc_up "/naos/${naos_pc_dir}" "/naos")
    string(REGEX REPLACE "/$" "" naos_pc_up "${naos_pc_up}")
    set(naos_pc_prefix "\${pcfiledir}/${naos_pc_up}")
endif()
foreach(naos_pc_kind IN ITEMS INCLUDEDIR LIBDIR)
    string(TOLOWER "${naos_pc_kind}" naos_pc_name)
    if(IS_ABSOLUTE "${CMAKE_INSTALL_${naos_pc_kind}}")
        set(naos_pc_${naos_pc_name} "${CMAKE_INSTALL_${naos_pc_kind}}")
    else()
        set(naos_pc_${naos_pc_name} "\${prefix}/${CMAKE_INSTALL_${naos_pc_kind}}")
    endif()
endforeach()
set(naos_pc_cflags "")
set(naos_pc_libs_private "")
if(naos_type STREQUAL "SHARED_LIBRARY")
    set(naos_pc_cflags " -DNAOS_SHARED")
elseif(UNIX)
    set(naos_pc_libs_private "Libs.private: -lm")
endif()
configure_file("${PROJECT_SOURCE_DIR}/cmake/naos.pc.in" "${PROJECT_BINARY_DIR}/naos.pc.in" @ONLY)
# One file for each configuration of a multi-config generator; cmake --install --config picks it.
file(GENERATE
    OUTPUT "${PROJECT_BINARY_DIR}/pkgconfig/$<CONFIG>/naos.pc"
    INPUT "${PROJECT_BINARY_DIR}/naos.pc.in"
)
install(FILES "${PROJECT_BINARY_DIR}/pkgconfig/$<CONFIG>/naos.pc" DESTINATION "${naos_pc_dir}")
