# スタブ

テスト対象の関数 A が呼ぶ関数 B は、テスト用に書いた偽物（スタブ）に差し替えられます。スタブの B を書いて、本物の B の代わりにリンクします。こうすると、B が返す値をテストから決めたり、A が B に渡した値を確かめたりできます。完成した例は [examples/stubs](../../../examples/stubs/) にあります。

## テスト対象

```c
/* module_a.c（テスト対象） */
#include "module_a.h"

#include "module_b.h"

int func_a(int x)
{
    if (func_b(x) != 0) // 本物の func_b は module_b.c にある
    {
        return A_ERROR;
    }

    return A_OK;
}
```

## スタブ

スタブは、呼ばれた様子を、テストから読み書きできる変数に記録します。

```c
/* fake_module_b.h */
extern int fake_b_return;   /* func_b が返す値 */
extern int fake_b_calls;    /* 呼ばれた回数 */
extern int fake_b_last_arg; /* 最後の呼び出しの引数 */
```

```c
/* fake_module_b.c */
#include "fake_module_b.h"

#include "module_b.h"

int fake_b_return;
int fake_b_calls;
int fake_b_last_arg;

int func_b(int x)
{
    fake_b_calls++;
    fake_b_last_arg = x;

    return fake_b_return;
}
```

## テスト

```c
/* module_a_test.c */
#include <naos/naos.h>

#include "fake_module_b.h"
#include "module_a.h"

/* スタブは前のテストの値を持ち越す。 */
NAOS_SETUP(setup)
{
    fake_b_return = 0;
    fake_b_calls = 0;
    fake_b_last_arg = 0;
}

NAOS_TEST(passes_x_to_b)
{
    NAOS_CHECK_INT_EQ(A_OK, func_a(3));
    NAOS_CHECK_INT_EQ(1, fake_b_calls);
    NAOS_CHECK_INT_EQ(3, fake_b_last_arg);
}

NAOS_TEST(propagates_b_failure)
{
    fake_b_return = -1;

    NAOS_CHECK_INT_EQ(A_ERROR, func_a(3));
}

NAOS_SUITE(module_a,
    NAOS_SET_SETUP(setup),
    NAOS_ADD_TEST(passes_x_to_b),
    NAOS_ADD_TEST(propagates_b_failure)
)

NAOS_MAIN(
    NAOS_ADD_SUITE(module_a)
)
```

```cmake
# 本物の module_b.c の代わりに fake_module_b.c をリンクする
naos_add_executable(module_a_tests SOURCES module_a_test.c module_a.c fake_module_b.c)
```

## 制約

- スタブの変数は前のテストの値を引き継ぐため、setup でリセットします。
- 本物の B を含むライブラリをリンクすると、スタブとシンボルが衝突することがあります。テスト対象のソースを直接並べます。
- 差し替えられるのは、A と別のファイルで定義された B だけです。同じファイルの中の関数や `static` 関数は差し替えられません。
