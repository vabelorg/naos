# テストの実行

テストを選んだり、実行の仕方を変えたりする、どのテスト実行ファイルにもあるオプションです。規則は[コマンドライン](../reference/command-line.md)にあります。

## 一部のテストだけを実行する

`--naos-filter` に `<suite>.<test>` を渡すと、指定したテストだけを実行します。失敗したテストだけをやり直すときや、デバッガで 1 つのテストを追うときに使います。

```sh
./my_tests --naos-filter parser.empty_object
```

デバッガでテストの中で止めるには、`NAOS_TEST` が定義する関数の名前 `naos_gen_test_<name>` にブレークポイントを置きます。[生成される関数名](../reference/tests.md#生成される関数名)を参照してください。

パターンに `*`（任意の文字列）や `?`（任意の 1 文字）を使うと、複数のテストをまとめて指定できます。

- 複数のパターンを並べるときは `:` で区切ります。
- 除外したいパターンは `-` の後ろに書きます。
- シェルが `*` や `?` を展開しないよう、パターンは `'parser.*'` のように引用符で囲みます。

| 指定 | 実行されるテスト |
|---|---|
| `parser.*` | スイート `parser` のすべてのテスト |
| `*.empty_*` | すべてのスイートの、名前が `empty_` で始まるテスト |
| `parser.*:writer.*` | スイート `parser` と `writer` のすべてのテスト |
| `parser.*-*.slow_*` | スイート `parser` のテストのうち、名前が `slow_` で始まるもの以外 |
| `-*.slow_*` | 名前が `slow_` で始まるもの以外のすべてのテスト |

テストを 1 つも選ばないフィルタは、書き間違いとして使い方エラー（終了コード `2`）になります。登録されているテストの名前は `--naos-list` で一覧できます。

## 失敗したものだけを見る

テストが多いと、成功したテストの行に失敗したテストが埋もれます。`--naos-quiet` を付けると、失敗したテストとその診断、そして集計だけを出力します。

```
$ ./my_tests --naos-quiet
RUN: parser.nested
parser_test.c:41: error: depth == 3
  left:  2
  right: 3
FAIL: parser.nested (1 failure, 0 ms)
SUMMARY: 211 passed, 1 failed, 0 skipped (1834 ms)
```

- スキップしたテストは集計で数えるだけです。
- クラッシュなどでプロセスを終わらせたテストは、その名前を示す行を残しません。どのテストだったかは、同じコマンドを `--naos-quiet` なしで実行し直すと分かります。

省く行の一覧は[出力](../reference/output.md#--naos-quiet-のとき)にあります。

## テストの間の依存を見つける

テストは登録順に同じプロセスで実行されます。そのため、前のテストが残した状態に意図せず依存しているテストは、登録順によっては通ってしまうことがあります。

`--naos-shuffle` を付けると、スイートの順序とスイートの中のテストの順序が入れ替わり、こうした依存が失敗として表に出ます。

```
$ ./my_tests --naos-shuffle
RANDOM SEED: 4242
...
$ ./my_tests --naos-shuffle --naos-random-seed 4242   # 同じ順序で実行し直す
```

失敗したら、`RANDOM SEED` の行に出力されたシード値を `--naos-random-seed` に渡すと、同じ順序で実行し直せます。

`--naos-filter` で絞り込んでも、残ったテストの順序は絞り込む前と変わらないため、疑わしいテストだけを残して原因を調べられます。

## 不安定なテストを見つける

まれにしか失敗しないテストは、`--naos-repeat` で繰り返し実行して見つけます。`--naos-shuffle` と組み合わせると回ごとに異なる順序で実行され、失敗した回のシード値は `ITERATION FAILED` の行に表示されます。

最初の失敗で止めたい場合は、`--naos-fail-fast` を追加します。次の例では、ランダムな順序でテストを 50 回繰り返し、最初の失敗で止めます。

```sh
./my_tests --naos-shuffle --naos-repeat 50 --naos-fail-fast
```

## 遅いテストを見つける

結果の行の最後には、setup と teardown を含めてテストにかかった時間が付き、`SUMMARY:` には合計が付きます。

```
$ ./my_tests
RUN: storage.open_empty
PASS: storage.open_empty (0 ms)
RUN: storage.rebuild_index
PASS: storage.rebuild_index (3120 ms)
SUMMARY: 2 passed, 0 failed, 0 skipped (3120 ms)
```

変更で遅くなったテストは、その行で分かります。CTest の時間制限（`--timeout`、[CTest と CI](ctest-and-ci.md)）を決めるのにも役立ちます。

- 遅い順に並べるには、結果の行を時間で並べ替えるか、`--naos-output` の [JUnit XML](../reference/xml.md) のファイルのテストケースを `time` で並べ替えます。
- `slow_*` のように名前で分かる遅いテストは、`--naos-filter '-*.slow_*'` で、編集しては実行するループから外せます。

時間は実時間です。単調時計のないシステムではプロセッサ時間になり、そのことは `--help` に出ます（[時間](../reference/output.md#時間)）。

## 1 テストずつ別のプロセスで実行する

すべてのテストを 1 つのプロセスで実行すると速いですが、クラッシュすると実行全体が終わり、テストが止まったままになると実行が終わりません。

テストを分離したい、並列に実行したい、時間制限を付けたいときは、[CTest と CI](ctest-and-ci.md) を見てください。
