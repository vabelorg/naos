# Shared library

Naos is a static library by default. It can also be built as a shared library (DLL).

## Build Naos as a shared library

Configure Naos on its own, as the top-level project, with `-DBUILD_SHARED_LIBS=ON`, then install it:

```sh
cmake -S naos -B naos/build -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build naos/build --config Release
cmake --install naos/build --config Release --prefix /opt/naos
```

Inside another project (`add_subdirectory` or `FetchContent`), Naos is always a static library, whatever `BUILD_SHARED_LIBS` is.

- Through `find_package(naos)`, CMake adds the definition that the headers need, `NAOS_SHARED`. Without CMake, define `NAOS_SHARED` yourself when compiling the tests.
- The public functions name no calling convention. Build Naos and the tests with the same default convention (no `/Gz` on 32-bit Windows, for example).

## Make the DLL findable at build time

With CMake, each test executable is started right after it is built, to list its tests for CTest. The shared library must therefore be found at build time already: put it in the directory of the executable, or add its directory to `PATH` (`LD_LIBRARY_PATH` on Linux, `DYLD_LIBRARY_PATH` on macOS). An rpath works too.

To copy the DLL after the build, add the `POST_BUILD` command after `add_executable()` and before `naos_discover_tests()`. Commands run in the order they were added, and with `naos_add_executable()` the listing always comes first.

```cmake
add_executable(my_tests main.c parser_test.c)
target_link_libraries(my_tests PRIVATE naos::naos)
if(WIN32)
    add_custom_command(TARGET my_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:my_tests> $<TARGET_FILE_DIR:my_tests>
        COMMAND_EXPAND_LISTS
    )
endif()
naos_discover_tests(my_tests)
```

When the DLL is not found, the build fails at the listing with a message that says what to do. On Windows, the executable prints nothing and exits with `0xc0000135`.

## Check the version loaded

An old DLL may be loaded in place of the one the tests were built with. To catch it, compare the version of the headers with that of the library loaded, in a [custom `main()`](custom-main.md) before `naos_run()`:

```c
if (naos_version_number() != NAOS_VERSION_NUMBER)
{
    fprintf(stderr, "naos %s is loaded, but the tests were built with %s\n",
        naos_version_string(), NAOS_VERSION_STRING);
    return 1;
}
```

The version macros and functions are in [version](../reference/version.md).
