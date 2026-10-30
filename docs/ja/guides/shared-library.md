# 共有ライブラリ

Naos は既定では静的ライブラリです。共有ライブラリ（DLL）としてビルドすることもできます。

## 共有ライブラリとしてビルドする

Naos を単独で（トップレベルのプロジェクトとして）`-DBUILD_SHARED_LIBS=ON` を付けて configure し、インストールします。

```sh
cmake -S naos -B naos/build -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build naos/build --config Release
cmake --install naos/build --config Release --prefix /opt/naos
```

ほかのプロジェクトに組み込んだとき（`add_subdirectory` や `FetchContent`）は、`BUILD_SHARED_LIBS` によらず常に静的ライブラリです。

- `find_package(naos)` で使えば、ヘッダが必要とする定義 `NAOS_SHARED` は CMake が付けます。CMake を使わない場合は、テストをコンパイルするときに `NAOS_SHARED` を定義してください。
- 公開関数は呼び出し規約を指定していません。Naos とテストは同じ既定の規約でビルドしてください（32 ビット Windows で `/Gz` などを使わない）。

## ビルド時に DLL が見つかるようにする

CMake でビルドする場合、テストの実行ファイルは CTest に登録するため、ビルドの直後に一度起動されます。そのため、ビルドの時点で共有ライブラリが見つかる必要があります。実行ファイルと同じディレクトリに置くか、そのディレクトリを `PATH`（Linux なら `LD_LIBRARY_PATH`、macOS なら `DYLD_LIBRARY_PATH`）に通しておきます。rpath でも構いません。

DLL をビルド後にコピーするなら、`add_executable()` の後、`naos_discover_tests()` より前に `POST_BUILD` のコマンドを足します。コマンドは足した順に実行され、`naos_add_executable()` では列挙が常に先になります。

```cmake
add_executable(my_tests main.c parser_test.c)
target_link_libraries(my_tests PRIVATE naos::naos)
if(WIN32)
    add_custom_command(TARGET my_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:my_tests> $<TARGET_FILE_DIR:my_tests>
        COMMAND_EXPAND_LISTS
    )
endif()
naos_discover_tests(my_tests)
```

DLL が見つからないと、ビルドは列挙のところで失敗し、対処をメッセージに出します。Windows では、実行ファイルは何も出力せず `0xc0000135` で終了します。

## 読み込んだバージョンを確かめる

テストをビルドしたときのものではなく、古い DLL を読み込んでいることがあります。これを見つけるには、ヘッダのバージョンと、実際に読み込んだライブラリのバージョンを、[独自の `main()`](custom-main.md) の `naos_run()` の前で比べます。

```c
if (naos_version_number() != NAOS_VERSION_NUMBER)
{
    fprintf(stderr, "naos %s is loaded, but the tests were built with %s\n",
        naos_version_string(), NAOS_VERSION_STRING);
    return 1;
}
```

バージョンのマクロと関数は[バージョン](../reference/version.md)にあります。
