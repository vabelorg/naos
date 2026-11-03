# リファレンス

公開 API の規則を、ページごとに 1 つの部分ずつ、今のバージョンのとおりに載せています。後のバージョンでも変わらないものは、[互換性](compatibility.md)に挙げたものです。

マクロが呼ぶ関数（`naos_check_int()`、`naos_fail()` など）、それらが受け取る型（`naos_cmp_t`）、名前が `_` で終わる補助マクロ（`NAOS_CHECK_`）は、マクロを動かすためだけにヘッダにあり、共有ビルドでは export されます。直接は使わないでください（[共有ライブラリ](compatibility.md#共有ライブラリ)）。

| ページ | 内容 |
|---|---|
| [テストとスイート](tests.md) | `NAOS_TEST`、`NAOS_SUITE`、前後処理、`NAOS_MAIN`、`naos_init()` と `naos_run()` |
| [アサーション](assertions.md) | 型ごとの `NAOS_CHECK_*` と `NAOS_REQUIRE_*` |
| [失敗・スキップ・メモ](control.md) | テストの止まり方、`NAOS_FAIL`、`NAOS_SKIP`、`NAOS_NOTE`、`NAOS_TRACE_*` |
| [バージョン](version.md) | `NAOS_VERSION_*`、`naos_version_number()`、`naos_version_string()` |
| [コマンドライン](command-line.md) | `--naos-*` オプションと終了コード |
| [出力](output.md) | 標準出力の人間向けの出力と、そのうち互換を保つもの |
| [JUnit XML](xml.md) | `--naos-output xml:<path>` のファイル、ツール向けの出力と、そのうち互換を保つもの |
| [CMake](cmake.md) | `Naos::naos`、`naos_add_executable()`、`naos_discover_tests()`、オプション、パッケージ |
| [互換性](compatibility.md) | 後のバージョンでも変わらないものと、変わることがあるもの |
