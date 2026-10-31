# インストール

Naos をプロジェクトに取り込む方法です。CMake を使う 3 通りの方法と、CMake を使わずにソースをコンパイルする方法があります。

## 必要なもの

| もの | バージョン |
|---|---|
| C コンパイラ | C99。GCC、Clang、MSVC |
| C++ コンパイラ | C++11。C++ でテストを書くときだけ |
| CMake | 3.21 以上。なくてもよい（[CMake を使わない場合](#cmake-を使わない場合)） |

すべての変更は、Linux（GCC と Clang）、macOS（Clang）、Windows（MSVC）で、静的ライブラリと共有ライブラリの両方でビルドしてテストしています。

## CMake を使う場合

次のどの方法でも、ターゲット `naos::naos` と、関数 `naos_add_executable()`・`naos_discover_tests()` が使えるようになります（[CMake のリファレンス](../reference/cmake.md)）。あとはテストの実行ファイルを作り、CTest から実行します。

```cmake
enable_testing()   # または include(CTest)

naos_add_executable(my_tests SOURCES my_tests.c LIBRARIES my_lib)
```

トップレベルの `CMakeLists.txt` に `enable_testing()`（または `include(CTest)`）がないと、CTest はテストを見つけられません。

### FetchContent

configure の時点で Naos をダウンロードします。いちばん簡単な方法で、[クイックスタート](quickstart.md)もこれを使います。

```cmake
include(FetchContent)
FetchContent_Declare(naos
    GIT_REPOSITORY https://github.com/vabelorg/naos.git
    GIT_TAG v0.1.0
)
FetchContent_MakeAvailable(naos)
```

`GIT_TAG` はリリースのタグに固定してください。

### サブディレクトリ

Naos の複製（Git のサブモジュールや、ソースのコピー）をプロジェクトの中に置く場合です。

```cmake
add_subdirectory(third_party/naos)
```

ほかのプロジェクトに組み込んだ場合、指定しない限り、Naos は静的ライブラリとしてビルドされ、自身のテストを登録せず、インストール規則も作りません（[オプション](../reference/cmake.md#オプション)）。

### インストール済みのパッケージ

Naos を一度ビルドしてインストールします。

```sh
cmake -S naos -B naos/build -DCMAKE_BUILD_TYPE=Release
cmake --build naos/build --config Release
cmake --install naos/build --config Release --prefix /opt/naos
```

インストール先を `CMAKE_PREFIX_PATH` に入れて（`-DCMAKE_PREFIX_PATH=/opt/naos`）、見つけます。

```cmake
find_package(naos 0.1 REQUIRED)
```

どのバージョンを受け付けるかは [CMake のリファレンス](../reference/cmake.md#パッケージ)にあります。

## CMake を使わない場合

`include/` をインクルードパスに加え、`src/` の `.c` ファイルをビルドに加えます。

CTest を使わずにテストを 1 つずつ別のプロセスで実行する方法は、[CTest と CI](../guides/ctest-and-ci.md#cmake-を使わない場合) にあります。

## ライセンス

Naos は [MIT License](../../../LICENSE) で配布します。ソースを取り込む場合も、ビルドしたライブラリを配布する場合も、`LICENSE` の著作権表示と許諾表示を含めてください。`cmake --install` は `LICENSE` を `<prefix>/share/licenses/naos/` に置きます。
