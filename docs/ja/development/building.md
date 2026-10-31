# Naos のビルド

Naos をソースからビルドし、自己テストを実行する方法です。プロジェクトで Naos を使う場合は[インストール](../getting-started/installation.md)を見てください。

## 必要なもの

- C99 コンパイラ: GCC、Clang、MSVC
- CMake 3.21 以上
- C++ コンパイラ（C++ の自己テストとサンプルに使う。なければそれらを除く）
- Git

## プリセットを使う

`CMakePresets.json` には 2 つのプリセットがあり、それぞれ `build/<preset>/` にビルドします。

| プリセット | ビルドするもの |
|---|---|
| `default` | 静的ライブラリ、Debug、自己テスト付き |
| `shared` | 共有ライブラリ（DLL）、Debug、自己テスト付き |

```sh
cmake --preset default
cmake --build --preset default
ctest --preset default
```

`shared` で同じことをすると、共有ビルドを確かめられます。ライブラリや CMake のファイルに触れる変更は、送る前に両方で実行してください。

## 手で実行する

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

マルチコンフィグのジェネレータ（Visual Studio、Ninja Multi-Config）では、ビルドに `--config Debug` を、`ctest` に `-C Debug` を付けます。ほかのコンパイラは、いつもどおり最初の configure で `CC` と `CXX`、または `-DCMAKE_C_COMPILER=...` で選びます。

トップレベルのプロジェクトのとき、Naos は次のようにします。

- 自己テストをビルドし（`NAOS_SELFTEST`）、すべての警告をエラーにする。GCC / Clang は `-Wall -Wextra -Wpedantic -Werror`、MSVC は `/W4 /WX`
- インストール規則を作る（`NAOS_INSTALL`）
- 実行ファイルと DLL をすべて `build/bin/` に置く。共有ライブラリの Naos を、自己テストがすぐ隣で見つけられるようにするため

オプションは [CMake のリファレンス](../reference/cmake.md#オプション)にあります。

## 一部の自己テストを実行する

自己テストの CTest 名はすべて `naos.` で始まり、ラベル `naos` が付いています。[サンプルのテスト](testing.md#サンプルとドキュメント)は、実行ファイルの名前で始まります。

```sh
ctest --preset default -R '^naos\.cli\.'          # 1 つの観点
ctest --preset default -R '^naos\.docs\.links$'   # 文書のリンク
ctest --preset default -j8                        # 並列
```

自己テストの仕組みと足し方は[テスト](testing.md)にあります。

## インストールする

```sh
cmake --install build/default --prefix /tmp/naos-install
```

インストールされる配置は [CMake のリファレンス](../reference/cmake.md#パッケージ)にあります。プロジェクトからは `find_package(naos)` と `-DCMAKE_PREFIX_PATH=/tmp/naos-install` で使えます。`tests/package/` がそのようなプロジェクトで、CI はインストールしたパッケージごとにこれをビルドしてテストします。

## CMake を使わない場合

ライブラリは、コンパイラだけでもビルドできなければなりません。自己テスト `naos.no_cmake.build` が、GCC 互換のコンパイラで、[インストール](../getting-started/installation.md#cmake-を使わない場合)に書いたとおりにビルドして確かめます。

## 継続的インテグレーション

`.github/workflows/ci.yml` は、すべての push とプルリクエストを、プリセットを使って Linux（GCC と Clang）、macOS（Clang）、Windows（MSVC）で、それぞれ静的ライブラリと共有ライブラリとしてビルドしてテストします。すべてのジョブが通った変更だけをマージします。
