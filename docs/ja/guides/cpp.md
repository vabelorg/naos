# C++

Naos のヘッダは C++ からもそのまま読み込めるので、C++11 以降なら同じマクロでテストを書けます。完成した例は [examples/cpp](../../../examples/cpp/) にあります。

```cmake
naos_add_executable(cpp_tests SOURCES words_test.cpp)
set_target_properties(cpp_tests PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES)
```

## テストが止まりうる場所にデストラクタを置かない

テストを止めるマクロは C++ の例外ではなく `longjmp` で抜けるため、途中のデストラクタは呼ばれません。この形でデストラクタを飛ばすのは未定義動作です。

テストが止まる可能性のある関数（呼び出し先で止まる場合も含む）には、`std::string` や `std::vector` などデストラクタを持つローカル変数を置かないでください。次のどちらかにします。

- そうしたオブジェクトは関数の外に置き、setup で作って teardown で片付ける
- その関数では、テストを止めない `NAOS_CHECK*` だけを使う

```cpp
// 止まりうるテストは longjmp で抜け、デストラクタは呼ばれない。デストラクタを持つオブジェクトは
// テストの外に置き、setup で作って teardown で壊す。
static std::vector<std::string> *words;

NAOS_SETUP(setup)
{
    words = new std::vector<std::string>();
}

NAOS_TEARDOWN(teardown)
{
    delete words;
    words = nullptr;
}

NAOS_TEST(push_back_adds)
{
    words->push_back("naos");

    NAOS_REQUIRE_UINT_EQ(1, words->size()); // テストが止まりうる。デストラクタを持つローカル変数は置かない
    NAOS_CHECK_STR_EQ("naos", (*words)[0].c_str());
}

// NAOS_CHECK* はテストを止めないので、ここではローカルの std::string でよい。
NAOS_TEST(only_checks)
{
    std::string word = "harness";

    NAOS_CHECK_UINT_EQ(7, word.size());
    NAOS_CHECK_STR_EQ("harness", word.c_str());
}
```

- アサーションは C の型を受け取るので、文字列は `c_str()` で比べます。
- テストを止めるマクロは、コンストラクタとデストラクタの中では使えません。
- Naos は例外を捕まえません。テストから漏れた例外は Naos の C のコードを通って上がり、ふつうはプロセスが終わります。例外はテストの中で捕まえ、必要なら `NAOS_FAIL` に変えます。

規則は[テストの止まり方](../reference/control.md#テストの止まり方)にあります。
