# Releasing

How a version of Naos is released. Releases are tagged `v<major>.<minor>.<patch>` on `main`. Two workflows do the work: you choose the version and review the pull request they open.

## Choose the version

- **Patch** (0.0.0 → 0.0.1): fixes that change nothing public.
- **Minor** (0.0.x → 0.1.0): anything else. Before 1.0, a minor version may break compatibility.
- **Major**: from 1.0 on, for incompatible changes.

The entries under `[Unreleased]` in `CHANGELOG.md` decide it, as [changelog](changelog.md#at-a-release) describes. The release is refused when they do not allow the version.

## Steps

1. Make sure CI passes on `main`, and that `[Unreleased]` in `CHANGELOG.md` records every change users can see.
2. In the Actions tab, run **Release PR** with the version. Tick **dry_run** first to see the changes and the notes in the summary of the run, without opening anything.
3. The workflow opens the pull request `Release <version>`. It:
   - sets the version in `include/naos/version.h`, its only home: CMake derives the rest from it;
   - for a new minor or major version, updates the tag used by `FetchContent` (`GIT_TAG v…`) and the minor version named in `find_package(naos 0.0)` in the README files, the documentation and the examples;
   - closes the changelog: renames `[Unreleased]` to the version and its date, and updates the links at the end of `CHANGELOG.md`.
4. CI runs on the pull request: the self tests with both presets, and a project that uses the installed package with `find_package(naos <version>)` (`tests/package/`). Review the changes and merge.
5. Merging runs **Release**: it tags the merge commit `v<version>`, creates the GitHub release with the section of this version in `CHANGELOG.md` as its notes, and deletes the branch.

For 1.0, the workflow warns that the text on compatibility in the README files and the documentation must change by hand (from the same minor version to the same major version): push the fix to the pull request before merging.

## Setup

**Release PR** needs a repository secret `RELEASE_TOKEN`: CI does not run on a pull request opened with the `GITHUB_TOKEN` of a workflow. Create a fine-grained personal access token for this repository with the permissions *Contents* and *Pull requests* (read and write), and add it under *Settings → Secrets and variables → Actions*.

## By hand

The workflows run two CMake scripts that also work locally, from the root of Naos:

```sh
cmake -D ROOT=. -D VERSION=0.1.0 -P scripts/prepare-release.cmake
cmake -D ROOT=. -D VERSION=0.1.0 -D OUTPUT=notes.md -P scripts/release-notes.cmake
```

The first changes the files as in step 3, the second writes the notes of the release. Then commit as `Release <version>`, tag, push and create the GitHub release from the tag with these notes:

```sh
git tag -a v0.1.0 -m "Naos 0.1.0"
git push origin main v0.1.0
gh release create v0.1.0 --verify-tag --title "Naos 0.1.0" --notes-file notes.md
```
