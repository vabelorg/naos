# テストとスイート

テストは関数で、スイートにまとめます。`main()` がスイートを登録して実行します。自動で登録されるものはありません。テストの名前は `<suite>.<test>` で、`--naos-filter` で使い、`RUN:` と結果の行に出力します。

```c
#include <naos/naos.h>

NAOS_TEST(addition)
{
    NAOS_CHECK_INT_EQ(4, 2 + 2);
}

NAOS_SUITE(math,
    NAOS_ADD_TEST(addition)
)

NAOS_MAIN(
    NAOS_ADD_SUITE(math)
)
```

テストは 1 つのプロセスの中で、スイートを登録した順に、スイートの中ではテストを並べた順に実行します。テストごとに初期化されるのは失敗の記録だけで、`static` 変数の値は前のテストから引き継がれます。

マクロが定義する関数は `naos_gen_<役割>_<名前>` という名前になる（[生成される関数名](#生成される関数名)）ので、テストやスイートの名前が Naos の API と衝突することはありません。

## テスト

```c
NAOS_TEST(name)
{
    /* アサーション */
}
```

`NAOS_TEST(name)` はテスト関数を定義します。`static void naos_gen_test_<name>(void)` を定義します。デバッガに表示されるのはこの名前です（[生成される関数名](#生成される関数名)）。`NAOS_ADD_TEST` でスイートに並べたときだけ実行されます。

- 閉じ括弧の後ろに `;` を付けません。`NAOS_TEST` は関数を定義するので、余分な `;` は `-Wpedantic` が警告します。
- 定義したのにスイートに並べなかったテストは検出しません。コンパイラの未使用関数の警告で気付けます。

## スイート

```c
NAOS_SUITE(name, entry, ...)
NAOS_ADD_TEST(test)
NAOS_DECLARE_SUITE(name);
```

`NAOS_SUITE(name, ...)` は、カンマで区切った項目からスイート `name` を定義します。項目は、テストごとの `NAOS_ADD_TEST(test)`（並べた順に実行します）と、必要なら[前後処理](#前後処理)です。並べるテストより後ろ、ふつうはファイルの末尾に書きます。

`NAOS_DECLARE_SUITE(name);` は、別のファイルで定義したスイートを宣言し、そのファイルで登録できるようにします。

```c
/* parser_test.c */
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

NAOS_MAIN(
    NAOS_ADD_SUITE(parser)
)
```

- 最後の項目の後ろにカンマを付けません（コンパイルできません）。`NAOS_SUITE(...)` の後ろにも `;` を付けません。`NAOS_DECLARE_SUITE` は宣言なので `;` が要ります。
- スイートは 1 つのファイルの中で定義します。1 つのファイルに複数あっても構いません。テスト関数は `static` なので、ファイルが違えば同じ名前を使えます。
- 同じ名前のスイートが 1 つの実行ファイルに 2 つあると、リンクエラーになります。
- 引数の途中に `#ifdef` は書けません（C99 では未定義動作）。プラットフォームごとに分けたいテストは別のスイートにして、スイート全体を `#ifdef` で囲みます。
- 名前が空か英字・数字・`_` 以外の文字を含む（マクロで書いた名前は常に正しい）、1 つのスイートに同じテストを 2 回並べた、同じスイートを 2 回登録した、のいずれかがあると、テストを実行する前にすべて標準エラー出力に報告し、終了コード 2 で終了します。

## 前後処理

```c
NAOS_SET_SETUP(name)
NAOS_SET_TEARDOWN(name)
```

`NAOS_SUITE` の項目に並べると、`NAOS_SET_SETUP(name)` はスイートの各テストの前に `NAOS_SETUP(name)` で定義した関数を、`NAOS_SET_TEARDOWN(name)` は各テストの後に `NAOS_TEARDOWN(name)` で定義した関数を呼びます。どちらも省略でき、どこに書いてもスイート全体に効きます（先頭に書くことを勧めます）。それぞれスイートに 1 回だけ書けます。2 回書くと、テストを実行する前に報告し、終了コード 2 で終了します。

- `NAOS_SETUP(name)` と `NAOS_TEARDOWN(name)` は、それぞれ `naos_gen_setup_<name>` と `naos_gen_teardown_<name>` を定義します。`NAOS_SET_SETUP` に teardown を、`NAOS_SET_TEARDOWN` に setup を指定すると、コンパイルエラーになります（[生成される関数名](#生成される関数名)）。
- setup で致命的な失敗かスキップが起きたら、テストの本体は実行しません。
- teardown は、setup や本体が途中で止まっても必ず実行します。途中までしか準備できていない状態でも片付けられるように書きます。
- setup と teardown での失敗は、そのテストの失敗として数えます。
- スイートごとに 1 回だけ呼ぶ前後処理はありません。

setup、テスト、teardown で共有する状態は、ファイルの `static` 変数に置きます。例は [setup と teardown のガイド](../guides/fixtures.md)にあります。

## 生成される関数名

```
NAOS_TEST(x)         →  static void naos_gen_test_x(void)
NAOS_SETUP(x)        →  static void naos_gen_setup_x(void)
NAOS_TEARDOWN(x)     →  static void naos_gen_teardown_x(void)
NAOS_SUITE(x, ...)   →  void naos_gen_suite_x(naos_builder_t *builder)
```

マクロが定義する関数は `naos_gen_<役割>_<名前>` という名前になります。コードには例のとおり素の名前 `x` でマクロを書き、`naos_gen_...` と書くことはありません。

- 役割が名前に含まれるので、`NAOS_SET_SETUP`、`NAOS_SET_TEARDOWN`、`NAOS_ADD_TEST` は、それぞれの役割で定義した関数だけを受け付けます。`NAOS_SET_SETUP` に teardown を、`NAOS_ADD_TEST` に setup を指定すると、コンパイルエラー（「`naos_gen_setup_x` が未宣言」）になります。テストと setup と teardown に同じ名前 `x` を使っても衝突しません。
- テスト、setup、teardown は `static` なので、ファイルが違えば同じ名前を使えます。スイートの関数は外部リンケージで、`NAOS_DECLARE_SUITE` が宣言するのがこれです。
- デバッガ、スタックトレース、プロファイラにはこの名前が表示されます。テストにブレークポイントを置くなら、`naos_gen_test_<name>` に置きます。
- `naos_gen_` で始まる名前は Naos が使います。その形の関数を自分で定義したり、生成された関数を呼んだりしないでください。テストは、それを並べたスイートが実行します。

## main()

```c
NAOS_MAIN(entry, ...)
```

`NAOS_MAIN` は `main()` を定義します。Naos のオプションを解析し、カンマで区切った `NAOS_ADD_SUITE(name)` の項目で、書いた順にスイートを登録して実行し、終了コードを返します。1 つの実行ファイルに 1 回だけ使い、後ろに `;` を付けません。

## 独自の main()

```c
void naos_init(int *argc, char **argv);
NAOS_ADD_SUITE(name);
int  naos_run(void);
```

独自の引数を解析したいときや、実行時にスイートを選びたいときは、`NAOS_MAIN` の代わりにこれらを呼びます。順番は `naos_init()` を 1 回、`NAOS_ADD_SUITE` を何回でも、`naos_run()` を 1 回です。

- `naos_init()` は、Naos のオプション（`--naos-*`）を `argv` のどこにあっても取り除き、残りの引数を順番どおりに詰めて `*argc` を減らします。`--help` は、プログラムが自分のヘルプも出せるように `argv` に残します。ヘルプと使い方エラーはここで出力します。その後の `naos_run()` はテストを実行しません。スイートは集めて誤りを報告し、使い方エラーかスイートの誤りがあれば 2 を、`--help` だけなら 0 を返します。
- `NAOS_ADD_SUITE(name)` は、それまでに登録したスイートの後に実行するスイートを登録します。
- `naos_run()` はスイートを集め、その中の誤りをすべて報告してから、選ばれたテストを実行し、終了コードを返します（[終了コード](command-line.md#終了コード)）。スイートを集めるメモリが確保できなかったときは、1 行報告し、何も実行せずに 1 を返します。

例は[独自の main() のガイド](../guides/custom-main.md)にあります。

- 順番を外れた呼び出しはメッセージを出します。`naos_run()` は 2 を返し、`naos_init()` と `NAOS_ADD_SUITE` は終了コード 2 で終了します。テストやスイートの関数の中で `NAOS_ADD_SUITE` を呼ぶのも、順番を外れた呼び出しです。
- CMake は、引数なしで実行した `--naos-list` の結果から CTest にテストを登録します。特定の引数のときだけ登録するスイートは CTest には登録されないので、実行ファイルを直接実行して使います。
