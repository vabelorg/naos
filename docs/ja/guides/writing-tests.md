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

構造体のように専用のアサーションがない型は、`NAOS_CHECK_TRUE` で比較し、`NAOS_NOTE` で値を表示する小さなマクロを作ります。関数ではなくマクロにすると、失敗の行がマクロを呼び出した行を指します。ただし、引数はそれぞれ複数回評価されるため、`points[i++]` のような副作用のある式は渡さないでください。

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
SKIP: io.reads_large_file
```

- setup の中で `NAOS_SKIP` を呼ぶと、スイートのテストをまとめてスキップできます（[setup と teardown](fixtures.md)）。
- 失敗がスキップに優先します。スキップする前に失敗していたテストは `FAIL` です。
- 実行したテストがすべてスキップだった場合、終了コードは `77` になり、CTest ではスキップとして数えられます。
- `--naos-fail-fast` はスキップでは止まりません。
