# Coding style

How the code of Naos is written. New code follows the code around it; this page states the rules it follows.

## Language

- C99, with no compiler extension (`C_EXTENSIONS NO`). The public headers must also compile as C++11.
- It builds without warnings on GCC, Clang and MSVC with `-Wall -Wextra -Wpedantic` and `/W4`; the self tests treat warnings as errors.

## Layout

- Braces on their own line (Allman style), for functions and statements alike.
- Braces even around a one-line body of `if`, `for` and `while`.
- Four spaces of indentation, no tabs.
- A blank line before a `return` that follows other statements, and between groups of statements.
- Parentheses around each operand of `&&`, `||` and `?:` that is itself an operation.

```c
/* a + b, or SIZE_MAX when the sum does not fit. */
static size_t add_saturated(size_t a, size_t b)
{
    return (a > (SIZE_MAX - b)) ? SIZE_MAX : (a + b);
}
```

```c
    if ((orders->suites == NULL) || (orders->tests == NULL))
    {
        free(orders->suites);
        free(orders->tests);

        return 0;
    }

    return 1;
```

## Names

| Kind | Rule | Example |
|---|---|---|
| Files | The concern, in lowercase. A file implementing a public header has its name; an internal header has the name of its `.c`; only an internal header with a public header of the same name gets `_internal.h` | `diagnostic.c`, `control.c`, `filter.h`, `suite_internal.h` |
| Functions | `naos_<module>_<verb>`; functions used in one file only are `static`, without prefix | `naos_diagnostic_begin`, `naos_options_get`, `take_filter` |
| Types | `naos_<name>_t` | `naos_cmp_t`, `naos_report_kind_t` |
| Public macros | `NAOS_` in uppercase | `NAOS_CHECK_INT_EQ` |
| Helper macros | A public name ending with `_` | `NAOS_CHECK_` |
| State of a file | `static` variables with the prefix `g_` | `g_options`, `g_traces` |
| Include guards | `NAOS_<NAME>_H` | `NAOS_CHECK_H` |

## Includes

```c
#include "runner.h"          /* the header this file implements, public or internal, first */

#include <stdint.h>          /* standard headers */
#include <stdlib.h>

#include <naos/exit.h>       /* other public headers */

#include "filter.h"          /* other internal headers */
#include "order.h"
```

A blank line between groups, each group in alphabetical order. Public headers are included as `<naos/...>`, internal ones as `"..."`.

## Comments

- In English.
- Each file starts with the copyright notice and `SPDX-License-Identifier: MIT`, followed by a short statement of what the file is responsible for.
- Each function declaration says in one sentence what it returns and what its arguments promise (whether `NULL` is allowed, how long a borrowed pointer must live).
- Comments say why, or what is not obvious; not what the next line plainly does.

## CMake

- Lowercase commands, four spaces of indentation.
- Functions for users are `naos_*`; internal ones are `_naos_*`.
- Files follow how they are used: a module read with `include()` is `Naos<Name>.cmake` (`NaosVersion.cmake`), as the modules of CMake are; a script run with `cmake -P` is in lowercase words joined by `-` (`prepare-release.cmake`, `check-output.cmake`). The package configuration has the name `find_package()` looks for (`naos-config.cmake`).
- A file starts with the copyright notice and a comment on what it is for and how it is used.

## Examples

The projects of `examples/` are read by users: they follow the same style, with comments for the reader rather than for maintainers, and build without warnings under the strict flags of the self tests.
