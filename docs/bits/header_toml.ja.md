<!-- xer-reference-source-sha256: 21ed6e78be9e4691104ec73b137e6b90f34c9458448012285c90fe0eefb227d2 -->

# `<xer/toml.h>`

## 目的

`<xer/toml.h>` は、xer における TOML のデコードおよびエンコード機能を提供します。

TOML は、型付きの設定データ形式として扱われます。
このヘッダーの目的は、xer の段階的な開発方針に収まる程度に実装を小さく保ちながら、単純な UTF-8 TOML テキストの実用的な読み書きを支援することです。

現在の実装は、TOML の実用的なサブセットをサポートします。
TOML v1.0.0 への完全な互換性は主張しません。

---

## 主な役割

`<xer/toml.h>` の主な役割は、次のことを可能にすることです。

- UTF-8 TOML テキストを構造化された xer の値モデルへ解析する
- 真偽値、整数、浮動小数点数、文字列、配列、テーブルを調べる
- サポートされる値モデルを UTF-8 TOML テキストへシリアライズする
- INI とは異なる型付き設定形式として TOML を使用する

これにより、このヘッダーは、INI より多くの構造を必要としつつ、初期段階では TOML の全機能までは必要としない設定データに有用です。

---

## 主なエンティティ

少なくとも、`<xer/toml.h>` は次のエンティティを提供します。

```cpp
struct toml_value;

using toml_array = std::vector<toml_value>;
using toml_table = std::vector<std::pair<std::u8string, toml_value>>;

auto toml_decode(std::u8string_view text)
    -> xer::result<toml_value, parse_error_detail>;
auto toml_encode(const toml_value& value) -> xer::result<std::u8string>;
```

正確なヘルパー関数とサポートされる TOML 構文は今後拡張される可能性がありますが、これらは初期 TOML 機能の中核となる公開エンティティです。

---

## `toml_value`

`toml_value` は、xer における TOML の中心的な値型です。

1つの TOML 値を構造化された形で格納します。

### サポートされる値の種類

現在の実装は、次の値の種類をサポートします。

* 真偽値
* 整数
* 浮動小数点数
* 文字列
* ローカル日付
* ローカル時刻
* ローカル日時
* オフセット付き日時
* 配列
* テーブル

内部表現は次に対応します。

```cpp
std::variant<
    bool,
    std::int64_t,
    double,
    std::u8string,
    toml_local_date,
    toml_local_time,
    toml_local_datetime,
    toml_offset_datetime,
    toml_array,
    toml_table
>
```

### 注意

* 整数値は `std::int64_t` として格納されます
* 浮動小数点値は `double` として格納されます
* 文字列は UTF-8 の `std::u8string` として格納されます
* 日付/時刻値は TOML 専用の小さな値構造体に格納されます
* 配列は `std::vector<toml_value>` として格納されます
* テーブル配列は、要素がテーブルである配列として表現されます
* テーブルは順序付きのキー値ペアとして格納されます

---

## `toml_array`

```cpp
using toml_array = std::vector<toml_value>;
```

`toml_array` は TOML 値の配列を表します。

現在の実装では、配列は通常の順序付きベクターとして格納されます。

### 注意

* 配列要素の順序は保持されます
* 配列は、サポートされる種類が異なる値を含められます
* 配列はテーブル値を含められます
* テーブル配列構文は、要素がテーブル値である配列として表現されます

---

## `toml_table`

```cpp
using toml_table = std::vector<std::pair<std::u8string, toml_value>>;
```

`toml_table` は TOML テーブルを表します。

テーブルは、map 風のコンテナーではなく、順序付きのキー値ペアとして表現されます。

### 順序付きペアを使う理由

この表現は値モデルを単純に保ち、ソース上の順序を保持します。

また、公開データ形式モデルをハッシュマップやツリーマップの意味論へ早すぎる段階で固定しないという、xer の一般的な傾向にも合っています。

### 重複キー

TOML では、同じテーブル内の重複キーは許可されません。

デコーダーは重複キーを不正な入力として拒否します。

---

## 検査とアクセサー

`toml_value` は検査関数とアクセサー関数を提供します。

少なくとも、次の検査関数が提供されます。

```cpp
is_bool
is_integer
is_float
is_string
is_array
is_table
```

少なくとも、次のアクセサー関数が提供されます。

```cpp
as_bool
as_integer
as_float
as_string
as_array
as_table
```

アクセサー関数は、値が要求された種類を持つ場合は格納値へのポインターを返し、そうでない場合は `nullptr` を返します。

### 例

```cpp
const auto* table = value.as_table();
if (table == nullptr) {
    return 1;
}
```

このスタイルは、型検査を明示的にし、通常の種類判定のために例外を投げることを避けます。

---

## サポートされる TOML サブセット

現在の実装は、次のような TOML 風入力をサポートします。

```toml
title = "xer"
enabled = true
count = 123
hex = 0xFF
mask = 0b1010_0101
ratio = 1.5
large = 1_000_000
ports = [8000, 8001, 8002]
point = { x = 1, y = 2 }
items = [{ name = "one" }, { name = "two" }]
released = 2026-04-30
created = 2026-04-30T23:59:58+09:00

[project]
name = "xer"
version = "example"
```

### サポートされる形式

初期デコーダーは次をサポートします。

* 空行
* `#` で始まるコメント
* 文字列外の行末コメント
* 裸キー
* 引用符付きキー
* ドット付きキー
* キー値ペア
* 通常のテーブルと入れ子テーブル
* 基本二重引用符付き文字列
* リテラル文字列
* 複数行の基本文字列とリテラル文字列
* 真偽値
* 符号付き10進整数
* 16進、8進、2進整数
* 数字間の数値区切り文字
* 有限および特殊な浮動小数点数
* 配列
* インラインテーブル
* ローカル日付、ローカル時刻、ローカル日時、オフセット付き日時の値
* テーブル配列

### 改行

デコーダーは次の改行を受け付けます。

* LF
* CRLF
* CR

---

## 後回しの TOML 機能と制限事項

現在の実装は、日付/時刻値とテーブル配列を含む、TOML の実用的なサブセットをサポートします。

次の領域は意図的に制限されているか、後回しにされています。

* TOML v1.0.0 への完全準拠
* エンコード時に元の整形やコメントを保持すること
* TOML のオフセット付き日時構文を超えるタイムゾーン名
* TOML v1.0.0 が要求するすべての意味論的規則の完全な検証

したがって、この実装は完全な TOML 実装ではなく、実用的な TOML サブセットとして説明する必要があります。

---

## キー

実装は裸キー、引用符付きキー、ドット付きキーをサポートします。

裸キーには次を含められます。

* ASCII 英字
* ASCII 数字
* `_`
* `-`

例:

```toml
name = "xer"
build-target = "ucrt64"
version_1 = "example"
```

引用符付きキーは、値と同じ単一行の基本文字列またはリテラル文字列構文を使用します。

```toml
"site.name" = "xer"
'literal.key' = 10
```

ドット付きキーは、必要に応じて入れ子テーブルを暗黙に作成します。

```toml
server.port = 8080
database."connection.pool".size = 4
```

この表現では、各ドット付きキーセグメントは通常のテーブルキーとして格納されます。引用符付きセグメントには、分割されないドットを含められます。

---

## テーブル

テーブル行は次の形式です。

```toml
[project]
```

入れ子テーブル名も、ドット付きキー構文を通じてサポートされます。

```toml
[project.package]
name = "xer"

[project."build.target"]
name = "ucrt64"
```

テーブル名はキーパスとして解析されます。そのテーブルは、別のテーブルが宣言されるまで、後続のキー値エントリーの出力先になります。

### 注意

* 明示的なテーブル宣言の重複は拒否されます
* ドット付きキーによって暗黙に作成されたテーブルは、後で明示的に宣言できます
* `[[project]]` のようなテーブル配列構文はサポートされ、要素がテーブルである配列として表現されます

---

## 文字列

実装は、基本文字列、リテラル文字列、複数行基本文字列、複数行リテラル文字列をサポートします。

```toml
name = "xer"
path = 'C:\\Users\\xer'
description = """
first line
second line"""
literal_block = '''
C:\\Users\\xer
'''
```

基本文字列は次のエスケープをサポートします。

```text
\"
\\
\b
\t
\n
\f
\r
\uXXXX
\UXXXXXXXX
```

リテラル文字列はエスケープシーケンスを処理しません。複数行文字列は、TOML 風の挙動に従い、開始デリミターの直後の最初の改行を取り除きます。

---

## 真偽値

次の真偽値がサポートされます。

```toml
enabled = true
disabled = false
```

これらは `bool` として格納されます。

---

## 整数

現在の実装は、符号付き10進整数と、接頭辞付きの非10進整数をサポートします。数値区切り文字は数字間で使用できます。

```toml
count = 123
offset = -5
positive = +123
large = 1_000_000
hex = 0xFF
octal = 0o755
binary = 0b1010_0101
```

これらは `std::int64_t` として格納されます。

サポートされる整数接頭辞は、16進の `0x`、8進の `0o`、2進の `0b` です。接頭辞の前に先頭の `+` または `-` を使用できます。区切り文字は、対応する基数の数字間でのみ有効です。

---

## 浮動小数点数

実装は、有限の10進浮動小数点数と TOML の特殊な浮動小数点値をサポートします。数値区切り文字は、有限10進形式の10進数字間で使用できます。

```toml
ratio = 1.5
scale = 1e-3
large = 1_000.25_5
positive = +1.5
negative = -1.5
positive_inf = inf
negative_inf = -inf
not_a_number = nan
```

これらは `double` として格納されます。

特殊値 `inf`、`+inf`、`-inf`、`nan`、`+nan`、`-nan` が受け付けられます。区切り文字は有限10進形式の数字間でのみ有効です。`1_.0`、`1._0`、`1.0e_3` のような形式は拒否されます。

---

## 日付と時刻の値

TOML デコーダーは、ローカル日付、ローカル時刻、ローカル日時、オフセット付き日時をサポートします。

例:

```toml
date = 2026-04-30
time = 23:59:58.123456
local = 2026-04-30T23:59:58
offset = 2026-04-30T23:59:58+09:00
utc = 2026-04-30T14:59:58Z
```

値モデルはこれらを `toml_local_date`、`toml_local_time`、`toml_local_datetime`、`toml_offset_datetime` として格納します。小数秒はマイクロ秒精度で格納されます。

---

## 配列

実装は配列をサポートします。

```toml
ports = [8000, 8001, 8002]
mixed = ["xer", true, 3]
```

配列は要素の順序を保持します。

実装は配列を `toml_array` として格納します。

### 注意

* 配列は、サポートされるスカラー値、配列、インラインテーブルを含められます
* テーブル配列構文は、要素がテーブルである配列として表現されます

---

## インラインテーブル

実装はインラインテーブルをサポートします。

```toml
point = { x = 1, y = 2 }
package = { name = "xer", metadata.version = "example" }
items = [{ name = "one" }, { name = "two" }]
```

インラインテーブルは通常の `toml_table` 値として格納されます。インラインテーブル内のドット付きキーは、通常のドット付きキーで使われる表現と同じ表現で入れ子テーブル値を作成します。

インラインテーブル内の末尾カンマは拒否されます。

---

## テーブル配列

デコーダーは TOML のテーブル配列構文をサポートします。

```toml
[[products]]
name = "Hammer"

[[products]]
name = "Nail"
```

値モデルでは、これは要素がテーブル値である `toml_array` として表現されます。入れ子のテーブル配列は、親配列内の最新のテーブル要素に追加されます。

エンコーダーは、テーブル文脈に現れる、要素がテーブルである配列を `[[...]]` として出力します。

---

## コメント

`#` は、文字列外に現れるとコメントを開始します。

```toml
name = "xer" # comment
```

文字列内の `#` は、文字列の一部として扱われます。

```toml
name = "x#r"
```

---

## `toml_decode`

```cpp
auto toml_decode(std::u8string_view text)
    -> xer::result<toml_value, parse_error_detail>;
```

### 目的

`toml_decode` は UTF-8 TOML テキストを解析し、`toml_value` を返します。

### 入力モデル

入力テキストは次の形で渡されます。

```cpp
std::u8string_view
```

これは、xer の UTF-8 指向の公開テキストモデルに従います。

### 戻り値モデル

成功時、`toml_decode` は `toml_value` を返します。

返される値はテーブル値です。

### 失敗条件

少なくとも、次の場合にデコードは失敗します。

* 入力に不正な UTF-8 が含まれる
* キー値行に `=` が含まれない
* キーが不正な形式である
* テーブル宣言が不正な形式である
* キーが重複している
* テーブルが重複している
* 値がサポートされない構文を使用している
* 値が不正な形式である

不正な UTF-8 はエンコーディングエラーとして扱われます。
不正な TOML 構造は不正な引数として扱われます。

---

## 解析エラー詳細

`toml_decode` は `xer::result<toml_value, parse_error_detail>` を返します。

解析失敗時、エラーオブジェクトは通常の xer エラーコードに加え、`<xer/parse.h>` の `offset`、`line`、`column`、`reason` フィールドを含みます。

位置フィールドは UTF-8 コード単位で数えられます。`line` と `column` は 1 始まり、`offset` は 0 始まりです。

---

## `toml_encode`

```cpp
auto toml_encode(const toml_value& value) -> xer::result<std::u8string>;
```

### 目的

`toml_encode` は、サポートされる TOML 値モデルを UTF-8 TOML テキストへシリアライズします。

### 入力モデル

エンコードする値はテーブル値でなければなりません。

### 出力モデル

成功時は次を返します。

```cpp
std::u8string
```

生成されるテキストは `\n` 改行を使用します。

### シリアライズ形式

エンコーダーは、通常のキー値エントリーを先に出力します。

```toml
title = "sample"
enabled = true
count = 123
```

その後、テーブルセクションを出力します。

```toml
[project]
name = "xer"
version = "example"
```

先行する出力がある場合、テーブルの前に空行が挿入されます。

### 表現可能性

現在の TOML 実装はサブセットのみをサポートするため、`toml_encode` はそのサブセットで表現できない値を拒否します。

たとえば、次を拒否します。

* トップレベル値がテーブルではない
* サポートされないキー形式
* 不正な UTF-8 文字列

---

## エラー処理

`<xer/toml.h>` は xer の通常の失敗モデルに従います。

つまり、次のようになります。

* 解析失敗は `xer::result` を通じて報告されます
* シリアライズ失敗は `xer::result` を通じて報告されます
* 通常の失敗は、既定では例外として表現されません

通常のパターンは次のとおりです。

```cpp
const auto decoded = xer::toml_decode(text);
if (!decoded.has_value()) {
    return 1;
}
```

これは、失敗可能な公開 API は `xer::result` を返し、通常の公開 API は `xer::result` 引数ではなく通常の値を受け取るべきである、という xer の一般方針に従っています。

---

## 他のヘッダーとの関係

`<xer/toml.h>` は、次の文書およびヘッダーと合わせて理解する必要があります。

* `policy_project_outline.md`
* `policy_result_arguments.md`
* `policy_encoding.md`
* `header_string.md`
* `header_json.md`
* `header_ini.md`

おおまかな境界は次のとおりです。

* `<xer/string.h>` は一般的な文字列およびテキストユーティリティを扱います
* `<xer/json.h>` は構造化データ形式としての JSON を扱います
* `<xer/ini.h>` は単純な文字列値設定データ形式としての INI を扱います
* `<xer/toml.h>` は型付き設定データ形式としての TOML を扱います

この分離は意図的なものです。
TOML は、文字列ヘルパーやストリームヘルパーではなく、独立したデータ形式機能として扱われます。

---

## ドキュメント上の注意

このヘッダーを生成ドキュメントで扱う場合、通常は次を説明すれば十分です。

* TOML 処理は UTF-8 テキストを使用すること
* 現在の実装は実用的な TOML サブセットであること
* トップレベルのデコード結果はテーブル値であること
* TOML 値は型付きであること
* 重複キーと重複テーブルは拒否されること
* 16進、8進、2進整数がサポートされること
* 数値区切り文字は数字間でのみサポートされること
* 残る制限事項は実用的なサブセットの制限として説明すべきであること

サポート機能の詳細は、サポートが拡張されるたびに実装と同期しておく必要があります。

---

## 例として示す価値がある話題

このヘッダーには、次のような例が特に適しています。

* 単純な TOML テキストのデコード
* トップレベルテーブルから値を読む
* セクションテーブルから値を読む
* `toml_value` テーブルを TOML テキストへエンコードする

これらは `examples/` の実行可能な例に適した候補です。

---

## 例

```cpp
#include <xer/toml.h>

auto main() -> int
{
    const auto decoded = xer::toml_decode(
        u8"title = \"sample\"\n"
        u8"enabled = true\n"
        u8"\n"
        u8"[project]\n"
        u8"name = \"xer\"\n"
        u8"version = \"example\"\n");

    if (!decoded.has_value()) {
        return 1;
    }

    const auto* root = decoded->as_table();
    if (root == nullptr) {
        return 1;
    }

    const auto encoded = xer::toml_encode(*decoded);
    if (!encoded.has_value()) {
        return 1;
    }

    return 0;
}
```

この例は一般的なスタイルを示しています。

* `toml_decode` で UTF-8 TOML テキストを解析する
* 結果のトップレベルテーブルを調べる
* `toml_encode` で再びシリアライズする
* 失敗可能な各手順で `xer::result` を明示的に確認する

---

## 関連項目

* `policy_project_outline.md`
* `policy_result_arguments.md`
* `policy_encoding.md`
* `header_json.md`
* `header_ini.md`

---

## TOML の検索および読み込み/保存ヘルパー

このヘッダーは、デコード済み TOML 値を調べるためのヘルパー関数と、TOML ファイルを読み込みまたは保存するためのヘルパー関数も提供します。

```cpp
auto toml_find(toml_value& value, std::u8string_view path) noexcept
    -> toml_value*;

auto toml_find(const toml_value& value, std::u8string_view path) noexcept
    -> const toml_value*;

auto toml_load(const path& filename)
    -> xer::result<toml_value, parse_error_detail>;

auto toml_save(const path& filename, const toml_value& value)
    -> xer::result<void>;
```

`toml_find` ヘルパーは、すでにデコードされたメモリー上の値を調べ、既存の値へのポインターを返します。要求された項目が存在しない場合、または検索対象の値の形が合わない場合は `nullptr` を返します。

`toml_find` の `path` 引数は、`project.name` や `database.port` のような単純なドット区切りの検索パスです。これは完全な TOML キーパーサーではありません。特に、TOML の引用符付きキー構文はこのヘルパーでは解釈されず、引用符付きのドットは検索パス内で特別扱いされません。

読み込みヘルパーは UTF-8 ファイル読み込みとデコードを組み合わせ、`xer::result<toml_value, parse_error_detail>` を返します。解析が始まる前にファイル I/O が失敗した場合、返されるエラーは `parse_error_reason::none` を使用し、`offset`、`line`、`column` は 0 のままにします。

保存ヘルパーはエンコードと UTF-8 ファイル書き込みを組み合わせ、`xer::result<void>` を返します。
