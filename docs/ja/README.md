# Naos の文書

これは英語の文書の日本語訳です。英語版が正で、内容が食い違う場合は英語版に従います。

## はじめて使う

1. [インストール](getting-started/installation.md): 必要なものと、プロジェクトへの取り込み方
2. [クイックスタート](getting-started/quickstart.md): 最初のテストを CTest で実行するまでを 5 分で
3. [チュートリアル](getting-started/tutorial.md): 複数のファイル、setup と teardown、`CHECK` と `REQUIRE`、メモ、フィルタ

## 使う

- [ガイド](guides/): やりたいこと別。テストの書き方と実行から、CTest、CI、スタブ、C++ まで
- [リファレンス](reference/): マクロ、コマンドライン、出力、CMake のすべての規則
- [サンプル](../../examples/): 自己テストでビルドして実行している、完成したプロジェクト

## 開発に参加する

まず [CONTRIBUTING.md](CONTRIBUTING.md) を読み、次に以下を読みます。

- [ビルド](development/building.md): Naos のビルドと自己テストの実行
- [アーキテクチャ](development/architecture.md): モジュールとその依存
- [テスト](development/testing.md): 自己テストと、その足し方
- [拡張](development/extending.md): 変更の種類ごとの置き場所
- [コードの書き方](development/coding-style.md) · [文書の書き方](development/docs-style.md)
- [変更履歴](development/changelog.md) · [リリース](development/release.md)
