# アサーション

どのアサーションにも 2 つの形があります。

- `NAOS_CHECK*` は失敗を記録して、テストを続けます。
- `NAOS_REQUIRE*` は失敗を記録して、テストを止めます。`NAOS_REQUIRE*` の失敗は**致命的な失敗**です。それが何を意味するかは[テストの止まり方](control.md#テストの止まり方)を見てください。

どちらも型が `void` の式なので、カンマ式の中などにも書けます。各引数は 1 回だけ評価します。

失敗したアサーションは、場所と、書いたとおりの引数を示す `error` の行を出力し、続けて値を詳細の行として出力します。その後に[メモ](control.md#メモ)と [trace](control.md#trace) が続きます。形式の全体は[出力](output.md)にあります。

```
parser_test.c:42: error: count < limit
  left:  8
  right: 8
```

## 真偽値

```c
NAOS_CHECK_TRUE(expr)       NAOS_REQUIRE_TRUE(expr)
NAOS_CHECK_FALSE(expr)      NAOS_REQUIRE_FALSE(expr)
```

`*_TRUE` は `expr` が真（0 以外）なら、`*_FALSE` は偽（0）なら成功です。`expr` には `bool` のほか、整数やポインタも渡せます。値は出力しません。`error` の行は、`*_TRUE` では `expr` を書いたとおりに、`*_FALSE` では `!(expr)` の形で示します。

```
parser_test.c:57: error: !(is_empty(list))
```

専用の比較がない型は、`NAOS_CHECK_TRUE` と `NAOS_NOTE` を組み合わせたマクロを作れば、失敗したときに両辺を出力できます。関数ではなくマクロにすると、`error` の行が呼び出した場所を指します。

```c
#define CHECK_POINT_EQ(a, b) \
    do \
    { \
        NAOS_NOTE("left (%d, %d), right (%d, %d)", (a).x, (a).y, (b).x, (b).y); \
        NAOS_CHECK_TRUE(point_eq(&(a), &(b))); \
    } while (0)
```

## 整数

```c
NAOS_CHECK_INT_EQ(a, b)     NAOS_REQUIRE_INT_EQ(a, b)
NAOS_CHECK_INT_NE(a, b)     NAOS_REQUIRE_INT_NE(a, b)
NAOS_CHECK_INT_LT(a, b)     NAOS_REQUIRE_INT_LT(a, b)
NAOS_CHECK_INT_LE(a, b)     NAOS_REQUIRE_INT_LE(a, b)
NAOS_CHECK_INT_GT(a, b)     NAOS_REQUIRE_INT_GT(a, b)
NAOS_CHECK_INT_GE(a, b)     NAOS_REQUIRE_INT_GE(a, b)

NAOS_CHECK_UINT_EQ(a, b)    NAOS_REQUIRE_UINT_EQ(a, b)
NAOS_CHECK_UINT_NE(a, b)    NAOS_REQUIRE_UINT_NE(a, b)
NAOS_CHECK_UINT_LT(a, b)    NAOS_REQUIRE_UINT_LT(a, b)
NAOS_CHECK_UINT_LE(a, b)    NAOS_REQUIRE_UINT_LE(a, b)
NAOS_CHECK_UINT_GT(a, b)    NAOS_REQUIRE_UINT_GT(a, b)
NAOS_CHECK_UINT_GE(a, b)    NAOS_REQUIRE_UINT_GE(a, b)
```

末尾の `EQ`、`NE`、`LT`、`LE`、`GT`、`GE` は、それぞれ `a == b`、`a != b`、`a < b`、`a <= b`、`a > b`、`a >= b` なら成功です。`*_INT_*` は両方の引数を `intmax_t` に、`*_UINT_*` は `uintmax_t` に変換して比較します。`left` と `right` は 10 進で出力します。

`INTMAX_MAX` を超えうる符号なしの値（`SIZE_MAX` など）には `*_UINT_*` を、負になりうる値には `*_INT_*` を使います。負の値を `uintmax_t` に変換すると大きな正の数になり、大きな符号なしの値を `intmax_t` に変換した結果は処理系定義です。

## double

```c
NAOS_CHECK_DOUBLE_NEAR(a, b, tol)
NAOS_REQUIRE_DOUBLE_NEAR(a, b, tol)
```

引数を `double` に変換し、`|a - b| <= tol` なら成功です。等しい値は、同じ符号の無限大どうしも含めて、`tol` によらず常に成功します。どちらかが NaN なら常に失敗します。

```
geom_test.c:31: error: area ~= expected
  left:  0.3333333333333333
  right: 0.33
  tolerance: 0.001
```

値は、読み戻すと同じ `double` になる最も短い 10 進表記（`0.1`、`100`、`1e-05`、`1.5e+20`）か、`-0`、`inf`、`-inf`、`nan` で出力します。小数点はロケールによらず `.` です。[値の表記](output.md#値の表記)を見てください。

## 文字列

```c
NAOS_CHECK_STR_EQ(a, b)     NAOS_REQUIRE_STR_EQ(a, b)
NAOS_CHECK_STR_NE(a, b)     NAOS_REQUIRE_STR_NE(a, b)
```

`*_STR_EQ` は、`a` と `b` が終端の NUL まで同じバイト列なら成功です。`*_STR_NE` はそうでないとき成功です。`NULL` どうしは等しく、`NULL` と文字列は、空文字列であっても等しくありません。比較はバイト単位で、ロケール、大文字と小文字の同一視、Unicode の正規化は行いません。

```
test.c:12: error: name == "alice"
  left:  "alicia"
  right: "alice"
  first difference: offset 4, left 0x69, right 0x65
```

- 値は二重引用符で囲んで 1 行で出力し、制御文字、`"`、`\`、0x80 以上のバイトをエスケープします。長くても切り詰めません。`NULL` は `(null)` と出力します。[値の表記](output.md#値の表記)を見てください。
- `first difference` は最初に違うバイトの位置を示します。一方がもう一方の先頭部分と一致するときは `offset N, end of left` / `end of right` です。出力するのは `*_STR_EQ` だけで、それもどちらの値も `NULL` でないときだけです。
- NUL を含みうるバッファを比べるときは[メモリ](#メモリ)の比較を使います。

## メモリ

```c
NAOS_CHECK_MEM_EQ(a, a_size, b, b_size)
NAOS_REQUIRE_MEM_EQ(a, a_size, b, b_size)
```

`a` からの `a_size` バイトと、`b` からの `b_size` バイトが同じなら成功です。つまり、サイズが等しく、すべてのバイトが等しいときです。サイズが 0 のポインタは `NULL` でも構いません。

```
codec_test.c:58: error: expected == actual
  left size:  3
  right size: 3
  first difference: offset 1, left 0x02, right 0xFF
```

バイト列そのものは出力しません。3 行目は次のどれかです。

- `first difference: offset N, left 0xXX, right 0xYY`: 最初に違うバイト。
- `first difference: offset N, end of left`（または `end of right`）: 一方がもう一方の先頭部分と一致し、短いとき。`N` は短いほうのサイズです。
- `invalid: left is NULL with non-zero size`（または `right is ...`、`left and right are ...`）。

## ポインタ

```c
NAOS_CHECK_PTR_EQ(a, b)
NAOS_REQUIRE_PTR_EQ(a, b)
```

`a` と `b` を `const void *` に変換して等しければ成功です。オブジェクトへのポインタに使います（関数ポインタは、標準 C では `const void *` に変換できません）。

```
list_test.c:77: error: list.head == &node
  left:  (null)
  right: 0x7ffd812c93a4
```

値は `%p` で出力し、形はプラットフォームによります。`NULL` は `(null)` です。

## NULL ポインタ

```c
NAOS_CHECK_NULL(ptr)        NAOS_REQUIRE_NULL(ptr)
NAOS_CHECK_NOT_NULL(ptr)    NAOS_REQUIRE_NOT_NULL(ptr)
```

`*_NULL` は `ptr` が NULL ポインタなら、`*_NOT_NULL` はそうでなければ成功です。`*_NULL` は失敗したときのポインタを `value` として `%p` で出力し、`*_NOT_NULL` は値を出力しません（失敗したときは必ず NULL なので）。

結果を使う前の確認には、`NAOS_REQUIRE_NOT_NULL` がよく使われます。

```c
struct node *node = list_find(&list, 3);

NAOS_REQUIRE_NOT_NULL(node);
NAOS_CHECK_INT_EQ(3, node->value);
```
