# テスト

Naos の自己テストについて、何を確かめるか、出力をどう照合するか、どう足すかを説明します。実行の仕方は[ビルド](building.md)にあります。

## 観点

`tests/` は観点ごとのディレクトリに分けます。1 つのディレクトリに、その観点のプログラム、ケースを登録する `CMakeLists.txt`、期待する出力（`expected/<ケース>.txt`）がそろいます。CTest の名前は `naos.<観点>.<ケース>` です。

| ディレクトリ | 確かめること | 形 |
|---|---|---|
| `api/` | 公開 API を利用者と同じ形で使う（C と C++）。公開ヘッダごとに 1 ファイル | テスト自身が判定する（Naos のテスト 1 件が CTest 1 件） |
| `unit/` | 結果が引数（`order.c` は種）だけで決まる内部の関数（`value.c`、`filter.c`、`order.c`）。モジュールごとに 1 ファイル | 同上 |
| `cli/` | オプション、フィルタ、`--naos-list`、`--help`、使い方の誤り | 出力を照合する |
| `session/` | `naos_init` → `NAOS_ADD_SUITE` → `naos_run` の順番、スイートの誤り | 同上 |
| `assertions/` | アサーションの種類ごとの失敗の出力。種類ごとに 1 ファイル | 同上 |
| `control/` | 続行と停止（CHECK / REQUIRE / FAIL / SKIP）、結果、終了コード、途中で終わるプロセス | 同上 |
| `fixtures/` | setup と teardown | 同上 |
| `context/` | メモと trace | 同上 |
| `runs/` | `--naos-shuffle`、`--naos-repeat`、`--naos-fail-fast` | 同上 |
| `discovery/` | テストの列挙に失敗したときの `NaosDiscover.cmake` のメッセージ | 同上 |
| `no_cmake/` | CMake を使わないビルド（[インストール](../getting-started/installation.md#cmake-を使わない場合)のとおり） | ビルドして実行する |
| `examples/` | `examples/` のプロジェクトと、文書がそこから引用する出力 | テストが通ること、出力の照合 |
| `docs/` | すべての Markdown ファイルの相対リンクとアンカー | `check-links.cmake` |
| `misuse/` | コンパイルできてはいけないプログラム（`NAOS_SET_SETUP` に teardown を、`NAOS_ADD_TEST` に setup を指定するなど） | コンパイラが拒否し、誤りの名前を示すこと |

仕組みは `tests/support/` にあります（`NaosSelftest.cmake` の関数と、ケースを 1 つ実行する `check-output.cmake`）。

`tests/package/` は自己テストではありません。インストールしたパッケージを `find_package(naos <version>)` で使うプロジェクトで、CI がビルドごとにインストールした後、単独で configure します（[ビルド](building.md#インストールする)）。

## 出力を照合するケース

ふつうは**期待する出力の全体**（ゴールデンファイル）と照合します。`expected/<ケース>.txt` には、引数、終了コード、標準出力、標準エラー出力をそのまま書きます。

```
args: --naos-filter integer.eq
exit code: 1
--- stdout
RUN: integer.eq
integer.c:LINE: error: -1 == 2
  left:  -1
  right: 2
FAIL: integer.eq (1 failure)
--- stderr
```

どの環境でも同じ内容になるよう、照合の前に次だけを置き換えます。

- 診断の場所: `<ディレクトリ>/<ファイル>:<行>: `（MSVC では `(<行>)`）を `<ファイル>:LINE: ` に。テストを編集しても期待する出力が変わらないようにするため
- 実行ファイルのパス: `PROG` に。使い方の行は、パスの長さで折り返し位置が変わるので 1 行につなげる
- CR を除く

期待する出力に出ていない行は、出てはいけない行です（正規表現の `REJECT` を並べる必要はありません）。テストが通ってはいけない場所の後に `reached-...` を出力しておくと、それが期待する出力にないことで確かめられます。

出力が実行や環境で変わるもの（ポインタの値、時刻から選ぶ種、`long` の幅で変わる数、テストを足すと伸びる出力）だけは、**正規表現**で照合します（`naos_expect_match`）。

## ケースを足す

1. その観点のプログラムにテストを書く（失敗するのが目的のテストも、ふつうのテストと同じに書く）。テスト関数の名前はファイルの中で重ならないようにする必要があるので、スイートごとにファイルを分けてよい
2. `CMakeLists.txt` に登録する。テスト 1 件の出力を見るなら `naos_expect_tests(<target> <suite>.<test>...)`、引数を指定するなら `naos_expect(<ケース> <target> ARGS ...)`
3. `NAOS_UPDATE_EXPECTED=1 ctest --preset default -R <ケース>` で期待する出力を書き出し、**中身を読んで正しいことを確かめてから**コミットする

出力の形式を変えたときも同じです。`NAOS_UPDATE_EXPECTED=1 ctest --preset default` で書き直し、`git diff` で変わった行がすべて意図どおりかを確かめます。照合に失敗したケースは、最初に違う行（空白がわかるよう `[...]` で囲む）と、実際の出力の全体を表示します。

新しい観点は、ディレクトリを作り、`tests/CMakeLists.txt` の一覧に足します。

## サンプルとドキュメント

`examples/` のプロジェクトは、利用者がそのまま写せる完成したプロジェクトです。単独でビルドすると `FetchContent` で Naos を取得し、`tests/examples/` から追加されたときは、テスト中の Naos を使います。そのテストは、実行ファイルの名前で始まる CTest のテスト（`clamp_tests.clamp.inside_range`）として、自己テストと同じ厳しい警告の下で実行されます。

文書が引用する出力は、`tests/examples/` のゴールデンファイルのケースです（`naos.examples.quickstart` など）。サンプルやその出力が変わったら、それを引用している文書を直します（日本語訳は[文書の書き方](docs-style.md#言語)のとおり後から追いついてもかまいません）。

`naos.docs.links` は、Markdown ファイルのすべての相対リンクが存在するファイルを指し、すべての `#アンカー` がそのファイルの見出しを指していることを確かめます。ほかの自己テストと一緒に実行されます。
