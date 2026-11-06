# Releasing

How a version of Naos is released. Releases are tagged `v<major>.<minor>.<patch>` on `main`. Two workflows do the work: you choose the version and review the pull request they open.

## Choose the version

- **Patch** (0.0.0 → 0.0.1): fixes that change nothing public.
- **Minor** (0.0.x → 0.1.0): anything else. Before 1.0, a minor version may break compatibility.
- **Major**: from 1.0 on, for incompatible changes.

The entries under `[Unreleased]` in `CHANGELOG.md` decide which versions are allowed, as [changelog](changelog.md#at-a-release) describes. The release is refused when they do not allow the version.

## Steps

1. Make sure CI passes on `main`, and that `[Unreleased]` in `CHANGELOG.md` records every change users can see.
2. In the Actions tab, run **Release PR** with the version. Tick **dry_run** for the first run: it opens nothing, and shows the changes and the notes in the summary of the run. Check them.
3. If they are right, run **Release PR** again with the same version and **dry_run** unticked. The workflow opens the pull request `Release <version>`. It:
   - sets the version in `include/naos/version.h`, its only home: CMake derives the rest from it;
   - for a new minor or major version, updates the tag used by `FetchContent` (`GIT_TAG v…`) and the minor version named in `find_package(Naos 0.0)` in the README files, the documentation and the examples;
   - closes the changelog: renames `[Unreleased]` to the version and its date, and updates the links at the end of `CHANGELOG.md`.
4. CI runs on the pull request: lint, which tests the release scripts too, the self tests with every preset and platform, and a project that uses the installed package with `find_package(Naos <version>)` (`tests/consumer/`, with `scripts/TestPackage.cmake`). Review the changes and merge.
5. Merging runs **Release**: it tags the merge commit `v<version>`, creates the GitHub release with the section of this version in `CHANGELOG.md` as its notes, and deletes the branch.

For 1.0, the workflow warns that the text on compatibility in the README files and the documentation must change by hand (from the same minor version to the same major version): push the fix to the pull request before merging.

## Setup

**Release PR** needs a repository secret `RELEASE_TOKEN`: CI does not run on a pull request opened with the `GITHUB_TOKEN` of a workflow. Before the first release, add it as follows:

1. In the *Settings → Developer settings → Personal access tokens → Fine-grained tokens* of your GitHub account, press *Generate new token*.
2. Under *Repository access*, choose *Only select repositories* and select this repository.
3. Under *Repository permissions*, set *Contents* and *Pull requests* to *Read and write*.
4. Generate the token and copy its value.
5. In *Settings → Secrets and variables → Actions* of this repository, press *New repository secret*. Name it `RELEASE_TOKEN`, and paste the token as its value.

## By hand

The workflows run two Python scripts that also work locally, from the root of Naos:

```sh
python3 scripts/prepare_release.py 0.1.0
python3 scripts/release_notes.py 0.1.0 notes.md
```

The first changes the files as in step 3, the second writes the notes of the release. Then commit as `Release <version>`, tag, push and create the GitHub release from the tag with these notes:

```sh
git tag -a v0.1.0 -m "Naos 0.1.0"
git push origin main v0.1.0
gh release create v0.1.0 --verify-tag --title "Naos 0.1.0" --notes-file notes.md
```
