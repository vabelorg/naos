# Changelog

How `CHANGELOG.md` is kept. It follows [Keep a Changelog 1.1](https://keepachangelog.com/en/1.1.0/). It tells a user what a new version changes for their tests and build; it is not a copy of the git log.

## What to record

Record a change a user can see: the API, the output, the command line, the CMake functions, the behavior, the supported platforms. One entry is one line (or a short paragraph), in the present tense, from the point of view of the user.

Leave out what changes nothing for users: internal refactoring, self tests, documentation fixes, CI.

## Sections

Under each version, use these sections, in this order, and only the ones that have entries:

| Section | For |
|---|---|
| `Added` | New macros, functions, options, CMake functions |
| `Changed` | Changes to existing behavior |
| `Deprecated` | What will be removed in a later version |
| `Removed` | What was removed |
| `Fixed` | Bug fixes: what was wrong and how it showed |
| `Security` | Fixes of vulnerabilities (see [SECURITY.md](../SECURITY.md)) |

A change that breaks compatibility goes in `Changed` or `Removed`, starts with `**BREAKING**:`, and says what a user has to change in their tests or build:

```markdown
- **BREAKING**: `--naos-foo` is renamed `--naos-bar`. Change the option in scripts and in `naos_discover_tests()` calls.
```

## During development

A pull request that makes a change users can see adds an entry under `## [Unreleased]`, in the same change. If it adds none, the pull request says why.

## At a release

The workflows of [releasing](release.md) do this, with `scripts/prepare-release.cmake` and `release-notes.cmake`:

1. Rename `## [Unreleased]` to `## [<version>] - <YYYY-MM-DD>`, and add an empty `## [Unreleased]` above it.
2. Update the links at the end of the file: `[Unreleased]` compares the new tag with `HEAD`, and `[<version>]` points to the release.
3. Use the section of the version as the body of the GitHub release.

The version follows from the entries: a `BREAKING` entry or an `Added` entry needs a minor version (before 1.0) and a patch version can have `Fixed` and `Security` entries only. The script refuses a version the entries do not allow, and an empty `[Unreleased]`.

## The first release

`0.1.0` is the first version recorded. Its section is `Added` only, with a few lines for the main features, not one line for each.
