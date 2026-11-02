# テストの再設計

Naos の自己テスト、ビルドのプリセット、スクリプト、CI と、それらを説明するドキュメントを作り直す計画です。今の構成は壊してよく、ディレクトリ、ファイル、CTest のテストの名前は変えてよく、CI は増えてもかまいません。計画を終えたらこのページは消し、結果は[テスト](testing.md)と[ビルド](building.md)が説明します。

## 方針

- **判定する者で分ける。** テストを判定するのは、Naos 自身、Naos の外のスクリプト、コンパイラ、CMake のいずれかです。テストのディレクトリ、CTest の名前、ラベルはそこから決まります。
- **契約と表示を分ける。** 互換を保つもの(exit code、診断の行の先頭、`--naos-list`、JUnit XML ファイルの構造)は、`NAOS_UPDATE_EXPECTED=1` が書き換えられないケースで確かめます。golden ファイルが確かめるのは、どのバージョンで変わってもよい残りの部分です([出力](../reference/output.md#互換を保つもの)、[JUnit XML](../reference/xml.md#互換性))。
- **ドキュメントにした約束にはすべてテストがある。** 振る舞いのテストは[リファレンス](../reference/README.md)の 1 ページに 1 ディレクトリとし、ページの規則からそのケースにたどり着けるようにします。
- **ビルドオプションは増やさない。** 最適化やサニタイザのビルドは、CMake のふつうの変数を設定するプリセットにします。
- **CI の YAML に処理を書かない。** 手順はすべて `scripts/` のスクリプトにし、手元でも CI でも同じものを実行します。

## いつ何を実行するか

| いつ | 実行するもの | 目的 |
|---|---|---|
| ビルドの前(数秒) | `cmake -D ROOT=. -P scripts/Lint.cmake` | ソースを読むだけで済む検査: リンク、printf の呼び出し、リリーススクリプトのテスト(新規) |
| 開発中 | `ctest --preset default`。`-L contract` や `-LE build` などで絞る | 何も壊していないこと |
| プルリクエストごと(CI) | 下のマトリクスのすべてと、インストールしたパッケージ | プラットフォームやビルドに依存する問題 |
| リリースのプルリクエスト | 同じ CI | 追加はない |

## 構成

```
tests/
  CMakeLists.txt          層と、層が付けるラベル
  harness/                ← support/: NaosSelftest.cmake、RunCase.cmake(← CheckOutput.cmake)
  contract/               新規。スクリプト(正規表現、XSD)が判定: 互換を保つもの
    contract.c  junit.xsd  ListRoundTrip.cmake
  unit/                   Naos が判定: 内部の関数と writer
  api/                    Naos が判定: 公開 API(C と C++)
  behavior/               スクリプト(golden ファイル)が判定: リファレンスの 1 ページに 1 ディレクトリ
    tests/                ← session/ と fixtures/      (reference/tests.md)
    assertions/           ← assertions/                (reference/assertions.md)
    control/              ← control/ と context/       (reference/control.md)
    command-line/         ← cli/ と runs/              (reference/command-line.md)
    output/               ← quiet/ と timing/          (reference/output.md)
    xml/                  ← report/                    (reference/xml.md)
  build/                  コンパイラと CMake が判定
    misuse/  no_cmake/  discovery/
    functions/            新規: naos_add_executable() と naos_discover_tests()
    subproject/           新規: add_subdirectory() で加えた Naos
  examples/               examples/ のプロジェクトと、ドキュメントが引用する出力
  consumer/               ← package/: find_package() と add_subdirectory() の両方で使う 1 つのプロジェクト
scripts/
  Lint.cmake              新規: 下の 3 つの検査を実行する(CI の lint ジョブはこの 1 行)
  CheckMarkdownLinks.cmake  CheckPrintfCalls.cmake
  CheckReleaseScripts.cmake 新規。scripts/fixtures/release/ を使う
  TestPackage.cmake       新規: インストールし、consumer/ を configure、ビルド、テストする。WerrorOption.cmake を取り込む
  PrepareRelease.cmake  ReleaseNotes.cmake
  fixtures/author-warning/  ← tests/package/author-warning/
```

ケースの CTest の名前は `naos.<層>.<ページ>.<ケース>` です(`naos.behavior.assertions.integer.eq` など)。`tests/` の下のディレクトリのパスの `/` を `.` にしたものです。どのケースにもラベル `naos` と層のラベル(`contract`、`unit`、`api`、`behavior`、`build`、`examples`)が付きます。

## 層

### contract/(新規)

どのケースも exit code と正規表現で判定するので、golden ファイルを更新しても変わりません。

| 契約 | ケース |
|---|---|
| exit code | 成功は 0、失敗は 1、引数の誤りは 2、スイートの誤りは 2、順序の誤った呼び出しは 2、全テストのスキップは 77、成功に混じったスキップは 0、選ばれたテストなしは 0、`--naos-list` と `--help` は 0、開けない `--naos-output` のファイルは 2 |
| 診断の行の先頭 | `error`、`skip`、`note` のそれぞれで、位置がコンパイラの形式(`f.c:12:` または `f.c(12):`)であること。`assertions/location_form` をここへ移す |
| `--naos-list` | 標準出力には名前だけが 1 行に 1 つあること。どの名前も `--naos-filter` に渡すとそのテストだけを実行すること(`ListRoundTrip.cmake`)。CTest の discovery はこれに頼っている |
| XML ファイルの構造 | [互換性](../reference/xml.md#互換性)が保つものを `junit.xsd` に書き、すべての結果、shuffle 付きの repeat、fail fast のファイルを `xmllint --schema` で確かめる。xmllint があるときだけ(CI では Linux) |
| XML ファイルを書くとき | クラッシュでは空のまま、`--naos-list` と `--help` では書かない、開けないファイルは 2。`report/list`、`help`、`crash`、`cannot_open` をここへ移す |

**ブートストラップ**: exit code のケースを `FIXTURES_SETUP naos_trust` にし、`unit/` と `api/` のテストを `PROPERTIES FIXTURES_REQUIRED naos_trust` で登録します。失敗が失敗と判定されることが示されるまで、CTest は Naos が自分で判定するテストを実行しません。

### unit/

- 残す: `value`、`filter`、`order`。
- **足す: `text.c`、`xml.c`、`human.c`。** `writer.h` のイベントを writer に直接呼び、作られたテキストを比べます。XML のエスケープの境界(不正な UTF-8、XML が許さない制御文字、属性の改行とタブ、`]]>`)は、テストプログラムを通すよりこの方が網羅しやすいためです。golden のケース `xml/text` は代表するテスト 1 つだけにします。
- 先に確かめること: writer がグローバルな状態なしに単独で呼べること。呼べなければ、この追加はやめます。

### api/

- C と C++ は残します。
- `run_all` はなくします。`run_in_any_order`(shuffle し、50 回 repeat する)が含んでいるためです。

### behavior/

- **golden ファイルは変えません。** 移すだけにし、ケースの数とすべての成功で、何も失われていないことを示します。
- テストプログラムはページに 1 つにし、`naos_selftest_program()` で作ります。
- cli、control、context の `run_all` は、テストを足すたびに大きくなります。それぞれの目的(失敗のあとも続く、テストごとにまっさらから始まる、サマリが数える)を確かめ、`control/run_all` だけにまとめる候補とします。

### build/

今いちばんテストが薄いのは CMake の関数です。

| ケース | 確かめること |
|---|---|
| `functions/errors` | configure のときのエラー: 知らない引数、ないターゲット、IMPORTED なターゲット、2 度目の登録、不正な `DISCOVERY_TIMEOUT`、値の数が奇数の `PROPERTIES`、`SOURCES` なし。今はどれもテストがない |
| `functions/registration` | ビルドの前は `ctest -N` に `<prefix>NOT_BUILT` がラベル付きで出ること。ビルドのあとは `ctest --show-only=json-v1` に `PREFIX`、`LABELS`、`PROPERTIES`、`SKIP_RETURN_CODE 77` が反映されていること |
| `subproject` | `consumer/` が Naos を `add_subdirectory()` で加える: 外側で `BUILD_SHARED_LIBS=ON` でも Naos は静的のまま、自己テストは登録されない、`-Werror` は外のプロジェクトに及ばない、インストールのルールは作られない、プロジェクトはビルドでき、そのテストは成功する |

入れ子の configure には、外のビルドのジェネレータ、コンパイラ、make のプログラム、構成を渡します。これらのケースは遅いので、ラベル `build` で `-LE build` で外せるようにします。

### 入れないもの

- 公開ヘッダを 1 つずつコンパイルする検査: 利用者に案内するのは `naos/naos.h` だけです。
- ゲートとしてのカバレッジ、ファジング、夜間のジョブ: この規模では手間に見合いません。
- git からの `FetchContent`: `add_subdirectory()` と同じように Naos を加えるので、`subproject` で確かめられます。
- 新しい CMake のオプション。

## プリセット

| プリセット | ビルド | 理由 |
|---|---|---|
| `default` | 静的、Debug | 今と同じ |
| `shared` | 共有、Debug | 今と同じ |
| **`release`**(新規) | 静的、Release | `test.c` は `setjmp` / `longjmp` を使う。最適化のときだけ壊れるローカル変数も、GCC の `-Wclobbered`(`-Wextra` に含まれ、最適化のときだけ出る)も、Debug では出ない |
| **`sanitize`**(新規) | 静的、Debug、`-fsanitize=address,undefined -fno-sanitize-recover=all` | エスケープや伸びるテキストでのメモリの誤り。テストのプリセットで `ASAN_OPTIONS` を設定する。GCC と Clang だけ |

`CMakePresets.json` は version 3 のままにします。workflow プリセットには CMake 3.25 が要り、最低バージョンの 3.21 を確かめられなくなるためです。

## CI

| ジョブ | ビルド |
|---|---|
| `lint` | `cmake -D ROOT=. -P scripts/Lint.cmake` だけ |
| `test` | linux-gcc {default, shared, **release**}、linux-clang {default, **sanitize**}、macos-clang {default, shared}、msvc {default, shared, **release**}、**cmake-3.21**(linux-gcc、default) |
| `mingw` | UCRT64 {default, shared}、MINGW64 {default}。MINGW64 は msvcrt の printf のためにあり、静的で確かめられる |

- インストールしたパッケージは `cmake -D PRESET=<preset> -P scripts/TestPackage.cmake` の 1 行でテストし、`sanitize` 以外のすべてのジョブで実行します。CMake 3.21 では、利用者の持ちうるいちばん古い CMake で `find_package(Naos)` を確かめることになります。
- **CMake 3.21 のジョブは新規です。** 今は `cmake_minimum_required(VERSION 3.21)` を一度も確かめていません。
- ジョブは 12 から 15 になります(`lint` 1、`test` 11、`mingw` 3)。

## 更新するドキュメント

| ファイル | 変更 |
|---|---|
| `docs/development/testing.md` | 書き直す: 層とそれぞれが確かめるもの、判定する者、ラベル、ケースの足し方、golden ファイルの更新が触れない契約のケース |
| `docs/development/building.md` | プリセット、`-R` と `-L` の例、CI の節 |
| `docs/development/extending.md` | 変更の種類ごとにテストを足す層(今は `tests/cli/` などの古いパスを挙げている) |
| `docs/development/architecture.md` | ディレクトリの表の `tests/` と `scripts/` の行 |
| `docs/development/release.md` | 手順 4(`tests/package/`)と、リリーススクリプトのテスト |
| `docs/CONTRIBUTING.md`、`.github/PULL_REQUEST_TEMPLATE.md` | 送る前に実行するもの: `default`、`shared`、`Lint.cmake` |
| `docs/reference/xml.md` | `tests/report/` を指す 2 か所 |
| `tests/CMakeLists.txt` と各 `CMakeLists.txt` の冒頭のコメント | 新しい構成 |
| `docs/ja/` | 同じページを、[文書の書き方](docs-style.md#言語)のとおり後のプルリクエストで |

利用者に見える変更はないので `CHANGELOG.md` の項目は要りません。ただし `release` や `sanitize` がバグを見つけたら、その修正には項目を付けます。

## 手順

手順ごとに 1 つのプルリクエストとし、変える英語のドキュメントも含めます。

- [ ] 1. **ハーネス**: `support/` を `harness/` にする。パスから作る名前、ラベル、exit code だけで判定するケース。既存のケースはすべて成功したまま。
- [ ] 2. **`behavior/` と `build/` へ移す**: `git mv` だけで、golden ファイルは変えない。ケースの数は前後で同じ。
- [ ] 3. **`contract/`**。`junit.xsd` と `FIXTURES_REQUIRED naos_trust` も。
- [ ] 4. **`unit/`**: `text`、`xml`、`human`。
- [ ] 5. **`build/functions` と `build/subproject`**。`package/` を `consumer/` に、`scripts/TestPackage.cmake`。
- [ ] 6. **プリセット `release` と `sanitize`**。見つかったバグはそれぞれ別のコミットで直す。
- [ ] 7. **CI**: `Lint.cmake`、`CheckReleaseScripts.cmake`、CMake 3.21 のジョブ。
- [ ] 8. 変えたページの**日本語訳**。

手順 6 では、大きさのわからない `-Wclobbered` やサニタイザの報告が出そうなので、手順 1〜5 とは分けておきます。
