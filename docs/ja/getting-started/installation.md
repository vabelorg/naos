# インストール

Naos をプロジェクトに取り込む方法です。Naos は CMake でビルドします。

- CMake のプロジェクトは、[FetchContent](#fetchcontent)、[サブディレクトリ](#サブディレクトリ)、[インストール済みのパッケージ](#インストール済みのパッケージ)の 3 通りで取り込めます。
- ほかのビルドシステムのプロジェクトは、CMake でビルドしてインストールしたライブラリをリンクします（[ほかのビルドシステム](#ほかのビルドシステム)）。

## 必要なもの

| もの | バージョン |
|---|---|
| C コンパイラ | C99。GCC、Clang、MSVC |
| C++ コンパイラ | C++11。C++ でテストを書くときだけ |
| CMake | 3.21 以上。Naos のビルドに使う。[ほかのビルドシステム](#ほかのビルドシステム)のプロジェクトでも必要 |

すべての変更は、次の環境で、静的ライブラリと共有ライブラリの両方をビルドしてテストしています。

- Linux: GCC と Clang
- macOS: Clang
- Windows: MSVC と、MSYS2 の UCRT64・MINGW64 環境の MinGW-w64 の GCC

Linux から Windows 向けにクロスコンパイルするなど、MinGW-w64 でのほかのビルドも動くはずですが、テストはしていません。

## CMake を使う場合

次のどの方法でも、ターゲット `Naos::naos` と、関数 `naos_add_executable()`・`naos_discover_tests()` が使えるようになります（[CMake のリファレンス](../reference/cmake.md)）。

あとはテストの実行ファイルを作り、CTest から実行します。

```cmake
enable_testing()   # または include(CTest)

naos_add_executable(my_tests SOURCES my_tests.c LIBRARIES my_lib)
```

トップレベルの `CMakeLists.txt` に `enable_testing()`（または `include(CTest)`）がないと、CTest はテストを見つけられません。

### FetchContent

configure の時点で Naos をダウンロードします。いちばん簡単な方法で、[クイックスタート](quickstart.md)もこれを使います。

```cmake
include(FetchContent)
FetchContent_Declare(Naos
    GIT_REPOSITORY https://github.com/vabelorg/naos.git
    GIT_TAG v0.3.0
)
FetchContent_MakeAvailable(Naos)
```

`GIT_TAG` はリリースのタグに固定してください。

### サブディレクトリ

Naos の複製（Git のサブモジュールや、ソースのコピー）をプロジェクトの中に置く場合です。

```cmake
add_subdirectory(third_party/naos)
```

ほかのプロジェクトに組み込んだ場合、指定しない限り、Naos は静的ライブラリとしてビルドされ、自身のテストを登録せず、インストール規則も作りません（[オプション](../reference/cmake.md#オプション)）。

### インストール済みのパッケージ

1. Naos を一度ビルドしてインストールする。

   ```sh
   cmake -S naos -B naos/build -DCMAKE_BUILD_TYPE=Release
   cmake --build naos/build --config Release
   cmake --install naos/build --config Release --prefix /opt/naos
   ```

2. インストール先を `CMAKE_PREFIX_PATH` に入れて（`-DCMAKE_PREFIX_PATH=/opt/naos`）、見つける。

   ```cmake
   find_package(Naos 0.3 REQUIRED)
   ```

どのバージョンを受け付けるかは [CMake のリファレンス](../reference/cmake.md#パッケージ)にあります。

## ほかのビルドシステム

[インストール済みのパッケージ](#インストール済みのパッケージ)と同じように、CMake で Naos を一度ビルドしてインストールします。そのうえで、フラグを `pkg-config` で得ます。`cmake --install` は `<libdir>/pkgconfig/` に `naos.pc` を入れます。そのフラグはビルドしたライブラリに合わせてあり、Debug ビルドでは `-lnaosd`、共有ライブラリでは `-DNAOS_SHARED` が付きます。

```sh
export PKG_CONFIG_PATH=/opt/naos/lib/pkgconfig
cc -std=c99 $(pkg-config --cflags naos) -c my_tests.c
cc -o my_tests my_tests.o $(pkg-config --libs naos)
```

Meson は、同じ `PKG_CONFIG_PATH` で `dependency('naos')` から見つけます。Unix の静的ライブラリには C の数学ライブラリが要ることがあり、`pkg-config --static --libs naos` がそれを加えます（`-lm`）。`naos.pc` のパスはそれ自身の場所からの相対なので、インストールしたツリーは移動できます。

`pkg-config` がなければ、インストール先のヘッダとライブラリをコンパイラに渡します。

```sh
cc -std=c99 -I/opt/naos/include -c my_tests.c
cc -o my_tests my_tests.o -L/opt/naos/lib -lnaos
```

- **Debug ビルド**: ライブラリの名前の末尾に `d` が付きます（`-lnaosd`）。MSVC では、ライブラリは `naos.lib`（`naosd.lib`）です。
- **共有ライブラリ**: テストをコンパイルするときに `NAOS_SHARED` を定義し（`-DNAOS_SHARED`）、テストの実行時にライブラリが見つかるようにします（[共有ライブラリ](../guides/shared-library.md)）。
- **ディレクトリ**: `GNUInstallDirs` に従うので、ライブラリが `lib/` ではなく `lib64/` に入ることがあります。インストールされるファイルは [CMake のリファレンス](../reference/cmake.md#パッケージ)にあります。

Naos のビルドを通さずに、`src/` のソースを自分のプロジェクトでビルドする方法は、サポートしていません。

CTest を使わずにテストを 1 つずつ別のプロセスで実行する方法は、[CTest と CI](../guides/ctest-and-ci.md#ctest-を使わない場合) にあります。

## ライセンス

Naos は [MIT License](../../../LICENSE) で配布します。ソースを取り込む場合も、ビルドしたライブラリを配布する場合も、`LICENSE` の著作権表示と許諾表示を含めてください。`cmake --install` は `LICENSE` を `<prefix>/share/licenses/naos/` に置きます。
