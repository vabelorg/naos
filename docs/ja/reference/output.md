# 出力

テストの実行中に Naos が標準出力に書く行は、次の 3 種類だけです。これらは人が読むための行で、このページは今のバージョンが書く形を説明しています。[互換を保つもの](#互換を保つもの)を除き、読みやすくするためにどのバージョンでも変わることがあります。結果を読むツールは、代わりに `--naos-output` の [JUnit XML](xml.md) のファイルを読んでください。`--naos-list` と `--help` の出力、標準エラー出力に書く `naos: ...` のメッセージは、この出力に含みません。テスト自身が書いた出力は、そのまま間に混ざります。

| 種類 | 形 | 内容 |
|---|---|---|
| 流れの行 | `KEY: value` | 各テストの開始と結果、回の区切りと集計 |
| 診断の行 | `<location> <kind>: <text>` | アサーションの失敗、スキップ、それに添える trace |
| 詳細の行 | `  <label>: <value>` | 行頭に 2 文字の空白。直前の診断の行に属する |

## 流れの行

| 行 | 出すとき |
|---|---|
| `RUN: <suite>.<test>` | テストを始めるとき（setup の前）。すぐに書き出すので、プロセスが落ちても残る |
| `PASS: <suite>.<test> (<t> ms)` | 失敗もスキップもなく終わったとき |
| `FAIL: <suite>.<test> (<N> failure, <t> ms)` | 失敗を記録したとき。N が 2 以上なら `failures` |
| `SKIP: <suite>.<test> (<t> ms)` | スキップし、失敗を記録しなかったとき |
| `SUMMARY: <p> passed, <f> failed, <s> skipped (<t> ms)` | 各回の最後。実行したテストが 1 つや 0 のときも出る。`--naos-fail-fast` で止めたときは、実行したテストだけを数える |
| `RANDOM SEED: <seed>` | `--naos-shuffle` のとき、各回の最初 |
| `ITERATION: <i> of <n>` | `--naos-repeat` が 2 以上のとき、各回の前 |
| `ITERATION FAILED: <i> of <n>` | `--naos-repeat` が 2 以上のとき、失敗した回の後。`--naos-shuffle` なら後ろに ` (random seed <seed>)` |
| `REPEAT: <f> of <n> iterations failed` | `--naos-repeat` が 2 以上のとき、すべての回の後。途中で止めたときは後ろに `, <k> not run` |
| `FAIL FAST: stopped at the first failure; <n> not run` | `--naos-fail-fast` で止め、実行しなかったテストがあるとき |

<t> はミリ秒の[時間](#時間)です。

1 回の中の並びは `ITERATION:`、`RANDOM SEED:`、各テスト（`RUN:` から結果の行まで）、`FAIL FAST:`、`SUMMARY:`、`ITERATION FAILED:` の順で、最後に `REPEAT:` が来ます。1 つのテストの setup・本体・teardown で出た診断は、すべてそのテストの `RUN:` と結果の行の間に出ます。

## `--naos-quiet` のとき

`--naos-quiet` を付けると、目を向ける必要のないテストを省きます。出力する行は、付けないときと同じ形、同じ並びです。

| 行 | `--naos-quiet` のとき |
|---|---|
| `RUN:` | 失敗したテストだけ。最初の失敗の直前に出る |
| `PASS:` | 出ない |
| スキップしたテスト（その `RUN:`、`skip` の行、`SKIP:`） | 出ない。`SUMMARY:` で数える |
| `FAIL:` と、失敗したテストのすべての診断 | 出る |
| `SUMMARY:`、`RANDOM SEED:`、`FAIL FAST:`、`ITERATION FAILED:`、`REPEAT:` | 出る |
| `ITERATION:` | 出ない |

- スキップしたあと teardown で失敗したテストは `FAIL` です。その `skip` の行はテストが終わるまで保持し、失敗の前に出力します。そのため、失敗したテストは `--naos-quiet` がないときと同じに読めます。
- テストが自分で出力したものはそのまま出ますが、前に `RUN:` の行がないので、どのテストが出力したのかは分かりません。
- `RUN:` を各テストの開始時に出さなくなるので、失敗する前にプロセスを終わらせたテストは、その名前を示す行を残しません。`--naos-output` のファイルは実行の終わりにしか書かないので、そこにも残りません。CTest の下では各テストが別のプロセスで動き、終わったテストは CTest が示します。そうでなければ、同じコマンドを `--naos-quiet` なしで実行し直してください。

```
$ prog --naos-quiet
RUN: text.greeting
greet_test.c:12: error: greeting == "hello"
  left:  "hello\n"
  right: "hello"
  first difference: offset 5, end of right
FAIL: text.greeting (1 failure, 1 ms)
SUMMARY: 497 passed, 1 failed, 2 skipped (2318 ms)
```

## 時間

結果の行と `SUMMARY:` の最後には、テストにかかった時間が付きます。指定しなくても、遅いテストが目に入るようにするためです。

- テストの時間は、setup・本体・teardown にかかった時間です。テストの開始と終了に合わせ、setup の直前から teardown の直後までを測ります。行を出力する時間は含みません。
- `SUMMARY:` の時間は、その回のテストの時間の合計です。`--naos-fail-fast` で止めたときは、実行したテストだけの合計です。`--naos-repeat` では回ごとに出ます。
- どちらも切り捨てたミリ秒の整数です。1 ミリ秒に満たないテストは `0 ms` になります。`SUMMARY:` は合計を最後に 1 回だけ切り捨てるので、表示された時間の和より少し大きいことがあります。
- 時計は、システムの単調時計による実時間です。システムの時刻を変えても飛びません。Windows では `QueryPerformanceCounter`、macOS とそのほかの POSIX システムでは `clock_gettime(CLOCK_MONOTONIC)` を使います。それ以外では標準 C の `clock()` に頼り、これはプロセッサ時間です。眠ったり待ったりするテストは、実際より短く出ます。どちらを使っているかは `--help` の最後の行に出ます（[コマンドライン](command-line.md)）。
- 時間は実行のたびに変わるので、標準出力も変わります。2 回の実行の結果を比べるときは、`time` を除いた [JUnit XML](xml.md) のファイルか、時間を除いた出力を比べてください。

[JUnit XML](xml.md) のファイルにも、同じ時間を秒で書きます。

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

## 互換を保つもの

標準出力のうち、互換を保つのは次のものだけです（[バージョン](version.md)）。ツールがこれなしでは動かないためです。[JUnit XML](xml.md#互換性) のファイルには、それ自身の規則があります。

- 診断の行の始まり。その形式の[場所](#診断の行)、続く種類（`error`、`skip`、`note`）と `: ` です。エディタがこれを読みます（[失敗した行をエディタで開く](../guides/ctest-and-ci.md#失敗した行をエディタで開く)）。その後ろの内容は変わることがあります。
- `--naos-list` が出力する名前（1 行に 1 つ。[コマンドライン](command-line.md#オプション)）。Naos の CMake の関数がこれを読んで、テストを CTest に登録します。
- [終了コード](command-line.md#終了コード)。

このページのそのほかのもの（流れの行、詳細の行、並び、集計など）は、どのバージョンでも変わることがあります。変わるときは変更履歴に書きます。

## 出力例

```
RUN: math.addition
PASS: math.addition (0 ms)
RUN: text.greeting
greet_test.c:12: error: greeting == "hello"
  left:  "hello\n"
  right: "hello"
  first difference: offset 5, end of right
greet_test.c:30: note: locale C
FAIL: text.greeting (1 failure, 1 ms)
SUMMARY: 1 passed, 1 failed, 0 skipped (1 ms)
```
