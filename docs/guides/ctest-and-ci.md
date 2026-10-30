# CTest and CI

To isolate crashes, run in parallel or set time limits, start each test in a process of its own: list the names with `--naos-list` and run them one by one with `--naos-filter`. An exit code of `0` is a pass, `77` a skip, anything else a failure. Naos leaves process management to the caller: CTest, a script or a CI service.

## With CMake

The tests of an executable made with `naos_add_executable()` are registered with CTest one by one, at each build. CTest starts each test in a process of its own, so running CTest is all there is to do. The CTest name of a test is `<target>.<suite>.<test>`, such as `my_tests.parser.empty_object`. `-C Debug` chooses the configuration with a multi-config generator such as Visual Studio.

```sh
ctest --test-dir build -C Debug --output-on-failure      # every test
ctest --test-dir build -C Debug -j8 --timeout 30         # in parallel, 30 seconds each at most
ctest --test-dir build -C Debug -R '^my_tests\.parser\.' # the suite parser only
```

An executable made with `add_executable()` is registered the same way with `naos_discover_tests(<target>)`. Both functions take `LABELS`, to label the tests, and `PROPERTIES`, for other CTest properties such as `ENVIRONMENT`:

```cmake
naos_add_executable(io_tests
    SOURCES io_test.c
    LIBRARIES my_lib
    LABELS io slow
    PROPERTIES ENVIRONMENT "LARGE_FILE=${CMAKE_CURRENT_SOURCE_DIR}/data/large.bin"
)
```

`ctest -L slow` then selects the tests labeled `slow`, and `ctest -LE slow` leaves them out. Every argument is in the [CMake reference](../reference/cmake.md).

## Without CMake

A shell script does the same.

```sh
#!/bin/sh
exe=$1
fail=0

for c in $("$exe" --naos-list); do
  "$exe" --naos-filter "$c"
  case $? in
    0|77) ;;
    *)    fail=$((fail+1)) ;;
  esac
done

[ "$fail" -eq 0 ]
```

## In CI

A CI job configures, builds and runs CTest. `--output-junit` writes a JUnit XML report (relative to the build directory) that most CI services can show; CTest has it since 3.21. For GitHub Actions:

```yaml
- name: Build
  run: |
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
    cmake --build build --config Debug
- name: Test
  run: ctest --test-dir build -C Debug --output-on-failure --output-junit junit.xml
```

To shake out hidden dependencies between tests, a job can also run an executable directly with `--naos-shuffle --naos-repeat 20` ([running tests](running-tests.md#find-hidden-dependencies)). The seed of a failed iteration is in its `ITERATION FAILED` line, to replay the order locally.

## Open failures in your editor

Failures are printed like compiler diagnostics, `file:line: error: ...`, so editors can list them with the compiler errors. For VS Code, put this problem matcher in `.vscode/tasks.json`: failures and their traces appear in the Problems panel, and a click opens the line. It also reads the form `file(line): error: ...` that Naos prints when its library is built with MSVC.

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "naos tests",
            "type": "shell",
            "command": "ctest --test-dir build -C Debug --output-on-failure",
            "problemMatcher": [
                {
                    "owner": "naos",
                    "fileLocation": ["autoDetect", "${workspaceFolder}"],
                    "severity": "error",
                    "pattern": {
                        "regexp": "^(.*?)[:(](\\d+)\\)?: error: (.*)$",
                        "file": 1,
                        "line": 2,
                        "message": 3
                    }
                },
                {
                    "owner": "naos",
                    "fileLocation": ["autoDetect", "${workspaceFolder}"],
                    "severity": "info",
                    "pattern": {
                        "regexp": "^(.*?)[:(](\\d+)\\)?: note: (.*)$",
                        "file": 1,
                        "line": 2,
                        "message": 3
                    }
                }
            ]
        }
    ]
}
```

A tool that reads the output, to collect results in CI for example, should follow [output](../reference/output.md), whose format is kept compatible.
