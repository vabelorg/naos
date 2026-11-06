#!/usr/bin/env python3
# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
"""Tests the scripts that prepare a release (docs/development/release.md).

Runs prepare_release.py and release_notes.py on a small copy of the files they read and change, made
in a temporary directory: each case writes the files, runs a script with a version, and compares
what it changed, or the message with which it refused the version. Needs no build: run by
scripts/lint.py, and by hand with  python3 scripts/check_release_scripts.py
"""

import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

SCRIPTS = Path(__file__).resolve().parent
URL = "https://github.com/example/naos"

# The pages prepare_release.py updates for a new minor or major version.
VERSIONED_PAGES = ["README.md", "README.ja.md", "examples/quickstart/CMakeLists.txt"] + [
    "docs/{}{}.md".format(lang, page)
    for lang in ("", "ja/")
    for page in ("getting-started/installation", "getting-started/quickstart", "reference/cmake",
                 "guides/troubleshooting")
]

FIXED = "### Fixed\n\n- A fix.\n"
ADDED = "### Added\n\n- A feature.\n"
BREAKING = "### Changed\n\n- **BREAKING**: A change.\n"


def version_header(major, minor, patch):
    return ("#ifndef NAOS_VERSION_H\n#define NAOS_VERSION_MAJOR {}\n#define NAOS_VERSION_MINOR {}\n"
            "#define NAOS_VERSION_PATCH {}\n#endif\n").format(major, minor, patch)


def page(version):
    major, minor, _ = version.split(".")
    return ("GIT_REPOSITORY {0}.git\nGIT_TAG v{1}\nfind_package(Naos {2}.{3} REQUIRED)\n"
            "accepts {2}.{3}.x only\n").format(URL, version, major, minor)


class ReleaseScripts(unittest.TestCase):
    def setUp(self):
        directory = tempfile.TemporaryDirectory()
        self.addCleanup(directory.cleanup)
        self.work = Path(directory.name)

    def write_repository(self, version, unreleased):
        """Writes the files of a repository at version, whose [Unreleased] holds unreleased."""
        files = {
            "include/naos/version.h": version_header(*version.split(".")),
            "CHANGELOG.md": (
                "# Changelog\n\n## [Unreleased]\n\n{1}\n## [{0}] - 2026-01-01\n\n### Added\n\n"
                "- The start.\n\n[Unreleased]: {2}/compare/v{0}...HEAD\n[{0}]: {2}/releases/tag/v{0}\n"
            ).format(version, unreleased, URL),
        }
        for name in VERSIONED_PAGES:
            files[name] = page(version)
        for name, text in files.items():
            path = self.work / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(text.encode("utf-8"))

    def run_script(self, script, *args):
        """Runs a script of scripts/ on the copy and returns its exit code and output."""
        result = subprocess.run(
            [sys.executable, str(SCRIPTS / script), "--root", str(self.work)] + list(args),
            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=True)
        return result.returncode, result.stdout

    def assert_success(self, result):
        code, output = result
        self.assertEqual(code, 0, "the script failed:\n" + output)
        return output

    def assert_refused(self, result, regex):
        code, output = result
        self.assertNotEqual(code, 0, "the version was not refused:\n" + output)
        self.assertRegex(output, regex)

    def assert_file(self, name, expected):
        self.assertEqual((self.work / name).read_bytes().decode("utf-8"), expected, name)

    def test_patch(self):
        """A patch version: the changelog closed and linked, the version set, the pages left alone."""
        self.write_repository("0.2.0", FIXED)
        self.assert_success(self.run_script("prepare_release.py", "0.2.1", "--date", "2026-02-03"))
        self.assert_file("CHANGELOG.md", (
            "# Changelog\n\n## [Unreleased]\n\n## [0.2.1] - 2026-02-03\n\n### Fixed\n\n- A fix.\n\n"
            "## [0.2.0] - 2026-01-01\n\n### Added\n\n- The start.\n\n"
            "[Unreleased]: {0}/compare/v0.2.1...HEAD\n[0.2.1]: {0}/compare/v0.2.0...v0.2.1\n"
            "[0.2.0]: {0}/releases/tag/v0.2.0\n").format(URL))
        self.assert_file("include/naos/version.h", version_header(0, 2, 1))
        self.assert_file("README.md", page("0.2.0"))

        # The notes of that release: its section, without the heading.
        notes = str(self.work / "notes.md")
        self.assert_success(self.run_script("release_notes.py", "0.2.1", notes))
        self.assert_file("notes.md", "### Fixed\n\n- A fix.\n")
        self.assert_refused(self.run_script("release_notes.py", "0.9.0", notes),
                            r"CHANGELOG\.md has no section for 0\.9\.0")

    def test_minor(self):
        """A minor version: the tag and the minor version of every page, a leading zero dropped."""
        self.write_repository("0.2.1", ADDED)
        self.assert_success(self.run_script("prepare_release.py", "0.03.0", "--date", "2026-02-03"))
        for name in VERSIONED_PAGES:
            self.assert_file(name, page("0.3.0"))

    def test_major(self):
        """1.0 changes what find_package() accepts, which the pages tell in words: a warning says so."""
        self.write_repository("0.3.0", BREAKING)
        output = self.assert_success(
            self.run_script("prepare_release.py", "1.0.0", "--date", "2026-02-03"))
        self.assertRegex(output, r"first version from 1\.0 on")

    def test_refused(self):
        """The versions refused, and why."""
        self.write_repository("0.2.0", ADDED)
        self.assert_refused(self.run_script("prepare_release.py", "0.2.1"),
                            r"0\.2\.1 is a patch version")
        self.write_repository("0.2.0", FIXED)
        self.assert_refused(self.run_script("prepare_release.py", "0.2.0"),
                            r"is not greater than 0\.2\.0")
        self.assert_refused(self.run_script("prepare_release.py", "0.3"),
                            r"is not MAJOR\.MINOR\.PATCH")
        self.assert_refused(self.run_script("prepare_release.py", "0.256.0"),
                            r"each component must be < 256")
        self.write_repository("1.0.0", BREAKING)
        self.assert_refused(self.run_script("prepare_release.py", "1.1.0"),
                            r"from 1\.0 on it needs a major version")

    def test_nothing_to_release(self):
        """An empty [Unreleased] is refused, and a refused version changes no file."""
        self.write_repository("0.2.0", "")
        self.assert_refused(self.run_script("prepare_release.py", "0.3.0"),
                            r"is empty: record the changes first")
        self.assert_file("include/naos/version.h", version_header(0, 2, 0))


class VersionReading(unittest.TestCase):
    def test_same_as_cmake(self):
        """prepare_release.py reads include/naos/version.h as naos_read_version() of
        cmake/NaosVersion.cmake does for the build: both give the version of the sources."""
        sys.dont_write_bytecode = True
        sys.path.insert(0, str(SCRIPTS))
        try:
            import prepare_release
        finally:
            sys.path.pop(0)
        root = SCRIPTS.parent
        header = root / "include/naos/version.h"
        script = (
            'include("{}")\nnaos_read_version("{}")\nmessage(NOTICE "${{naos_version}}")\n'
        ).format((root / "cmake/NaosVersion.cmake").as_posix(), header.as_posix())
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "ReadVersion.cmake"
            path.write_text(script, encoding="utf-8")
            result = subprocess.run(["cmake", "-P", str(path)], stdout=subprocess.PIPE,
                                    stderr=subprocess.STDOUT, universal_newlines=True)
        self.assertEqual(result.returncode, 0, result.stdout)
        self.assertEqual(result.stdout.strip(),
                         "{}.{}.{}".format(*prepare_release.read_version(header)))


if __name__ == "__main__":
    unittest.main()
