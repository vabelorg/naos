# テストを書く

## ファイルを分ける

テストが増えたら、スイートごとにファイルを分けます。`main()` を書くファイルでは、ほかのファイルのスイートを `NAOS_DECLARE_SUITE` で宣言した後、`NAOS_MAIN` に並べます。

```c
/* parser_test.c */
#include <naos/naos.h>

NAOS_TEST(empty_object) { ... }
NAOS_TEST(nested) { ... }

NAOS_SUITE(parser,
    NAOS_ADD_TEST(empty_object),
    NAOS_ADD_TEST(nested)
)
```

```c
/* main.c */
#include <naos/naos.h>

NAOS_DECLARE_SUITE(parser);
NAOS_DECLARE_SUITE(writer);

NAOS_MAIN(
    NAOS_ADD_SUITE(parser),
    NAOS_ADD_SUITE(writer)
)
```

```cmake
naos_add_executable(my_tests SOURCES main.c parser_test.c writer_test.c LIBRARIES my_lib)
```

- テスト関数は `static` です。ファイルが異なれば同じテスト名を使えますが、1 つのスイートのテストは 1 つのファイルに収めます。
- スイートの名前は、1 つの実行ファイルの中で重複しないようにします（重複するとリンクエラーになります）。
- スイートが大きくなったら、`parser_object`、`parser_array` のように名前の先頭をそろえたスイートに分けます。`--naos-filter 'parser_*'` でまとめて指定できます。

## 追加し忘れたテストやスイートに気付く

テストは 2 か所に書きます。`NAOS_TEST` で定義し、`NAOS_ADD_TEST` でスイートに追加します。スイートも同じで、`NAOS_SUITE` の後に `NAOS_ADD_SUITE` です。後半が抜けると、そのテストは一度も実行されず、それでも実行は成功します。

`naos_add_executable()` は、どちらもビルドエラーにします（[登録漏れの確認](../reference/cmake.md#登録漏れの確認)）。

- どのスイートも追加していないテスト・setup・teardown はコンパイルできません。これらの関数は、コンパイラが未使用だと気付けるよう、わざと `static` にしてあります。エラーには `naos_gen_test_<name>` が出ます。
- 定義したのに追加していないスイートは、リンクの後にビルドを失敗させ、定義した行を示します。

  ```
  slow_test.c:21: error: suite slow is defined but never added, so its tests never run: add it with NAOS_ADD_SUITE(slow), or name it in IGNORE_SUITES of naos_add_executable() or naos_discover_tests() when it is left out on purpose
  ```

抜けているほうを書き足すか、要らなくなったものを消します。[独自の main()](custom-main.md) が引数でだけ追加するスイートのように、わざと外しているスイートは `IGNORE_SUITES` に書きます。

`naos_discover_tests()` はスイートを確かめますが、ターゲットのコンパイルオプションには手を付けません。そこや、ほかのビルドシステムでは、テストを `-Werror=unused-function`（GCC、Clang）か `/we4505`（MSVC）を付けてビルドします。CMake の関数を使わないときは、スイートを[スクリプト](../reference/cmake.md#cmake-の関数を使わないとき)で確かめます。

## アサーションを選ぶ

アサーションは、整数、符号なし整数、double、文字列、メモリ、ポインタと、値の型ごとに用意されています（[アサーション](../reference/assertions.md)）。型に合ったものを使うと、失敗したときに両辺の値が型に合った形で表示されます。

```c
NAOS_CHECK_STR_EQ("hello", greeting); // greeting は "hello\n"
```

```
greet_test.c:12: error: "hello" == greeting
  left:  "hello"
  right: "hello\n"
  first difference: offset 5, end of left
```

型ごとのアサーションに当てはまらない条件は、`NAOS_CHECK_TRUE(expr)` や `NAOS_CHECK_FALSE(expr)` で確かめます。ただし、値は表示されません。

`NAOS_CHECK*` は、失敗してもテストの実行を続けます。続けても意味がない場合（`NULL` かもしれないポインタの指す先を読む場合など）は、対応する `NAOS_REQUIRE*` を使います。こちらは失敗した時点でテストを止めます。

```c
NAOS_REQUIRE_NOT_NULL(node); // NULL なら、ここでテストが止まる
NAOS_CHECK_STR_EQ("root", node->name);
```

## アサーションがない型を比べる

構造体のように専用のアサーションがない型は、`NAOS_CHECK_TRUE` で比較し、`NAOS_NOTE` で値を表示する小さなマクロを作ります。

関数ではなくマクロにすると、失敗の行がマクロを呼び出した行を指します。ただし、引数はそれぞれ複数回評価されるため、`points[i++]` のような副作用のある式は渡さないでください。

```c
#define CHECK_POINT_EQ(a, b) \
    do \
    { \
        NAOS_NOTE("left (%d, %d), right (%d, %d)", (a).x, (a).y, (b).x, (b).y); \
        NAOS_CHECK_TRUE(point_eq(&(a), &(b))); \
    } while (0)
```

```
geom_test.c:20: error: point_eq(&(expected), &(actual))
  note: left (2, 1), right (1, 1)
```

## コードが動くことを確かめる

アサーションを 1 つも実行しないテストは失敗します。Naos は、それを何も確かめていないテストとみなします。クラッシュせず、`assert` でも止まらずに動くこと自体が確かめたいことなら、必ず成功するアサーション `NAOS_PASS()` でそれを示します。

```c
NAOS_TEST(free_null_is_harmless)
{
    buffer_free(NULL);   /* the check: this returns */
    NAOS_PASS();
}
```

テストを先に書いて本体を後回しにすると、何かを確かめるまでそのテストは失敗します。名前だけでは成功になりません（[アサーションを実行しないテスト](../reference/control.md#アサーションを実行しないテスト)）。

## 今は失敗するはずのテストに印を付ける

コードより先に書いたテストは、はじめは失敗します。まだ直していない既知のバグのテストも同じです。こうしたテストは `NAOS_ADD_TEST_XFAIL` で理由とともに登録します。テストが失敗しているあいだは実行が成功し、成功したとたんに失敗するので、印はコードができたときに外すことになり、ずっと残りません。

```c
NAOS_TEST(parses_negative_numbers)
{
    NAOS_CHECK_INT_EQ(-42, parse_int("-42"));
}

NAOS_SUITE(parser,
    NAOS_ADD_TEST(parses_positive_numbers),
    NAOS_ADD_TEST_XFAIL(parses_negative_numbers, "sign handling not implemented yet")
)
```

```
RUN: parser.parses_negative_numbers
parser_test.c:3: error: -42 == parse_int("-42")
  left:  -42
  right: 42
XFAIL: parser.parses_negative_numbers (0 ms)
```

失敗の内容を読んでください。思ったとおりの失敗で、書き間違いによるものではないはずです。`parse_int()` が符号を扱えるようになると、印の場所で実行が失敗します。`NAOS_ADD_TEST` に書き換える行です。

```
parser_test.c:8: error: passed, but is expected to fail
  reason: "sign handling not implemented yet"
XPASS: parser.parses_negative_numbers (0 ms)
```

`NAOS_SKIP` やスイートから外しておくより、こちらを使ってください。スキップしたテストは、通るようになっても知らせてくれません。setup と teardown も本体と合わせて数え、クラッシュは期待どおりの失敗になりません（[失敗するはずのテスト](../reference/control.md#失敗するはずのテスト)）。

## 条件がそろわないテストをスキップする

必要なファイルや環境変数がないなど、テストを実行しても意味がない場合は、`NAOS_SKIP` でテストをスキップします。スキップしたテストは失敗として数えられず、結果は `SKIP` と表示されます。

```c
if (getenv("LARGE_FILE") == NULL)
{
    NAOS_SKIP("LARGE_FILE is not set");
}
```

```
io_test.c:8: skip: LARGE_FILE is not set
SKIP: io.reads_large_file (0 ms)
```

- setup の中で `NAOS_SKIP` を呼ぶと、スイートのテストをまとめてスキップできます（[setup と teardown](fixtures.md)）。
- 失敗がスキップに優先します。スキップする前に失敗していたテストは `FAIL` です。
- 実行したテストがすべてスキップだった場合、終了コードは `77` になり、CTest ではスキップとして数えられます。
- `--naos-fail-fast` はスキップでは止まりません。
