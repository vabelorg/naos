# 出力

テストの実行中に Naos が標準出力に書く行は、次の 3 種類だけです。この形式は互換を保つ対象です。`--naos-list` と `--help` の出力、標準エラー出力に書く `naos: ...` のメッセージは、この形式に含みません。テスト自身が書いた出力は、そのまま間に混ざります。

| 種類 | 形 | 内容 |
|---|---|---|
| 流れの行 | `KEY: value` | 各テストの開始と結果、回の区切りと集計 |
| 診断の行 | `<location> <kind>: <text>` | アサーションの失敗、スキップ、それに添える trace |
| 詳細の行 | `  <label>: <value>` | 行頭に 2 文字の空白。直前の診断の行に属する |

## 流れの行

| 行 | 出すとき |
|---|---|
| `RUN: <suite>.<test>` | テストを始めるとき（setup の前）。すぐに書き出すので、プロセスが落ちても残る |
| `PASS: <suite>.<test>` | 失敗もスキップもなく終わったとき |
| `FAIL: <suite>.<test> (<N> failure)` | 失敗を記録したとき。N が 2 以上なら `failures` |
| `SKIP: <suite>.<test>` | スキップし、失敗を記録しなかったとき |
| `SUMMARY: <p> passed, <f> failed, <s> skipped` | 選んだテストが 2 つ以上のとき、各回の最後。`--naos-fail-fast` で止めたときは、実行したテストだけを数える |
| `RANDOM SEED: <seed>` | `--naos-shuffle` のとき、各回の最初 |
| `ITERATION: <i> of <n>` | `--naos-repeat` が 2 以上のとき、各回の前 |
| `ITERATION FAILED: <i> of <n>` | `--naos-repeat` が 2 以上のとき、失敗した回の後。`--naos-shuffle` なら後ろに ` (random seed <seed>)` |
| `REPEAT: <f> of <n> iterations failed` | `--naos-repeat` が 2 以上のとき、すべての回の後。途中で止めたときは後ろに `, <k> not run` |
| `FAIL FAST: stopped at the first failure; <n> not run` | `--naos-fail-fast` で止め、実行しなかったテストがあるとき |

1 回の中の並びは `ITERATION:`、`RANDOM SEED:`、各テスト（`RUN:` から結果の行まで）、`FAIL FAST:`、`SUMMARY:`、`ITERATION FAILED:` の順で、最後に `REPEAT:` が来ます。1 つのテストの setup・本体・teardown で出た診断は、すべてそのテストの `RUN:` と結果の行の間に出ます。

## 診断の行

| 種類 | 出すとき | 内容 |
|---|---|---|
| `error` | アサーションの失敗 | アサーションの式。マクロの引数を書いたとおりに使う（`NAOS_CHECK_INT_LT(count, limit)` なら `count < limit`） |
| | `NAOS_FAIL(msg)` | `msg`。`NULL` なら `(no message)` |
| | trace の書き間違い | `NAOS_TRACE_PUSH without NAOS_TRACE_POP: <trace>`、`NAOS_TRACE_POP without NAOS_TRACE_PUSH`、`NAOS_TRACE_PUSH beyond 16 levels` |
| `skip` | `NAOS_SKIP(msg)` | `msg`。`NULL` なら `(no message)` |
| `note` | 失敗とスキップに添える trace | その trace。場所は積んだ場所 |

診断の行の内容が空になることはなく、値も含みません。値は続く詳細の行にあります。

場所は `<file>:<line>: `（GCC 形式）です。`_MSC_VER` を定義するコンパイラ（MSVC、clang-cl）で Naos のライブラリをビルドしたときは `<file>(<line>): `（MSVC 形式）になります。形式が決まるのは、テストではなくライブラリをビルドしたときです。そのため、MSVC でビルドした Naos を MinGW の GCC でビルドしたテストから使うと MSVC 形式になります。実行時に形式を選ぶオプションはありません。ファイル名はコンパイラが与えた `__FILE__` のままです。

GCC 形式の `error` の行は、VS Code の `$gcc` など GCC の診断を読む problem matcher がそのまま拾います。MSVC 形式の行は VS Code の `$msCompile` には拾われません。どちらの形式も拾う problem matcher は[ガイド](../guides/ctest-and-ci.md#失敗した行をエディタで開く)にあります。

## 詳細の行

詳細の行は、2 文字の空白、ラベル、コロン、1 文字の空白、値の順です。`left` と `left size` は、同じブロックに `right` か `right size` があるとき空白を 1 文字増やし、2 つの値の始まりを揃えます。ほかのラベルは揃えません。

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

## 値の表記

文字列と double は、どのプラットフォーム・ロケールでも同じ表記で出力します。整数、ポインタ、メモリ比較の値、メモ、trace、`NAOS_FAIL` / `NAOS_SKIP` のメッセージ、診断の行の式は、この規則によらず、そのまま出力します。

**文字列**は二重引用符で囲んで 1 行で出力し、切り詰めません。`NULL` は引用符なしの `(null)` です。0x80 以上のバイトは、UTF-8 として正しいかどうかによらずエスケープします。

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

## 並び

1 つの失敗またはスキップは次の順に出力し、出し終えたら標準出力を書き出します。

1. `error` または `skip` の行
2. 値の詳細の行
3. `NAOS_NOTE` のメモがあれば `note: <text>`
4. 16 段を超えて trace を積んでいれば `traces: <n> innermost not kept`
5. 積んである trace ごとの `note` の診断の行（内側から）

## 出力例

```
RUN: math.addition
PASS: math.addition
RUN: text.greeting
greet_test.c:12: error: greeting == "hello"
  left:  "hello\n"
  right: "hello"
  first difference: offset 5, end of right
greet_test.c:30: note: locale C
FAIL: text.greeting (1 failure)
SUMMARY: 1 passed, 1 failed, 0 skipped
```
