# リリース

Naos のバージョンをリリースする手順です。リリースは `main` に `v<major>.<minor>.<patch>` のタグを付けて行います。作業は 2 つのワークフローが行い、人はバージョンを決めて、ワークフローが作るプルリクエストを確かめます。

## バージョンを決める

- **パッチ**（0.0.0 → 0.0.1）: 公開するものを何も変えない修正。
- **マイナー**（0.0.x → 0.1.0）: それ以外。1.0 より前は、マイナーバージョンで互換が切れることがある。
- **メジャー**: 1.0 以後、互換のない変更のとき。

許されるバージョンは、[変更履歴](changelog.md#リリースのとき)に書いたとおり、`CHANGELOG.md` の `[Unreleased]` の項目で決まります。項目に合わないバージョンでは、リリースは止まります。

## 手順

1. `main` で CI が通っていること、`CHANGELOG.md` の `[Unreleased]` に利用者から見える変更がすべて書かれていることを確かめる。
2. Actions タブで、バージョンを入れて **Release PR** を実行する。最初は **dry_run** にチェックを入れて実行する。何も作らずに、変更とリリースノートが実行のサマリに出るので、確認する。
3. 問題がなければ、同じバージョンで **dry_run** のチェックを外して、もう一度 **Release PR** を実行すると、ワークフローがプルリクエスト `Release <version>` を作る。中身は次のとおり。
   - `include/naos/version.h` にバージョンを書く。バージョンを書くのはここだけで、残りは CMake がそこから導く。
   - 新しいマイナーかメジャーのバージョンなら、README と文書とサンプルで、`FetchContent` が使うタグ（`GIT_TAG v…`）と、`find_package(Naos 0.0)` に書いたマイナーバージョンを更新する。
   - 変更履歴を閉じる。`[Unreleased]` をバージョンと日付に変え、`CHANGELOG.md` の末尾のリンクを更新する。
4. プルリクエストで CI が走る。リリースのスクリプトもテストする lint、すべてのプリセットとプラットフォームでの自己テスト、インストールしたパッケージを `find_package(Naos <version>)` で使うプロジェクト（`tests/consumer/`。`scripts/TestPackage.cmake` で）のテストです。変更を確かめてマージする。
5. マージすると **Release** が走る。マージコミットに `v<version>` のタグを付け、`CHANGELOG.md` のこのバージョンの節をリリースノートにして GitHub のリリースを作り、ブランチを消す。

1.0 のときは、README と文書にある互換性の説明（同じマイナーバージョンから、同じメジャーバージョンへ）を手で直す必要があると、ワークフローが警告します。マージの前に、直したものをプルリクエストに push してください。

## 準備

**Release PR** には、リポジトリのシークレット `RELEASE_TOKEN` が要ります。ワークフローの `GITHUB_TOKEN` で作ったプルリクエストでは CI が走らないためです。最初のリリースの前に、次の手順で登録します。

1. GitHub のアカウントの *Settings → Developer settings → Personal access tokens → Fine-grained tokens* で、*Generate new token* を押す。
2. *Repository access* で *Only select repositories* を選び、このリポジトリを選ぶ。
3. *Repository permissions* の *Contents* と *Pull requests* を、*Read and write* にする。
4. トークンを作り、表示された値をコピーする。
5. このリポジトリの *Settings → Secrets and variables → Actions* で *New repository secret* を押す。名前を `RELEASE_TOKEN`、値をコピーしたトークンにして登録する。

## 手作業で行う

ワークフローが実行する 2 つの Python スクリプトは、手元でも Naos のルートで実行できます。

```sh
python3 scripts/prepare_release.py 0.1.0
python3 scripts/release_notes.py 0.1.0 notes.md
```

1 つ目は手順 3 のとおりにファイルを書き換え、2 つ目はリリースノートを書き出します。その後、`Release <version>` としてコミットし、タグを付けて push し、そのノートでタグから GitHub のリリースを作ります。

```sh
git tag -a v0.1.0 -m "Naos 0.1.0"
git push origin main v0.1.0
gh release create v0.1.0 --verify-tag --title "Naos 0.1.0" --notes-file notes.md
```
