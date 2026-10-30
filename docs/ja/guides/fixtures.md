# setup と teardown

スイートの各テストで同じ前処理・後処理を行う場合は、`NAOS_SET_SETUP` と `NAOS_SET_TEARDOWN` で setup と teardown を指定します。setup と teardown は引数を受け取らないため、テストと共有する状態は、次の例の `buf` のようにファイルの `static` 変数に置きます。

```c
static struct buffer buf;

NAOS_SETUP(setup)
{
    NAOS_REQUIRE_TRUE(buffer_init(&buf) == 0);
}

NAOS_TEARDOWN(teardown)
{
    buffer_free(&buf);
}

NAOS_TEST(append_grows) { ... }

NAOS_SUITE(buffer,
    NAOS_SET_SETUP(setup),
    NAOS_SET_TEARDOWN(teardown),
    NAOS_ADD_TEST(append_grows)
)
```

- setup が途中で止まると、そのテストの本体は実行されません。
- teardown は、setup やテストが途中で止まっても必ず呼び出されます。setup が途中で止まった状態でも片付けられるように書きましょう（この例なら、`buffer_free` を、初期化されていない状態や解放済みの状態に対して呼んでも安全にしておく）。
- setup と teardown での失敗は、そのテストの失敗として数えます。

テストは 1 つのプロセスで順に実行され、`static` 変数の値は次のテストに引き継がれます。テストごとに状態を初期状態に戻したい場合も、setup か teardown で戻します。

[チュートリアル](../getting-started/tutorial.md#setup-と-teardown)でこの形のスイートを作っています。規則は[前後処理](../reference/tests.md#前後処理)にあります。条件がそろわないときにスイートのテストをまとめてスキップするには、setup の中で `NAOS_SKIP` を呼びます（[テストをスキップする](writing-tests.md#条件がそろわないテストをスキップする)）。
