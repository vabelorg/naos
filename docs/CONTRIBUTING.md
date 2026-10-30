# Contributing to Naos

Thank you for your interest in Naos. Bug reports, documentation fixes, translations and code are all welcome. Everyone taking part follows the [code of conduct](CODE_OF_CONDUCT.md).

## Before you start

- **Bugs**: open an [issue](https://github.com/vabelorg/naos/issues/new/choose) with the version, the platform and compiler, a minimal test that shows the problem, and its output.
- **Features**: open an issue first and describe the problem it solves. Naos keeps a narrow scope on purpose, and its [principles](development/architecture.md#principles) come before features: process isolation, parallel runs and report formats, for example, are left to CTest and CI.
- **Security issues**: do not open an issue; report them privately as described in the [security policy](SECURITY.md).
- **Small fixes** (typos, broken links, clearer wording) can go straight to a pull request.

## Set up

You need a C99 compiler (GCC, Clang or MSVC), CMake 3.21 or later and Git; a C++ compiler for the C++ tests.

```sh
git clone https://github.com/vabelorg/naos.git
cd naos
cmake --preset default
cmake --build --preset default
ctest --preset default
```

Every self test should pass before you change anything. [Building](development/building.md) has the details, other compilers and the shared build.

## Make the change

1. Create a branch from `main`.
2. Read the page for your kind of change:
   - code: [architecture](development/architecture.md), [extending](development/extending.md) and [coding style](development/coding-style.md)
   - tests: [testing](development/testing.md)
   - documentation: [documentation style](development/docs-style.md)
3. Add or update the self tests: every behavior change comes with a test that fails without it.
4. Update the documentation in the same change: the reference for any change to the API, the output or the CMake functions. The English pages are updated in the same change; the Japanese translation (`docs/ja/`) may follow in a later pull request ([documentation style](development/docs-style.md#languages)).
5. Add an entry under `[Unreleased]` in `CHANGELOG.md` for any change users can see ([changelog](development/changelog.md)).
6. Run the self tests with both presets, `default` and `shared`.

## Commits and pull requests

- Commit messages, in English: a short summary in the imperative ("Add NAOS_CHECK_MEM_NE"), a blank line, then why, if it is not obvious.
- One topic per pull request. Fill in the template: what changes, why, and how it was tested.
- CI must pass on every platform. A maintainer reviews the change and may ask for changes before merging.

## License

Naos is under the [MIT License](../LICENSE). By contributing, you agree that your contribution is distributed under it. New source files start with the copyright notice and `SPDX-License-Identifier: MIT`, like the existing ones.
