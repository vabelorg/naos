# Reference

Every rule of the public API, one part per page. Everything documented here is kept compatible.

The functions the macros call (`naos_check_int()`, `naos_fail()` and others), the types they take (`naos_cmp_t`) and the helper macros whose names end with `_` (`NAOS_CHECK_`) are in the headers, and exported by a shared build, only so that the macros work. Do not use them directly. They are kept compatible all the same, as everything Naos makes public: before 1.0 within a minor version, from 1.0 on within a major version ([version](version.md)).

| Page | Content |
|---|---|
| [Tests and suites](tests.md) | `NAOS_TEST`, `NAOS_SUITE`, setup and teardown, `NAOS_MAIN`, `naos_init()` and `naos_run()` |
| [Assertions](assertions.md) | `NAOS_CHECK_*` and `NAOS_REQUIRE_*` for each type |
| [Failing, skipping and notes](control.md) | How a test stops, `NAOS_FAIL`, `NAOS_SKIP`, `NAOS_NOTE`, `NAOS_TRACE_*` |
| [Version](version.md) | `NAOS_VERSION_*`, `naos_version_number()`, `naos_version_string()` |
| [Command line](command-line.md) | The `--naos-*` options and the exit codes |
| [Output](output.md) | Every line printed, and how values are written |
| [CMake](cmake.md) | `naos::naos`, `naos_add_executable()`, `naos_discover_tests()`, options, package |
