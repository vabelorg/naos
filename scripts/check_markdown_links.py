#!/usr/bin/env python3
# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
"""Checks the relative links of the Markdown files of Naos.

Every linked file must exist and every #anchor must name a heading (as GitHub makes its id) or an
<a id="..."> of the file linked. Links with a scheme (https:, mailto:) are not checked. Needs no
build: run by scripts/lint.py, and by hand with  python3 scripts/check_markdown_links.py
"""

import os
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

# The punctuation GitHub drops from a heading to make its id.
PUNCTUATION = set(
    "!\"#$%&'()*+,./:;<=>?@^`{|}~"
    "（）「」『』、。・：，！？／【】〜—–→…‘’“”"
)


def markdown_files():
    """Returns the Markdown files checked: those at the root, and those of docs/, examples/ and .github/."""
    files = [p for p in ROOT.glob("*.md") if p.is_file()]
    for directory in ("docs", "examples", ".github"):
        files += [p for p in (ROOT / directory).rglob("*.md") if p.is_file()]
    return sorted(files)


def doc_text(path):
    """Returns the content of the file without its fenced code blocks, after a newline so that every
    line, the first included, starts with one."""
    text = path.read_text(encoding="utf-8").replace("\r", "")
    return "\n" + re.sub(r"```[^`]*```", "", text)


def heading_id(heading):
    """Returns the id GitHub gives the heading text: its words, lowercase, without punctuation and
    with '-' for each space."""
    text = re.sub(r"\[([^]]*)\]\([^)]*\)", r"\1", heading)
    text = text.replace("[", "").replace("]", "").replace("\\", "").lower()
    return "".join(c for c in text if c not in PUNCTUATION).replace(" ", "-")


def doc_anchors(path):
    """Returns the anchors of the Markdown file: the ids of its headings, numbered as GitHub numbers
    repeated ones, and those of its <a id="..."> and <a name="...">."""
    text = doc_text(path)
    anchors = []
    for heading in re.findall(r"\n#+ ([^\n]*)", text):
        base = heading_id(re.sub(r"[ #]+$", "", heading.lstrip(" ")))
        unique, count = base, 0
        while unique in anchors:
            count += 1
            unique = "{}-{}".format(base, count)
        anchors.append(unique)
    anchors += re.findall(r"<a (?:id|name)=\"([^\"]+)\"", text)
    return set(anchors)


def main():
    files = markdown_files()
    anchors_of = {}
    errors = []
    link_count = 0

    for path in files:
        shown = path.relative_to(ROOT).as_posix()
        # Inline code is no link.
        text = re.sub(r"`[^`\n]*`", "", doc_text(path))
        for target in re.findall(r"\]\(([^) \n]+)\)", text):
            if re.match(r"[a-z]+:", target):
                continue
            link_count += 1

            file_part, _, anchor = target.partition("#")
            linked = path if file_part == "" else Path(os.path.normpath(path.parent / file_part))
            if not linked.exists():
                errors.append("{}: no file for the link to '{}'".format(shown, target))
                continue

            if anchor and linked.suffix == ".md":
                if linked not in anchors_of:
                    anchors_of[linked] = doc_anchors(linked)
                if anchor not in anchors_of[linked]:
                    errors.append("{}: no anchor for the link to '{}'".format(shown, target))

    if errors:
        print("{} broken links in {} files:".format(len(errors), len(files)), file=sys.stderr)
        print("\n".join(errors), file=sys.stderr)
        return 1
    print("{} links checked in {} files".format(link_count, len(files)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
