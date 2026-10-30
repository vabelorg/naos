# コードの書き方

Naos のコードの書き方です。新しいコードは周りのコードに合わせます。このページは、そのコードが従っている規則を書いたものです。

## 言語

- C99。コンパイラ拡張は使わない（`C_EXTENSIONS NO`）。公開ヘッダは C++11 としてもコンパイルできること。
- GCC、Clang、MSVC で、`-Wall -Wextra -Wpedantic` と `/W4` の警告なしにビルドできること。自己テストでは警告をエラーにする。

## 書式

- 波括弧は Allman スタイル（関数でも文でも改行して開く）。
- `if`・`for`・`while` の本体が 1 行でも波括弧を付ける。
- インデントは空白 4 文字。タブは使わない。
- ほかの文に続く `return` の前と、文のまとまりの間は空行。
- `&&`・`||`・`?:` のオペランドがそれ自体演算なら、括弧で囲む。

```c
/* a + b。和が収まらなければ SIZE_MAX。 */
static size_t add_saturated(size_t a, size_t b)
{
    return (a > (SIZE_MAX - b)) ? SIZE_MAX : (a + b);
}
```

```c
    if ((orders->suites == NULL) || (orders->tests == NULL))
    {
        free(orders->suites);
        free(orders->tests);

        return 0;
    }

    return 1;
```

## 名前

| 種類 | 規則 | 例 |
|---|---|---|
| ファイル | 小文字の関心名。公開ヘッダを実装するファイルはヘッダと同名、内部ヘッダは `.c` と同名、同名の公開ヘッダがある内部ヘッダだけ `_internal.h` | `diagnostic.c`、`control.c`、`filter.h`、`suite_internal.h` |
| 関数 | `naos_<モジュール>_<動詞>`。ファイル内だけの関数は `static` で接頭辞なし | `naos_diagnostic_begin`、`naos_options_get`、`take_filter` |
| 型 | `naos_<名前>_t` | `naos_cmp_t`、`naos_report_kind_t` |
| 公開マクロ | `NAOS_` で始まる大文字 | `NAOS_CHECK_INT_EQ` |
| 補助マクロ | 公開の名前の末尾に `_` | `NAOS_CHECK_` |
| ファイル内の状態 | `static` 変数に接頭辞 `g_` | `g_options`、`g_traces` |
| インクルードガード | `NAOS_<名前>_H` | `NAOS_CHECK_H` |

## インクルード

```c
#include "runner.h"          /* このファイルが実装するヘッダ（公開でも内部でも）が先頭 */

#include <stdint.h>          /* 標準ヘッダ */
#include <stdlib.h>

#include <naos/exit.h>       /* ほかの公開ヘッダ */

#include "filter.h"          /* ほかの内部ヘッダ */
#include "order.h"
```

各グループの間は空行で、グループの中はアルファベット順。公開ヘッダは `<naos/...>`、内部ヘッダは `"..."` で読み込みます。

## コメント

- 英語で書く。
- ファイルの先頭に著作権表示と `SPDX-License-Identifier: MIT`、続けてそのファイルの責務を短く書く。
- 関数の宣言には、何を返すか、引数の約束（`NULL` を許すか、借用したポインタがいつまで有効である必要があるか）を 1 文で書く。
- コメントには理由や自明でないことを書く。次の行がそのまま示すことは書かない。

## CMake

- コマンドは小文字、インデントは空白 4 文字。
- 利用者向けの関数は `naos_*`、内部用は `_naos_*`。
- ファイル名は使われ方で決める。`include()` で読み込むモジュールは CMake 本体のモジュールにならって `Naos<名前>.cmake`（`NaosVersion.cmake`）、`cmake -P` で実行するスクリプトは小文字の単語を `-` でつなぐ（`prepare-release.cmake`、`check-output.cmake`）。パッケージの設定ファイルは `find_package()` が探す名前にする（`naos-config.cmake`）。
- ファイルの先頭に著作権表示と、何のためのファイルで、どう使われるかのコメントを書く。

## サンプル

`examples/` のプロジェクトは利用者が読むものです。同じ書き方に従い、コメントは保守する人ではなく読む人に向けて書きます。自己テストの厳しいフラグの下でも警告なしにビルドできるようにします。
