# Naos

[English](README.md) · 日本語

[![CI](https://github.com/vabelorg/naos/actions/workflows/ci.yml/badge.svg)](https://github.com/vabelorg/naos/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
![C99](https://img.shields.io/badge/C-99-blue.svg)

C と C++ 向けのテストハーネスです。標準 C ライブラリ以外に依存せず、コンパイラ拡張も使いません。素の C99 で、CMake でも、コンパイラだけでもビルドできます。

```c
#include <naos/naos.h>

#include "clamp.h"

NAOS_TEST(below_range)
{
    NAOS_CHECK_INT_EQ(0, clamp(-3, 0, 10));
}

NAOS_TEST(above_range)
{
    NAOS_CHECK_INT_EQ(10, clamp(42, 0, 10));
}

NAOS_SUITE(clamp,
    NAOS_ADD_TEST(below_range),
    NAOS_ADD_TEST(above_range)
)

NAOS_MAIN(
    NAOS_ADD_SUITE(clamp)
)
```

`clamp()` が上限を確かめ忘れていると、次のように出力されます。

```
$ ./clamp_tests
RUN: clamp.below_range
PASS: clamp.below_range
RUN: clamp.above_range
clamp_test.c:12: error: 10 == clamp(42, 0, 10)
  left:  10
  right: 42
FAIL: clamp.above_range (1 failure)
SUMMARY: 1 passed, 1 failed, 0 skipped
```

## 特徴

- **1 つの実行ファイルで全テストを実行**: テストごとの行と集計を出力し、結果を終了コードでも返します。
- **1 テストだけ実行**: `--naos-filter clamp.above_range` で、デバッガに掛けるのも簡単です。ワイルドカードと除外でまとめて選べます。
- **1 テストずつ別プロセスで実行**: `naos_add_executable()` がテストを 1 件ずつ CTest に登録し、クラッシュの分離、並列実行、時間制限は CTest が担います。
- **隠れた依存をあぶり出す**: 種付きで再現できる `--naos-shuffle`、`--naos-repeat`、`--naos-fail-fast`。
- **エディタが読める失敗**: `file:line: error: ...` の形で、文書化された安定した形式です。
- **導入しやすい**: `FetchContent`、`add_subdirectory`、`find_package`、または `src/` のソースだけ。

## 必要なもの

- C99 のコンパイラ（GCC、Clang、MSVC）。C++ でテストを書く場合は C++11。
- CMake 3.21 以降（なくても使えます）。

すべての変更を Linux（GCC、Clang）、macOS（Clang）、Windows（MSVC）でビルドしてテストしています。

## インストール

CMake では `FetchContent` がいちばん簡単です。

```cmake
include(FetchContent)
FetchContent_Declare(naos
    GIT_REPOSITORY https://github.com/vabelorg/naos.git
    GIT_TAG v0.1.0
)
FetchContent_MakeAvailable(naos)
```

`add_subdirectory()` や `find_package(naos)` でも取り込めます。CMake を使わない場合は、`include/` をインクルードパスに、`src/` の `.c` ファイルをビルドに加えます。4 通りすべてを[インストール](docs/ja/getting-started/installation.md)にまとめています。

## クイックスタート

```cmake
enable_testing()
naos_add_executable(clamp_tests SOURCES clamp_test.c LIBRARIES clamp)
```

```sh
cmake -S . -B build && cmake --build build && ctest --test-dir build
```

[クイックスタート](docs/ja/getting-started/quickstart.md)で、5 分で一通り試せます。

## 文書

文書は英語が正で、[日本語訳](docs/ja/README.md)があります。

| | |
|---|---|
| **はじめる** | [インストール](docs/ja/getting-started/installation.md) · [クイックスタート](docs/ja/getting-started/quickstart.md) · [チュートリアル](docs/ja/getting-started/tutorial.md) |
| **使う** | [ガイド](docs/ja/guides/) · [トラブルシューティング](docs/ja/guides/troubleshooting.md) · [サンプル](examples/) |
| **調べる** | [リファレンス](docs/ja/reference/): テスト、アサーション、コマンドライン、出力、CMake |

全体の案内は [docs/ja/](docs/ja/README.md) にあります。

## 状態

Naos は 1.0 より前です。マイナーバージョンが上がると互換が切れることがあり、`find_package(naos 0.1)` は 0.1.x だけを受け付けます。バージョンごとの変更は[変更履歴](CHANGELOG.md)にあります（英語）。

## 参加する

不具合の報告、提案、プルリクエストを歓迎します。まず [CONTRIBUTING](docs/ja/CONTRIBUTING.md) を読んでください。コードについては[ビルド](docs/ja/development/building.md)、[アーキテクチャ](docs/ja/development/architecture.md)、[テスト](docs/ja/development/testing.md)にあります。参加する人はみな[行動規範](docs/ja/CODE_OF_CONDUCT.md)に従います。

## セキュリティ

脆弱性は公開の issue ではなく、[SECURITY](docs/ja/SECURITY.md) の手順で非公開に報告してください。

## ライセンス

[MIT](LICENSE)
