# リファレンス

公開 API の規則を、ページごとに 1 つの部分ずつ載せています。ここに載せたものは、互換を保ちます。

マクロが呼ぶ関数（`naos_check_int()`、`naos_fail()` など）、それらが受け取る型（`naos_cmp_t`）、名前が `_` で終わる補助マクロ（`NAOS_CHECK_`）は、マクロを動かすためだけにヘッダにあり、共有ビルドでは export されます。直接は使わないでください。それでも、Naos が公開するものはすべて互換を保ちます。1.0 より前は同じマイナーバージョンの間、1.0 以後は同じメジャーバージョンの間です（[バージョン](version.md)）。

| ページ | 内容 |
|---|---|
| [テストとスイート](tests.md) | `NAOS_TEST`、`NAOS_SUITE`、前後処理、`NAOS_MAIN`、`naos_init()` と `naos_run()` |
| [アサーション](assertions.md) | 型ごとの `NAOS_CHECK_*` と `NAOS_REQUIRE_*` |
| [失敗・スキップ・メモ](control.md) | テストの止まり方、`NAOS_FAIL`、`NAOS_SKIP`、`NAOS_NOTE`、`NAOS_TRACE_*` |
| [バージョン](version.md) | `NAOS_VERSION_*`、`naos_version_number()`、`naos_version_string()` |
| [コマンドライン](command-line.md) | `--naos-*` オプションと終了コード |
| [出力](output.md) | 出力するすべての行と、値の表記 |
| [CMake](cmake.md) | `naos::naos`、`naos_add_executable()`、`naos_discover_tests()`、オプション、パッケージ |
