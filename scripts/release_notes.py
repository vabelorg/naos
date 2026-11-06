#!/usr/bin/env python3
# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
"""Writes the notes of a release: the section of its version in CHANGELOG.md, without the heading.

Used by .github/workflows/release-pr.yml and release.yml:

    python3 scripts/release_notes.py <version> <output>

--root <path> names the root of Naos; by default the one holding this script.
"""

import argparse
import re
import sys
from pathlib import Path


def section_body(log, start):
    """Returns the text of log from start to the next version heading, or to the links at the end."""
    rest = log[start:]
    end = len(rest)
    for marker in ("\n## [", "\n[Unreleased]: "):
        at = rest.find(marker)
        if at != -1 and at < end:
            end = at
    return rest[:end]


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("version", help="the version released, x.y.z")
    parser.add_argument("output", type=Path, help="the file to write")
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parent.parent,
                        help="root of Naos")
    args = parser.parse_args()

    log = (args.root / "CHANGELOG.md").read_text(encoding="utf-8").replace("\r", "")
    heading = re.search(r"\n## \[" + re.escape(args.version) + r"\][^\n]*\n", log)
    if not heading:
        sys.exit("error: CHANGELOG.md has no section for {}".format(args.version))

    notes = section_body(log, heading.end()).strip()
    if not notes:
        sys.exit("error: The section of {} in CHANGELOG.md is empty".format(args.version))

    args.output.write_bytes((notes + "\n").encode("utf-8"))
    return 0


if __name__ == "__main__":
    sys.exit(main())
