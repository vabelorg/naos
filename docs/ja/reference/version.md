# バージョン

複数のバージョンに対応するコードのために、ビルドに使った Naos と実行時の Naos のバージョンを知るマクロと関数です。`#include <naos/naos.h>` で使えます。

Naos は[セマンティックバージョニング](https://semver.org/lang/ja/)に従います。メジャー・マイナー・パッチの各番号は 0〜255 の範囲です。1.0 より前は、マイナーバージョンが上がると互換が切れることがあります。

## コンパイル時

次のマクロは、読み込んだ Naos のヘッダのバージョンを表します。コンパイル時にコードを切り替えるために使います。

ヘッダとリンクしたライブラリのバージョンは異なることがあり（特に共有ライブラリ）、これらのマクロで書いた条件が実際のライブラリと合わないことがあります。ライブラリのバージョンは[実行時](#実行時)の関数で確かめます。

| マクロ | 値 |
|---|---|
| `NAOS_VERSION_MAJOR` | メジャーバージョン |
| `NAOS_VERSION_MINOR` | マイナーバージョン |
| `NAOS_VERSION_PATCH` | パッチバージョン |
| `NAOS_VERSION_NUMBER` | バージョンを数値 `0xMMNNPP` にしたもの。メジャー（`MM`）、マイナー（`NN`）、パッチ（`PP`）をそれぞれ 16 進 2 桁で表す。1.2.3 は `0x010203` |
| `NAOS_VERSION_STRING` | バージョンを文字列 `"MAJOR.MINOR.PATCH"` にしたもの |

Naos がバージョン `X.Y.Z` 以上であることをコンパイル時に確かめる例です。

```c
#if NAOS_VERSION_NUMBER >= NAOS_VERSION_ENCODE(1, 2, 3)
    /* 1.2.3 以上で使えるもの */
#endif
```

## 実行時

次の関数は、実際にリンクしたライブラリのバージョンを返します。

```c
unsigned long naos_version_number(void);
const char   *naos_version_string(void);
```

- `naos_version_number()` は、バージョンを数値 `0xMMNNPP` で返します。
- `naos_version_string()` は、バージョンを文字列 `"MAJOR.MINOR.PATCH"` で返します。

Naos がバージョン `X.Y.Z` 以上であることを実行時に確かめる例です。

```c
if (naos_version_number() >= NAOS_VERSION_ENCODE(1, 2, 3))
{
    /* 1.2.3 以上で使えるもの */
}
```

読み込んだ共有ライブラリが、テストをビルドしたときのものかを確かめる方法は、[共有ライブラリのガイド](../guides/shared-library.md#読み込んだバージョンを確かめる)にあります。

## 変換

```c
#define NAOS_VERSION_ENCODE(major, minor, patch)
#define NAOS_VERSION_STRINGIFY(major, minor, patch)
```

- `NAOS_VERSION_ENCODE()` は、バージョンを数値 `0xMMNNPP` にします。`NAOS_VERSION_NUMBER` と `naos_version_number()` はこれで作ります。
- `NAOS_VERSION_STRINGIFY()` は、バージョンを文字列 `"MAJOR.MINOR.PATCH"` にします。`NAOS_VERSION_STRING` と `naos_version_string()` はこれで作ります。
