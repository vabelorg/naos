# C++

The headers of Naos can be included from C++ as they are: with C++11 or later, tests are written with the same macros. The complete example is [examples/cpp](../../examples/cpp/).

```cmake
naos_add_executable(cpp_tests SOURCES words_test.cpp)
set_target_properties(cpp_tests PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES)
```

## No destructor where a test may stop

The macros that stop a test leave by `longjmp`, not by a C++ exception, so no destructor runs on the way. Skipping a destructor this way is undefined behavior.

In a function where the test may stop (directly or in a function it calls), put no local variable with a destructor, such as `std::string` or `std::vector`. Either:

- keep such objects outside the function, made in setup and destroyed in teardown, or
- use only `NAOS_CHECK*` in that function, which never stops the test.

```cpp
// A test that may stop leaves by longjmp, which runs no destructor: objects with one live outside
// it, made in setup and destroyed in teardown.
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

    NAOS_REQUIRE_UINT_EQ(1, words->size()); // may stop the test: no local object with a destructor
    NAOS_CHECK_STR_EQ("naos", (*words)[0].c_str());
}

// NAOS_CHECK* never stops the test, so a local std::string is fine here.
NAOS_TEST(only_checks)
{
    std::string word = "harness";

    NAOS_CHECK_UINT_EQ(7, word.size());
    NAOS_CHECK_STR_EQ("harness", word.c_str());
}
```

- Strings are compared through `c_str()`: the assertions take C types.
- The stopping macros cannot be used in constructors and destructors.
- Naos does not catch exceptions. One that escapes a test goes up through the C code of Naos, which usually ends the process. Catch exceptions in the test, and turn them into a `NAOS_FAIL` if needed.

The rules are in [how a test stops](../reference/control.md#how-a-test-stops).
