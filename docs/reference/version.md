# Version

The macros and functions below tell which version of Naos the code is built with and runs with, for code that supports several versions. They come with `#include <naos/naos.h>`.

Naos follows [Semantic Versioning](https://semver.org/). The major, minor and patch numbers range from 0 to 255. Before 1.0, a new minor version may break compatibility.

## At compile time

These macros give the version of the Naos headers included, to choose code at compile time.

The headers and the library linked may differ in version, shared libraries in particular, so a condition on these macros may not match the library actually used. The functions [at run time](#at-run-time) give the version of the library.

| Macro | Value |
|---|---|
| `NAOS_VERSION_MAJOR` | The major version |
| `NAOS_VERSION_MINOR` | The minor version |
| `NAOS_VERSION_PATCH` | The patch version |
| `NAOS_VERSION_NUMBER` | The version as the number `0xMMNNPP`: two hexadecimal digits each for the major (`MM`), minor (`NN`) and patch (`PP`) versions. 1.2.3 is `0x010203` |
| `NAOS_VERSION_STRING` | The version as the string `"MAJOR.MINOR.PATCH"` |

Checking at compile time that Naos is version `X.Y.Z` or later:

```c
#if NAOS_VERSION_NUMBER >= NAOS_VERSION_ENCODE(1, 2, 3)
    /* what needs 1.2.3 or later */
#endif
```

## At run time

These functions return the version of the library actually linked.

```c
unsigned long naos_version_number(void);
const char   *naos_version_string(void);
```

- `naos_version_number()` returns the version as the number `0xMMNNPP`.
- `naos_version_string()` returns the version as the string `"MAJOR.MINOR.PATCH"`.

Checking at run time that Naos is version `X.Y.Z` or later:

```c
if (naos_version_number() >= NAOS_VERSION_ENCODE(1, 2, 3))
{
    /* what needs 1.2.3 or later */
}
```

Checking that a shared library is the one the tests were built with is in the [shared library guide](../guides/shared-library.md#check-the-version-loaded).

## Encoding

```c
#define NAOS_VERSION_ENCODE(major, minor, patch)
#define NAOS_VERSION_STRINGIFY(major, minor, patch)
```

- `NAOS_VERSION_ENCODE()` makes the number `0xMMNNPP` of a version. `NAOS_VERSION_NUMBER` and `naos_version_number()` are made with it.
- `NAOS_VERSION_STRINGIFY()` makes the string `"MAJOR.MINOR.PATCH"` of a version. `NAOS_VERSION_STRING` and `naos_version_string()` are made with it.
