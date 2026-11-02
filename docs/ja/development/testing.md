# テスト

Naos の自己テストについて、層ごとに何を確かめ、誰が判定するか、出力をどう照合するか、ケースをどう足すかを説明します。実行の仕方は[ビルド](building.md)にあります。

## 層

Naos はテストハーネスなので、自己テストの多くは Naos 自身には判定させられません。判定を誤る Naos は、そのテストをすべて通してしまうからです。`tests/` の最初のディレクトリは**層**で、そのケースを誰が判定するかを表します。その下のディレクトリには、1 つの領域のプログラム、ケースを登録する `CMakeLists.txt`、golden のケースなら期待する出力（`expected/<case>.txt`）を置きます。

| 層 | 判定するもの | 確かめること |
|---|---|---|
| `contract/` | 終了コード、正規表現、XML のスキーマ。golden ファイルは使わない | Naos が互換を保つもの（[出力](../reference/output.md#互換を保つもの)、[JUnit XML](../reference/xml.md#互換性)）: 終了コード、診断の行の先頭、`--naos-list` の名前、JUnit XML のファイル |
| `api/` | Naos 自身 | 公開 API を、プロジェクトが使うとおりに C と C++ で。公開ヘッダごとに 1 ファイル |
| `unit/` | Naos 自身 | 内部のモジュール。結果が引数だけで決まるもの（`value`、`filter`、`order`、`text`）と、イベントで動かす writer（`xml` の JUnit XML のファイル、`human` の診断）。モジュールごとに 1 ファイル |
| `behavior/` | golden ファイル（[下記](#出力を照合するケース)） | テストの実行について[リファレンス](../reference/README.md)が述べることすべて。ページごとに 1 ディレクトリ: `tests/`、`assertions/`、`control/`、`command-line/`、`output/`、`xml/` |
| `build/` | コンパイラと CMake | コンパイルできてはいけないプログラム（`misuse/`）、CMake を使わないビルド（`no_cmake/`）、テストの検出のメッセージ（`discovery/`）、[CMake の関数](../reference/cmake.md#関数)（`functions/`）、`add_subdirectory()` で加えた Naos（`subproject/`） |
| `examples/` | 両方 | `examples/` のプロジェクトと、ドキュメントが引用する出力（[下記](#サンプルとドキュメント)） |

`tests/consumer/` は層ではありません。Naos を使うプロジェクトで、`build/subproject/` が `add_subdirectory()` で、`scripts/TestPackage.cmake` がビルドをインストールしたあと `find_package(Naos <version>)` で Naos を加えます（[ビルド](building.md#インストールする)）。仕組みは `tests/harness/` にあります。`NaosSelftest.cmake` の関数、ケースを 1 つ実行する `RunCase.cmake`、自分のプロジェクトをビルドするケースのための `NestedProject.cmake` です。

### 名前とラベル

ケースの CTest 名は `naos.<path>.<case>` です。`<path>` は `tests/` の下のディレクトリで、`/` を `.` にしたもの: `naos.behavior.assertions.integer.eq`、`naos.contract.exit_code.fail`。どのケースにもラベル `naos` と層のラベルが付き、自分のプロジェクトをビルドするケースにはラベル `slow` も付きます。`examples/` のプロジェクトのテストは、利用者が `CMakeLists.txt` を写すので、実行ファイルの名前のままです（`clamp_tests.clamp.inside_range`）。

```sh
ctest --preset default -L contract     # 1 つの層
ctest --preset default -LE slow        # プロジェクトをビルドするケース以外すべて
ctest --preset default -R '^naos\.behavior\.xml\.'
```

### Naos を信頼する

`contract/` のうち、成功・失敗・スキップの終了コードを確かめるケースが CTest のフィクスチャ `naos_trust` をセットアップし、`api/` と `unit/` のテストがそれを必要とします。失敗が失敗と判定されることが示されるまで、CTest は Naos が自分で判定するテストを実行せず、成功ではなく未実行として示します。`-R` や `-L` でそれらを選んだ実行にも、CTest はセットアップを加えます。

## いつ何を実行するか

| いつ | 実行するもの | 目的 |
|---|---|---|
| ビルドの前 | `cmake -D ROOT=. -P scripts/Lint.cmake`（[lint](#lint)） | ソースだけでわかることを数秒で |
| Naos を変えている間 | `ctest --preset default`、またはその 1 つの層 | 何も壊していないことを数秒で |
| 変更を送る前 | プリセット `default` と `shared`。`src/` の変更には `release` と `sanitize` も（[ビルド](building.md#プリセットを使う)） | ライブラリの種類、最適化、サニタイザが見つけるもの |
| プルリクエストごと | CI: lint、すべてのプラットフォームのすべてのプリセット、インストールしたパッケージ（[ビルド](building.md#継続的インテグレーション)） | プラットフォーム、コンパイラ、対応するいちばん古い CMake が見つけるもの |

## 契約のケース

`contract/` のケースは、終了コード、正規表現、スキーマ `contract/junit.xsd` で判定し、golden ファイルは使いません。`NAOS_UPDATE_EXPECTED=1` で golden ファイルを書き直しても、確かめることは変わりません。ここのケースを変えることは Naos が互換を保つものを変えることで、[変更履歴](changelog.md)に記録します。

- **終了コード**: [終了コード](../reference/command-line.md#終了コード)の各行に 1 ケース。
- **診断の行の先頭**: ライブラリをビルドしたコンパイラの形式の位置（ファイルはコンパイラが与えたとおり）、続いて種類と `: `。
- **`--naos-list`**: `ListRoundTrip.cmake` が、標準出力には名前だけが 1 行に 1 つあること、どの名前も `--naos-filter` に渡すとそのテストだけを実行すること（JUnit XML のファイルでわかる）を確かめます。CTest の検出はこの両方に頼っています。
- **JUnit XML のファイル**: xmllint があるとき、すべての結果、shuffle 付きの repeat、fail fast のファイルが、[互換性](../reference/xml.md#互換性)が保つものを書いた `junit.xsd` に合うこと。また、ファイルが[リファレンスのとおりに](../reference/xml.md#ファイルを書くとき)書かれる、空のまま残る、書かれないこと。CI は Linux に xmllint を入れます。macOS にはあります。

## 出力を照合するケース

`behavior/` のケースはふつう、**期待する出力の全体**、golden ファイルと照合します。`expected/<case>.txt` は、引数、終了コード、標準出力、標準エラーをそのまま持ちます。

```
args: --naos-filter integer.eq
exit code: 1
--- stdout
RUN: integer.eq
integer.c:LINE: error: -1 == 2
  left:  -1
  right: 2
FAIL: integer.eq (1 failure, N ms)
SUMMARY: 0 passed, 1 failed, 0 skipped (N ms)
--- stderr
```

どのプラットフォームでも同じ内容になるように、照合の前に次だけを置き換えます。

- 診断の位置 `<directory>/<file>:<line>: `（MSVC では `(<line>)`）: `<file>:LINE: ` に。テストを書き換えても期待する出力が変わらないようにするため。位置そのものは `contract/` で確かめる
- 結果の行や `SUMMARY:` の最後の時間 `(<n> ms)` や `, <n> ms)`: `N ms` に。実行のたびに変わるため。時間を測っていることは `behavior/output/` で正規表現を使って確かめる
- `--help` の最後の行が挙げる時計: `<clock>` に。システムによって変わるため。`wall-clock time, from a monotonic clock` か `processor time, from clock(), not wall-clock time` で、それ以外は置き換えない
- 実行ファイルのパス: `PROG` に。パスの長さで折り返す位置が変わる使い方の行は、1 行につなぐ
- CR: 取り除く

期待する出力にない行は、現れてはならない行です。`REJECT` の正規表現を並べる必要はありません。ある場所に到達しないことを確かめるには、そこで `reached-...` を出力します。期待する出力にそれがないことで確かめられます。

実行やプラットフォームで変わる出力（ポインタの値、時刻から取る seed、`long` の幅で桁数が変わる数、テストを足すと増える出力）だけは、代わりに**正規表現**で照合します（`naos_expect_match`）。

`--naos-output` の XML のように実行ファイルが書くファイルのケースは、`naos_expect` に `OUTPUT_FILE <name>` を渡します。引数の `{output}` はビルドディレクトリのそのファイルを表します。ファイルは実行の前に消し、実行のあと 3 つ目の部分 `--- output` で照合します（書かれないときは `[no file]`）。位置は同じように揃え（`file="<file>" line="LINE"`）、時間は `time="N.NNN"` にします。ほかの形の時間はそのまま残り、照合に失敗します。xmllint があるときは、ファイルが整形式の XML であることも確かめます。標準出力に ASCII 以外のバイトを出すテストは、`FILE_ONLY` も渡してファイルだけを照合します。CMake は標準出力をプラットフォームのコードページで読むので、そのバイトが変わってしまうためです。

## プロジェクトをビルドするケース

`build/functions/` と `build/subproject/` のケースは、Naos のビルドと同じジェネレータ、コンパイラ、構成で、自分のプロジェクトを configure し、ビルドし、テストします（`naos_selftest_add_project_case()` と `NestedProject.cmake`）。`functions/` は、CMake の関数の引数の誤りを 1 つずつ configure だけで確かめ、テストの登録のされ方を `Registration.cmake` で確かめます。`subproject/` は `add_subdirectory()` で Naos を加えた `tests/consumer/` をビルドします。Naos が自分の外に何かを及ぼすと、その configure が失敗します。これらのケースは 1 つ数秒かかるので、ラベル `slow` により `-LE slow` で外せます。

## ケースを足す

変えたものを判定する層にケースを足します。

| 変えるもの | ケースを足す場所 |
|---|---|
| 互換を保つもの: 終了コード、診断の行の先頭、`--naos-list`、JUnit XML のファイル | `contract/`。同じ変更でリファレンスと変更履歴も |
| リファレンスが述べる振る舞い: アサーション、オプション、出力の行、XML のファイルの要素 | `behavior/<page>/`。それを述べる[リファレンス](../reference/README.md)のページ |
| エスケープのように境界の多い内部の関数 | `unit/<module>.c` |
| プロジェクトが使うとおりの公開 API（C と C++） | `api/<header>.c` と `api/cpp.cpp` |
| CMake の関数、パッケージ、ビルド | `build/` |
| サンプル、またはドキュメントが引用する出力 | `examples/` |

golden のケースは次のように足します。

1. 領域のプログラムにテストを書く。失敗させるためのテストも、ほかと同じように書く。テスト関数の名前はファイルの中で一意でなければならないので、スイートを別のファイルにしてもよい。
2. `CMakeLists.txt` に登録する。1 つのテストの出力を見るなら `naos_expect_tests(<target> <suite>.<test>...)`、引数を渡すなら `naos_expect(<case> <target> ARGS ...)`。
3. `NAOS_UPDATE_EXPECTED=1 ctest --preset default -R <case>` で期待する出力を書き、**読んで正しいことを確かめてから**コミットする。

出力の形式を変えたときも同じです。`NAOS_UPDATE_EXPECTED=1 ctest --preset default` で書き直し、変わった行がすべて意図したものであることを `git diff` で確かめます。合わないケースは、最初に違う行（空白が見えるように `[...]` で囲む）と、実際の出力の全体を示します。`contract/` のケースはこの更新では変わらず、互換を保つものに触れる変更なら失敗します。

新しい領域は新しいディレクトリにし、その層の `CMakeLists.txt` の一覧に足します。

## サンプルとドキュメント

`examples/` のプロジェクトは、利用者がそのまま写せる完全なプロジェクトです。単独でビルドするときは `FetchContent` で Naos を取得し、`tests/examples/` から加えるときはテスト中の Naos を使います。テストは実行ファイルの名前の CTest テスト（`clamp_tests.clamp.inside_range`）として、自己テストと同じ厳しい警告で実行されます。

ドキュメントが引用する出力は `tests/examples/` の golden のケースです（たとえば `naos.examples.quickstart`）。サンプルやその出力を変えたら、それを引用するドキュメントも直します（日本語訳は[文書の書き方](docs-style.md#言語)のとおり、後でもかまいません）。

ドキュメントのリンクは、自己テストではなく [lint](#lint) のスクリプトで確かめます。

## lint

ビルドが要らずソースを読むだけの検査は、自己テストではなく `scripts/` のスクリプトです。自己テストにすると、ビルド全体を待ち、CI のプリセットとコンパイラごとに同じ結果のために何度も実行されるためです。`scripts/Lint.cmake` がそれらをすべて実行し、失敗したものを挙げます。CI の `lint` ジョブはこのスクリプトです。Naos のルートで実行します。

```sh
cmake -D ROOT=. -P scripts/Lint.cmake
```

| スクリプト | 確かめること |
|---|---|
| `CheckMarkdownLinks.cmake` | Markdown のファイルの相対リンクがすべて存在するファイルを指し、`#anchor` がそのファイルの見出しを指すこと |
| `CheckPrintfCalls.cmake` | `src/` のソースが書式付きの出力関数を `format.h` を通して呼ぶこと（[コーディングスタイル](coding-style.md#書式付きの出力)） |
| `CheckReleaseScripts.cmake` | `PrepareRelease.cmake` と `ReleaseNotes.cmake` を、それらが変えるファイルの小さな写しで実行し、バージョンの種類ごとに変えるもの、拒むバージョンを確かめる（[リリース](release.md)） |

どのスクリプトも、同じ `-D ROOT=.` で単独でも実行できます。
