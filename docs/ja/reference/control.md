# 失敗・スキップ・メモ

## テストの止まり方

`NAOS_REQUIRE*` の失敗、`NAOS_FAIL`、`NAOS_SKIP` は、`longjmp` でテストを止めます。ヘルパーの奥深くの関数からでも、値を返す関数の中からでも、呼び出し側に何も書かずに止められます。setup と本体の中からは teardown へ、teardown の中からは teardown の終わりへ抜けます。`NAOS_REQUIRE*` の失敗と `NAOS_FAIL` は**致命的な失敗**で、`NAOS_CHECK*` の失敗と `NAOS_SKIP` はそうではありません。

間にある関数をまとめて抜けるので、次の制約があります。

- 抜けた関数の残りは実行されません。`free` などの後始末は teardown に置き、状態はファイルの `static` 変数に置きます。
- C++ では、抜ける途中の関数にデストラクタを持つ自動変数を置けません（`longjmp` でデストラクタを飛ばすのは未定義動作です）。コンストラクタとデストラクタの中でもこれらのマクロを使えません。
- 実行中のテストの外（`naos_run()` の前やスイートの関数の中）では使えません。メッセージを出して終了コード 2 で終了します。
- テストを実行しているスレッド以外からは抜けられません。

## 失敗

```c
NAOS_FAIL(message)
```

致命的な失敗を記録してテストを止めます。戻りません。`message` は `const char *` か `NULL` で、`NULL` なら `(no message)` と出力します。

```c
static int find_index(const int *values, int count, int value)
{
    int i;

    for (i = 0; i < count; i++)
    {
        if (values[i] == value)
        {
            return i;
        }
    }

    NAOS_FAIL("not found");   /* 戻らないので、この後に return は要らない */
}
```

```
search_test.c:14: error: not found
```

GCC、Clang、MSVC では、`NAOS_FAIL` で終わる関数に `return` がなくても警告は出ません。それ以外のコンパイラでは警告が出ることがありますが、動作は変わりません。

## スキップ

```c
NAOS_SKIP(message)
```

テストをスキップとして記録し、止めます。失敗は記録しません。戻りません。`message` は `const char *` か `NULL` で、`NULL` なら `(no message)` と出力します。

- setup で呼ぶと、テストの本体は実行しません。teardown は実行します。
- 失敗が優先します。スキップしたテストでも、失敗を記録していれば結果は `FAIL` です。
- 実行したテストがすべてスキップなら、終了コード 77 で終了します。CTest はこれをスキップとして数えます。`--naos-fail-fast` はスキップでは止まりません。

例は[テストを書く](../guides/writing-tests.md#条件がそろわないテストをスキップする)にあります。

## メモ

```c
NAOS_NOTE(format, ...)
```

`printf` と同じ書式でメモを作ります。次のアサーションが失敗すると、その値の後にメモを出力します。次のアサーションは、成功しても失敗してもメモを使い切ります。例は[診断](../guides/diagnostics.md#ループの何回目で失敗したかを表示する)にあります。

- `NAOS_FAIL` と `NAOS_SKIP` もメモを出力し、使い切ります。
- 使われる前に次のメモを作ると、前のメモは置き換わります。各テストと teardown はメモがない状態で始まります。
- 書式だけの `NAOS_NOTE("text")` も書けます。
- 511 バイトを超えるメモは切り詰め、末尾を `...` にします。
- GCC と Clang では、書式と引数の食い違いを警告します。

## trace

```c
NAOS_TRACE_PUSH(format, ...)
NAOS_TRACE_POP()
```

`NAOS_TRACE_PUSH` は `printf` と同じ書式のメモを積み、対応する `NAOS_TRACE_POP` で下ろすまで、すべての失敗とスキップにそれを添えます。[メモ](#メモ)と違い、ヘルパーの中も含めて、いくつのアサーションにも添えられます。

```c
for (i = 0; i < count; i++)
{
    NAOS_TRACE_PUSH("item %d", i);
    check_item(&items[i]);      /* 中のどの失敗にも trace が添えられる */
    NAOS_TRACE_POP();
}
```

```
item_test.c:12: error: item->size > 0
  left:  0
  right: 0
item_test.c:40: note: item 3
```

各 trace は、積んだ場所を指す `note` の行として、詳細の行の後に内側から順に出力します。

- `PUSH` と `POP` は対にします。setup と本体は 1 つのスタックを共有し（setup で積んで本体で下ろしてもよい）、teardown は自分の空のスタックで始まります。
- 致命的な失敗やスキップで抜けたときは、飛ばされた `POP` を補う必要はありません。
- 255 バイトを超える trace は切り詰め、末尾を `...` にします。

次の書き間違いは、そのテストの失敗として記録します（致命的な失敗ではありません）。

- `error: NAOS_TRACE_PUSH without NAOS_TRACE_POP: <最も内側の trace>`: setup と本体、または teardown が、trace を積んだまま終わった。最も内側の trace を積んだ場所で出力し、それより外側の trace を `note` の行で続けます。
- `error: NAOS_TRACE_POP without NAOS_TRACE_PUSH`: 積んでいないのに下ろした。
- `error: NAOS_TRACE_PUSH beyond 16 levels`: 16 段を超えて積んだ。超えた分も数えるので、対応する `POP` はそのまま書けますが、内容は保持しません。超えた分が積まれている間の失敗は、詳細の行の最後に `traces: <n> innermost not kept` を出力します。

`NAOS_TRACE_POP without NAOS_TRACE_PUSH` と `NAOS_TRACE_PUSH beyond 16 levels` は 1 行だけを出力し、[メモ](#メモ)は使わずに次のアサーションに残します。
