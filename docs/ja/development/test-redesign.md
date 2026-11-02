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

ケースの CTest の名前は `naos.<層>.<ページ>.<ケース>` です(`naos.behavior.assertions.integer.eq` など)。`tests/` の下のディレクトリのパスの `/` を `.` にしたものです。どのケースにもラベル `naos` と層のラベル(`contract`、`unit`、`api`、`behavior`、`build`、`examples`)が付きます。例外は `examples/` のプロジェクトのテストです。その `CMakeLists.txt` は利用者が写すものなので、CTest の名前は実行ファイルの名前のまま(`clamp_tests.clamp.inside_range`)で、ラベルも付きません。

1 つのディレクトリにまとめるケースは名前が重なってはいけません。今は `control/run_all` と `context/run_all` が重なります([behavior](#behavior) を参照)。

## 層

### contract/(新規)

どのケースも exit code と正規表現で判定するので、golden ファイルを更新しても変わりません。

| 契約 | ケース |
|---|---|
| exit code | [終了コード](../reference/command-line.md#終了コード)のとおり: 成功は 0、失敗は 1、引数の誤りは 2、どのテストにも合わないフィルタは 2、スイートの誤りは 2、順序の誤った呼び出しは 2、全テストのスキップは 77、成功に混じったスキップは 0、テストが 1 つも登録されていない実行は 0、`--naos-list` と `--help` は 0、開けない `--naos-output` のファイルは 2 |
| 診断の行の先頭 | `error`、`skip`、`note` のそれぞれで、位置がコンパイラの形式(`f.c:12:` または `f.c(12):`)であること。`assertions/location_form` をここへ移す |
| `--naos-list` | 標準出力には名前だけが 1 行に 1 つあること。どの名前も `--naos-filter` に渡すとそのテストだけを実行すること(`ListRoundTrip.cmake`)。CTest の discovery はこれに頼っている |
| XML ファイルの構造 | [互換性](../reference/xml.md#互換性)が保つものを `junit.xsd` に書き、すべての結果、shuffle 付きの repeat、fail fast のファイルを `xmllint --schema` で確かめる。xmllint があるときだけ(CI では Linux と macOS) |
| XML ファイルを書くとき | クラッシュでは空のまま、`--naos-list` と `--help` では書かない、開けないファイルは 2。`report/list`、`help`、`crash`、`cannot_open` をここへ移す |

**ブートストラップ**: exit code のケースを `FIXTURES_SETUP naos_trust` にし、`unit/` と `api/` のテストを `PROPERTIES FIXTURES_REQUIRED naos_trust` で登録します。失敗が失敗と判定されることが示されるまで、CTest は `unit/` と `api/` のテストを実行しません。`-R` や `-L` で絞った実行にも CTest はセットアップを加えるので、いつも先に実行されます。`examples/` のテストも Naos が判定しますが、対象にしません。その `CMakeLists.txt` は利用者が写すものなので、自己テストのプロパティを持たせてはいけないためです。

### unit/

- 残す: `value`、`filter`、`order`。
- **足す: `text.c` と `xml.c` の writer。** `naos_xml_writer()` を通して、ビルドディレクトリのファイルで `begin` し、`writer.h` のイベントを呼び、`end` してから、ファイルを読み戻して比べます。XML のエスケープの境界(不正な UTF-8、XML が許さない制御文字、属性の改行とタブ、`]]>`)は、テストプログラムを通すよりこの方が網羅しやすいためです。golden のケース `xml/text` は代表するテスト 1 つだけにします。
- `human.c` の writer は入れません。標準出力に書くので同じプロセスのテストからは読み戻せず、書く行はすべて `behavior/` の golden ファイルが確かめているためです。
- 先に確かめること: `xml.c` の writer は状態を `static` 変数(`g_cases`、`g_suites` など)に持ちます。1 つのプロセスで何度もファイルを書くには、`end` のあとの状態が `begin` の期待どおりである必要があります。そうでなければ、`end` で状態を戻すようにする(ライブラリの変更として別のコミットで)か、この追加をやめます。

### api/

- C と C++ は残します。
- `run_all` はなくします。`run_in_any_order`(shuffle し、50 回 repeat する)が含んでいるためです。

### behavior/

- **golden ファイルは変えません。** 移すだけにし、ケースの数とすべての成功で、何も失われていないことを示します。テストプログラムもそのまま移します。まとめると `--naos-list` やプログラム全体の実行の出力が変わり、`session/own_main.c` は自分の `main()` を持つので別のプログラムのままでなければならないためです。
- cli、control、context の `run_all` は、テストを足すたびに大きくなります。それぞれの目的(失敗のあとも続く、テストごとにまっさらから始まる、サマリが数える)を確かめ、`control/run_all` だけにまとめる候補とします。それまでは、context のものを `control/` に移すときに `context_run_all` と改名します。まとめるディレクトリどうしで重なる名前はこれだけです。

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
| **`release`**(新規) | 静的、Release | `test.c` は `longjmp` でテストを止め、どのローカル変数も `volatile` を要らないように書かれている。それが成り立つことを確かめられるのは最適化したビルドだけ。GCC の `-Wclobbered` や `-Wmaybe-uninitialized` のように最適化が要る警告も、そこでエラーになる |
| **`sanitize`**(新規) | 静的、Debug、`-fsanitize=address,undefined -fno-sanitize-recover=all` | エスケープや伸びるテキストでのメモリの誤り。テストのプリセットで `ASAN_OPTIONS` を設定する。GCC と Clang だけ |

`CMakePresets.json` は version 3 のままにします。workflow プリセットには CMake 3.25 が要り、最低バージョンの 3.21 を確かめられなくなるためです。

## CI

| ジョブ | ビルド |
|---|---|
| `lint` | `cmake -D ROOT=. -P scripts/Lint.cmake` だけ |
| `test` | linux-gcc {default, shared, **release**}、linux-clang {default, **sanitize**}、macos-clang {default, shared}、msvc {default, shared, **release**}、**cmake-3.21**(linux-gcc、default) |
| `mingw` | UCRT64 {default, shared}、MINGW64 {default}。MINGW64 は msvcrt の printf のためにあり、静的で確かめられる([決定](#決定)) |

- インストールしたパッケージは `cmake -D PRESET=<preset> -P scripts/TestPackage.cmake` の 1 行でテストし、`sanitize` 以外のすべてのジョブで実行します。CMake 3.21 では、利用者の持ちうるいちばん古い CMake で `find_package(Naos)` を確かめることになります。
- **CMake 3.21 のジョブは新規です。** 今は `cmake_minimum_required(VERSION 3.21)` を一度も確かめていません。
- `test` のジョブは `lint` を待ちません。どちらも同時に始まり、プルリクエストにはすべての成功が要ります。
- CMake 3.21 のジョブで、自己テストや `cmake/` がもっと新しい機能を使っているとわかるかもしれません。その使い方を置き換えるか、別の変更で最低バージョンを上げます。
- ジョブは 13 から 15 になります(`lint` 1、`test` 11、`mingw` 3。今は `test` 8、`mingw` 4)。

## 更新するドキュメント

| ファイル | 変更 |
|---|---|
| `docs/development/testing.md` | 書き直す: 層とそれぞれが確かめるもの、判定する者、ラベル、ケースの足し方、golden ファイルの更新が触れない契約のケース |
| `docs/development/building.md` | プリセット、`-R` と `-L` の例(今は `-R '^naos\.cli\.'`)、CTest の名前 `naos.no_cmake.build`、CI の節 |
| `docs/development/extending.md` | 変更の種類ごとにテストを足す層(今は `tests/cli/` などの古いパスを挙げている) |
| `docs/development/architecture.md` | ディレクトリの表の `tests/` と `scripts/` の行 |
| `docs/development/release.md` | 手順 4(`tests/package/`)と、リリーススクリプトのテスト |
| `docs/CONTRIBUTING.md`、`.github/PULL_REQUEST_TEMPLATE.md` | 送る前に実行するもの: `default`、`shared`、`Lint.cmake` |
| `docs/reference/xml.md` | `tests/report/` を指す 2 か所 |
| `tests/CMakeLists.txt` と各 `CMakeLists.txt` の冒頭のコメント | 新しい構成 |
| `docs/ja/` | 同じページを、[文書の書き方](docs-style.md#言語)のとおり後のプルリクエストで |

利用者に見える変更はないので `CHANGELOG.md` の項目は要りません。ただし `release` や `sanitize` がバグを見つけたら、その修正には項目を付けます。

## 決定

プランをレビューして決めたこと:

- **契約が確かめるものは golden ファイルにも残す。** exit code と診断の行はそのまま残し、正規化で落としません。この重なりが契約を効かせます。golden ファイルの更新で exit code が変わっても、更新が触れない契約のケースは失敗します。落とすと、golden ファイルが出力の全体を表さなくもなります。
- **MINGW64 は静的だけでビルドする。** MINGW64 が CI にあるのは msvcrt の printf のためで、それはライブラリの種類によりません。DLL が頼るもの(エクスポート、`NAOS_SHARED`、DLL が見つかること)はコンパイラとリンカで、共有ビルドを残す UCRT64 と同じ GCC と binutils です。CI を変える前(手順 7)に確かめること: 公開 API が `FILE *` も、反対側で解放するメモリも渡さないこと。C ライブラリの違う DLL と実行ファイルで食い違いうるのはそれだけです。渡すなら、MINGW64 は共有ビルドを残します。
- **`examples/` のテストは `naos_trust` に入れない。** discovery のテストの名前はビルドまでわからないので、プロパティは利用者が写す `CMakeLists.txt` に書くしかありません。入れなくても失敗は失われません。Naos の判定が誤れば契約が失敗し、実行全体も失敗します。フィクスチャが防ぐのは誤った成功が原因を隠すことだけで、それには `unit/` と `api/` で足ります。

## 手順

手順ごとに 1 つのプルリクエストとし、変える英語のドキュメントも含めます。

- [ ] 1. **ハーネス**: `support/` を `harness/` にする。パスから作る名前、ラベル、exit code だけで判定するケース。既存のケースはすべて成功したまま。
- [ ] 2. **`behavior/` と `build/` へ移す**: `git mv` だけで、golden ファイルは変えない。ケースの数は前後で同じ。
- [ ] 3. **`contract/`**。`junit.xsd` と `FIXTURES_REQUIRED naos_trust` も。
- [ ] 4. **`unit/`**: `text` と `xml` の writer。
- [ ] 5. **`build/functions` と `build/subproject`**。`package/` を `consumer/` に、`scripts/TestPackage.cmake`。
- [ ] 6. **プリセット `release` と `sanitize`**。見つかったバグはそれぞれ別のコミットで直す。
- [ ] 7. **CI**: `Lint.cmake`、`CheckReleaseScripts.cmake`、CMake 3.21 のジョブ。
- [ ] 8. 変えたページの**日本語訳**。
- [ ] 9. **このページを消す**(英語と日本語)。

手順 6 で見つかるもの(最適化の警告やサニタイザの報告)は前もって大きさを見積もれないので、手順 1〜5 とは分けておきます。
