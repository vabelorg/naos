# チュートリアル

小さな文字列バッファを題材に、多くのテストで必要になること（複数のファイル、`CHECK` と `REQUIRE`、setup と teardown、失敗の読み方、メモ、一部のテストだけの実行）を順に試します。[クイックスタート](quickstart.md)を終えていることを前提にします。完成したプロジェクトは [examples/tutorial](../../../examples/tutorial/) にあります。

## テストするコード

`strbuf` は、文字列を追加するたびに必要に応じて領域を広げる文字列バッファです。中身は常に NUL で終わります。

```c
/* strbuf.h */
struct strbuf
{
    char *data;
    size_t length;   /* 使っているバイト数（NUL を除く） */
    size_t capacity; /* 確保したバイト数 */
};

/* sb を空のバッファにする。0 を返す。メモリ不足なら -1（sb はゼロで埋まる）。 */
int strbuf_init(struct strbuf *sb);

/* sb のメモリを解放し、ゼロで埋める。ゼロのバッファや解放済みのバッファにも安全。 */
void strbuf_free(struct strbuf *sb);

/* sb に text を追加する。0 を返す。メモリ不足なら -1（sb は変わらない）。 */
int strbuf_append(struct strbuf *sb, const char *text);

/* sb の内容を C 文字列で返す。sb が変わるまで有効。 */
const char *strbuf_cstr(const struct strbuf *sb);
```

実装は [strbuf.c](../../../examples/tutorial/strbuf.c) にあります。

## プロジェクト

テストは 2 つのファイルに分けます。`strbuf_test.c` でスイートを定義し、`main.c` で実行するスイートを並べます。スイートが増えたら、スイートごとにファイルを足し、`main.c` に並べていきます。

```cmake
# CMakeLists.txt（FetchContent はクイックスタートと同じ）
add_library(strbuf strbuf.c)
target_include_directories(strbuf PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

naos_add_executable(strbuf_tests
    SOURCES main.c strbuf_test.c
    LIBRARIES strbuf
)
```

```c
/* main.c */
#include <naos/naos.h>

NAOS_DECLARE_SUITE(strbuf);

NAOS_MAIN(
    NAOS_ADD_SUITE(strbuf)
)
```

`NAOS_DECLARE_SUITE(strbuf);` は、別のファイルで定義したスイートを宣言します。スイートの名前は 1 つの実行ファイルの中で重複できません。テスト関数は `static` なので、ファイルが違えば同じテスト名を使えます。

## setup と teardown

どのテストも、空のバッファから始めたいとします。バッファを作る処理と解放する処理を、テストごとに書くのは手間です。作る処理を setup に、解放する処理を teardown に書いておくと、各テストの前に setup が、後に teardown が実行されます。

```c
/* strbuf_test.c */
#include <naos/naos.h>

#include "strbuf.h"

/* setup、テスト、teardown で共有する。 */
static struct strbuf sb;

NAOS_SETUP(setup)
{
    NAOS_REQUIRE_INT_EQ(0, strbuf_init(&sb));
}

NAOS_TEARDOWN(teardown)
{
    strbuf_free(&sb);
}
```

- setup と teardown は引数を受け取らないので、テストと共有する状態はファイルの `static` 変数に置きます。
- `strbuf_init()` が失敗すると、`NAOS_REQUIRE_INT_EQ` がその場でテストを止め、テストの本体は実行されません。
- teardown は、setup やテストが途中で止まっても実行されます。

スイートには、テストと一緒にファイルの末尾で並べます。

```c
NAOS_SUITE(strbuf,
    NAOS_SET_SETUP(setup),
    NAOS_SET_TEARDOWN(teardown),
    NAOS_ADD_TEST(starts_empty),
    NAOS_ADD_TEST(append_joins),
    NAOS_ADD_TEST(append_grows)
)
```

## CHECK と REQUIRE

```c
NAOS_TEST(starts_empty)
{
    NAOS_CHECK_UINT_EQ(0, sb.length);
    NAOS_CHECK_STR_EQ("", strbuf_cstr(&sb));
}

NAOS_TEST(append_joins)
{
    NAOS_REQUIRE_INT_EQ(0, strbuf_append(&sb, "hello, "));
    NAOS_REQUIRE_INT_EQ(0, strbuf_append(&sb, "world"));

    NAOS_CHECK_STR_EQ("hello, world", strbuf_cstr(&sb));
    NAOS_CHECK_UINT_EQ(12, sb.length);
}
```

アサーションには 2 つの形があります。

- `NAOS_CHECK_*` は失敗を記録して続けます。1 回の実行で、違っている値をすべて確認できます。
- `NAOS_REQUIRE_*` は失敗を記録してテストを止めます。失敗した後に続けても意味がないときに使います。

アサーションは値の型に合わせて選びます（`size_t` には `*_UINT_*`、文字列には `*_STR_*`）。失敗したとき、値が型に合った形で表示されます。

## 失敗を読む

`strbuf_append()` に、`length` を更新し忘れるバグがあったとします。毎回バッファの先頭に書き込むことになるため、`append_joins` は次のように出力します。

```
RUN: strbuf.append_joins
strbuf_test.c:32: error: "hello, world" == strbuf_cstr(&sb)
  left:  "hello, world"
  right: "world"
  first difference: offset 0, left 0x68, right 0x77
strbuf_test.c:33: error: 12 == sb.length
  left:  12
  right: 0
FAIL: strbuf.append_joins (2 failures)
```

`CHECK` は最初の失敗の後も続けるので、2 つのチェックがどちらも失敗として報告されています。文字列では、`first difference` が最初に違うバイトの位置を示します。出力の各行は[出力](../reference/output.md)で説明しています。

## ループの中のメモ

ループの中のアサーションが失敗しても、表示される行番号だけでは何回目で失敗したかが分かりません。`NAOS_NOTE` でメモを付けておくと、失敗と一緒に表示されます。

```c
NAOS_TEST(append_grows)
{
    int i;

    for (i = 0; i < 100; i++)
    {
        NAOS_NOTE("append %d", i);
        NAOS_REQUIRE_INT_EQ(0, strbuf_append(&sb, "x"));
    }

    NAOS_CHECK_UINT_EQ(100, sb.length);
    NAOS_CHECK_UINT_GT(sb.capacity, sb.length);
}
```

メモは、直後のアサーションが失敗したときだけ、`note: append 37` のように表示されます。成功したときは何も表示されません。ヘルパー関数をどこから呼んだかを、入れ子の呼び出しまでたどって表示するには、trace を使います（[診断](../guides/diagnostics.md)）。

## 一部のテストだけを実行する

調べているテストだけを実行すれば、修正と確認をすばやく繰り返せます。`--naos-list` は各テストの名前を表示します。

```
$ ./build/strbuf_tests --naos-list
strbuf.starts_empty
strbuf.append_joins
strbuf.append_grows
```

`--naos-filter` は、パターンに一致するテストだけを実行します。`*` は任意の文字列に一致します。

```
$ ./build/strbuf_tests --naos-filter 'strbuf.append_*'
RUN: strbuf.append_joins
PASS: strbuf.append_joins
RUN: strbuf.append_grows
PASS: strbuf.append_grows
SUMMARY: 2 passed, 0 failed, 0 skipped
```

## 隠れた依存

テストはすべて 1 つのプロセスで実行されるため、`static` 変数の値は次のテストに引き継がれます。その結果、前のテストが残した状態に依存して通るテストができてしまうことがあります。`--naos-shuffle` はテストの順序を入れ替えて実行し、使ったシード値を表示します。`--naos-repeat` と組み合わせると、回ごとに異なる順序で繰り返し実行できます。

```sh
./build/strbuf_tests --naos-shuffle --naos-repeat 20
```

ここでは setup がテストごとに新しいバッファを作るので、どの順序でも通ります。失敗した順序を再現する方法は[テストの実行](../guides/running-tests.md)にあります。
