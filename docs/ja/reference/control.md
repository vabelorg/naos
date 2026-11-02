# 失敗・スキップ・メモ

テストを止めるもの、テストを失敗やスキップにするもの、失敗に添えるメモと trace です。

## テストの止まり方

`NAOS_REQUIRE*` の失敗、`NAOS_FAIL`、`NAOS_SKIP` は、`longjmp` でテストを止めます。ヘルパーの奥深くの関数からでも、値を返す関数の中からでも、呼び出し側に何も書かずに止められます。

setup と本体の中からは teardown へ、teardown の中からは teardown の終わりへ抜けます。

`NAOS_REQUIRE*` の失敗と `NAOS_FAIL` は**致命的な失敗**で、`NAOS_CHECK*` の失敗と `NAOS_SKIP` はそうではありません。

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

## 失敗するはずのテスト

```c
NAOS_ADD_TEST_XFAIL(test, reason)
```

`NAOS_SUITE` の項目に `NAOS_ADD_TEST` の代わりに書くと、今は失敗するはずのテストを、その理由 `reason`（空でない文字列）とともに登録します。コードより先に書いたテスト、まだ直していない既知のバグのテスト、1 つのプラットフォームだけの既知の問題などに使います。Naos はこの印を確かめます。テストが失敗しているあいだは成功とし、成功したとたんに失敗とするので、印を外すことが独立した 1 つの手順になります。

| テストが… | 結果の行 | 数え方 |
|---|---|---|
| 1 つ以上の失敗を記録した（アサーション、`NAOS_FAIL`。setup、本体、teardown のどこでも） | `XFAIL: <suite>.<test> (<t> ms)` | 成功 |
| 失敗もスキップもなく終わった | `XPASS: <suite>.<test> (<t> ms)`。その前に `NAOS_ADD_TEST_XFAIL` の項目の場所で失敗を出す | 失敗 |
| 失敗せずにスキップした | `SKIP: <suite>.<test> (<t> ms)` | スキップ |

```
RUN: parser.parses_negative_numbers
parser_test.c:42: error: passed, but is expected to fail
  reason: "sign handling not implemented yet"
XPASS: parser.parses_negative_numbers (0 ms)
```

- `XFAIL` の失敗は、ほかのテストと同じように出力します。テストが思ったとおりの理由で失敗していて、書き間違いで失敗しているのではないことを確かめられます。
- `XPASS` の失敗の場所は、スイートの `NAOS_ADD_TEST_XFAIL` の項目、つまりテストが通るようになったら書き換える行です。値 `reason` は登録した理由です。ほかの失敗と同じ扱いで、終了コードは 1 になり、`--naos-fail-fast` はここで止まり、`--naos-quiet` でも出力します。
- `XFAIL` は失敗ではありません。失敗したテストがなければ、ほかのテストがすべてスキップでも終了コードは 0 で、`--naos-fail-fast` は止まらず、`--naos-quiet` はその失敗ごと省きます。`SUMMARY:` は期待どおりの失敗として数え（[出力](output.md#流れの行)）、[JUnit XML](xml.md) のファイルはスキップとして数えます。
- アサーションを 1 つも実行しなかったテストは失敗していないので、`--naos-allow-no-assertions` の有無にかかわらず `XPASS` です。
- クラッシュは、期待どおりの失敗にはなりません。1 つのプロセスで実行していれば、ほかのテストと同じように実行が終わります。CTest の下では、そのテストは失敗します。
- `--naos-list` はこれらのテストに印を付けません。名前をそのままに保つためです。見分けるには JUnit XML のファイルを読みます。
- 理由が空か `NULL` なら、スイートのほかの誤りと同じく、テストを実行する前に報告し、終了コード 2 で終了します（[テストとスイート](tests.md#スイート)）。
- 1 つのプラットフォームだけで失敗するなら、`#ifdef` は項目の間に書けないので、スイートの外で `#ifdef` を使って項目を選びます。

```c
#if defined(_WIN32)
#define PARSES_PATHS NAOS_ADD_TEST_XFAIL(parses_paths, "drive letters not handled yet")
#else
#define PARSES_PATHS NAOS_ADD_TEST(parses_paths)
#endif

NAOS_SUITE(parser,
    PARSES_PATHS
)
```

例は[テストを書く](../guides/writing-tests.md#今は失敗するはずのテストに印を付ける)にあります。

## アサーションを実行しないテスト

アサーションを 1 つも実行しないテストは失敗します。何も確かめていないテストが成功に見えないようにするためです。たとえば、本体が空や書きかけのテスト、ケースのない表を回すループ、アサーションの前の `return`、アサーションを持つヘルパーを呼んでいないテストです。

```
RUN: parser.parses_negative_numbers
naos: error: no assertion ran
  hint: check something, or call NAOS_PASS() when the check is that the code runs
FAIL: parser.parses_negative_numbers (1 failure, 0 ms)
```

- 数えるもの: 実行した `NAOS_CHECK*` と `NAOS_REQUIRE*` のすべて（成功しても失敗しても）、`NAOS_PASS`、`NAOS_FAIL`、`NAOS_SKIP`。テストの実行中に数えるので、ヘルパーの中のアサーションも数え、書いてあっても到達しなかったものは数えません。
- 数えないもの: `NAOS_NOTE`、`NAOS_TRACE_PUSH`、`NAOS_TRACE_POP`、テストが出力したもの。
- setup、本体、teardown をまとめて数えます。setup や teardown がテストごとに何かを確かめるスイートでは、どのテストも報告しません。
- 報告するのは、成功するはずだったテストだけです。失敗したテストやスキップしたテストは、何が起きたかをすでに示しているので、そのままにします。
- テストには場所がないので、この失敗にも場所がありません。診断の行は `naos: ` で始まり（[出力](output.md#診断の行)）、[JUnit XML](xml.md) のファイルではそのテストケースに `file` と `line` がありません。
- ほかの失敗と同じ扱いです。終了コードは 1 になり、`--naos-fail-fast` はここで止まり、`--naos-quiet` でも出力します。

`--naos-allow-no-assertions` を付けると、Naos 0.4 より前と同じように、このテストを成功にします（[コマンドライン](command-line.md#オプション)）。[CMake の関数](cmake.md#関数)の `ALLOW_NO_ASSERTIONS` は、登録するすべてのテストにこれを付けます。

### 必ず通るアサーション

```c
NAOS_PASS()
```

必ず成功するアサーションです。コードが動くこと自体を確かめるテストに使います。たとえば、呼び出しがクラッシュせずに戻ることだけを確かめるテストです。アサーションとして数えるほかは何もせず、テストは続きます。

```c
NAOS_TEST(close_twice_is_harmless)
{
    file_t *file = file_open("data.bin");

    file_close(file);
    file_close(file);   /* the check: this returns */
    NAOS_PASS();
}
```

成功したほかのアサーションと同じく、[メモ](#メモ)を使い切ります。

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
