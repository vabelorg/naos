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

It also makes a test, a setup or a teardown that no suite adds an error of the compiler ([registration check](#registration-check)); `NO_REGISTRATION_CHECK` leaves the compile options as they are.

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
| `NO_REGISTRATION_CHECK` | No [registration check](#registration-check): neither the suites at discovery, nor, with `naos_add_executable()`, the tests and fixtures at compile time |

- Only an executable target made in the same directory can be registered, and only once. Anything else (a library, a name with no target, a second registration), `PROPERTIES` that are not name and value pairs, and an `IGNORE_SUITES` entry that is not a suite name, are errors at configure time.
- The compile options of the target are left as they are.
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
- When a suite is defined and not added, the build fails too ([registration check](#registration-check)).
- Commands added to the target with `add_custom_command(TARGET ... POST_BUILD)` run in the order added. Add one that must run before the listing (copying a DLL, for example) after `add_executable()` and before `naos_discover_tests()`; with `naos_add_executable()`, the listing always comes first.

## Registration check

A test is written twice, with `NAOS_TEST` and with `NAOS_ADD_TEST` in its suite; a suite too, with `NAOS_SUITE` and with `NAOS_ADD_SUITE` ([tests and suites](tests.md)). A test or a suite whose second half is missing never runs, and the run still passes. Both functions check for it, unless given `NO_REGISTRATION_CHECK`.

**Tests, setup and teardown**, with `naos_add_executable()` only: their functions are `static`, so the compiler finds those that no suite uses. The function makes that warning an error for the C and C++ sources of the target:

| Compiler | Option |
|---|---|
| GCC, and Clang with its own command line | `-Werror=unused-function` |
| MSVC | `/we4505` |

- The error names the function, `naos_gen_test_<name>`, `naos_gen_setup_<name>` or `naos_gen_teardown_<name>` ([generated names](tests.md#generated-names)). GCC gives the location of the macro in `naos/suite.h`, Clang and MSVC that of the test.
- It applies to every `static` function of the sources that nothing uses, such as a helper left over: remove it, or give `NO_REGISTRATION_CHECK`.
- Other compilers, Clang with the command line of MSVC (`clang-cl`) among them, get no option.
- `naos_discover_tests()` leaves the compile options of its target as they are. Give it the option yourself, or turn on the warning (`-Wall`, `/W4`) and read it.

**Suites**, with both functions: at [discovery](#discovery), the suites the executable lists are compared with those its sources define. A suite defined and not listed fails the build, one line for each, in the form of a compiler diagnostic:

```
slow_test.c:21: error: suite slow is defined but never added, so its tests never run: add it with NAOS_ADD_SUITE(slow), or name it in IGNORE_SUITES of naos_add_executable() or naos_discover_tests() when it is left out on purpose
```

- The sources are those of the target, its `SOURCES` property, once the build has made them: those of `target_sources()` count too. Only C and C++ sources and headers are read, by their extension.
- A definition is `NAOS_SUITE(<name>` outside comments, preprocessor directives and conditionals. Two kinds of suite are not checked:
  - A suite inside `#if`, `#ifdef` or `#ifndef`, up to its `#endif`: whether it is built depends on the condition, which only the compiler knows. Its `NAOS_ADD_SUITE` is usually behind the same condition.
  - A suite defined through a macro of your own (one that expands to `NAOS_SUITE`), which is not found.
- The suites listed are those with at least one test in `--naos-list`, run without arguments; a suite with no test is a mistake in the suites ([tests and suites](tests.md#suites)). A custom `main()` that adds a suite only on an argument has it named in `IGNORE_SUITES`.

### Without the CMake functions

The suite check is a script of its own, installed beside the functions (`NaosCheckSuites.cmake`). Run it after building the executable:

```sh
cmake -D NAOS_EXECUTABLE=build/my_tests -D "NAOS_SOURCES=main.c;parser_test.c" -P NaosCheckSuites.cmake
```

| Variable | Content |
|---|---|
| `NAOS_EXECUTABLE` | The test executable, run with `--naos-list` |
| `NAOS_SOURCES` | Its sources, separated by `;`. Relative paths start at the current directory |
| `NAOS_IGNORE_SUITES` | Optional: the suites left out on purpose, separated by `;` |

It exits with 0 when every suite defined is listed, and fails with the lines above otherwise. For the tests, build with `-Werror=unused-function` (GCC, Clang) or `/we4505` (MSVC).

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
| `lib/cmake/Naos/` | `NaosConfig.cmake`, `NaosConfigVersion.cmake`, `NaosTargets*.cmake`, `Naos.cmake`, `NaosDiscover.cmake`, `NaosCheckSuites.cmake` |
| `share/licenses/naos/LICENSE` | The license |

The directories follow `GNUInstallDirs` (`CMAKE_INSTALL_LIBDIR` and so on).
