# Naos のビルド

Naos をソースからビルドし、自己テストを実行する方法です。プロジェクトで Naos を使う場合は[インストール](../getting-started/installation.md)を見てください。

## 必要なもの

- C99 コンパイラ: GCC、Clang、MSVC
- CMake 3.21 以上
- C++ コンパイラ（C++ の自己テストとサンプルに使う。なければそれらを除く）
- Git
- Python 3.8 以上（lint とリリースのスクリプトに使う。[lint](testing.md#lint)、[リリース](release.md)）

## プリセットを使う

`CMakePresets.json` には 4 つのプリセットがあり、それぞれ `build/<preset>/` にビルドします。

| プリセット | ビルドするもの |
|---|---|
| `default` | 静的ライブラリ、Debug、自己テスト付き |
| `shared` | 共有ライブラリ（DLL）、Debug、自己テスト付き |
| `release` | 静的ライブラリ、Release（最適化あり）、自己テスト付き |
| `sanitize` | 静的ライブラリ、Debug、自己テスト付き。AddressSanitizer と UndefinedBehaviorSanitizer でビルドして実行する。GCC と Clang のみ |

```sh
cmake --preset default
cmake --build --preset default
ctest --preset default
```

`shared` で同じことをすると、共有ビルドを確かめられます。ライブラリや CMake のファイルに触れる変更は、送る前に両方で実行してください。`release` は最適化でしか見つからないものを示します。`test.c` は `longjmp` でテストを止めるので、最適化したビルドでも成り立たなければならず、GCC の `-Wclobbered` のように最適化が要る警告もあります。`sanitize` はライブラリとテストのメモリの誤りと未定義動作を見つけます。CI は 4 つすべてを実行します。

Windows の MinGW-w64 では、CI と同じく、MSYS2 のシェル（UCRT64 または MINGW64）で、その `gcc`・`cmake`・`ninja` のパッケージを入れ、`CMAKE_GENERATOR=Ninja` にして実行します。

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

自己テストの CTest 名はすべて `naos.` で始まり、ラベル `naos` と層のラベルが付いています。[サンプルのテスト](testing.md#サンプルとドキュメント)は、実行ファイルの名前で始まります（[名前とラベル](testing.md#名前とラベル)）。

```sh
ctest --preset default -L contract                             # 1 つの層
ctest --preset default -R '^naos\.behavior\.command-line\.'    # 1 つの領域
ctest --preset default -LE slow                                # プロジェクトをビルドするケース以外すべて
ctest --preset default -j8                                     # 並列
```

文書のリンクのように、ビルドを必要としない検査は自己テストではありません（[lint](testing.md#lint)）。

自己テストの仕組みと足し方は[テスト](testing.md)にあります。

## インストールする

```sh
cmake --install build/default --prefix /tmp/naos-install
```

インストールされる配置は [CMake のリファレンス](../reference/cmake.md#パッケージ)にあります。プロジェクトからは `find_package(Naos)` と `-DCMAKE_PREFIX_PATH=/tmp/naos-install` で使えます。`tests/consumer/` がそのようなプロジェクトです。`cmake -D PRESET=<preset> -P scripts/TestPackage.cmake` が、ビルドしたプリセットをインストールしてこのプロジェクトをテストします。CI も `sanitize` 以外のビルドごとにこれを実行します。`pkg-config` が見つかれば（MSVC を除く）、このスクリプトは続いて、インストールされた `naos.pc` のフラグを確かめ、そのフラグで `tests/consumer/consumer_test.c` を CMake を使わずにビルドして実行します。

## 継続的インテグレーション

`.github/workflows/ci.yml` は、すべての push とプルリクエストを確かめます。どのステップも、手元でも同じように実行できるコマンドです。

| ジョブ | 実行するもの |
|---|---|
| `lint` | `python3 scripts/lint.py`（[lint](testing.md#lint)） |
| `test` | プリセットで configure、ビルド、`ctest`。続いて `sanitize` 以外では `cmake -D PRESET=<preset> -P scripts/TestPackage.cmake`（[インストールする](#インストールする)）。Linux の GCC で `default`、`shared`、`release`、`sanitize`、および `default` を対応するいちばん古い CMake 3.21 で。Linux の Clang で `default` と `release`。macOS の Clang で `default` と `shared`。MSVC で `default`、`shared`、`release` |
| `mingw` | 同じことを MSYS2 の MinGW-w64 の GCC で。UCRT64 で `default` と `shared`、MINGW64 で `default`。MINGW64 は、printf が C99 に従わない Windows の古い C ライブラリ msvcrt のためにある（[コーディングスタイル](coding-style.md#書式付きの出力)） |
