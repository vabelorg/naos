# JUnit XML

`--naos-output xml:<path>` を指定すると、Naos でビルドした実行ファイルは、実行の結果をファイル `<path>` にも JUnit XML で書きます。JUnit XML は CI サービスやテストレポートのツールが読む形式です。標準出力の人間向けの出力は変わりません。

このファイルはツール向けの出力で、形式は互換を保ちます（[バージョン](version.md)）。結果を読むツールは、標準出力ではなくこのファイルを読んでください（[出力](output.md)）。

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
<testsuites name="<program>" tests="<n>" failures="<n>" errors="0" skipped="<n>">
  <testsuite name="<suite>" tests="<n>" failures="<n>" errors="0" skipped="<n>">
    <properties>
      <property name="iteration" value="<i>"/>
      <property name="random_seed" value="<seed>"/>
    </properties>
    <testcase classname="<suite>" name="<test>" file="<file>" line="<line>">
      <failure type="error" message="<text>">...</failure>
      <skipped message="<message>"/>
    </testcase>
  </testsuite>
</testsuites>
```

| 要素 | 書くとき | 属性 |
|---|---|---|
| `<testsuites>` | 1 回、全体を囲む | `name`: プログラムの名前。`argv[0]` からディレクトリと `.exe` を除いたもの。`tests`、`failures`、`skipped`: ファイル全体のテストケースの数。`errors`: 常に `0` |
| `<testsuite>` | 各回の各スイートごと。実行した順 | `name`: スイート。`tests`、`failures`、`skipped`、`errors`: `<testsuites>` と同じで、そのテストケースの数 |
| `<properties>` | `--naos-repeat` が 2 以上か、`--naos-shuffle` のときの `<testsuite>` の中 | `<property name="iteration">`（1 から。`--naos-repeat` が 2 以上のとき）と `<property name="random_seed">`（`--naos-shuffle` のとき）を持つ |
| `<testcase>` | 実行したテストごと | `classname`: スイート。`name`: テスト。`file`、`line`: 最初の失敗かスキップが起きた場所。どちらもなければ省く |
| `<failure>` | 失敗したテストの失敗ごと。起きた順 | `type`: 常に `error`。`message`: `error` の診断の行の内容（[出力](output.md#診断の行)）。アサーションの式、`NAOS_FAIL` のメッセージ、trace の書き間違いのどれか |
| `<skipped>` | スキップしたテストの中 | `message`: `NAOS_SKIP` のメッセージ |

テストケースは、テストが失敗したときは `<failure>` 要素を、スキップしたときは `<skipped>` を 1 つ持ち、成功したときは何も持ちません。失敗した後にスキップしたテストは、失敗だけを持ちます。`--naos-fail-fast` で実行しなかったテストには、テストケースがありません。Naos は時間を測らず、テストが出力したものも取り込まないので、`time`、`timestamp`、`<system-out>` は書きません。

## 失敗

`<failure>` の中身は `<label>: <value>` の行で、次の順に並びます。

1. `file: <file>` と `line: <line>`: 失敗した場所
2. アサーションの[値](#値)。`left: <value>`、`right: <value>` など。`NAOS_FAIL` にはない
3. `NAOS_NOTE` のメモがあれば `note: <text>`
4. 16 段を超えて trace を積んでいれば `traces: <n> innermost not kept`
5. 積んである trace ごとに `trace: <file>:<line>: <text>`（内側から）

ラベルはそのまま書き、値を揃える空白は足しません。後のバージョンで行が増えることがあるので、ツールは知らないラベルを読み飛ばしてください。

## 値

| アサーション | ラベル（順に） |
|---|---|
| `*_INT_*`、`*_UINT_*` | `left`、`right`（10 進） |
| `*_DOUBLE_NEAR` | `left`、`right`、`tolerance` |
| `*_STR_EQ`（どちらも `NULL` でない） | `left`、`right`、`first difference` |
| `*_STR_EQ`（片方が `NULL`）、`*_STR_NE` | `left`、`right` |
| `*_PTR_EQ` | `left`、`right`（`%p`、または `(null)`） |
| `*_NULL` | `value`（`%p`） |
| `*_MEM_EQ` | `left size`、`right size`、続けて `first difference` か `invalid` |
| `*_TRUE`、`*_FALSE`、`*_NOT_NULL` | なし |

`first difference` は `offset N, left 0xXX, right 0xYY` です。一方がもう一方の先頭部分と一致するときは `offset N, end of left` / `end of right` で、N は短いほうの長さです。

## テキスト

ファイルは UTF-8 です。テキストはすべて XML 用にエスケープします（`&amp;`、`&lt;`、`&gt;`。属性では `&quot;` と `&apos;` も使い、改行とタブを `&#10;` と `&#9;` にします。復帰は常に `&#13;` です）。

- **文字列の値**は[値の表記](#値の表記)のとおりに書くので、印字できる ASCII だけになります。0x80 以上のバイトは `\xNN` にエスケープします。
- **ほかのテキスト**（式、`NAOS_FAIL` と `NAOS_SKIP` のメッセージ、メモ、trace、ファイル名）は、正しい UTF-8 ならそのまま書きます。どの言語で書いたメッセージも書いたとおりに読めます。正しい UTF-8 の一部でないバイトと、XML 1.0 で使えない制御文字（改行、タブ、復帰以外の 0x20 未満）は `\xNN` と書きます。そのため、テキスト自身が `\xNN` を含むものとは区別できないことがあります。

## 値の表記

文字列と double は、ここでも標準出力でも、どのプラットフォーム・ロケールでも同じ表記で書きます。整数、ポインタ、メモリ比較の値、メモ、trace、`NAOS_FAIL` / `NAOS_SKIP` のメッセージ、診断の行の式は、この規則によらず、そのまま書きます。

**文字列**は二重引用符で囲んで 1 行で書き、切り詰めません。`NULL` は引用符なしの `(null)` です。0x80 以上のバイトは、UTF-8 として正しいかどうかによらずエスケープします。

| バイト | 書く形 |
|---|---|
| 0x0A、0x09、0x0D | `\n`、`\t`、`\r` |
| 0x22、0x5C | `\"`、`\\` |
| 上記以外の 0x20 未満、0x7F、0x80〜0xFF | `\x` と大文字の 16 進 2 桁 |
| 上記以外の 0x20〜0x7E | そのまま |

**double** は、読み戻すと同じ値になる最も短い 10 進表記で書きます。10 進の指数が -4 以上 17 未満なら指数を使わず、それ以外は `<digits>e<sign><exponent>` の形で、指数は少なくとも 2 桁です。末尾の不要な `0` と、小数部のない小数点は付けません。負のゼロは `-0`、無限大は `inf` と `-inf`、NaN はすべて `nan` です。小数点は常に `.` です。

| 値 | 書く形 |
|---|---|
| `0.1` | `0.1` |
| `1.0 / 3.0` | `0.3333333333333333` |
| `100.0` | `100` |
| `1e-5` | `1e-05` |
| `1.5e20` | `1.5e+20` |

## 互換性

このページに書いた要素、属性、失敗の行は互換を保ちます。後のバージョンで、属性、プロパティ、要素、失敗の行、`xml` 以外の形式が増えることがあります。ツールは知らないものを読み飛ばしてください。

## 出力例

Naos の自己テストのうち 4 つのテスト（`tests/report/report.c`）を、`--naos-output xml:results.xml --naos-filter 'results.passes:results.fails_twice:results.skips:results.with_context'` で実行したもの:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<testsuites name="naos_selftest_report" tests="4" failures="2" errors="0" skipped="1">
  <testsuite name="results" tests="4" failures="2" errors="0" skipped="1">
    <testcase classname="results" name="passes"/>
    <testcase classname="results" name="fails_twice" file="report.c" line="26">
      <failure type="error" message="1 == 2">file: report.c
line: 26
left: 1
right: 2</failure>
      <failure type="error" message="&quot;hello\n&quot; == &quot;hello&quot;">file: report.c
line: 27
left: "hello\n"
right: "hello"
first difference: offset 5, end of right</failure>
    </testcase>
    <testcase classname="results" name="skips" file="report.c" line="32">
      <skipped message="not here"/>
    </testcase>
    <testcase classname="results" name="with_context" file="report.c" line="47">
      <failure type="error" message="0.1 ~= 0.25">file: report.c
line: 47
left: 0.1
right: 0.25
tolerance: 0.001
note: locale C
trace: report.c:45: inner 2
trace: report.c:44: outer</failure>
      <failure type="error" message="&quot;ab&quot; == &quot;ac&quot;">file: report.c
line: 48
left size: 2
right size: 2
first difference: offset 1, left 0x62, right 0x63
trace: report.c:45: inner 2
trace: report.c:44: outer</failure>
    </testcase>
  </testsuite>
</testsuites>
```

実行全体は、あらゆる種類のエスケープを含めて `tests/report/expected/all.txt` にあります。
