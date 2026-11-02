# CMake

What a CMake project gets from Naos: the target `Naos::naos`, two functions that make test executables and register their tests with CTest, and a few options. [Installation](../getting-started/installation.md) tells how to bring Naos into a project.

## Target

| Target | Content |
|---|---|
| `Naos::naos` | The library, with its include directory `include/` and the C99 requirement. Shared, it also carries the definition `NAOS_SHARED` |

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
    [ALLOW_NO_ASSERTIONS]
    [IGNORE_SUITES <suite>...]
    [NO_REGISTRATION_CHECK]
)
```

Makes the executable `<target>` from the `SOURCES`, links it to `Naos::naos` and the `LIBRARIES` (privately), and registers each of its tests with CTest. The arguments other than `SOURCES` and `LIBRARIES` are those of [`naos_discover_tests()`](#naos_discover_tests).

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
    [ALLOW_NO_ASSERTIONS]
    [IGNORE_SUITES <suite>...]
    [NO_REGISTRATION_CHECK]
)
```

Registers each test of an executable made with `add_executable()`, which must call `naos_init()` and `naos_run()` (directly or through `NAOS_MAIN`).

| Argument | Content |
|---|---|
| `PREFIX <prefix>` | The start of the CTest names. The default is `<target>.` |
| `LABELS <label>...` | Labels given to the CTest tests |
| `PROPERTIES <name> <value>...` | Other properties given to the CTest tests. A list value is one argument: `ENVIRONMENT "A=1;B=2"` |
| `DISCOVERY_TIMEOUT <seconds>` | The time limit of listing the tests (`--naos-list`): a positive integer, 60 by default. Raise it where listing is slow, under an emulator for example |
| `ALLOW_NO_ASSERTIONS` | Each test also runs with `--naos-allow-no-assertions`, so that a test that runs no assertion passes ([a test that runs no assertion](control.md#a-test-that-runs-no-assertion)) |
| `IGNORE_SUITES <suite>...` | Suites defined in the sources and not added on purpose, such as those a [custom `main()`](../guides/custom-main.md) adds on an argument: the [registration check](#registration-check) leaves them out. Each is a suite name |
| `NO_REGISTRATION_CHECK` | No [registration check](#registration-check): a test or a suite defined and not added does not fail the build |

- Only an executable target made in the same directory can be registered, and only once. Anything else (a library, a name with no target, a second registration), `PROPERTIES` that are not name and value pairs, and an `IGNORE_SUITES` entry that is not a suite name, are errors at configure time.
- Each test becomes the CTest test `<prefix><suite>.<test>`, running `<exe> --naos-filter <suite>.<test>` with `SKIP_RETURN_CODE 77`; with `ALLOW_NO_ASSERTIONS`, `<exe> --naos-filter <suite>.<test> --naos-allow-no-assertions`.
- A target with the property `CROSSCOMPILING_EMULATOR` is listed and run through the emulator.

## Discovery

The tests are listed after each build of the executable, by running it with `--naos-list`. Adding or removing a test needs no new configure.

- Until the executable is built, and after a build whose discovery failed, CTest has a single failing test in place of its tests, `<prefix>NOT_BUILT`. It carries the `LABELS` too, so that `ctest -L` does not miss it. The same happens with a multi-config generator for a configuration that is not built, chosen with `-C`.
- What a change to the arguments (`PREFIX`, `LABELS`, `PROPERTIES`, `DISCOVERY_TIMEOUT`, `ALLOW_NO_ASSERTIONS`, `IGNORE_SUITES`, `NO_REGISTRATION_CHECK`) does depends on the generator.
  - The single-config Ninja and Makefile generators: the executable is linked again at the next build, which lists its tests again.
  - Other generators, such as Visual Studio, Xcode and Ninja Multi-Config: the tests keep what they had until the executable is built again.
- When listing fails, the build fails, with the reason and what to do in its message.
  - After the time limit: it says to raise `DISCOVERY_TIMEOUT`.
  - Otherwise: it tells what to do when a shared library (DLL) is not found ([shared library](../guides/shared-library.md)). On Windows, a missing DLL gives no output, only `Exit code 0xc0000135`; in that case the message names the cause outright.
- When a test or a suite is defined and not added, the build fails too ([registration check](#registration-check)).
- Commands added to the target with `add_custom_command(TARGET ... POST_BUILD)` run in the order added. Add one that must run before the listing (copying a DLL, for example) after `add_executable()` and before `naos_discover_tests()`; with `naos_add_executable()`, the listing always comes first.

## Registration check

A test is written twice, with `NAOS_TEST` and with `NAOS_ADD_TEST` in its suite; a suite too, with `NAOS_SUITE` and with `NAOS_ADD_SUITE` ([tests and suites](tests.md)). A test or a suite whose second half is missing never runs, and the run still passes. Both functions check for it at [discovery](#discovery), unless given `NO_REGISTRATION_CHECK`: they compare what the executable lists with what its sources define. The check reads text only, so it is the same with every compiler.

Each test or suite defined and not added fails the build, one line for each, in the form of a compiler diagnostic:

```
slow_test.c:21: error: suite slow is defined but never added, so its tests never run: add it with NAOS_ADD_SUITE(slow), or name it in IGNORE_SUITES of naos_add_executable() or naos_discover_tests() when it is left out on purpose
left_out_test.c:15: error: test left_out is defined but never added, so it never runs: add it with NAOS_ADD_TEST(left_out) to a suite of this file
```

- **Sources**: those of the target, its `SOURCES` property, once the build has made them: those of `target_sources()` count too. Only C and C++ sources and headers are read, by their extension.
- **Definitions**: `NAOS_SUITE(<name>` and `NAOS_TEST(<name>` outside comments, preprocessor directives and conditionals.
- **A suite is added** when `--naos-list`, run without arguments, lists one of its tests; a suite with no test is a mistake in the suites ([tests and suites](tests.md#suites)).
- **A test is added** when the list holds `<suite>.<test>` for a suite of its own file: a test is `static`, so only a suite of its file can add it, and a test of the same name in another file does not stand for it. A suite whose definition is not found (see below) may be of that file, so it counts too.

Not checked:

- A suite or a test inside `#if`, `#ifdef` or `#ifndef`, up to its `#endif`: whether it is built depends on the condition, which only the compiler knows. Its `NAOS_ADD_SUITE` or `NAOS_ADD_TEST` is usually behind the same condition.
- A suite or a test defined through a macro of your own (one that expands to `NAOS_SUITE` or `NAOS_TEST`), which is not found.
- The tests of a file that defines a suite in `IGNORE_SUITES`, or a suite not added: they may be in that suite, whose own line is enough.
- Setup and teardown, which `--naos-list` does not show. A setup or a teardown that no suite sets is an unused `static` function, which the warnings of GCC and Clang (`-Wall`) report.

A custom `main()` that adds a suite only on an argument has it named in `IGNORE_SUITES` ([custom main()](../guides/custom-main.md)).

### Without the CMake functions

The check is a script of its own, installed beside the functions (`NaosCheckRegistration.cmake`). Run it after building the executable:

```sh
cmake -D NAOS_EXECUTABLE=build/my_tests -D "NAOS_SOURCES=main.c;parser_test.c" -P NaosCheckRegistration.cmake
```

| Variable | Content |
|---|---|
| `NAOS_EXECUTABLE` | The test executable, run with `--naos-list` |
| `NAOS_SOURCES` | Its sources, separated by `;`. Relative paths start at the current directory |
| `NAOS_IGNORE_SUITES` | Optional: the suites left out on purpose, separated by `;` |

It exits with 0 when every test and suite defined is listed, and fails with the lines above otherwise.

## Options

| Option | Default | Content |
|---|---|---|
| `NAOS_SELFTEST` | `ON` when Naos is the top-level project | Registers the self tests of Naos. Takes effect only when `BUILD_TESTING` is also `ON` (after `include(CTest)`). When Naos is the top-level project, it also builds with warnings as errors (`-Wall -Wextra -Wpedantic -Werror` for GCC and Clang, `/W4 /WX` for MSVC) |
| `NAOS_INSTALL` | `ON` when Naos is the top-level project | Generates the install rules |
| `BUILD_SHARED_LIBS` | `OFF` | `ON` makes Naos a shared library (DLL). Takes effect only when Naos is the top-level project; inside another project, Naos is always a static library |

## Package

After `cmake --install`, `find_package(Naos)` finds Naos and gives `Naos::naos` and the functions above.

```cmake
find_package(Naos 0.3 REQUIRED)
```

Before 1.0, a version asks for the same minor version: `find_package(Naos 0.3)` accepts 0.3.x only. From 1.0 on, it asks for the same major version.

The installed files, under the install prefix:

| Path | Content |
|---|---|
| `include/naos/` | The public headers |
| `lib/` (`bin/` for a DLL) | The library |
| `bin/` | The PDB of a DLL built with MSVC in Debug or RelWithDebInfo |
| `lib/cmake/Naos/` | `NaosConfig.cmake`, `NaosConfigVersion.cmake`, `NaosTargets*.cmake`, `Naos.cmake`, `NaosDiscover.cmake`, `NaosCheckRegistration.cmake` |
| `share/licenses/naos/LICENSE` | The license |

The directories follow `GNUInstallDirs` (`CMAKE_INSTALL_LIBDIR` and so on).
