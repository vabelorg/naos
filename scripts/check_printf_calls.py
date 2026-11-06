#!/usr/bin/env python3
# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
"""Checks that the sources of src/ call the formatted output functions through src/format.h.

They must call naos_printf, naos_snprintf, ..., never printf, snprintf and the others directly:
those are not C99 on every platform (src/format.h tells why). Comments are not checked. Needs no
build: run by scripts/lint.py, and by hand with  python3 scripts/check_printf_calls.py
"""

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

CALL = re.compile(
    r"(^|[^A-Za-z0-9_])(printf|fprintf|vprintf|vfprintf|sprintf|vsprintf|snprintf|vsnprintf)[ \t]*\("
)


def code_lines(text):
    """Yields the number and the code of each line of the C text, without its comments: /* ... */
    may span lines, // ends the line."""
    in_comment = False
    for number, line in enumerate(text.split("\n"), start=1):
        code = ""
        rest = line
        while rest:
            if in_comment:
                end = rest.find("*/")
                if end == -1:
                    rest = ""
                else:
                    rest = rest[end + 2:]
                    in_comment = False
            else:
                begin = rest.find("/*")
                line_comment = rest.find("//")
                if line_comment != -1 and (begin == -1 or line_comment < begin):
                    code += rest[:line_comment]
                    rest = ""
                elif begin == -1:
                    code += rest
                    rest = ""
                else:
                    code += rest[:begin] + " "
                    rest = rest[begin + 2:]
                    in_comment = True
        yield number, code


def main():
    files = sorted(p for p in (ROOT / "src").glob("*.[ch]") if p.name != "format.h")
    errors = []
    for path in files:
        shown = path.relative_to(ROOT).as_posix()
        text = path.read_text(encoding="utf-8").replace("\r", "")
        for number, code in code_lines(text):
            match = CALL.search(code)
            if match:
                name = match.group(2)
                errors.append("{}:{}: calls {} directly; use naos_{} of format.h".format(
                    shown, number, name, name))

    if errors:
        print("formatted output must go through src/format.h:", file=sys.stderr)
        print("\n".join(errors), file=sys.stderr)
        return 1
    print("{} files of src/ call the formatted output functions through format.h".format(len(files)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
