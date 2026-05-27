<!-- xer-reference-source-sha256: 6e713538a6903f34b58d2938b46888a1ca8abd0bf23b360a6a2967c78e21e3ea -->

# `<xer/parse.h>`

## 目的

`<xer/parse.h>` は、解析済み入力のエラーに使う共通の構造化詳細型を提供します。

このヘッダーは、JSON、INI、TOML、XBF ビットマップフォントデータ、その他の個別のパーサーやローダーから意図的に独立しています。解析失敗位置と解析失敗理由は複数の構造化入力 API で有用なので、xer は形式ごとの詳細型ではなく共通語彙を提供します。

---

## 主なエンティティ

少なくとも、`<xer/parse.h>` は次のエンティティを提供します。

```cpp
enum class xer::parse_error_reason;

struct xer::parse_error_detail;
```

`parse_error_detail` は `xer::result<T, Detail>` の `Detail` 引数として使うことを想定しています。

---

## `parse_error_reason`

`parse_error_reason` は、構造化入力の失敗に対する形式非依存の理由コードです。

現在の理由集合には次のものがあります。

```cpp
enum class parse_error_reason {
    none,
    invalid_syntax,
    invalid_encoding,
    invalid_token,
    invalid_key,
    duplicate_key,
    duplicate_table,
    invalid_string,
    invalid_escape,
    invalid_unicode_escape,
    invalid_number,
    integer_out_of_range,
    invalid_date_time,
    invalid_array,
    invalid_table,
    invalid_magic,
    unsupported_version,
    invalid_header,
    invalid_range,
    invalid_offset,
    truncated_input,
};
```

理由は `error_t` より詳しい分類です。たとえば次のように使います。

- テキストパーサーは `error_t::invalid_argument` と `parse_error_reason::duplicate_key` を組み合わせて使う場合があります。
- XBF ビットマップフォントローダーは `error_t::invalid_argument` と `parse_error_reason::invalid_magic` を組み合わせて使う場合があります。

### バイナリ構造向けの理由

次の理由は、固定ヘッダー、オフセット、範囲、有限のバイト範囲を持つバイナリ形式またはその他の構造化入力を対象としています。

- `invalid_magic`
- `unsupported_version`
- `invalid_header`
- `invalid_range`
- `invalid_offset`
- `truncated_input`

これらは現在、XBF ビットマップフォントデータの検証時に `xer::image::bitmap_font_load` で使われています。

---

## `parse_error_detail`

```cpp
struct parse_error_detail {
    std::size_t offset = 0;
    std::size_t line = 0;
    std::size_t column = 0;
    parse_error_reason reason = parse_error_reason::none;
};
```

### 位置の意味

`offset` は入力先頭からの 0 始まりの位置です。

- UTF-8 テキスト形式では、UTF-8 コード単位で数えます。
- バイナリ形式では、バイト単位で数えます。

`line` は行情報が利用できる場合の 1 始まりの行番号です。行情報が利用できない場合は `0` です。

`column` は列情報が利用できる場合の 1 始まりの列番号です。UTF-8 テキスト形式では、表示セル、Unicode スカラー値、書記素クラスタではなく、UTF-8 コード単位で数えます。列情報が利用できない場合は `0` です。

この規則により、パーサー診断を xer の `char8_t` ベースのテキストモデルに合わせつつ、バイナリローダーは擬似的な行番号や列番号を作らずに正確なバイト位置を報告できます。

---

## 想定される使い方

構造化詳細を報告できるテキストパーサーは、次のような結果を返せます。

```cpp
auto toml_decode(std::u8string_view text)
    -> xer::result<toml_value, parse_error_detail>;
```

バイナリローダーも同じ詳細型を使えます。

```cpp
auto bitmap_font_load(const xer::path& filename)
    -> xer::result<xer::image::bitmap_font, parse_error_detail>;
```

解析または読み込みが失敗した場合、呼び出し側は共通のエラーコードと解析固有の詳細の両方を調べられます。

```cpp
const auto result = xer::toml_decode(text);
if (!result.has_value()) {
    const auto& error = result.error();
    // error.code
    // error.offset
    // error.line
    // error.column
    // error.reason
}
```

`error<Detail>` はクラス型の詳細を自然に公開するため、`offset`、`line`、`column`、`reason` は返されたエラーオブジェクトから直接利用できます。

---

## 他のヘッダーとの関係

`<xer/parse.h>` は次のヘッダーと関係します。

- `<xer/error.h>`
- `<xer/json.h>`
- `<xer/ini.h>`
- `<xer/toml.h>`
- `<xer/image.h>`

大まかな境界は次のとおりです。

- `<xer/error.h>` は共通のエラーコードと結果モデルを提供します。
- `<xer/parse.h>` は構造化入力の失敗に対する共通の解析詳細を提供します。
- 個別の形式ヘッダーは、その形式固有の値型とパーサーを提供します。

---

## ドキュメント上の注意

このヘッダーを生成マニュアルで参照する場合、通常は次の点を説明すれば十分です。

- `parse_error_detail` は解析失敗位置と理由を保持すること
- `line` と `column` は利用できない場合 `0` であること
- UTF-8 テキストの `offset` と `column` は UTF-8 コード単位で数えること
- 同じ詳細型をテキストパーサーとバイナリローダーの両方で使えること

詳細な形式固有の構文エラー規則は、それぞれの形式ヘッダーの説明に置くべきです。
