#!/usr/bin/env python3
# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
"""Prepares the release of a version, as docs/development/release.md describes.

Checks that the entries of [Unreleased] in CHANGELOG.md allow the version, sets it in
include/naos/version.h, updates the tag and the version named in the documentation and the examples
for a new minor or major version, and closes the changelog. Run by .github/workflows/release-pr.yml;
changes the files only, commits nothing:

    python3 scripts/prepare_release.py <version> [--repo-url <url>] [--date <YYYY-MM-DD>]

--repo-url  the repository the changelog links to; by default the one README.md fetches with
            FetchContent
--date      the date of the release; by default today (UTC)
--root      root of Naos; by default the one holding this script
"""

import argparse
import datetime
import re
import subprocess
import sys
from pathlib import Path

NAOS_VERSION_CMAKE = Path(__file__).resolve().parent.parent / "cmake/NaosVersion.cmake"

# The pages naming the tag fetched by FetchContent or the minor version asked of find_package(),
# besides the examples. The pages of docs/development/ are left alone: their versions are examples.
VERSIONED_PAGES = ["README.md", "README.ja.md"] + [
    "docs/{}{}.md".format(lang, page)
    for lang in ("", "ja/")
    for page in ("getting-started/installation", "getting-started/quickstart", "reference/cmake",
                 "guides/troubleshooting")
]


def fail(message):
    sys.exit("error: " + message)


def read_text(path):
    """Returns the content of the file with LF line endings."""
    return path.read_text(encoding="utf-8").replace("\r", "")


def write_text(path, text):
    """Writes text to the file as it is, with LF line endings on every platform."""
    path.write_bytes(text.encode("utf-8"))


def read_version(header):
    """Returns (major, minor, patch) from include/naos/version.h, read by naos_read_version() of
    cmake/NaosVersion.cmake, as the build reads it."""
    result = subprocess.run(
        ["cmake", "-D", "HEADER=" + str(header), "-P", str(NAOS_VERSION_CMAKE)],
        stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
    if result.returncode != 0:
        fail("cannot read the version of {}:\n{}".format(header, result.stderr.strip()))
    return tuple(int(part) for part in result.stdout.strip().split("."))


def parse_version(text):
    """Returns (major, minor, patch) of MAJOR.MINOR.PATCH, without leading zeros: the tag and the
    macros have none, and in C, 08 is not a number."""
    match = re.fullmatch(r"([0-9]+)\.([0-9]+)\.([0-9]+)", text)
    if not match:
        fail("VERSION '{}' is not MAJOR.MINOR.PATCH".format(text))
    version = tuple(int(part) for part in match.groups())
    if any(part >= 256 for part in version):
        fail("VERSION '{}': each component must be < 256".format(text))
    return version


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("version", help="the version to release, x.y.z")
    parser.add_argument("--repo-url", help="the repository the changelog links to")
    parser.add_argument("--date", help="the date of the release, YYYY-MM-DD")
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parent.parent,
                        help="root of Naos")
    args = parser.parse_args()
    root = args.root

    version_header = root / "include/naos/version.h"
    changelog = root / "CHANGELOG.md"

    # --- The version ---

    new = parse_version(args.version)
    version = "{}.{}.{}".format(*new)
    old = read_version(version_header)
    old_version = "{}.{}.{}".format(*old)
    if new <= old:
        fail("VERSION {} is not greater than {}, the version in include/naos/version.h".format(
            version, old_version))
    old_minor_version = "{}.{}".format(*old[:2])
    new_minor_version = "{}.{}".format(*new[:2])
    if new[0] != old[0]:
        kind = "major"
    elif new[1] != old[1]:
        kind = "minor"
    else:
        kind = "patch"

    # --- The changelog ---

    log = read_text(changelog)
    unreleased_heading = "## [Unreleased]\n"
    unreleased_at = log.find(unreleased_heading)
    if unreleased_at == -1:
        fail("CHANGELOG.md has no '## [Unreleased]' heading")
    rest = log[unreleased_at + len(unreleased_heading):]

    # The section ends at the next version heading, or at the links at the end of the file.
    body_length = len(rest)
    for marker in ("\n## [", "\n[Unreleased]: "):
        at = rest.find(marker)
        if at != -1 and at < body_length:
            body_length = at
    unreleased = rest[:body_length].strip()
    after_unreleased = rest[body_length:]
    if not unreleased:
        fail("[Unreleased] in CHANGELOG.md is empty: record the changes first")

    # docs/development/changelog.md: a patch version has Fixed and Security entries only, and a
    # BREAKING entry needs a minor version before 1.0 and a major version from 1.0 on.
    if kind == "patch":
        for section in re.findall(r"(?:^|\n)(### [^\n]*)", unreleased):
            section = section.strip()
            if not re.fullmatch(r"### (Fixed|Security)", section):
                fail("[Unreleased] has '{}': {} is a patch version, which can have Fixed and "
                     "Security entries only".format(section, version))
    elif "**BREAKING**" in unreleased and kind == "minor" and new[0] > 0:
        fail("[Unreleased] has a BREAKING entry: from 1.0 on it needs a major version")

    if args.repo_url:
        repo_url = re.sub(r"/$", "", args.repo_url)
    else:
        match = re.search(r"GIT_REPOSITORY (https://[^ \n]+)\.git", read_text(root / "README.md"))
        if not match:
            fail("README.md has no GIT_REPOSITORY to link to; give --repo-url <url>")
        repo_url = match.group(1)

    date = args.date or datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%d")

    # The version released before, if any: the first version heading after [Unreleased].
    match = re.search(r"\n## \[([0-9]+\.[0-9]+\.[0-9]+)\]", after_unreleased)
    if match:
        previous_link = "{}/compare/v{}...v{}".format(repo_url, match.group(1), version)
    else:
        previous_link = "{}/releases/tag/v{}".format(repo_url, version)

    log = "{}{}\n## [{}] - {}\n\n{}\n{}".format(
        log[:unreleased_at], unreleased_heading, version, date, unreleased, after_unreleased)
    links = "[Unreleased]: {0}/compare/v{1}...HEAD\n[{1}]: {2}\n".format(repo_url, version,
                                                                          previous_link)
    match = re.search(r"\n\[Unreleased\]: [^\n]*\n", log)
    if match:
        log = log.replace(match.group(0), "\n" + links)
    else:
        log = log.rstrip("\n") + "\n\n" + links

    # Every check is done: from here on the files change.
    write_text(changelog, log)

    # --- include/naos/version.h ---

    header = read_text(version_header)
    for component, value in zip(("MAJOR", "MINOR", "PATCH"), new):
        header = re.sub(r"\n#define NAOS_VERSION_{} [0-9]+\n".format(component),
                        "\n#define NAOS_VERSION_{} {}\n".format(component, value), header)
    write_text(version_header, header)

    # --- The documentation and the examples ---

    if kind != "patch":
        files = [root / page for page in VERSIONED_PAGES]
        files += sorted((root / "examples").glob("*/CMakeLists.txt"))
        for path in files:
            text = old_text = read_text(path)
            text = re.sub(r"GIT_TAG v[0-9]+\.[0-9]+\.[0-9]+", "GIT_TAG v" + version, text)
            text = text.replace("find_package(Naos " + old_minor_version,
                                "find_package(Naos " + new_minor_version)
            text = text.replace(old_minor_version + ".x", new_minor_version + ".x")
            if text != old_text:
                write_text(path, text)

    if kind == "major" and old[0] == 0:
        print("warning: {} is the first version from 1.0 on: find_package() now accepts the same "
              "major version, not the same minor one. Update the text that says so in README.md, "
              "README.ja.md and docs/ by hand.".format(version), file=sys.stderr)

    print("Prepared {} ({}, from {})".format(version, kind, old_version))
    return 0


if __name__ == "__main__":
    sys.exit(main())
