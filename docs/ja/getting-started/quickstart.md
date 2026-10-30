# クイックスタート

空のディレクトリから、CTest でテストを実行するところまでを 5 分で進めます。C コンパイラ、CMake 3.21 以上、Git が必要です。以下で作る `clamp.h`、`clamp.c`、`clamp_test.c`、`CMakeLists.txt` の 4 つのファイルは、同じディレクトリに置きます。完成したプロジェクトは [examples/quickstart](../../../examples/quickstart/) にあります。

## 1. テストするコード

値を `low` から `high` の範囲に収める関数 `clamp()` だけを持つ、小さなライブラリです。次の節でこれをテストします。

```c
/* clamp.h */
#ifndef CLAMP_H
#define CLAMP_H

/* value を範囲 [low, high] に収めた値を返す。 */
int clamp(int value, int low, int high);

#endif
```

```c
/* clamp.c */
#include "clamp.h"

int clamp(int value, int low, int high)
{
    if (value < low)
    {
        return low;
    }

    if (value > high)
    {
        return high;
    }

    return value;
}
```

## 2. テスト

`clamp()` の動きを 3 つの場合に分けて確かめます。範囲の内側の値はそのまま返ること（`inside_range`）、下限を下回る値は `low` になること（`below_range`）、上限を上回る値は `high` になること（`above_range`）です。

```c
/* clamp_test.c */
#include <naos/naos.h>

#include "clamp.h"

NAOS_TEST(inside_range)
{
    NAOS_CHECK_INT_EQ(5, clamp(5, 0, 10));
}

NAOS_TEST(below_range)
{
    NAOS_CHECK_INT_EQ(0, clamp(-3, 0, 10));
}

NAOS_TEST(above_range)
{
    NAOS_CHECK_INT_EQ(10, clamp(42, 0, 10));
}

NAOS_SUITE(clamp,
    NAOS_ADD_TEST(inside_range),
    NAOS_ADD_TEST(below_range),
    NAOS_ADD_TEST(above_range)
)

NAOS_MAIN(
    NAOS_ADD_SUITE(clamp)
)
```

コードは上から順に、次のことをしています。

- `#include <naos/naos.h>` で、以下のマクロをすべて使えるようにします。
- `NAOS_TEST` はテストを関数として定義します。
- `NAOS_SUITE` はテストをスイートにまとめます。テストは自動では登録されないので、どのスイートにも並べていないテストは実行されません。
- `NAOS_MAIN` は、並べたスイートを実行する `main()` を定義します。
- `NAOS_CHECK_INT_EQ(expected, actual)` は 2 つの整数を比べ、違っていれば両方の値を表示します。

## 3. ビルドの設定

`CMakeLists.txt` を次のように書きます。

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.21)
project(quickstart LANGUAGES C)

enable_testing()

include(FetchContent)
FetchContent_Declare(naos
    GIT_REPOSITORY https://github.com/vabelorg/naos.git
    GIT_TAG v0.0.0
)
FetchContent_MakeAvailable(naos)

add_library(clamp clamp.c)
target_include_directories(clamp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

naos_add_executable(clamp_tests SOURCES clamp_test.c LIBRARIES clamp)
```

- `enable_testing()` は CTest を有効にします。これがないとテストを登録できません。
- `FetchContent` は、指定したタグの Naos を Git から取得してビルドに取り込みます。事前のインストールは要りません。
- `add_library(clamp ...)` と `target_include_directories(...)` は、テスト対象の `clamp.c` をライブラリにし、`clamp.h` を include できるようにします。
- `naos_add_executable(clamp_tests SOURCES clamp_test.c LIBRARIES clamp)` は、`clamp_test.c` と `clamp` ライブラリをリンクした実行ファイル `clamp_tests` を作り、ビルドのたびに、そのテストを 1 件ずつ CTest に登録します。

## 4. ビルドして実行する

設定、ビルド、テスト実行の 3 つのコマンドを順に実行します。

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

CTest はテストごとに別のプロセスを起動し、`<target>.<suite>.<test>` という名前で表示します。

```
    Start 1: clamp_tests.clamp.inside_range
1/3 Test #1: clamp_tests.clamp.inside_range ...   Passed
    Start 2: clamp_tests.clamp.below_range
2/3 Test #2: clamp_tests.clamp.below_range ....   Passed
    Start 3: clamp_tests.clamp.above_range
3/3 Test #3: clamp_tests.clamp.above_range ....   Passed

100% tests passed, 0 tests failed out of 3
```

これにより、テスト用の実行ファイル `clamp_tests` が生成され、3 つのテストがそれぞれ別のテストとして実行され、すべて通りました。このように、Naos なら、テストを書いて CMake に数行足すだけで、C のテストをすぐに実行できます。

実行ファイルは単独でも実行でき、その場合はすべてのテストを 1 つのプロセスで実行します。以下は Makefile や Ninja などのシングルコンフィグのジェネレータでの例です。

```
$ ./build/clamp_tests
RUN: clamp.inside_range
PASS: clamp.inside_range
RUN: clamp.below_range
PASS: clamp.below_range
RUN: clamp.above_range
PASS: clamp.above_range
SUMMARY: 3 passed, 0 failed, 0 skipped
```

Visual Studio などのマルチコンフィグのジェネレータでは、ビルドに `--config Debug` を、`ctest` に `-C Debug` を付けます。実行ファイルは `build\Debug\clamp_tests.exe` になります。

## 5. 失敗させてみる

テストが失敗したときの表示を確かめるために、わざと間違った期待値を書きます。

`inside_range` の期待する値を `5` から `6` に変え、ビルドし直して実行します。

```
$ ./build/clamp_tests
RUN: clamp.inside_range
clamp_test.c:10: error: 6 == clamp(5, 0, 10)
  left:  6
  right: 5
FAIL: clamp.inside_range (1 failure)
RUN: clamp.below_range
PASS: clamp.below_range
RUN: clamp.above_range
PASS: clamp.above_range
SUMMARY: 2 passed, 1 failed, 0 skipped
```

`inside_range` だけが失敗し、残りの 2 つは通りました。1 つのテストが失敗しても、ほかのテストは実行されます。失敗の表示は次のように読みます。

- `error` の行はコンパイラの診断と同じ `file:line: error: ...` の形なので、エディタからその行に移動できます。
- 式はアサーションの引数を書いたとおりに示し、`left` と `right` はその値です。
- 終了コードは 1 になり、`ctest` はそのテストを失敗として報告します。

このテストだけを実行するには、名前を指定します。

```sh
./build/clamp_tests --naos-filter clamp.inside_range
```

最後に、値を `5` に戻しておきます。
