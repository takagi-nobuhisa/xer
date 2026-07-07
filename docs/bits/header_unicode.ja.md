<!-- xer-reference-source-sha256: 92229fefe4d0a2a51466306ee2c3996e5007ffafc62e65f42a2152baeadc97d2 -->

# `<xer/unicode.h>`

## 目的

`<xer/unicode.h>` は、実用的な Unicode ユーティリティを提供します。

現在の対象範囲は次のとおりです。

- `std::u8string_view` のコードポイント走査
- `std::u16string_view` のコードポイント走査
- `std::wstring_view` のコードポイント走査
- `std::u8string_view` の拡張書記素クラスタ走査
- `std::u16string_view` の拡張書記素クラスタ走査
- `std::wstring_view` の拡張書記素クラスタ走査
- `std::u8string_view` の書記素クラスタ単位の文字列操作
- `std::u16string_view` の書記素クラスタ単位の文字列操作
- `std::wstring_view` の書記素クラスタ単位の文字列操作
- コードポイントおよび単一書記素クラスタに対する実用的な絵文字判定
- UTF-8 テキストの NFC 正規化
- UTF-8 テキストの NFC 状態確認

コードポイント走査、書記素クラスタ走査、書記素クラスタ単位の文字列操作、および絵文字判定そのものは ICU を使用しません。NFC 正規化は ICU C API を使用します。

現時点では、`<xer/unicode.h>` は ICU ベースの正規化実装を含むため、この公開ヘッダーをインクルードするには ICU 開発用ヘッダーが利用可能である必要があります。

---

## 外部依存

`<xer/unicode.h>` は NFC 正規化ユーティリティを公開するため、ICU C API ヘッダーを必要とします。

```cpp
#include <unicode/utypes.h>
#include <unicode/ustring.h>
#include <unicode/unorm2.h>
```

正規化実装では、`u_strFromUTF8`、`u_strToUTF8`、`unorm2_getNFCInstance`、`unorm2_normalize`、`unorm2_isNormalized` などの ICU C API 関数を使用します。

xer はアプリケーションのビルドシステム設定を管理しません。ユーザーは、自分の環境に適したインクルードパスとリンクオプションを指定する必要があります。

たとえば、多くの Unix 系環境では、必要なリンクオプションを `pkg-config` で取得できます。

```bash
pkg-config --cflags --libs icu-uc
```

直接コンパイルするコマンドは、たとえば次のようになります。

```bash
g++ -std=c++23 -I. example.cpp -licuuc -licudata
```

MSYS2 環境では、ICU データライブラリ名が `icudata` ではなく `icudt` の場合があるため、典型的なコマンドは次のようになります。

```bash
g++ -std=c++23 -I. example.cpp -licuuc -licudt
```

xer のテストランナーは、既知の環境を個別に処理します。Visual Studio 2026 の clang-cl または MSVC cl.exe では、xer のテストとコード例は vcpkg manifest mode により `vcpkg_installed\x64-windows` にインストールされた ICU を使用します。

---

## 主なエンティティ

`<xer/unicode.h>` は、次のコードポイント走査型を提供します。

```cpp
struct code_point {
    std::size_t offset;
    std::size_t size;
    char32_t value;
};
```

`offset` および `size` フィールドは、元のコード単位で表されます。

- `std::u8string_view` では UTF-8 コード単位、実質的にはバイト
- `std::u16string_view` では UTF-16 コード単位
- `std::wstring_view` ではワイドコード単位

デコードされた `value` は、`char32_t` で表される Unicode スカラー値です。

このヘッダーは、次のデコード関数を提供します。

```cpp
auto next_code_point(std::u8string_view text, std::size_t offset = 0)
    -> xer::result<xer::code_point>;

auto prev_code_point(std::u8string_view text, std::size_t offset)
    -> xer::result<xer::code_point>;

auto next_code_point(std::u16string_view text, std::size_t offset = 0)
    -> xer::result<xer::code_point>;

auto prev_code_point(std::u16string_view text, std::size_t offset)
    -> xer::result<xer::code_point>;

auto next_code_point(std::wstring_view text, std::size_t offset = 0)
    -> xer::result<xer::code_point>;

auto prev_code_point(std::wstring_view text, std::size_t offset)
    -> xer::result<xer::code_point>;
```

また、範囲ヘルパーも提供します。

```cpp
auto code_points(std::u8string_view text)
    -> xer::code_point_range<char8_t>;

auto code_points(std::u16string_view text)
    -> xer::code_point_range<char16_t>;

auto code_points(std::wstring_view text)
    -> xer::code_point_range<wchar_t>;
```

範囲要素を参照解除した値は次の型です。

```cpp
xer::result<xer::code_point>
```

これにより、走査中の不正な入力が明示的になります。

`<xer/unicode.h>` は、次の書記素クラスタ走査型を提供します。

```cpp
struct grapheme_cluster {
    std::size_t offset;
    std::size_t size;
};
```

`offset` および `size` フィールドは、元のコード単位で表されます。書記素クラスタは複数のコードポイントを含むことがあるため、`grapheme_cluster` は意図的に `char32_t` 値を持ちません。

このヘッダーは、次の書記素クラスタ関数を提供します。

```cpp
auto next_grapheme_cluster(std::u8string_view text, std::size_t offset = 0)
    -> xer::result<xer::grapheme_cluster>;

auto prev_grapheme_cluster(std::u8string_view text, std::size_t offset)
    -> xer::result<xer::grapheme_cluster>;

auto next_grapheme_cluster(std::u16string_view text, std::size_t offset = 0)
    -> xer::result<xer::grapheme_cluster>;

auto prev_grapheme_cluster(std::u16string_view text, std::size_t offset)
    -> xer::result<xer::grapheme_cluster>;

auto next_grapheme_cluster(std::wstring_view text, std::size_t offset = 0)
    -> xer::result<xer::grapheme_cluster>;

auto prev_grapheme_cluster(std::wstring_view text, std::size_t offset)
    -> xer::result<xer::grapheme_cluster>;
```

また、範囲ヘルパーも提供します。

```cpp
auto grapheme_clusters(std::u8string_view text)
    -> xer::grapheme_cluster_range<char8_t>;

auto grapheme_clusters(std::u16string_view text)
    -> xer::grapheme_cluster_range<char16_t>;

auto grapheme_clusters(std::wstring_view text)
    -> xer::grapheme_cluster_range<wchar_t>;
```

範囲要素を参照解除した値は次の型です。

```cpp
xer::result<xer::grapheme_cluster>
```

これにより、走査中の不正な入力が明示的になります。

このヘッダーは、書記素クラスタ単位の文字列操作も提供します。

```cpp
auto grapheme_length(std::u8string_view text)
    -> xer::result<std::size_t>;

auto grapheme_substr(
    std::u8string_view text,
    std::size_t offset,
    std::size_t count = std::u8string_view::npos)
    -> xer::result<std::u8string_view>;

auto grapheme_left(std::u8string_view text, std::size_t count)
    -> xer::result<std::u8string_view>;

auto grapheme_right(std::u8string_view text, std::size_t count)
    -> xer::result<std::u8string_view>;
```

`std::u16string_view` および `std::wstring_view` のオーバーロードも提供されます。返される部分文字列値は元のテキストへのビューです。

このヘッダーは、実用的な絵文字判定も提供します。

```cpp
auto is_emoji(char32_t value) noexcept -> bool;

auto is_emoji(std::u8string_view text)
    -> xer::result<bool>;

auto is_emoji(std::u16string_view text)
    -> xer::result<bool>;

auto is_emoji(std::wstring_view text)
    -> xer::result<bool>;
```

文字列ビュー版は、入力全体が 1 個の実用的な絵文字書記素クラスタである場合にだけ `true` を返します。空入力は `false` を返します。不正な UTF-8、UTF-16、またはワイドテキストはエラーとして報告されます。

このヘッダーは NFC ユーティリティも提供します。

```cpp
auto normalize_nfc(std::u8string_view text)
    -> xer::result<std::u8string>;

auto is_normalized_nfc(std::u8string_view text)
    -> xer::result<bool>;
```

---

## `code_point`

```cpp
struct code_point {
    std::size_t offset;
    std::size_t size;
    char32_t value;
};
```

### 目的

`code_point` は、1 個のデコード済み Unicode スカラー値と、それに対応する元の範囲を表します。

元の文字列は所有しません。デコードされたコードポイントがどこで見つかり、元のコード単位をいくつ占めていたかだけを記録します。

### オフセットとサイズ

`offset` はコードポイントが始まるインデックスです。

`size` は、そのコードポイントが占める元のコード単位数です。

たとえば、UTF-8 テキストでは次のようになります。

```cpp
std::u8string_view text = u8"Aあ😀";
```

コードポイントは次のとおりです。

```text
offset=0 size=1 value=U+0041
offset=1 size=3 value=U+3042
offset=4 size=4 value=U+1F600
```

UTF-16 テキストでは、U+1F600 のような補助平面文字はサロゲートペアで表されるため、2 個のコード単位を占めます。

`std::wstring_view` では、単位はプラットフォームの `wchar_t` 表現に依存します。`wchar_t` が 16 ビットのプラットフォームでは、補助平面文字は 2 個のワイドコード単位を占めます。`wchar_t` が 32 ビットのプラットフォームでは、1 個のワイドコード単位を占めます。

---

## `next_code_point`

```cpp
auto next_code_point(std::u8string_view text, std::size_t offset = 0)
    -> xer::result<xer::code_point>;

auto next_code_point(std::u16string_view text, std::size_t offset = 0)
    -> xer::result<xer::code_point>;

auto next_code_point(std::wstring_view text, std::size_t offset = 0)
    -> xer::result<xer::code_point>;
```

### 目的

`next_code_point` は、`offset` から始まるコードポイントをデコードします。

### 入力モデル

`offset` 引数は、元の文字列ビューのコード単位で表されます。

入力は、対応するエンコーディングとして well-formed である必要があります。

- `std::u8string_view` は well-formed な UTF-8 を含んでいる必要があります
- `std::u16string_view` は well-formed な UTF-16 を含んでいる必要があります
- `std::wstring_view` はプラットフォームの `wchar_t` 幅に応じた well-formed なワイドテキストを含んでいる必要があります

### 戻り値モデル

戻り値の型は次のとおりです。

```cpp
xer::result<xer::code_point>
```

この関数は、次の場合に失敗する可能性があります。

- `offset` が文字列ビューの範囲外である
- `offset` が文字列ビューの末尾と等しい
- 入力に不正な UTF-8 が含まれている
- 入力に不正な UTF-16 サロゲートペアが含まれている
- デコードされた値が Unicode スカラー値ではない

範囲外のオフセットは `xer::error_t::out_of_range` として報告されます。
不正な符号化テキストは `xer::error_t::encoding_error` として報告されます。

### 例

```cpp
constexpr std::u8string_view text = u8"Aあ";

const auto first = xer::next_code_point(text, 0);
const auto second = xer::next_code_point(text, first->offset + first->size);
```

---

## `prev_code_point`

```cpp
auto prev_code_point(std::u8string_view text, std::size_t offset)
    -> xer::result<xer::code_point>;

auto prev_code_point(std::u16string_view text, std::size_t offset)
    -> xer::result<xer::code_point>;

auto prev_code_point(std::wstring_view text, std::size_t offset)
    -> xer::result<xer::code_point>;
```

### 目的

`prev_code_point` は、`offset` の直前にあるコードポイントをデコードします。

`offset` 引数は one-past 位置です。`text.size()` を渡すと、最後のコードポイントをデコードします。

### 戻り値モデル

戻り値の型は次のとおりです。

```cpp
xer::result<xer::code_point>
```

この関数は、次の場合に失敗する可能性があります。

- `offset` が文字列ビューの範囲外である
- `offset` が `0` である
- `offset` の直前のバイト列またはコード単位列が、ちょうど 1 個の well-formed なコードポイントを形成していない

### 例

```cpp
constexpr std::u8string_view text = u8"Aあ";

const auto last = xer::prev_code_point(text, text.size());
```

---

## `code_points`

```cpp
auto code_points(std::u8string_view text)
    -> xer::code_point_range<char8_t>;

auto code_points(std::u16string_view text)
    -> xer::code_point_range<char16_t>;

auto code_points(std::wstring_view text)
    -> xer::code_point_range<wchar_t>;
```

### 目的

`code_points` は、元のテキストを Unicode コードポイント単位で走査する軽量入力範囲を作成します。

### 要素型

イテレーターを参照解除した値は次の型です。

```cpp
const xer::result<xer::code_point>&
```

つまり、各要素は使用前に確認する必要があります。

不正なシーケンスはエラー要素として現れます。エラー後にイテレーターを進めると終端へ移動します。

### 例

```cpp
constexpr std::u8string_view text = u8"Aあ😀";

for (const auto& item : xer::code_points(text)) {
    if (!item.has_value()) {
        // malformed input
        break;
    }

    const xer::code_point cp = *item;
    // cp.offset, cp.size, cp.value
}
```

---

## 書記素クラスタとの関係

コードポイント走査は、書記素クラスタ走査とは同じではありません。

たとえば、次の組み合わせは 2 個のコードポイントですが、ユーザーから見た 1 文字になることがあります。

```text
U+304B HIRAGANA LETTER KA
U+3099 COMBINING KATAKANA-HIRAGANA VOICED SOUND MARK
```

コードポイント API は、これを意図的に 2 個のコードポイントとして公開します。

書記素クラスタ API はこの層の上に構築され、単一の `char32_t` ではなく元の文字列範囲を返します。これは、1 個の書記素クラスタが複数のコードポイントを含むことがあるためです。

---


## `grapheme_cluster`

```cpp
struct grapheme_cluster {
    std::size_t offset;
    std::size_t size;
};
```

### 目的

`grapheme_cluster` は、1 個の拡張書記素クラスタと、それに対応する元の範囲を表します。

元の文字列は所有しません。クラスタがどこで始まり、元のコード単位をいくつ占めているかだけを記録します。

`code_point` と異なり、`char32_t` 値は持ちません。ユーザーから見える 1 文字は、基底文字と結合記号、異体字セレクター付きの絵文字、または絵文字 ZWJ シーケンスのように、複数の Unicode コードポイントから構成されることがあります。

---

## `next_grapheme_cluster`

```cpp
auto next_grapheme_cluster(std::u8string_view text, std::size_t offset = 0)
    -> xer::result<xer::grapheme_cluster>;

auto next_grapheme_cluster(std::u16string_view text, std::size_t offset = 0)
    -> xer::result<xer::grapheme_cluster>;

auto next_grapheme_cluster(std::wstring_view text, std::size_t offset = 0)
    -> xer::result<xer::grapheme_cluster>;
```

### 目的

`next_grapheme_cluster` は、`offset` から始まる拡張書記素クラスタをデコードします。

実装はコードポイント走査層を基盤とし、結合記号、異体字セレクター、絵文字修飾子、絵文字 ZWJ シーケンス、地域指示記号ペア、CRLF、およびハングル音節シーケンスなど、実用的な拡張書記素クラスタ列をグループ化します。

### 戻り値モデル

戻り値の型は次のとおりです。

```cpp
xer::result<xer::grapheme_cluster>
```

この関数は、次の場合に失敗する可能性があります。

- `offset` が文字列ビューの範囲外である
- `offset` が文字列ビューの末尾と等しい
- 入力に不正な UTF-8 が含まれている
- 入力に不正な UTF-16 サロゲートペアが含まれている
- デコードされた値が Unicode スカラー値ではない

範囲外のオフセットは `xer::error_t::out_of_range` として報告されます。不正な符号化テキストは `xer::error_t::encoding_error` として報告されます。

### 例

```cpp
constexpr std::u8string_view text = u8"A\u0301B";

const auto first = xer::next_grapheme_cluster(text, 0);
// first->offset == 0
// first->size == 3
```

---

## `prev_grapheme_cluster`

```cpp
auto prev_grapheme_cluster(std::u8string_view text, std::size_t offset)
    -> xer::result<xer::grapheme_cluster>;

auto prev_grapheme_cluster(std::u16string_view text, std::size_t offset)
    -> xer::result<xer::grapheme_cluster>;

auto prev_grapheme_cluster(std::wstring_view text, std::size_t offset)
    -> xer::result<xer::grapheme_cluster>;
```

### 目的

`prev_grapheme_cluster` は、`offset` の直前にある拡張書記素クラスタをデコードします。

`offset` 引数は、クラスタ境界の one-past 位置です。`text.size()` を渡すと最後の書記素クラスタをデコードします。クラスタ内部のオフセットを渡した場合は `xer::error_t::encoding_error` として報告されます。

---

## `grapheme_clusters`

```cpp
auto grapheme_clusters(std::u8string_view text)
    -> xer::grapheme_cluster_range<char8_t>;

auto grapheme_clusters(std::u16string_view text)
    -> xer::grapheme_cluster_range<char16_t>;

auto grapheme_clusters(std::wstring_view text)
    -> xer::grapheme_cluster_range<wchar_t>;
```

### 目的

`grapheme_clusters` は、元のテキストを拡張書記素クラスタ単位で走査する軽量入力範囲を作成します。

参照解除された要素型は次のとおりです。

```cpp
xer::result<xer::grapheme_cluster>
```

### 例

```cpp
constexpr std::u8string_view text = u8"A\u0301B👩‍💻";

for (const auto& item : xer::grapheme_clusters(text)) {
    if (!item.has_value()) {
        // malformed input
        break;
    }

    // item->offset and item->size describe the cluster span.
}
```


---

## `grapheme_length`

```cpp
auto grapheme_length(std::u8string_view text)
    -> xer::result<std::size_t>;

auto grapheme_length(std::u16string_view text)
    -> xer::result<std::size_t>;

auto grapheme_length(std::wstring_view text)
    -> xer::result<std::size_t>;
```

### 目的

`grapheme_length` は、元の文字列ビューに含まれる拡張書記素クラスタ数を数えます。

これは、元のコード単位数を数える `text.size()` とは異なります。UTF-8 の日本語テキストでは、`text.size()` はバイト数です。`grapheme_length` は、xer の実用的な書記素クラスタ規則にもとづく、ユーザーから見える文字数を得るためのものです。

### 戻り値モデル

戻り値の型は次のとおりです。

```cpp
xer::result<std::size_t>
```

不正な UTF-8 または UTF-16 入力は `xer::error_t::encoding_error` として報告されます。

### 例

```cpp
constexpr std::u8string_view text = u8"A\u0301B👩‍💻";
const auto length = xer::grapheme_length(text);
// *length == 3
```

---

## `grapheme_substr`

```cpp
auto grapheme_substr(
    std::u8string_view text,
    std::size_t offset,
    std::size_t count = std::u8string_view::npos)
    -> xer::result<std::u8string_view>;

auto grapheme_substr(
    std::u16string_view text,
    std::size_t offset,
    std::size_t count = std::u16string_view::npos)
    -> xer::result<std::u16string_view>;

auto grapheme_substr(
    std::wstring_view text,
    std::size_t offset,
    std::size_t count = std::wstring_view::npos)
    -> xer::result<std::wstring_view>;
```

### 目的

`grapheme_substr` は、書記素クラスタのインデックスで選択した部分文字列ビューを返します。

`offset` および `count` パラメーターは、書記素クラスタ数です。バイト数でも UTF-16 コード単位数でもありません。返される値は元の文字列へのビューです。

`offset` が書記素クラスタ長と等しい場合、結果は入力末尾の空ビューになります。`offset` が書記素クラスタ長より大きい場合、この関数は `xer::error_t::out_of_range` を返します。

### 例

```cpp
constexpr std::u8string_view text = u8"A\u0301B👩‍💻C";
const auto part = xer::grapheme_substr(text, 1, 2);
// *part == u8"B👩‍💻"
```

---

## `grapheme_left`

```cpp
auto grapheme_left(std::u8string_view text, std::size_t count)
    -> xer::result<std::u8string_view>;

auto grapheme_left(std::u16string_view text, std::size_t count)
    -> xer::result<std::u16string_view>;

auto grapheme_left(std::wstring_view text, std::size_t count)
    -> xer::result<std::wstring_view>;
```

### 目的

`grapheme_left` は、先頭から `count` 個の書記素クラスタを、元の文字列へのビューとして返します。`count` が書記素クラスタ長より大きい場合は、入力全体のビューを返します。

---

## `grapheme_right`

```cpp
auto grapheme_right(std::u8string_view text, std::size_t count)
    -> xer::result<std::u8string_view>;

auto grapheme_right(std::u16string_view text, std::size_t count)
    -> xer::result<std::u16string_view>;

auto grapheme_right(std::wstring_view text, std::size_t count)
    -> xer::result<std::wstring_view>;
```

### 目的

`grapheme_right` は、末尾から `count` 個の書記素クラスタを、元の文字列へのビューとして返します。`count` が書記素クラスタ長より大きい場合は、入力全体のビューを返します。

`grapheme_right` は最初に書記素クラスタ長を求めるため、元のビュー内のどこかに不正な入力があれば報告されます。

### 例

```cpp
constexpr std::u8string_view text = u8"A\u0301B👩‍💻C";
const auto right = xer::grapheme_right(text, 2);
// *right == u8"👩‍💻C"
```

---

## `is_emoji`

```cpp
auto is_emoji(char32_t value) noexcept -> bool;

auto is_emoji(std::u8string_view text)
    -> xer::result<bool>;

auto is_emoji(std::u16string_view text)
    -> xer::result<bool>;

auto is_emoji(std::wstring_view text)
    -> xer::result<bool>;
```

### 目的

`is_emoji` は、英語・日本語のユーザー向けテキストで実用的に使える絵文字判定を提供します。

`char32_t` オーバーロードは、Unicode スカラー値が xer で絵文字基底として扱われるかどうかを調べます。これはコードポイント分類をすばやく行うためのものです。

文字列ビュー版は、入力全体が 1 個の絵文字書記素クラスタであるかどうかを調べます。旗、キーキャップ絵文字、異体字セレクター付き絵文字、肌色修飾子、ZWJ 絵文字シーケンスなどには、このオーバーロードを使用します。

### 入力モデル

文字列ビュー版は UTF-8、UTF-16、またはワイドテキストを受け取ります。`true` を返すには、入力がちょうど 1 個の書記素クラスタを含んでいる必要があります。空入力は `false` を返します。

### 戻り値モデル

`char32_t` オーバーロードは `bool` を返し、エラーを報告しません。

文字列ビュー版は次を返します。

```cpp
xer::result<bool>
```

入力に不正な UTF-8、UTF-16、またはワイドテキストが含まれている場合、失敗する可能性があります。

### 対象範囲

これは小さく実用的な判定器であり、すべての Unicode 絵文字プロパティを生成テーブルで完全実装したものではありません。xer の既存の書記素クラスタ処理を再利用し、英語・日本語テキストでよく使われる絵文字を対象にします。対象には、絵文字ピクトグラフ、旗、キーキャップ絵文字、異体字セレクター形式、絵文字修飾子、ZWJ シーケンスが含まれます。

### 例

```cpp
const auto face = xer::is_emoji(std::u8string_view{u8"😀"});
const auto worker = xer::is_emoji(std::u8string_view{u8"👩‍💻"});
const auto letter = xer::is_emoji(std::u8string_view{u8"A"});
```

この例では、`face` と `worker` は `true` を含み、`letter` は `false` を含むことが期待されます。

---

## `normalize_nfc`

```cpp
auto normalize_nfc(std::u8string_view text)
    -> xer::result<std::u8string>;
```

### 目的

`normalize_nfc` は、有効な UTF-8 テキストを Unicode 正規化形式 C に変換します。

NFC は、まず正準分解を適用し、その後に正準合成を行う正規化形式です。正準等価なテキストについて、合成済み文字が利用可能な場合に一貫したバイト表現へそろえるために有用です。

たとえば、次のシーケンスは、

```text
U+304B HIRAGANA LETTER KA
U+3099 COMBINING KATAKANA-HIRAGANA VOICED SOUND MARK
```

次のように正規化できます。

```text
U+304C HIRAGANA LETTER GA
```

同様に、次のシーケンスは、

```text
U+0041 LATIN CAPITAL LETTER A
U+030A COMBINING RING ABOVE
```

次のように正規化できます。

```text
U+00C5 LATIN CAPITAL LETTER A WITH RING ABOVE
```

### 入力モデル

入力は次の型で与えます。

```cpp
std::u8string_view
```

入力は有効な UTF-8 である必要があります。無効な UTF-8 はエラーとして報告されます。

### 出力モデル

成功時、この関数は次を返します。

```cpp
std::u8string
```

返される文字列は、NFC に正規化された有効な UTF-8 です。

### 戻り値モデル

戻り値の型は次のとおりです。

```cpp
xer::result<std::u8string>
```

この関数は、次の場合に失敗する可能性があります。

- 入力が有効な UTF-8 ではない
- 入力または出力サイズが ICU C API の長さパラメーターで受け付けられる範囲を超えている
- ICU が内部エラーを報告した

典型的な無効 UTF-8 入力は `xer::error_t::encoding_error` として報告されます。

### 例

```cpp
const auto result = xer::normalize_nfc(u8"か\u3099");
if (result.has_value()) {
    // *result is u8"が"
}
```

---

## `is_normalized_nfc`

```cpp
auto is_normalized_nfc(std::u8string_view text)
    -> xer::result<bool>;
```

### 目的

`is_normalized_nfc` は、有効な UTF-8 テキストがすでに Unicode NFC に正規化されているかどうかを確認します。

これは、すでに目的の正規化形式になっているテキストを書き換えずに済ませたい場合に有用です。

### 入力モデル

入力は次の型で与えます。

```cpp
std::u8string_view
```

入力は有効な UTF-8 である必要があります。無効な UTF-8 はエラーとして報告されます。

### 出力モデル

成功時、この関数は次を返します。

```cpp
bool
```

値は、入力がすでに NFC であれば `true`、そうでなければ `false` です。

### 戻り値モデル

戻り値の型は次のとおりです。

```cpp
xer::result<bool>
```

この関数は、次の場合に失敗する可能性があります。

- 入力が有効な UTF-8 ではない
- 入力サイズが ICU C API の長さパラメーターで受け付けられる範囲を超えている
- ICU が失敗を報告した

典型的な無効 UTF-8 入力は `xer::error_t::encoding_error` として報告されます。

### 例

```cpp
const auto before = xer::is_normalized_nfc(u8"か\u3099");
const auto after = xer::is_normalized_nfc(u8"が");
```

この例では、`before` は `false` を含み、`after` は `true` を含むことが期待されます。

---

## 設計メモ

`<xer/unicode.h>` は、`<xer/string.h>` のような通常の文字列処理ヘッダーから意図的に独立しています。

コードポイント走査層は小さく、テーブルを使いません。UTF-8 および UTF-16 構造を検証し、不正な入力を `xer::result` で報告し、元の範囲をコード単位で記録します。

書記素クラスタ走査層は、コードポイント層の上に構築されています。これは、実用的なユーザー可視文字の走査を目的としつつ、テキストをコピーせずに元の範囲を返します。書記素クラスタ単位の文字列操作は、長さおよび部分文字列の便利なヘルパーを提供しつつ、元のテキストへのビューを返します。絵文字判定も同じコードポイント層と書記素クラスタ層の上に構築されるため、複数コードポイントからなる絵文字シーケンスを 1 個のユーザー可視単位として確認できます。

この層の既定の言語対象は英語および日本語テキストです。結合記号、異体字セレクター、絵文字修飾子、絵文字 ZWJ シーケンス、地域指示記号ペア、CRLF、ハングル音節シーケンスなど、実用的な英語・日本語のユーザー向けテキストに必要な一般的シーケンスを扱います。あらゆる文字体系に対応する完全な Unicode テキスト境界エンジンではなく、言語固有の tailoring も提供しません。より広い文字体系への対応が必要なユーザーは、xer の公開ソースコードを拡張するか、専用の Unicode 境界サービスを使用できます。

Unicode 正規化は Unicode 正規化データを必要とするため、単純な UTF-8 文字列ユーティリティより重い処理です。xer はヘッダーオンリーライブラリ内に大きな生成済み Unicode テーブルを埋め込むのではなく、この責務を ICU に委譲します。

初期の正規化 API は NFC のみを公開します。NFC は、ファイル名、検索キー、辞書、テキスト整形など、多くの実用的な用途で最も使いやすい正規化形式だからです。その他の正規化形式は、基本的な API 形状を変更せずに後から追加できます。

---
