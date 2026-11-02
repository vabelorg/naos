# JUnit XML

`--naos-output xml:<path>` を指定すると、Naos でビルドした実行ファイルは、実行の結果をファイル `<path>` にも JUnit XML で書きます。JUnit XML は CI サービスやテストレポートのツールが読む形式です。標準出力の人間向けの出力は変わりません。

このファイルはツール向けの出力です。JUnit XML を読む一般的なツールが処理するもの（テスト、その結果と件数、各失敗のメッセージと場所）は互換を保ちます（[バージョン](version.md)）。それ以外は人が読むためのものです（[互換性](#互換性)）。結果を読むツールは、標準出力ではなくこのファイルを読んでください（[出力](output.md)）。

```
prog --naos-output xml:results.xml
prog --naos-filter 'parser.*' --naos-output=xml:build/parser.xml
```

## ファイルを書くとき

- ファイルは最初のテストを実行する前に開き、すでにあれば置き換えます。開けないときは `naos: cannot open the file of --naos-output: <path>` と出力し、テストを 1 つも実行せずに 2 で終了します。
- 結果はテストの実行中はメモリに貯め、実行の終わりにファイル全体を書きます。`--naos-repeat` の最後の回の後、または `--naos-fail-fast` で止まった後です。書けないとき、またはそのためのメモリが確保できないときは、標準エラー出力にメッセージを出し、1 で終了します。
- クラッシュや `exit()` で実行の途中にプロセスが終わると、ファイルは空のまま残ります。そのときは、標準出力の `RUN:` の行が実行中だったテストを示します（[トラブルシューティング](../guides/troubleshooting.md)）。
- `--naos-list` と `--help` はテストを実行せず、ファイルも書きません。

パスは作業ディレクトリからの相対パスです。CTest では、実行ファイルのテストがそれぞれ別のプロセスで同じ引数で動くので、どれも同じファイルに書くことになります。`--naos-output` は実行ファイルを直接実行するときに指定するか、CTest 自身のレポートを `--output-junit` で集めてください（[CTest と CI](../guides/ctest-and-ci.md#ci-で実行する)）。

## 要素

```xml
<?xml version="1.0" encoding="UTF-8"?>
<testsuites name="<program>" tests="<n>" failures="<n>" errors="0" skipped="<n>" time="<s>">
  <testsuite name="<suite>" tests="<n>" failures="<n>" errors="0" skipped="<n>" time="<s>">
    <properties>
      <property name="iteration" value="<i>"/>
      <property name="random_seed" value="<seed>"/>
    </properties>
    <testcase classname="<suite>" name="<test>" time="<s>" file="<file>" line="<line>">
      <failure type="error" message="<text>">...</failure>
      <skipped message="<message>"/>
    </testcase>
  </testsuite>
</testsuites>
```

| 要素 | 書くとき | 属性 |
|---|---|---|
| `<testsuites>` | 1 回、全体を囲む | `name`: プログラムの名前。`argv[0]` からディレクトリと `.exe` を除いたもの。`tests`、`failures`、`skipped`: ファイル全体のテストケースの数。`errors`: 常に `0`。`time`: ファイル全体のテストケースの時間の合計 |
| `<testsuite>` | 各回の各スイートごと。実行した順 | `name`: スイート。`tests`、`failures`、`skipped`、`errors`、`time`: `<testsuites>` と同じで、そのテストケースについてのもの |
| `<properties>` | `--naos-repeat` が 2 以上か、`--naos-shuffle` のときの `<testsuite>` の中 | `<property name="iteration">`（1 から。`--naos-repeat` が 2 以上のとき）と `<property name="random_seed">`（`--naos-shuffle` のとき）を持つ |
| `<testcase>` | 実行したテストごと | `classname`: スイート。`name`: テスト。`time`: テストの時間。`file`、`line`: 最初の失敗かスキップが起きた場所。どちらもないとき、またはテストが[アサーションを実行しなかった](control.md#アサーションを実行しないテスト)ために失敗したとき（場所がない）は省く |
| `<failure>` | 失敗したテストの失敗ごと。起きた順 | `type`: 常に `error`。`message`: `error` の診断の行の内容（[出力](output.md#診断の行)）。アサーションの式、`NAOS_FAIL` のメッセージ、trace の書き間違い、`no assertion ran` のどれか |
| `<skipped>` | スキップしたテストの中 | `message`: `NAOS_SKIP` のメッセージ |

テストケースは、テストが失敗したときは `<failure>` 要素を、スキップしたときは `<skipped>` を 1 つ持ち、成功したときは何も持ちません。失敗した後にスキップしたテストは、失敗だけを持ちます。

`--naos-fail-fast` で実行しなかったテストには、テストケースがありません。Naos はテストが出力したものを取り込まないので、`timestamp` と `<system-out>` は書きません。

`time` は秒で、小数 3 桁、ミリ秒に切り捨てて書きます（`time="3.120"`）。JUnit XML のツールが読む形です。

テストケースの時間は、標準出力の結果の行に出る時間と同じで、`--help` が示す時計で測った setup・本体・teardown の時間です（[時間](output.md#時間)）。スイートとファイル全体の時間は、テストケースの正確な時間の合計を最後に 1 回だけ切り捨てたものなので、書かれた時間の和より少し大きいことがあります。時間は実行のたびに変わるので、ファイルも変わります。

`<failure>` の中身は、標準出力が書くとおりの診断です。`error` の行、値を示す詳細の行、メモ、trace です（[出力](output.md)）。CI サービスはこれをそのまま表示するので、失敗は CI でも端末と同じように読めます。標準出力と同じく、人が読むためのものです。

## テキスト

ファイルは UTF-8 です。テキストはすべて XML 用にエスケープします（`&amp;`、`&lt;`、`&gt;`。属性では `&quot;` と `&apos;` も使い、改行とタブを `&#10;` と `&#9;` にします。復帰は常に `&#13;` です）。

正しい UTF-8 はそのまま書くので、どの言語で書いたメッセージも書いたとおりに読めます。正しい UTF-8 の一部でないバイトと、XML 1.0 で使えない制御文字（改行、タブ、復帰以外の 0x20 未満）は `\xNN` と書きます。そのため、テキスト自身が `\xNN` を含むものとは区別できないことがあります。

文字列の値は標準出力と同じく書き（[値の表記](output.md#値の表記)）、印字できる ASCII だけになります。

## 互換性

互換を保つのは、JUnit XML を読む一般的なツールが処理するものです。

- 要素 `<testsuites>`、`<testsuite>`、`<testcase>`、`<failure>`、`<skipped>` と、その入れ子、どのテストケースと、どの `<failure>`・`<skipped>` を書くか（[要素](#要素)）。
- 上に書いたそれらの属性。`name`、`classname`、件数の `tests`・`failures`・`errors`・`skipped`、`time`、`file`、`line`、`type`、`message` と、それぞれが持つもの、そのテキストのエスケープ（[テキスト](#テキスト)）。
- ファイルを書くときと終了コード（[ファイルを書くとき](#ファイルを書くとき)）。

一般的なツールは表示するか読み飛ばすだけなので、次は互換を保ちません。標準出力に従う `<failure>` の中身、スイートの `<properties>`、ファイルのレイアウト（字下げ、要素の間の改行、属性の順）です。後のバージョンで、属性、要素、`xml` 以外の形式が増えることがあります。ツールは知らないものを読み飛ばしてください。

## 出力例

Naos の自己テストのうち 4 つのテスト（`tests/behavior/xml/report.c`）を、`--naos-output xml:results.xml --naos-filter 'results.passes:results.fails_twice:results.skips:results.with_context'` で実行したもの:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<testsuites name="naos_selftest_report" tests="4" failures="2" errors="0" skipped="1" time="0.001">
  <testsuite name="results" tests="4" failures="2" errors="0" skipped="1" time="0.001">
    <testcase classname="results" name="passes" time="0.000"/>
    <testcase classname="results" name="fails_twice" time="0.000" file="report.c" line="26">
      <failure type="error" message="1 == 2">report.c:26: error: 1 == 2
  left:  1
  right: 2
</failure>
      <failure type="error" message="&quot;hello\n&quot; == &quot;hello&quot;">report.c:27: error: "hello\n" == "hello"
  left:  "hello\n"
  right: "hello"
  first difference: offset 5, end of right
</failure>
    </testcase>
    <testcase classname="results" name="skips" time="0.000" file="report.c" line="32">
      <skipped message="not here"/>
    </testcase>
    <testcase classname="results" name="with_context" time="0.000" file="report.c" line="47">
      <failure type="error" message="0.1 ~= 0.25">report.c:47: error: 0.1 ~= 0.25
  left:  0.1
  right: 0.25
  tolerance: 0.001
  note: locale C
report.c:45: note: inner 2
report.c:44: note: outer
</failure>
      <failure type="error" message="&quot;ab&quot; == &quot;ac&quot;">report.c:48: error: "ab" == "ac"
  left size:  2
  right size: 2
  first difference: offset 1, left 0x62, right 0x63
report.c:45: note: inner 2
report.c:44: note: outer
</failure>
    </testcase>
  </testsuite>
</testsuites>
```

スイート全体は `tests/behavior/xml/expected/all.txt` に、あらゆる種類のエスケープは `text.txt` にあります。
