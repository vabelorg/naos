# 独自の main()

重いテストを指定した場合だけ実行したいなど、テストの実行ファイルに独自の引数を持たせたい場合は、`NAOS_MAIN` の代わりに `main()` を自分で書きます。`naos_init()` が Naos のオプションを取り除くので、`argv` に残るのはプログラム自身の引数です。完成した例は [examples/custom-main](../../../examples/custom-main/) にあります。

```c
#include <naos/naos.h>

#include <stdio.h>
#include <string.h>

NAOS_DECLARE_SUITE(quick);
NAOS_DECLARE_SUITE(slow);

int main(int argc, char **argv)
{
    int i;

    /* --naos-* のオプションを取り除く。残るのはプログラムの引数だけ。 */
    naos_init(&argc, argv);

    /* 読み込んだ共有ライブラリが、テストをビルドしたヘッダより古いことがある。 */
    if (naos_version_number() != NAOS_VERSION_NUMBER)
    {
        fprintf(stderr, "naos %s is loaded, but the tests were built with %s\n",
            naos_version_string(), NAOS_VERSION_STRING);
        return 1;
    }

    NAOS_ADD_SUITE(quick);

    for (i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--slow") == 0)
        {
            NAOS_ADD_SUITE(slow);
        }
    }

    return naos_run();
}
```

```
$ ./custom_main_tests --slow
RUN: quick.adds
PASS: quick.adds
RUN: slow.sums_a_million
PASS: slow.sums_a_million
SUMMARY: 2 passed, 0 failed, 0 skipped
```

Naos のオプションも、引数のどこに置いてもそのまま使えます。

```sh
./custom_main_tests --slow --naos-filter 'slow.*'
```

- 順番は、`naos_init()` を 1 回、`NAOS_ADD_SUITE` を何回でも、`naos_run()` を 1 回です。`naos_run()` の戻り値をそのまま返します。
- `--help` は `argv` に残るので、Naos のヘルプの後にプログラム自身のヘルプを出せます。
- この例では、スイートを足す前に、読み込んだライブラリがテストをビルドしたときと同じバージョンかを確かめています（[共有ライブラリ](shared-library.md#読み込んだバージョンを確かめる)）。
- CTest に登録されるのは、引数なしで実行した場合に列挙されるテストだけです。引数で足すスイートは、実行ファイルを直接実行して使います。

規則は[独自の main()](../reference/tests.md#独自の-main) にあります。
