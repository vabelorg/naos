#!/usr/bin/env python3
# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
"""Runs every check that reads the sources and needs no build, and reports each that fails.

The lint job of CI is this script. Run it before sending a change:

    python3 scripts/lint.py

    check_markdown_links.py    the relative links of the Markdown files
    check_printf_calls.py      the formatted output of src/ goes through src/format.h
    check_release_scripts.py   prepare_release.py and release_notes.py
"""

import subprocess
import sys
from pathlib import Path

CHECKS = ["check_markdown_links.py", "check_printf_calls.py", "check_release_scripts.py"]


def main():
    scripts = Path(__file__).resolve().parent
    failed = []
    for check in CHECKS:
        print("-- " + check, flush=True)
        if subprocess.run([sys.executable, str(scripts / check)]).returncode != 0:
            failed.append(check)
    if failed:
        print("failed: " + ", ".join(failed), file=sys.stderr)
        return 1
    print("Every check passed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
