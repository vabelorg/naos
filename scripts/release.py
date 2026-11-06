#!/usr/bin/env python3
# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
"""Prepares a release and writes its notes, as docs/development/release.md describes.

    python3 scripts/release.py prepare <version> [--repo-url <url>] [--date <YYYY-MM-DD>]
    python3 scripts/release.py notes <version> <output>

prepare checks that the entries of [Unreleased] in CHANGELOG.md allow the version, sets it in
include/naos/version.h, updates the tag and the minor version named in the README files, the
documentation and the examples for a new minor or major version, and closes the changelog. It
changes the files only, commits nothing. notes writes the section of the version in CHANGELOG.md,
without its heading. Both are run by the workflows of .github/.
"""

import argparse
import datetime
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CHANGELOG = ROOT / "CHANGELOG.md"
VERSION_HEADER = ROOT / "include/naos/version.h"


def fail(message):
    sys.exit("error: " + message)


def read_text(path):
    return path.read_text(encoding="utf-8").replace("\r", "")


def write_text(path, text):
    """Writes with LF line endings on every platform."""
    path.write_bytes(text.encode("utf-8"))


def section(log, heading):
    """Returns the text of CHANGELOG.md after the heading, to the next version heading or to the
    links at the end of the file: (body, after), or None if the heading is not there."""
    match = re.search(heading, log)
    if not match:
        return None
    rest = log[match.end():]
    end = len(rest)
    for marker in ("\n## [", "\n[Unreleased]: "):
        at = rest.find(marker)
        if at != -1 and at < end:
            end = at
    return rest[:end], rest[end:]


def read_version():
    """Returns (major, minor, patch) of include/naos/version.h, read by naos_read_version() of
    cmake/NaosVersion.cmake, as the build reads it."""
    result = subprocess.run(
        ["cmake", "-D", "HEADER=" + str(VERSION_HEADER), "-P",
         str(ROOT / "cmake/NaosVersion.cmake")],
        stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
    if result.returncode != 0:
        fail("cannot read the version of {}:\n{}".format(VERSION_HEADER, result.stderr.strip()))
    return tuple(int(part) for part in result.stdout.strip().split("."))


def parse_version(text):
    """Returns (major, minor, patch) of MAJOR.MINOR.PATCH, each < 256 as the macros need."""
    match = re.fullmatch(r"([0-9]+)\.([0-9]+)\.([0-9]+)", text)
    if not match:
        fail("'{}' is not MAJOR.MINOR.PATCH".format(text))
    version = tuple(int(part) for part in match.groups())
    if any(part >= 256 for part in version):
        fail("'{}': each component must be < 256".format(text))
    return version


def prepare(args):
    new = parse_version(args.version)
    old = read_version()
    version = "{}.{}.{}".format(*new)
    if new <= old:
        fail("{} is not greater than {}.{}.{}, the version in include/naos/version.h".format(
            version, *old))
    kind = "major" if new[0] != old[0] else "minor" if new[1] != old[1] else "patch"

    # The changelog: [Unreleased] must allow the version (docs/development/changelog.md).
    log = read_text(CHANGELOG)
    found = section(log, r"## \[Unreleased\]\n")
    if not found:
        fail("CHANGELOG.md has no '## [Unreleased]' heading")
    unreleased, after = found
    unreleased = unreleased.strip()
    if not unreleased:
        fail("[Unreleased] in CHANGELOG.md is empty: record the changes first")
    if kind == "patch":
        for heading in re.findall(r"(?:^|\n)(### [^\n]*)", unreleased):
            if not re.fullmatch(r"### (Fixed|Security)", heading.strip()):
                fail("[Unreleased] has '{}': {} is a patch version, which can have Fixed and "
                     "Security entries only".format(heading.strip(), version))
    elif "**BREAKING**" in unreleased and kind == "minor" and new[0] > 0:
        fail("[Unreleased] has a BREAKING entry: from 1.0 on it needs a major version")

    if args.repo_url:
        repo_url = args.repo_url.rstrip("/")
    else:
        match = re.search(r"GIT_REPOSITORY (https://[^ \n]+)\.git", read_text(ROOT / "README.md"))
        if not match:
            fail("README.md has no GIT_REPOSITORY to link to; give --repo-url <url>")
        repo_url = match.group(1)
    date = args.date or datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%d")

    previous = re.search(r"\n## \[([0-9]+\.[0-9]+\.[0-9]+)\]", after)
    link = ("{}/compare/v{}...v{}".format(repo_url, previous.group(1), version) if previous
            else "{}/releases/tag/v{}".format(repo_url, version))
    links = "[Unreleased]: {0}/compare/v{1}...HEAD\n[{1}]: {2}\n".format(repo_url, version, link)
    before = log.split("## [Unreleased]\n", 1)[0]
    log = "{}## [Unreleased]\n\n## [{}] - {}\n\n{}\n{}".format(before, version, date, unreleased,
                                                               after)
    match = re.search(r"\n\[Unreleased\]: [^\n]*\n", log)
    log = log.replace(match.group(0), "\n" + links) if match else log.rstrip("\n") + "\n\n" + links

    # Every check is done: from here on the files change.
    write_text(CHANGELOG, log)

    header = read_text(VERSION_HEADER)
    for component, value in zip(("MAJOR", "MINOR", "PATCH"), new):
        header = re.sub(r"\n#define NAOS_VERSION_{} [0-9]+\n".format(component),
                        "\n#define NAOS_VERSION_{} {}\n".format(component, value), header)
    write_text(VERSION_HEADER, header)

    # The tag FetchContent fetches and the minor version find_package() asks for, wherever users
    # read them. The pages of docs/development/ are left alone: their versions are examples.
    if kind != "patch":
        old_minor, new_minor = "{}.{}".format(*old[:2]), "{}.{}".format(*new[:2])
        files = list(ROOT.glob("README*.md")) + list((ROOT / "examples").glob("*/CMakeLists.txt"))
        files += [p for p in (ROOT / "docs").rglob("*.md") if "development" not in p.parts]
        for path in sorted(files):
            text = old_text = read_text(path)
            text = re.sub(r"GIT_TAG v[0-9]+\.[0-9]+\.[0-9]+", "GIT_TAG v" + version, text)
            text = text.replace("find_package(Naos " + old_minor, "find_package(Naos " + new_minor)
            text = text.replace(old_minor + ".x", new_minor + ".x")
            if text != old_text:
                write_text(path, text)

    if kind == "major" and old[0] == 0:
        print("warning: {} is the first version from 1.0 on: find_package() now accepts the same "
              "major version, not the same minor one. Update the text that says so in README.md, "
              "README.ja.md and docs/ by hand.".format(version), file=sys.stderr)
    print("Prepared {} ({}, from {}.{}.{})".format(version, kind, *old))


def notes(args):
    version = "{}.{}.{}".format(*parse_version(args.version))
    found = section(read_text(CHANGELOG), r"\n## \[" + re.escape(version) + r"\][^\n]*\n")
    if not found:
        fail("CHANGELOG.md has no section for " + version)
    body = found[0].strip()
    if not body:
        fail("The section of {} in CHANGELOG.md is empty".format(version))
    write_text(Path(args.output), body + "\n")


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    commands = parser.add_subparsers(dest="command", required=True)
    p = commands.add_parser("prepare", help="set the version and close the changelog")
    p.add_argument("version", help="the version to release, x.y.z")
    p.add_argument("--repo-url", help="the repository the changelog links to; by default the one "
                   "README.md fetches with FetchContent")
    p.add_argument("--date", help="the date of the release; by default today (UTC)")
    p.set_defaults(run=prepare)
    p = commands.add_parser("notes", help="write the notes of a released version")
    p.add_argument("version", help="the version released, x.y.z")
    p.add_argument("output", help="the file to write")
    p.set_defaults(run=notes)
    args = parser.parse_args()
    args.run(args)


if __name__ == "__main__":
    main()
