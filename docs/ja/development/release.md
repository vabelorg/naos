# リリース

Naos のバージョンをリリースする手順です。リリースは `main` に `v<major>.<minor>.<patch>` のタグを付けて行います。作業は 2 つのワークフローが行い、人はバージョンを決めて、ワークフローが作るプルリクエストを確かめます。

## バージョンを決める

- **パッチ**（0.0.0 → 0.0.1）: 公開するものを何も変えない修正。
- **マイナー**（0.0.x → 0.1.0）: それ以外。1.0 より前は、マイナーバージョンで互換が切れることがある。
- **メジャー**: 1.0 以後、互換のない変更のとき。

どれになるかは、[変更履歴](changelog.md#リリースのとき)に書いたとおり、`CHANGELOG.md` の `[Unreleased]` の項目で決まります。項目に合わないバージョンでは、リリースは止まります。

## 手順

1. `main` で CI が通っていること、`CHANGELOG.md` の `[Unreleased]` に利用者から見える変更がすべて書かれていることを確かめる。
2. Actions タブで、バージョンを入れて **Release PR** を実行する。先に **dry_run** にチェックを入れて実行すると、何も作らずに、変更とリリースノートを実行のサマリで見られる。
3. ワークフローがプルリクエスト `Release <version>` を作る。中身は次のとおり。
   - `include/naos/version.h` にバージョンを書く。バージョンを書くのはここだけで、残りは CMake がそこから導く。
   - 新しいマイナーかメジャーのバージョンなら、README と文書とサンプルで、`FetchContent` が使うタグ（`GIT_TAG v…`）と、`find_package(naos 0.0)` に書いたマイナーバージョンを更新する。
   - 変更履歴を閉じる。`[Unreleased]` をバージョンと日付に変え、`CHANGELOG.md` の末尾のリンクを更新する。
4. プルリクエストで CI が走る。両方のプリセットでの自己テストと、インストールしたパッケージを `find_package(naos <version>)` で使うプロジェクト（`tests/package/`）のテストです。変更を確かめてマージする。
5. マージすると **Release** が走る。マージコミットに `v<version>` のタグを付け、`CHANGELOG.md` のこのバージョンの節をリリースノートにして GitHub のリリースを作り、ブランチを消す。

1.0 のときは、README と文書にある互換性の説明（同じマイナーバージョンから、同じメジャーバージョンへ）を手で直す必要があると、ワークフローが警告します。マージの前に、直したものをプルリクエストに push してください。

## 準備

**Release PR** には、リポジトリのシークレット `RELEASE_TOKEN` が要ります。ワークフローの `GITHUB_TOKEN` で作ったプルリクエストでは CI が走らないためです。このリポジトリに対して *Contents* と *Pull requests* の読み書きの権限を持つ fine-grained personal access token を作り、*Settings → Secrets and variables → Actions* に登録します。

## 手作業で行う

ワークフローが実行する 2 つの CMake スクリプトは、手元でも Naos のルートで実行できます。

```sh
cmake -D ROOT=. -D VERSION=0.1.0 -P scripts/prepare-release.cmake
cmake -D ROOT=. -D VERSION=0.1.0 -D OUTPUT=notes.md -P scripts/release-notes.cmake
```

1 つ目は手順 3 のとおりにファイルを書き換え、2 つ目はリリースノートを書き出します。その後、`Release <version>` としてコミットし、タグを付けて push し、そのノートでタグから GitHub のリリースを作ります。

```sh
git tag -a v0.1.0 -m "Naos 0.1.0"
git push origin main v0.1.0
gh release create v0.1.0 --verify-tag --title "Naos 0.1.0" --notes-file notes.md
```
