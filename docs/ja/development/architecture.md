# アーキテクチャ

Naos 自体を保守・拡張する人向けに、ソースの置き方とモジュールの依存の向きを説明します。モジュールの構成はこのページが正です。モジュールを足したり依存を変えたりしたら、ここと、`src/CMakeLists.txt` の依存のコメントを合わせて直します。

## 原則

何を Naos に入れるかは、個々の機能より先に、次の原則で決めます。

- **導入しやすさが最優先**: 標準 C ライブラリ以外に依存しない、コンパイラ拡張を使わない。このどちらかを損なう機能は入れません。
- **出力は契約**: 出力の行は人が読み、ツールが解析するので、形を安定させて文書にします（[出力](../reference/output.md)）。
- **公開 API は文書にあるものだけ**: 公開して互換を保つのは、[リファレンス](../reference/README.md)に載せたものだけです。
- **責務を絞る**: プロセスの分離、並列実行、時間制限、レポートの形式は呼び出し側（CTest、スクリプト、CI）に任せます。

## ディレクトリ

| 場所 | 内容 |
|---|---|
| `include/naos/` | 公開ヘッダ。利用者には `naos/naos.h` だけを案内する |
| `src/` | 実装と内部ヘッダ |
| `cmake/` | 利用者向けにインストールする `Naos.cmake`（`naos_add_executable()` / `naos_discover_tests()`）と `NaosDiscover.cmake`（ビルド後にテストを列挙して CTest に登録するスクリプト）、`find_package(naos)` 用の config。Naos のビルドだけが使う `NaosVersion.cmake`（バージョンと、そこから決まる互換性・SOVERSION）と `NaosInstall.cmake`（インストール規則）はインストールしない |
| `tests/` | 自己テスト。観点ごとのディレクトリに、プログラム、ケースを登録する `CMakeLists.txt`、期待する出力（`expected/`）を置く（[テスト](testing.md)） |
| `examples/` | 文書が引用する完成したプロジェクト。自己テストでビルドして実行する |
| `docs/` | 英語の文書と、`docs/ja/` にその日本語訳 |
| `scripts/` | Naos の保守に使うスクリプト。ビルドでは使わない。`prepare-release.cmake` と `release-notes.cmake` は、リリースの準備とリリースノートの書き出しをする（[リリース](release.md)） |
| `.github/` | ワークフロー（CI とリリース）と、Issue・プルリクエストのテンプレート |

## モジュール

`src/` は関心ごとのファイルに分けます。公開ヘッダを実装するファイルはそのヘッダと同じ名前にし（`naos/control.h` ↔ `control.c`）、それ以外はモジュール間の宣言を同じ名前の内部ヘッダに置きます（`diagnostic.c` ↔ `diagnostic.h`）。同じ名前の公開ヘッダがある内部ヘッダだけ `_internal.h` を付けます（`suite_internal.h`）。

ファイルは 3 つの群に分かれ、`src/CMakeLists.txt` と下の表ではこの順に並べています。公開 API（`session.c` から `version.c`）、テストの実行（`options.c` から `test.c`）、起きたことの出力（`diagnostic.c` から `value.c`）です。新しいファイルは該当する群に足します。

| ファイル | 役割 |
|---|---|
| `session.c` | `naos/session.h`: `naos_init` → `NAOS_ADD_SUITE` → `naos_run` の順番の管理 |
| `suite.c` | `naos/suite.h`: スイートの登録と、実行前の誤りの検出 |
| `check.c` | `naos/check.h`: アサーションの比較と、失敗したときの値 |
| `control.c` | `naos/control.h`: `NAOS_NOTE`・`NAOS_TRACE_*`・`NAOS_FAIL`・`NAOS_SKIP`・`NAOS_REQUIRE_*` の停止 |
| `version.c` | `naos/version.h`: 実行時のバージョン |
| `options.c` | コマンドライン（`--naos-*`）の解析、ヘルプと使い方 |
| `runner.c` | 繰り返し・シャッフル・fail fast を含む全体の実行と集計 |
| `filter.c` | `--naos-filter` のパターン照合 |
| `order.c` | `--naos-shuffle` の順序と種（自前の xorshift） |
| `test.c` | 1 つのテストの実行（setup・本体・teardown と `setjmp` / `longjmp`） |
| `diagnostic.c` | 1 つの診断（始め → 詳細を溜める → 終わり）と、テストの結果の集計 |
| `context.c` | 診断に添えるメモ（`NAOS_NOTE`）と trace のスタック。出力はしない |
| `report.c` | 出力のすべての行の形（標準出力）と、Naos 自身のメッセージ（標準エラー出力） |
| `value.c` | 詳細の行の値の表記（文字列のエスケープ、double の最短表記） |

依存は一方向に保ち、循環させません。矢印の先は使われる側です。`report` は何かを出力するモジュール（`session`・`suite`・`options`・`runner` など）のどれからも、登録されたスイート（`suite_internal.h`）はそれを読むモジュール（`runner`・`filter`・`test`）から使われるので、図ではそれらの矢印を省いています。すべての依存は `src/CMakeLists.txt` の依存のコメントにあります。

```mermaid
flowchart LR
  session --> options --> order
  session --> suite
  session --> runner
  runner --> filter & order
  runner --> test --> diagnostic
  test --> context
  diagnostic --> context
  diagnostic --> report --> value
  control --> test & diagnostic & context
  check --> diagnostic & value
```

## 主な仕組み

- **登録**: `NAOS_TEST`・`NAOS_SETUP`・`NAOS_TEARDOWN` は、役割を名前に含む関数 `naos_gen_<役割>_<name>` を定義します。`NAOS_ADD_TEST`・`NAOS_SET_SETUP`・`NAOS_SET_TEARDOWN` は自分の役割の関数しか受け付けないので、取り違えるとコンパイルできません。`NAOS_SUITE` は、ビルダー（`naos_begin_suite`・`naos_add_test`・`naos_set_*`）にテストを登録する関数 `naos_gen_suite_<name>` を定義します。`NAOS_ADD_SUITE` はこの関数を `naos_add_suite` に渡し、`naos_run()` がそれらを呼んでスイートを集め、テストを実行する前にすべてを検査します。
- **停止**: `test.c` は setup・本体・teardown を `setjmp` の下で実行します。停止（`naos_stop`）は、マクロをどの関数の中で使っていても、この `setjmp` の位置まで一気に戻ります。
- **診断**: 失敗はすべて `naos_diagnostic_begin()` → `naos_diagnostic_add*()` → `naos_diagnostic_end()` を通ります。ここで `context.c` のメモと trace を添え、`report.c` を通して出力します。
- **状態**: 共有する可変状態は各モジュールの `static` 変数（`g_` 接頭辞）に閉じ、`extern` 変数にはしません。例外は `naos_init` 以後読むだけのオプションで、`naos_options_get()` で読みます。
- **列挙**: `naos_add_executable()` は、ビルド後に `NaosDiscover.cmake` を実行する `POST_BUILD` のコマンドを足します。このスクリプトは `<exe> --naos-list` を実行し、出力の 1 行を 1 件のテストとして登録する CTest のファイルを書きます。パラメータはファイル経由で渡すので、リストをエスケープする必要はありません。列挙に失敗したときは、テストが 0 件のまま黙って進めず、ビルドを失敗させて原因と対処をメッセージに出します（[トラブルシューティング](../guides/troubleshooting.md#cmake-と-ctest)）。
- **バージョン**: バージョンの源は `include/naos/version.h` だけです。`NaosVersion.cmake` がそれを読み、`project(VERSION)`、パッケージの互換性、`SOVERSION` を決めます。

変更の種類ごとの置き場所は[拡張](extending.md)にあります。
