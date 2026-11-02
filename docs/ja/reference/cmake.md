# CMake

CMake のプロジェクトが Naos から得るもの、つまりターゲット `Naos::naos`、テストの実行ファイルを作って CTest にテストを登録する 2 つの関数、いくつかのオプションです。プロジェクトへの取り込み方は[インストール](../getting-started/installation.md)にあります。

## ターゲット

| ターゲット | 内容 |
|---|---|
| `Naos::naos` | ライブラリ。インクルードディレクトリ `include/` と C99 の要求を伴う。共有ライブラリなら定義 `NAOS_SHARED` も伴う |

- Debug ビルドは名前の末尾に `d` が付きます（`naosd.lib`、`libnaosd.a`）。Release ビルドと同じ場所に置けるようにするためです。
- 共有ビルドは公開関数だけを export します（`C_VISIBILITY_PRESET hidden`）。`SOVERSION` は 1.0 より前は `0.<minor>`、1.0 以後は `<major>` です。

## 関数

### naos_add_executable()

```cmake
naos_add_executable(<target>
    SOURCES <source>...
    [LIBRARIES <library>...]
    [PREFIX <prefix>]
    [LABELS <label>...]
    [PROPERTIES <name> <value>...]
    [DISCOVERY_TIMEOUT <seconds>]
    [ALLOW_NO_ASSERTIONS]
    [IGNORE_SUITES <suite>...]
    [NO_REGISTRATION_CHECK]
)
```

`SOURCES` から実行ファイル `<target>` を作り、`Naos::naos` と `LIBRARIES` を（PRIVATE で）リンクし、そのテストを 1 件ずつ CTest に登録します。`SOURCES` と `LIBRARIES` 以外の引数は [`naos_discover_tests()`](#naos_discover_tests) と同じです。

```cmake
naos_add_executable(my_tests SOURCES main.c parser_test.c LIBRARIES my_lib)
```

### naos_discover_tests()

```cmake
naos_discover_tests(<target>
    [PREFIX <prefix>]
    [LABELS <label>...]
    [PROPERTIES <name> <value>...]
    [DISCOVERY_TIMEOUT <seconds>]
    [ALLOW_NO_ASSERTIONS]
    [IGNORE_SUITES <suite>...]
    [NO_REGISTRATION_CHECK]
)
```

`add_executable()` で作った実行ファイルのテストを 1 件ずつ登録します。実行ファイルは（直接、または `NAOS_MAIN` を通して）`naos_init()` と `naos_run()` を呼ぶ必要があります。

| 引数 | 内容 |
|---|---|
| `PREFIX <prefix>` | CTest のテスト名の頭。既定値は `<target>.` |
| `LABELS <label>...` | CTest のテストに付けるラベル |
| `PROPERTIES <name> <value>...` | CTest のテストに付けるその他のプロパティ。リストの値は 1 つの引数として渡す（`ENVIRONMENT "A=1;B=2"`） |
| `DISCOVERY_TIMEOUT <seconds>` | テストの列挙（`--naos-list`）の制限時間。正の整数で、既定値は 60。エミュレータなど、列挙が遅い環境で延ばす |
| `ALLOW_NO_ASSERTIONS` | 各テストを `--naos-allow-no-assertions` も付けて実行する。アサーションを実行しないテストが成功になる（[アサーションを実行しないテスト](control.md#アサーションを実行しないテスト)） |
| `IGNORE_SUITES <suite>...` | ソースで定義しているが、わざと追加していないスイート。[独自の `main()`](../guides/custom-main.md) が引数で追加するスイートなど。[登録漏れの確認](#登録漏れの確認)の対象から外す。それぞれスイート名で書く |
| `NO_REGISTRATION_CHECK` | [登録漏れの確認](#登録漏れの確認)をしない。定義したのに追加していないテストやスイートがあってもビルドは失敗しない |

- 登録できるのは、同じディレクトリで作った実行ファイルターゲットだけで、1 つのターゲットにつき 1 回です。それ以外（ライブラリ、存在しない名前、2 回目の登録）や、`PROPERTIES` が名前と値の組になっていないとき、`IGNORE_SUITES` にスイート名でないものがあるときは、configure の時点でエラーになります。
- 各テストは CTest のテスト `<prefix><suite>.<test>` になり、`SKIP_RETURN_CODE 77` を付けて `<exe> --naos-filter <suite>.<test>` を実行します。`ALLOW_NO_ASSERTIONS` があれば `<exe> --naos-filter <suite>.<test> --naos-allow-no-assertions` を実行します。
- プロパティ `CROSSCOMPILING_EMULATOR` を持つターゲットは、そのエミュレータを通して列挙・実行します。

## 列挙

テストは、実行ファイルをビルドするたびに、`--naos-list` で起動して列挙します。テストを足したり消したりしても、configure し直す必要はありません。

- 実行ファイルをビルドするまでと、列挙に失敗したビルドの後は、テストの代わりに失敗するテスト `<prefix>NOT_BUILT` が 1 件だけ登録されています。`LABELS` も付くので、`ctest -L` で選んでも見落としません。マルチコンフィグのジェネレータで、ビルドしていない構成を `-C` で選んだときも同じです。
- 引数（`PREFIX`、`LABELS`、`PROPERTIES`、`DISCOVERY_TIMEOUT`、`ALLOW_NO_ASSERTIONS`、`IGNORE_SUITES`、`NO_REGISTRATION_CHECK`）を変えたときの動きは、ジェネレータで違います。
  - シングルコンフィグの Ninja と Makefile: 次のビルドで実行ファイルをリンクし直し、テストを列挙し直します。
  - Visual Studio、Xcode、Ninja Multi-Config などほかのジェネレータ: 実行ファイルをビルドし直すまで、テストは前のままです。
- 列挙に失敗するとビルドが失敗し、その理由と対処がメッセージに出ます。
  - 制限時間を超えたとき: `DISCOVERY_TIMEOUT` を延ばすよう出します。
  - それ以外: 共有ライブラリ（DLL）が見つからない場合の対処を出します（[共有ライブラリ](../guides/shared-library.md)）。Windows で DLL が見つからないと、出力はなく `Exit code 0xc0000135` とだけ返るので、この場合は原因を言い切ります。
- 定義したのに追加していないテストやスイートがあるときも、ビルドが失敗します（[登録漏れの確認](#登録漏れの確認)）。
- `add_custom_command(TARGET ... POST_BUILD)` でターゲットに足したコマンドは、足した順に実行されます。列挙より前に実行したいコマンド（DLL のコピーなど）は、`add_executable()` の後、`naos_discover_tests()` より前に足します。`naos_add_executable()` では、列挙が常に先になります。

## 登録漏れの確認

テストは `NAOS_TEST` と、スイートの中の `NAOS_ADD_TEST` の 2 か所に書きます。スイートも `NAOS_SUITE` と `NAOS_ADD_SUITE` の 2 か所です（[テストとスイート](tests.md)）。後半を書き忘れたテストやスイートは一度も実行されず、それでも実行は成功します。どちらの関数も、`NO_REGISTRATION_CHECK` を付けない限り、[列挙](#列挙)のときにこれを確かめます。実行ファイルが列挙したものと、ソースで定義しているものを比べます。テキストを読むだけなので、どのコンパイラでも同じに動きます。

定義したのに追加していないテストやスイートがあるとビルドが失敗し、1 つにつき 1 行、コンパイラの診断の形で出ます。

```
slow_test.c:21: error: suite slow is defined but never added, so its tests never run: add it with NAOS_ADD_SUITE(slow), or name it in IGNORE_SUITES of naos_add_executable() or naos_discover_tests() when it is left out on purpose
left_out_test.c:15: error: test left_out is defined but never added, so it never runs: add it with NAOS_ADD_TEST(left_out) to a suite of this file
```

- **ソース**: ターゲットのソース（`SOURCES` プロパティ）で、ビルドでそろった後のものです。`target_sources()` で足したものも含みます。拡張子で C と C++ のソースとヘッダだけを読みます。
- **定義**: コメント・プリプロセッサ指令・条件の中以外にある `NAOS_SUITE(<name>` と `NAOS_TEST(<name>` です。
- **スイートが追加されている**とは、引数なしで実行した `--naos-list` に、そのテストが 1 件以上あることです。テストのないスイートはスイートの誤りです（[スイート](tests.md#スイート)）。
- **テストが追加されている**とは、同じファイルのスイートについて `<suite>.<test>` が列挙にあることです。テストは `static` なので、追加できるのは同じファイルのスイートだけで、別のファイルにある同じ名前のテストは代わりになりません。定義が見つからないスイート（下記）は同じファイルのものかもしれないので、これも数えます。

確かめないもの:

- `#if`、`#ifdef`、`#ifndef` から `#endif` までの中のスイートやテスト: ビルドされるかは条件次第で、それが分かるのはコンパイラだけです。その `NAOS_ADD_SUITE` や `NAOS_ADD_TEST` もふつうは同じ条件の中にあります。
- 独自のマクロ（`NAOS_SUITE` や `NAOS_TEST` に展開されるもの）で定義したスイートやテスト: 見つけられません。
- `IGNORE_SUITES` に書いたスイートや、追加していないスイートを定義しているファイルのテスト: そのスイートの中にあるかもしれず、スイートの 1 行で足ります。
- setup と teardown: `--naos-list` に出ません。どのスイートも設定していない setup や teardown は使われない `static` 関数なので、GCC と Clang の警告（`-Wall`）が報告します。

引数でだけスイートを追加する独自の `main()` では、そのスイートを `IGNORE_SUITES` に書きます（[独自の main()](../guides/custom-main.md)）。

### CMake の関数を使わないとき

この確認は、関数と一緒にインストールされる単独のスクリプト（`NaosCheckRegistration.cmake`）でもできます。実行ファイルをビルドした後に実行します。

```sh
cmake -D NAOS_EXECUTABLE=build/my_tests -D "NAOS_SOURCES=main.c;parser_test.c" -P NaosCheckRegistration.cmake
```

| 変数 | 内容 |
|---|---|
| `NAOS_EXECUTABLE` | テストの実行ファイル。`--naos-list` で実行する |
| `NAOS_SOURCES` | そのソース。`;` で区切る。相対パスはカレントディレクトリから |
| `NAOS_IGNORE_SUITES` | 省略可。わざと追加していないスイート。`;` で区切る |

定義したテストとスイートがすべて列挙されれば 0 で終了し、そうでなければ上の行を出して失敗します。

## オプション

| オプション | 既定値 | 内容 |
|---|---|---|
| `NAOS_SELFTEST` | トップレベル時のみ `ON` | Naos 自身のテストを登録する。`BUILD_TESTING` も `ON` のとき（`include(CTest)` 済みのとき）だけ効く。Naos がトップレベルのときは、警告をエラーにしてビルドする（GCC / Clang は `-Wall -Wextra -Wpedantic -Werror`、MSVC は `/W4 /WX`） |
| `NAOS_INSTALL` | トップレベル時のみ `ON` | インストール規則を生成する |
| `BUILD_SHARED_LIBS` | `OFF` | `ON` なら共有ライブラリ（DLL）にする。Naos がトップレベルのときだけ効き、ほかのプロジェクトに組み込んだときは常に静的ライブラリ |

## パッケージ

`cmake --install` の後は、`find_package(Naos)` で Naos が見つかり、`Naos::naos` と上の関数が使えます。

```cmake
find_package(Naos 0.3 REQUIRED)
```

1.0 より前は、バージョンを指定すると同じマイナーバージョンだけを受け付けます。`find_package(Naos 0.3)` は 0.3.x だけです。1.0 以後は同じメジャーバージョンを受け付けます。

インストールされるファイル（インストール先からの相対パス）です。

| パス | 内容 |
|---|---|
| `include/naos/` | 公開ヘッダ |
| `lib/`（DLL は `bin/`） | ライブラリ |
| `bin/` | MSVC の Debug または RelWithDebInfo でビルドした DLL の PDB |
| `lib/cmake/Naos/` | `NaosConfig.cmake`、`NaosConfigVersion.cmake`、`NaosTargets*.cmake`、`Naos.cmake`、`NaosDiscover.cmake`、`NaosCheckRegistration.cmake` |
| `share/licenses/naos/LICENSE` | ライセンス |

ディレクトリは `GNUInstallDirs`（`CMAKE_INSTALL_LIBDIR` など）に従います。
