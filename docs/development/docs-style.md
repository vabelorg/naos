# Documentation style

How the documentation of Naos is organized and written. Read it before adding or changing a page.

## Languages

- English is the source. Every page is written in English first, under `docs/` (and `README.md`; `CONTRIBUTING.md`, `SECURITY.md` and `CODE_OF_CONDUCT.md` live in `docs/` too).
- Japanese is a full translation: `docs/ja/` mirrors `docs/` file by file, with the same names, and `README.ja.md` translates the English file next to it. `docs/ja/CODE_OF_CONDUCT.md` is the official Japanese translation of the Contributor Covenant, not ours.
- A change to an English page does not wait for its translation. The translation follows in a later pull request, and a change that only fixes the English wording needs none.
- `CHANGELOG.md` is in English only, with no Japanese translation.
- The comments of the code are in English only, in the sources and in `examples/`. A translated page translates the comments of its code samples too; the code itself and the file names stay as they are.

## Where a page goes

The pages are of three kinds, plus a part for contributors. A page belongs to one kind; when it needs another kind, it links to it.

| Directory | Kind | A page |
|---|---|---|
| `getting-started/` | Tutorials: learning | Leads a newcomer through a complete, working result, step by step |
| `guides/` | How-to guides: doing | Solves one task, for a reader who knows the basics |
| `reference/` | Reference: looking up | States every rule of one part of the API, completely and precisely |
| `development/` | For contributors | Building, the architecture, the tests, the style, the releases |

A rule is stated once, in the reference. Guides and tutorials show how to use it and link to the reference rather than repeating it in full.

## Writing

- Short sentences, in the present tense, addressed to the reader ("Run ...", "you").
- Headings in sentence case. Guide headings name the task ("Run some tests only"); reference headings name the thing ("Exit codes").
- Code, names, options, paths and output in backticks.
- One term per concept; use the same word for the same thing throughout.
- No promise the code does not keep: anything a page states as behavior is checked by a self test or quoted from one.

## Code and output

- Code samples longer than a few lines come from `examples/`, which the self tests build and run. Copy the code from there, and change the page when the example changes.
- An output shown as the result of running something is copied from a real run, preferably from a golden file of `tests/examples/expected/`. Locations are shown in the GCC form, with the file name only (`clamp_test.c:10:`).
- Short fragments that are not complete programs may be written by hand, but must follow the API exactly.

## Links

- Relative links between pages (`../reference/cmake.md#options`), so that they work on GitHub and in a clone.
- Links to sections use the id GitHub gives the heading: lowercase, punctuation removed, spaces made `-`. Renaming a heading breaks the links to it; the self test `naos.docs.links` finds them.
- Code comments and CMake files that point to the documentation name the English page.
