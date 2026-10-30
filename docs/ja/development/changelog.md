# 変更履歴

`CHANGELOG.md` の書き方です。[Keep a Changelog 1.1](https://keepachangelog.com/ja/1.1.0/) に従います。新しいバージョンで利用者のテストやビルドにとって何が変わるかを伝えるもので、git のログの写しではありません。

## 記録するもの

利用者に見える変更を記録します。API、出力、コマンドライン、CMake の関数、動作、対応するプラットフォームです。1 つの項目は 1 行（か短い段落）で、現在形で、利用者の立場から書きます。

利用者にとって何も変わらないもの（内部の整理、自己テスト、文書の修正、CI）は書きません。

## 節

各バージョンの下には次の節を、この順に、項目があるものだけ置きます。

| 節 | 内容 |
|---|---|
| `Added` | 新しいマクロ、関数、オプション、CMake の関数 |
| `Changed` | 既存の動作の変更 |
| `Deprecated` | 後のバージョンで取り除くもの |
| `Removed` | 取り除いたもの |
| `Fixed` | 不具合の修正。何が誤っていて、どう現れたか |
| `Security` | 脆弱性の修正（[SECURITY.md](../SECURITY.md) を参照） |

互換を壊す変更は `Changed` か `Removed` に入れ、`**BREAKING**:` で始め、利用者がテストやビルドの何を直せばよいかを書きます。

```markdown
- **BREAKING**: `--naos-foo` is renamed `--naos-bar`. Change the option in scripts and in `naos_discover_tests()` calls.
```

## 開発中

利用者に見える変更をするプルリクエストは、同じ変更で `## [Unreleased]` の下に項目を足します。足さない場合は、その理由をプルリクエストに書きます。

## リリースのとき

[リリース](release.md)のワークフローが、`scripts/prepare-release.cmake` と `release-notes.cmake` で次を行います。

1. `## [Unreleased]` を `## [<version>] - <YYYY-MM-DD>` に変え、その上に空の `## [Unreleased]` を足す。
2. ファイルの末尾のリンクを更新する。`[Unreleased]` は新しいタグと `HEAD` を比べ、`[<version>]` はそのリリースを指す。
3. そのバージョンの節を GitHub のリリースの本文にする。

バージョンは項目から決まります。`BREAKING` か `Added` の項目があれば（1.0 より前は）マイナーバージョンが必要で、パッチバージョンには `Fixed` と `Security` の項目しか入りません。項目に合わないバージョンと、空の `[Unreleased]` は、スクリプトが受け付けません。

## 最初のリリース

最初に記録するバージョンは `0.1.0` です。その節は `Added` だけにし、機能ごとに 1 行ずつではなく、主な機能を数行で書きます。
