# Naos への参加

Naos に関心を持っていただきありがとうございます。バグの報告、文書の修正、翻訳、コード、どれも歓迎します。参加する人は誰でも[行動規範](CODE_OF_CONDUCT.md)に従います。

## 始める前に

- **バグ**: バージョン、プラットフォームとコンパイラ、問題を再現する最小のテストとその出力を添えて、[Issue](https://github.com/vabelorg/naos/issues/new/choose) を作ってください。
- **機能**: まず Issue を作り、それが解く問題を書いてください。Naos は意図して責務を絞っており、機能より[原則](development/architecture.md#原則)を優先します。たとえば、プロセスの分離、並列実行、レポートの形式は CTest や CI に任せています。
- **セキュリティの問題**: Issue は作らず、[セキュリティポリシー](SECURITY.md)に従って非公開で報告してください。
- **小さな修正**（誤字、切れたリンク、分かりやすい言い回し）は、そのままプルリクエストにしてください。

## 準備

C99 コンパイラ（GCC、Clang、MSVC）、CMake 3.21 以上、Git が必要です。C++ のテストには C++ コンパイラも使います。

```sh
git clone https://github.com/vabelorg/naos.git
cd naos
cmake --preset default
cmake --build --preset default
ctest --preset default
```

何かを変える前に、すべての自己テストが通ることを確かめてください。詳細、ほかのコンパイラ、共有ビルドは[ビルド](development/building.md)にあります。

## 変更する

1. `main` からブランチを作る。
2. 変更の種類に応じたページを読む。
   - コード: [アーキテクチャ](development/architecture.md)、[拡張](development/extending.md)、[コードの書き方](development/coding-style.md)
   - テスト: [テスト](development/testing.md)
   - 文書: [文書の書き方](development/docs-style.md)
3. 自己テストを足すか直す。動作の変更には、それがないと失敗するテストを付ける。
4. 同じ変更で文書を直す。API、出力、CMake の関数を変えたら、リファレンスを直す。英語のページは同じ変更で直し、日本語訳（`docs/ja/`）は後のプルリクエストで追いついてもよい（[文書の書き方](development/docs-style.md#言語)）。
5. 利用者に見える変更なら、`CHANGELOG.md` の `[Unreleased]` に項目を足す（[変更履歴](development/changelog.md)）。
6. `default` と `shared` の両方のプリセットで自己テストを実行する。

## コミットとプルリクエスト

- コミットメッセージは英語で、命令形の短い要約（"Add NAOS_CHECK_MEM_NE"）、空行、必要なら理由の順に書いてください。
- 1 つのプルリクエストには 1 つの話題だけを入れてください。テンプレートに、何を変えるか、なぜか、どうテストしたかを書いてください。
- CI がすべてのプラットフォームで通る必要があります。メンテナがレビューし、マージの前に変更をお願いすることがあります。

## ライセンス

Naos は [MIT License](../../LICENSE) です。参加することで、あなたの貢献がこのライセンスで配布されることに同意したものとします。新しいソースファイルは、既存のものと同じく、著作権表示と `SPDX-License-Identifier: MIT` で始めてください。
