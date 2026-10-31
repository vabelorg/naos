# CTest と CI

クラッシュの分離、並列実行、時間制限が必要な場合は、テストを 1 つずつ別のプロセスで起動します。`--naos-list` で名前を列挙し、`--naos-filter` で 1 つずつ実行します。終了コードが `0` なら成功、`77` ならスキップ、それ以外は失敗です。プロセスの管理は呼び出し側（CTest、スクリプト、CI）に任せるのが Naos の方針です。

## CMake を使う場合

`naos_add_executable()` で作った実行ファイルのテストは、ビルドのたびに 1 テストずつ CTest に登録されます。CTest はテストごとに別のプロセスを起動するので、CTest で実行するだけで済みます。CTest でのテスト名は `<target>.<suite>.<test>`（例: `my_tests.parser.empty_object`）です。`-C Debug` は、Visual Studio などのマルチコンフィグのジェネレータでビルド構成を選ぶためのものです。

```sh
ctest --test-dir build -C Debug --output-on-failure      # すべて
ctest --test-dir build -C Debug -j8 --timeout 30         # 並列、30 秒で打ち切り
ctest --test-dir build -C Debug -R '^my_tests\.parser\.' # スイート parser だけ
```

`add_executable()` で作った実行ファイルは、`naos_discover_tests(<target>)` で同じように登録できます。どちらの関数でも、`LABELS` でテストにラベルを、`PROPERTIES` で環境変数（`ENVIRONMENT`）などの CTest のプロパティを付けられます。

```cmake
naos_add_executable(io_tests
    SOURCES io_test.c
    LIBRARIES my_lib
    LABELS io slow
    PROPERTIES ENVIRONMENT "LARGE_FILE=${CMAKE_CURRENT_SOURCE_DIR}/data/large.bin"
)
```

`ctest -L slow` でラベル `slow` のテストを選び、`ctest -LE slow` で除けます。引数の一覧は [CMake のリファレンス](../reference/cmake.md)にあります。

## CMake を使わない場合

シェルスクリプトで同じことを行います。

```sh
#!/bin/sh
exe=$1
fail=0

for c in $("$exe" --naos-list); do
  "$exe" --naos-filter "$c"
  case $? in
    0|77) ;;
    *)    fail=$((fail+1)) ;;
  esac
done

[ "$fail" -eq 0 ]
```

## CI で実行する

CI のジョブでは、configure、ビルド、CTest の実行を行います。`--output-junit` を付けると、多くの CI サービスが表示できる JUnit XML のレポートを（ビルドディレクトリからの相対パスで）書き出します。CTest 3.21 から使えます。GitHub Actions なら次のようになります。

```yaml
- name: Build
  run: |
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
    cmake --build build --config Debug
- name: Test
  run: ctest --test-dir build -C Debug --output-on-failure --output-junit junit.xml
```

テストの間の隠れた依存を見つけるため、実行ファイルを `--naos-shuffle --naos-repeat 20` で直接実行するジョブを足すのも有効です（[テストの実行](running-tests.md#テストの間の依存を見つける)）。失敗した回のシード値は `ITERATION FAILED` の行にあるので、手元で同じ順序を再現できます。

## 失敗した行をエディタで開く

失敗の行はコンパイラの診断と同じ `file:line: error: ...` の形で出力されるため、エディタのエラー一覧に表示できます。VS Code では、次の problem matcher を `.vscode/tasks.json` に書きます。失敗の行と trace の行が問題パネルに並び、クリックするとソースの該当行に移動できます。Naos のライブラリを MSVC でビルドした場合の `file(line): error: ...` の形にも対応しています。

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "naos tests",
            "type": "shell",
            "command": "ctest --test-dir build -C Debug --output-on-failure",
            "problemMatcher": [
                {
                    "owner": "naos",
                    "fileLocation": ["autoDetect", "${workspaceFolder}"],
                    "severity": "error",
                    "pattern": {
                        "regexp": "^(.*?)[:(](\\d+)\\)?: error: (.*)$",
                        "file": 1,
                        "line": 2,
                        "message": 3
                    }
                },
                {
                    "owner": "naos",
                    "fileLocation": ["autoDetect", "${workspaceFolder}"],
                    "severity": "info",
                    "pattern": {
                        "regexp": "^(.*?)[:(](\\d+)\\)?: note: (.*)$",
                        "file": 1,
                        "line": 2,
                        "message": 3
                    }
                }
            ]
        }
    ]
}
```

CI で結果を集計するなど、出力を読み取るツールを書く場合は、[出力](../reference/output.md)の形式に従ってください。その形式は互換を保つ対象です。
