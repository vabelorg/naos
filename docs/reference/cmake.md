# CMake

What a CMake project gets from Naos: the target `naos::naos`, two functions that make test executables and register their tests with CTest, and a few options. [Installation](../getting-started/installation.md) tells how to bring Naos into a project.

## Target

| Target | Content |
|---|---|
| `naos::naos` | The library, with its include directory `include/` and the C99 requirement. Shared, it also carries the definition `NAOS_SHARED` |

- Debug builds are named with the suffix `d` (`naosd.lib`, `libnaosd.a`), so that they can sit beside release builds.
- A shared build exports only the public functions (`C_VISIBILITY_PRESET hidden`). Its `SOVERSION` is `0.<minor>` before 1.0 and `<major>` from 1.0 on.

## Functions

### naos_add_executable()

```cmake
naos_add_executable(<target>
    SOURCES <source>...
    [LIBRARIES <library>...]
    [PREFIX <prefix>]
    [LABELS <label>...]
    [PROPERTIES <name> <value>...]
    [DISCOVERY_TIMEOUT <seconds>]
)
```

Makes the executable `<target>` from the `SOURCES`, links it to `naos::naos` and the `LIBRARIES` (privately), and registers each of its tests with CTest. The arguments other than `SOURCES` and `LIBRARIES` are those of [`naos_discover_tests()`](#naos_discover_tests).

```cmake
naos_add_executable(my_tests SOURCES main.c parser_test.c LIBRARIES my_lib)
```

### naos_discover_tests()

```cmake
naos_discover_tests(<target>
    [PREFIX <prefix>]
    [LABELS <label>...]
    [PROPERTIES <name> <value>...]
    [DISCOVERY_TIMEOUT <seconds>]
)
```

Registers each test of an executable made with `add_executable()`, which must call `naos_init()` and `naos_run()` (directly or through `NAOS_MAIN`).

| Argument | Content |
|---|---|
| `PREFIX <prefix>` | The start of the CTest names. The default is `<target>.` |
| `LABELS <label>...` | Labels given to the CTest tests |
| `PROPERTIES <name> <value>...` | Other properties given to the CTest tests. A list value is one argument: `ENVIRONMENT "A=1;B=2"` |
| `DISCOVERY_TIMEOUT <seconds>` | The time limit of listing the tests (`--naos-list`): a positive integer, 60 by default. Raise it where listing is slow, under an emulator for example |

- Only an executable target made in the same directory can be registered, and only once. Anything else (a library, a name with no target, a second registration), and `PROPERTIES` that are not name and value pairs, are errors at configure time.
- Each test becomes the CTest test `<prefix><suite>.<test>`, running `<exe> --naos-filter <suite>.<test>` with `SKIP_RETURN_CODE 77`.
- A target with the property `CROSSCOMPILING_EMULATOR` is listed and run through the emulator.

## Discovery

The tests are listed after each build of the executable, by running it with `--naos-list`. Adding or removing a test needs no new configure.

- Until the executable is built, CTest has a single failing test in place of its tests, `<prefix>NOT_BUILT`. It carries the `LABELS` too, so that `ctest -L` does not miss it. The same happens with a multi-config generator for a configuration that is not built, chosen with `-C`.
- When listing fails, the build fails, with the reason and what to do in its message. After the time limit, it says to raise `DISCOVERY_TIMEOUT`; otherwise it tells what to do when a shared library (DLL) is not found ([shared library](../guides/shared-library.md)). On Windows, a missing DLL gives no output, only `Exit code 0xc0000135`; in that case the message names the cause outright.
- Commands added to the target with `add_custom_command(TARGET ... POST_BUILD)` run in the order added. Add one that must run before the listing (copying a DLL, for example) after `add_executable()` and before `naos_discover_tests()`; with `naos_add_executable()`, the listing always comes first.

## Options

| Option | Default | Content |
|---|---|---|
| `NAOS_SELFTEST` | `ON` when Naos is the top-level project | Registers the self tests of Naos. Takes effect only when `BUILD_TESTING` is also `ON` (after `include(CTest)`). When Naos is the top-level project, it also builds with warnings as errors (`-Wall -Wextra -Wpedantic -Werror` for GCC and Clang, `/W4 /WX` for MSVC) |
| `NAOS_INSTALL` | `ON` when Naos is the top-level project | Generates the install rules |
| `BUILD_SHARED_LIBS` | `OFF` | `ON` makes Naos a shared library (DLL). Takes effect only when Naos is the top-level project; inside another project, Naos is always a static library |

## Package

After `cmake --install`, `find_package(naos)` finds Naos and gives `naos::naos` and the functions above.

```cmake
find_package(naos 0.0 REQUIRED)
```

Before 1.0, a version asks for the same minor version: `find_package(naos 0.0)` accepts 0.0.x only. From 1.0 on, it asks for the same major version.

The installed files, under the install prefix:

| Path | Content |
|---|---|
| `include/naos/` | The public headers |
| `lib/` (`bin/` for a DLL) | The library |
| `bin/` | The PDB of a DLL built with MSVC in Debug or RelWithDebInfo |
| `lib/cmake/naos/` | `naos-config.cmake`, `naos-config-version.cmake`, `naos-targets*.cmake`, `Naos.cmake`, `NaosDiscover.cmake` |
| `share/licenses/naos/LICENSE` | The license |

The directories follow `GNUInstallDirs` (`CMAKE_INSTALL_LIBDIR` and so on).
