# 診断

失敗がどこで、なぜ起きたのかを出力に示す方法と、ヘルパーからテストを止める方法です。規則は[失敗・スキップ・メモ](../reference/control.md)にあります。

## ループの何回目で失敗したかを表示する

ループの中のアサーションが失敗したとき、出力だけでは何回目で失敗したのかが分かりません。アサーションの直前に `NAOS_NOTE` でループ変数の値を書いておくと、失敗したときにだけ、その値が表示されます。

```c
for (i = 0; i < count; i++)
{
    NAOS_NOTE("i=%d", i);
    NAOS_CHECK_INT_EQ(expected[i], actual[i]);
}
```

```
parser_test.c:57: error: expected[i] == actual[i]
  left:  4
  right: 5
  note: i=3
```

メモが付くのは直後の 1 つのアサーションだけです。複数のアサーションに付けたい場合は、それぞれの前に書くか、trace を使います。

## ヘルパーをどこから呼んだかを表示する

アサーションをヘルパー関数にまとめると、失敗の行はヘルパーの中を指します。どの呼び出しで失敗したかを分かるようにするには、呼び出しを `NAOS_TRACE_PUSH` と `NAOS_TRACE_POP` で挟みます。挟んだ範囲で失敗すると、失敗の行に続けて、`NAOS_TRACE_PUSH` を書いた行とそのメモが表示されます。ヘルパーの中でさらに挟むこともでき、その場合は内側のものから順にすべて表示されます。

```c
static void check_order(const struct order *order)
{
    int i;

    for (i = 0; i < order->item_count; i++)
    {
        NAOS_TRACE_PUSH("item %d", i);
        check_item(&order->items[i]);
        NAOS_TRACE_POP();
    }
}

NAOS_TEST(orders)
{
    int i;

    for (i = 0; i < count; i++)
    {
        NAOS_TRACE_PUSH("order %d", i);
        check_order(&orders[i]);
        NAOS_TRACE_POP();
    }
}
```

`check_item` の中のアサーションが失敗すると、次のように表示されます。どの `order` の、どの `item` で失敗したかが分かります。

```
order_test.c:13: error: item->size > 0
  left:  0
  right: 0
order_test.c:22: note: item 3
order_test.c:34: note: order 1
```

`NAOS_TRACE_PUSH` と `NAOS_TRACE_POP` は必ず対にしてください。ただし、挟んだ範囲でテストが止まり、`NAOS_TRACE_POP` まで実行されなかった場合は、それを補う必要はありません。対にし忘れた場合の扱いは [trace](../reference/control.md#trace) にあります。

## ヘルパーの中でテストを止める

`NAOS_REQUIRE*`・`NAOS_FAIL`・`NAOS_SKIP` は、ヘルパー関数の奥深くからでもテストを止められます。呼び出し側で戻り値を確かめ直す必要はありません。

```c
static const char *require_header(const char *data, size_t size)
{
    NAOS_REQUIRE_TRUE(size >= 4);
    NAOS_REQUIRE_MEM_EQ("NAOS", 4, data, 4);

    return data + 4;
}

NAOS_TEST(reads_header)
{
    const char *body = require_header(input, input_size); // 失敗したら、ここから先は実行されない

    NAOS_CHECK_INT_EQ('v', body[0]);
}
```

止まったときは、ヘルパーとテストの残りの処理がまとめて飛ばされ、teardown に移ります。そのため、次の 2 点に注意してください。

- 確保したものの解放などの後片付けは、テストやヘルパーの末尾ではなく teardown に書いてください。
- C++ では、テストが止まる可能性のある関数（呼び出し先で止まる場合も含む）に、デストラクタを持つローカル変数を置かないでください（[C++](cpp.md)）。

どのアサーションにも当てはまらない場合は、`NAOS_FAIL(message)` で自分の言葉で失敗させてテストを止めます。テストを止めた後にどこへ抜けるか、どこで使えないかは[テストの止まり方](../reference/control.md#テストの止まり方)にあります。
