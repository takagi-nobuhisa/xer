<!-- xer-reference-source-sha256: 8e0fa4c7d079edaf345644d7a6950162c8953c890de8c846ab713ceb9110e90f -->
# `<xer/unicode_normalize.h>`

## 目的

`<xer/unicode_normalize.h>` は、Unicode 正規化機能を提供します。

現在の公開 API は、UTF-8 テキストの NFC 正規化と NFC 判定に限定されています。

このヘッダーは ICU C API を使用します。正規化データは大きく、ヘッダーオンリーライブラリ内に独自の Unicode 正規化テーブルを埋め込むのは現実的ではないためです。

---

## 主なエンティティ

```cpp
auto normalize_nfc(std::u8string_view text)
    -> xer::result<std::u8string>;

auto is_normalized_nfc(std::u8string_view text)
    -> xer::result<bool>;
```

---

## 外部依存関係

このヘッダーは ICU C API ヘッダーを必要とします。

```cpp
#include <unicode/utypes.h>
#include <unicode/ustring.h>
#include <unicode/unorm2.h>
```

実装は、たとえば次の ICU C API 関数を使用します。

- `u_strFromUTF8`
- `u_strToUTF8`
- `unorm2_getNFCInstance`
- `unorm2_normalize`
- `unorm2_isNormalized`

xer はアプリケーションのビルドシステム設定を管理しません。利用者は自分の環境に合ったインクルードパスとリンクオプションを指定する必要があります。

多くの Unix 系環境では、必要なオプションは `pkg-config` から取得できます。

```bash
pkg-config --cflags --libs icu-uc
```

直接コンパイルする場合の例です。

```bash
g++ -std=c++23 -I. example.cpp -licuuc -licudata
```

MSYS2 環境では、ICU データライブラリ名が `icudata` ではなく `icudt` である場合があるため、典型的には次のようになります。

```bash
g++ -std=c++23 -I. example.cpp -licuuc -licudt
```

xer のテストランナーは既知の環境を個別に扱います。

---

## `normalize_nfc`

```cpp
auto normalize_nfc(std::u8string_view text)
    -> xer::result<std::u8string>;
```

### 目的

`normalize_nfc` は、有効な UTF-8 テキストを Unicode Normalization Form C へ変換します。

NFC は、正準分解を行ったあと正準合成を行う正規化形式です。合成済み文字が利用できる場合に、正準等価なテキストを一貫したバイト表現へそろえる用途に適しています。

たとえば次の並びは、

```text
U+304B HIRAGANA LETTER KA
U+3099 COMBINING KATAKANA-HIRAGANA VOICED SOUND MARK
```

次へ正規化できます。

```text
U+304C HIRAGANA LETTER GA
```

同様に、次の並びは、

```text
U+0041 LATIN CAPITAL LETTER A
U+030A COMBINING RING ABOVE
```

次へ正規化できます。

```text
U+00C5 LATIN CAPITAL LETTER A WITH RING ABOVE
```

### 入力モデル

入力は `std::u8string_view` として渡します。

入力は有効な UTF-8 でなければなりません。不正な UTF-8 はエラーとして報告されます。

### 出力モデル

成功時、関数は `std::u8string` を返します。

返される文字列は NFC に正規化された有効な UTF-8 です。

### 戻り値モデル

戻り値の型は `xer::result<std::u8string>` です。

この関数は次の場合に失敗することがあります。

- 入力が有効な UTF-8 ではない
- 入力または出力のサイズが ICU C API の長さパラメータで扱える範囲を超える
- ICU が内部エラーを報告する

典型的な不正 UTF-8 入力は `xer::error_t::encoding_error` として報告されます。

### 例

```cpp
const auto result = xer::normalize_nfc(u8"か\u3099");
if (result.has_value()) {
    // *result は u8"が"
}
```

---

## `is_normalized_nfc`

```cpp
auto is_normalized_nfc(std::u8string_view text)
    -> xer::result<bool>;
```

### 目的

`is_normalized_nfc` は、有効な UTF-8 テキストがすでに Unicode NFC に正規化されているかどうかを調べます。

これは、すでに望ましい正規化形式になっているテキストを書き換えずに済ませたい場合に有用です。

### 入力モデル

入力は `std::u8string_view` として渡します。

入力は有効な UTF-8 でなければなりません。不正な UTF-8 はエラーとして報告されます。

### 出力モデル

成功時、関数は `bool` を返します。

入力がすでに NFC であれば `true`、そうでなければ `false` です。

### 戻り値モデル

戻り値の型は `xer::result<bool>` です。

この関数は次の場合に失敗することがあります。

- 入力が有効な UTF-8 ではない
- 入力サイズが ICU C API の長さパラメータで扱える範囲を超える
- ICU がエラーを報告する

典型的な不正 UTF-8 入力は `xer::error_t::encoding_error` として報告されます。

### 例

```cpp
const auto before = xer::is_normalized_nfc(u8"か\u3099");
const auto after = xer::is_normalized_nfc(u8"が");
```

この例では、`before` は `false`、`after` は `true` を含むことが期待されます。

---

## 設計上の注意

Unicode 正規化は、単純な UTF-8 文字列ユーティリティよりも重い処理です。正規化には Unicode 正規化データが必要なため、xer は巨大な生成テーブルをヘッダーオンリーライブラリへ埋め込む代わりに ICU へ委譲します。

初期正規化 API は NFC のみを公開します。NFC は、ファイル名、検索キー、辞書、テキスト整備など多くの実用的な用途で最も使いやすい正規化形式だからです。ほかの正規化形式は、基本的な API 形状を変えずにあとから追加できます。

---

## `<xer/unicode.h>` との関係

`<xer/unicode.h>` は、現在この正規化機能も含む実用 Unicode ヘッダーです。

正規化機能だけを明示的に扱いたい場合は `<xer/unicode_normalize.h>` を参照します。コードポイント走査、書記素クラスタ走査、絵文字判定などの実用 Unicode 処理全体を使う場合は `<xer/unicode.h>` を使います。
