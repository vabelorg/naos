# CMake

CMake のプロジェクトが Naos から得るもの、つまりターゲット `naos::naos`、テストの実行ファイルを作って CTest にテストを登録する 2 つの関数、いくつかのオプションです。プロジェクトへの取り込み方は[インストール](../getting-started/installation.md)にあります。

## ターゲット

| ターゲット | 内容 |
|---|---|
| `naos::naos` | ライブラリ。インクルードディレクトリ `include/` と C99 の要求を伴う。共有ライブラリなら定義 `NAOS_SHARED` も伴う |

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
)
```

`SOURCES` から実行ファイル `<target>` を作り、`naos::naos` と `LIBRARIES` を（PRIVATE で）リンクし、そのテストを 1 件ずつ CTest に登録します。`SOURCES` と `LIBRARIES` 以外の引数は [`naos_discover_tests()`](#naos_discover_tests) と同じです。

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
)
```

`add_executable()` で作った実行ファイルのテストを 1 件ずつ登録します。実行ファイルは（直接、または `NAOS_MAIN` を通して）`naos_init()` と `naos_run()` を呼ぶ必要があります。

| 引数 | 内容 |
|---|---|
| `PREFIX <prefix>` | CTest のテスト名の頭。既定値は `<target>.` |
| `LABELS <label>...` | CTest のテストに付けるラベル |
| `PROPERTIES <name> <value>...` | CTest のテストに付けるその他のプロパティ。リストの値は 1 つの引数として渡す（`ENVIRONMENT "A=1;B=2"`） |
| `DISCOVERY_TIMEOUT <seconds>` | テストの列挙（`--naos-list`）の制限時間。正の整数で、既定値は 60。エミュレータなど、列挙が遅い環境で延ばす |

- 登録できるのは、同じディレクトリで作った実行ファイルターゲットだけで、1 つのターゲットにつき 1 回です。それ以外（ライブラリ、存在しない名前、2 回目の登録）や、`PROPERTIES` が名前と値の組になっていないときは、configure の時点でエラーになります。
- 各テストは CTest のテスト `<prefix><suite>.<test>` になり、`SKIP_RETURN_CODE 77` を付けて `<exe> --naos-filter <suite>.<test>` を実行します。
- プロパティ `CROSSCOMPILING_EMULATOR` を持つターゲットは、そのエミュレータを通して列挙・実行します。

## 列挙

テストは、実行ファイルをビルドするたびに、`--naos-list` で起動して列挙します。テストを足したり消したりしても、configure し直す必要はありません。

- 実行ファイルをビルドするまでは、テストの代わりに失敗するテスト `<prefix>NOT_BUILT` が 1 件だけ登録されています。`LABELS` も付くので、`ctest -L` で選んでも見落としません。マルチコンフィグのジェネレータで、ビルドしていない構成を `-C` で選んだときも同じです。
- 列挙に失敗するとビルドが失敗し、その理由と対処がメッセージに出ます。制限時間を超えたときは `DISCOVERY_TIMEOUT` を延ばすよう、それ以外では共有ライブラリ（DLL）が見つからない場合の対処を出します（[共有ライブラリ](../guides/shared-library.md)）。Windows で DLL が見つからないと、出力はなく `Exit code 0xc0000135` とだけ返るので、この場合は原因を言い切ります。
- `add_custom_command(TARGET ... POST_BUILD)` でターゲットに足したコマンドは、足した順に実行されます。列挙より前に実行したいコマンド（DLL のコピーなど）は、`add_executable()` の後、`naos_discover_tests()` より前に足します。`naos_add_executable()` では、列挙が常に先になります。

## オプション

| オプション | 既定値 | 内容 |
|---|---|---|
| `NAOS_SELFTEST` | トップレベル時のみ `ON` | Naos 自身のテストを登録する。`BUILD_TESTING` も `ON` のとき（`include(CTest)` 済みのとき）だけ効く。Naos がトップレベルのときは、警告をエラーにしてビルドする（GCC / Clang は `-Wall -Wextra -Wpedantic -Werror`、MSVC は `/W4 /WX`） |
| `NAOS_INSTALL` | トップレベル時のみ `ON` | インストール規則を生成する |
| `BUILD_SHARED_LIBS` | `OFF` | `ON` なら共有ライブラリ（DLL）にする。Naos がトップレベルのときだけ効き、ほかのプロジェクトに組み込んだときは常に静的ライブラリ |

## パッケージ

`cmake --install` の後は、`find_package(naos)` で Naos が見つかり、`naos::naos` と上の関数が使えます。

```cmake
find_package(naos 0.0 REQUIRED)
```

1.0 より前は、バージョンを指定すると同じマイナーバージョンだけを受け付けます。`find_package(naos 0.0)` は 0.0.x だけです。1.0 以後は同じメジャーバージョンを受け付けます。

インストールされるファイル（インストール先からの相対パス）です。

| パス | 内容 |
|---|---|
| `include/naos/` | 公開ヘッダ |
| `lib/`（DLL は `bin/`） | ライブラリ |
| `bin/` | MSVC の Debug または RelWithDebInfo でビルドした DLL の PDB |
| `lib/cmake/naos/` | `naos-config.cmake`、`naos-config-version.cmake`、`naos-targets*.cmake`、`Naos.cmake`、`NaosDiscover.cmake` |
| `share/licenses/naos/LICENSE` | ライセンス |

ディレクトリは `GNUInstallDirs`（`CMAKE_INSTALL_LIBDIR` など）に従います。
