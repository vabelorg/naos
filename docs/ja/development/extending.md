# 拡張

変更の種類ごとに、どこを変え、何を一緒に変えるかをまとめます。公開 API や出力を変えたら、同じ変更で英語のリファレンスを直します。日本語訳は後から追いついてもかまいません（[文書の書き方](docs-style.md#言語)）。

## 出力の行を足す・変える

標準出力のすべての行の形は `human.c` にだけ書きます。実行でどの行を書くかも、そこで決めます。これらの行は人向けで、読みやすくするために変えてかまいません。ただし[互換を保つもの](../reference/output.md#互換を保つもの)（診断の行の始まり、`--naos-list` の名前、終了コード）は除きます。

1. [出力](../reference/output.md)を直す。
2. 自己テストの期待する出力を `NAOS_UPDATE_EXPECTED=1` で書き直し、差分を確かめる（[テスト](testing.md#ケースを足す)）。
3. 変更履歴の `Changed` に項目を足す。互換を保つものを変えるときは `**BREAKING**:` で始める。

## JUnit XML のファイルを変える

ファイルは `xml.c` が書きます。形式は互換を保つので、属性、プロパティ、要素、失敗の行は足してかまいませんが、[JUnit XML](../reference/xml.md) に書いたものは、バージョンが許さない限り変えたり消したりしません（[変更履歴](changelog.md)）。リファレンスと `tests/report/` の期待する出力を直します。

## 出力の形式を足す

形式は 1 つの書き手です。`naos_writer_t`（`writer.h`）の関数の表を、`xml.c` のように専用のファイルに書きます。

1. 形式を `naos_format_t`（`report.h`）に、名前を `options.c` の `take_output()` と `--naos-output` のヘルプに足し、`naos_report_begin()`（`report.c`）でその書き手を有効にする。
2. イベントは `writer.h` に書いた順に届く。診断は値、メモ、trace を含めて丸ごと届く。イベントが借りるものは、その呼び出しの間だけ有効。
3. `tests/report/` に `OUTPUT_FILE` を使ったケースを足し（[テスト](testing.md#出力を照合するケース)）、リファレンスにページを足し、[コマンドライン](../reference/command-line.md)を直す。

## アサーションを足す

1. 公開マクロ（`CHECK` と `REQUIRE` の 2 つの形）を `naos/check.h` に、結果を記録する関数を `check.c` に足す。
2. 比較の種類だけが違うものは、`naos_cmp_t` を受け取る 1 つの関数にする。
3. 引数の文字列化は公開マクロの側で行う。補助マクロに渡してから `#` を使うと、引数のマクロが展開されてしまう。
4. 失敗は `naos_diagnostic_begin()` → `naos_diagnostic_add*()` → `naos_diagnostic_end()` で書く。
5. 失敗の出力のために `tests/assertions/` にファイルを、`tests/api/check.c` にテストを足す。
6. [アサーション](../reference/assertions.md)と、[JUnit XML](../reference/xml.md#値)の値の表を直す。

## コマンドラインのオプションを足す

`options.c` のオプション表（`g_option_table`）に 1 行足します。解析、使い方の行、ヘルプはすべてこの表から作ります。`tests/cli/` にケースを足し、[コマンドライン](../reference/command-line.md)を直します。

## CTest への登録に引数を足す

1. `Naos.cmake` の 2 つの公開関数の `cmake_parse_arguments` にキーワードを足し、`_naos_discover()` に渡す。
2. `NaosDiscover.cmake` に値を渡すなら、`_naos_discovery_params()` のキーワードと、`NaosDiscover.cmake` の先頭の一覧と必須チェックに足す。自己テストの `discovery/` も同じ関数でパラメータを書くので、ずれない。
3. [CMake のリファレンス](../reference/cmake.md)を直す。

`_naos_` で始まる関数は内部用です。

## ソースファイルを足す

先頭に著作権表示と `SPDX-License-Identifier: MIT`、続けてそのファイルの責務の短いコメントを書き、`src/CMakeLists.txt` の該当する群に足します。新しいモジュールやモジュール間の新しい依存は、[アーキテクチャ](architecture.md)と `src/CMakeLists.txt` の依存のコメントも直します。

## バージョンを変える

`include/naos/version.h` だけを書き換えます。`project(VERSION)`、パッケージの互換性（1.0 より前は `SameMinorVersion`、以後は `SameMajorVersion`）、共有ライブラリの `SOVERSION`（1.0 より前は `0.<minor>`、以後は `<major>`）は、`NaosVersion.cmake` がそこから決めます。リリースには別の手順があります（[リリース](release.md)）。
