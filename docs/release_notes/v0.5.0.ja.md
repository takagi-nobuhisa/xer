# xer C++ Utility Library v0.5.0 Release Notes

xer C++ Utility Library v0.5.0 は、Unicode 処理、日本語処理、バイナリデータ補助を中心に機能を拡充したリリースです。

このリリースでは、UTF-8/UTF-16/ワイド文字列のコードポイント走査、書記素クラスタ走査、書記素クラスタ単位の文字列操作、実用的な絵文字判定、ICU を用いた NFC 正規化を追加しました。あわせて、日本語文字種判定、仮名変換、仮名正規化、全角・半角変換、バイナリと 16 進文字列の相互変換、MD5/SHA-1/SHA-256 などを整備しています。

## Highlights

- `<xer/unicode.h>` を追加し、実用的な Unicode 処理を強化しました。
- `<xer/ja.h>` を追加し、日本語固有の API を `xer::ja` 名前空間に整理しました。
- `<xer/binary.h>` に `bin2hex`、`hex2bin`、`md5`、`sha1`、`sha256` を追加しました。
- `xer::ctype_id` と `xer::ctrans_id` を拡張し、全角・半角・仮名関連の文字種判定と変換を強化しました。
- MeCab 連携、日本語分かち書き、ローマ字変換、点字変換まわりの実用性を高めました。
- README、公開ヘッダ一覧、リファレンスマニュアル、方針文書を現状に合わせて更新しました。
- サポート対象環境を整理し、MSYS2 MSYS および MSYS2 MINGW64 を対象外として明記しました。

## New Public Header

### `<xer/unicode.h>`

新しい公開ヘッダとして `<xer/unicode.h>` を追加しました。

主な機能は次のとおりです。

- Unicode コードポイント走査
- 拡張書記素クラスタ走査
- 書記素クラスタ単位の文字列長取得
- 書記素クラスタ単位の部分文字列取得
- 実用的な絵文字判定
- UTF-8 テキストの NFC 正規化
- UTF-8 テキストの NFC 正規化済み判定

コードポイント走査、書記素クラスタ走査、書記素クラスタ単位の文字列操作、絵文字判定は ICU に依存しません。NFC 正規化および NFC 判定は ICU C API を使用します。

現時点では `<xer/unicode.h>` が NFC 正規化 API を含むため、この公開ヘッダを利用する場合は ICU の開発用ヘッダとリンク設定が必要です。

## Unicode Utilities

### Code Point Traversal

`std::u8string_view`、`std::u16string_view`、`std::wstring_view` に対するコードポイント走査を追加しました。

主な API は次のとおりです。

```cpp
auto next_code_point(std::u8string_view text, std::size_t offset = 0)
    -> xer::result<xer::code_point>;

auto prev_code_point(std::u8string_view text, std::size_t offset)
    -> xer::result<xer::code_point>;

auto code_points(std::u8string_view text)
    -> xer::code_point_range<char8_t>;
```

`std::u16string_view` と `std::wstring_view` についても同様のオーバーロードを提供します。

不正な UTF-8、UTF-16、ワイド文字列は `xer::result` によって明示的なエラーとして扱います。

### Grapheme Cluster Traversal

拡張書記素クラスタを単位とした走査を追加しました。

主な API は次のとおりです。

```cpp
auto next_grapheme_cluster(std::u8string_view text, std::size_t offset = 0)
    -> xer::result<xer::grapheme_cluster>;

auto prev_grapheme_cluster(std::u8string_view text, std::size_t offset)
    -> xer::result<xer::grapheme_cluster>;

auto grapheme_clusters(std::u8string_view text)
    -> xer::grapheme_cluster_range<char8_t>;
```

`std::u16string_view` と `std::wstring_view` についても同様のオーバーロードを提供します。

### Grapheme-Cluster-Based String Operations

書記素クラスタ単位の文字列操作を追加しました。

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

`std::u16string_view` と `std::wstring_view` についても同様のオーバーロードを提供します。

### Emoji Detection

実用的な絵文字判定を追加しました。

```cpp
auto is_emoji(char32_t value) noexcept -> bool;

auto is_emoji(std::u8string_view text)
    -> xer::result<bool>;
```

文字列版は、入力全体が 1 個の実用的な絵文字書記素クラスタである場合に `true` を返します。空文字列は `false` です。

### NFC Normalization

ICU C API を用いた NFC 正規化と NFC 判定を追加しました。

```cpp
auto normalize_nfc(std::u8string_view text)
    -> xer::result<std::u8string>;

auto is_normalized_nfc(std::u8string_view text)
    -> xer::result<bool>;
```

現時点では、最も実用頻度の高い NFC のみを提供します。NFD、NFKC、NFKD などは将来の拡張候補です。

## Japanese Text Utilities

### `<xer/ja.h>`

日本語固有の API をまとめる利便ヘッダとして `<xer/ja.h>` を追加しました。

`<xer/ja.h>` は次の機能群をまとめて利用するための公開ヘッダです。

- ふりがな整形
- 漢数字変換
- MeCab 連携
- 日本語文字種判定
- 日本語用漢字分類
- 仮名変換
- 仮名正規化

日本語固有の API は `xer::ja` 名前空間に配置します。

### Japanese Character Classification

日本語文字種判定を追加・拡充しました。

主な API は次のとおりです。

```cpp
xer::ja::is_hiragana(U'あ');
xer::ja::is_katakana(U'ア');
xer::ja::is_kana(U'ｱ');
xer::ja::is_kanji(U'漢');
xer::ja::is_japanese_punctuation(U'。');
xer::ja::is_japanese(U'日');
```

UTF-8 文字列に対して、該当する文字種を 1 文字以上含むかどうかを調べる `contains_*` 系も追加しました。

```cpp
xer::ja::contains_hiragana(u8"abcあ");
xer::ja::contains_katakana(u8"abcア");
xer::ja::contains_kana(u8"abcｱ");
xer::ja::contains_kanji(u8"abc漢");
xer::ja::contains_japanese(u8"hello日本語");
```

また、文字列全体がひらがな、カタカナ、仮名で構成されているかを調べる `is_all_*` 系も追加しました。

```cpp
xer::ja::is_all_hiragana(u8"こんにちはー");
xer::ja::is_all_katakana(u8"コンニチハー");
xer::ja::is_all_kana(u8"こんにちはコンニチハー");
```

これらの文字列 API は、空文字列に対して `false` を返します。不正な UTF-8 入力は `encoding_error` として報告します。

### Kanji Classification

日本語用の漢字分類テーブルを追加しました。

主な分類は次のとおりです。

- 人名用漢字
- 常用漢字
- 教育漢字
- JIS 第1水準から第4水準の漢字

主な API は次のとおりです。

```cpp
xer::ja::is_name_kanji(U'凜');
xer::ja::is_jouyou_kanji(U'鬱');
xer::ja::is_kyouiku_kanji(U'日');
xer::ja::jis_kanji_level_of(U'亜');
```

### Kana Conversion and Normalization

ひらがな・カタカナ変換と、実用的な仮名正規化を追加しました。

```cpp
auto to_hiragana(std::u8string_view text)
    -> xer::result<std::u8string>;

auto to_katakana(std::u8string_view text)
    -> xer::result<std::u8string>;

auto normalize_kana(std::u8string_view text)
    -> xer::result<std::u8string>;
```

`normalize_kana` は、半角カタカナを全角カタカナに変換し、合成可能な濁点・半濁点を合成します。ひらがな・カタカナの文字種は可能な範囲で維持します。

## Character Classification and Conversion

`<xer/ctype.h>` の文字種判定と文字変換を拡張しました。

### Fullwidth and Halfwidth Classification

`ctype_id` に全角・半角関連の分類を追加しました。

主な分類は次のとおりです。

```cpp
ctype_id::fullwidth_kana
ctype_id::halfwidth_kana
ctype_id::fullwidth_digit
ctype_id::halfwidth_digit
ctype_id::fullwidth_alpha
ctype_id::halfwidth_alpha
ctype_id::fullwidth_punct
ctype_id::halfwidth_punct
ctype_id::fullwidth_space
ctype_id::halfwidth_space
ctype_id::fullwidth_graph
ctype_id::halfwidth_graph
ctype_id::fullwidth_print
ctype_id::halfwidth_print
ctype_id::fullwidth
ctype_id::halfwidth
```

### Fullwidth and Halfwidth Conversion

`ctrans_id` に全角・半角変換、ひらがな・カタカナ変換関連の分類を追加しました。

主な分類は次のとおりです。

```cpp
ctrans_id::fullwidth_kana
ctrans_id::halfwidth_kana
ctrans_id::fullwidth_digit
ctrans_id::halfwidth_digit
ctrans_id::fullwidth_alpha
ctrans_id::halfwidth_alpha
ctrans_id::fullwidth_punct
ctrans_id::halfwidth_punct
ctrans_id::fullwidth_space
ctrans_id::halfwidth_space
ctrans_id::fullwidth_graph
ctrans_id::halfwidth_graph
ctrans_id::fullwidth_print
ctrans_id::halfwidth_print
ctrans_id::fullwidth
ctrans_id::halfwidth
ctrans_id::katakana
ctrans_id::hiragana
```

全角・半角変換は、日本語テキスト処理での実用を主目的としています。ひらがなは全角・半角変換の対象外です。

現在の `toctrans` は 1 入力コードポイントから 1 出力コードポイントを返すモデルです。そのため、濁点・半濁点付きの全角カタカナを半角カタカナへ変換する場合、1 文字で表現できない情報は保持できないことがあります。文字列単位で正確に扱う必要がある場合は、`strtoctrans` や `xer::ja::normalize_kana` の利用を検討してください。

## Binary Data Utilities

`<xer/binary.h>` のバイナリデータ補助を拡充しました。

### Hex Conversion

バイナリデータと 16 進文字列の相互変換を追加しました。

```cpp
auto bin2hex(std::span<const std::byte> bytes)
    -> std::u8string;

auto bin2hex(const void* data, std::size_t size) noexcept
    -> std::u8string;

template<std::input_iterator InputIt>
auto bin2hex(InputIt first, InputIt last)
    -> std::u8string;

auto hex2bin(std::u8string_view hex)
    -> xer::result<std::vector<std::byte>>;
```

`bin2hex` は小文字の 16 進文字列を返します。

`hex2bin` は `0` から `9`、`a` から `f`、`A` から `F` を受け付けます。入力長は偶数である必要があります。

### Hash Functions

MD5、SHA-1、SHA-256 を追加しました。

```cpp
auto md5(std::span<const std::byte> bytes) noexcept
    -> std::array<std::byte, 16>;

auto sha1(std::span<const std::byte> bytes) noexcept
    -> std::array<std::byte, 20>;

auto sha256(std::span<const std::byte> bytes) noexcept
    -> std::array<std::byte, 32>;
```

`const void*` とサイズを受け取るオーバーロード、反復子範囲を受け取るテンプレート、ファイルパスを受け取るオーバーロードも提供します。

返却値は 16 進文字列ではなく、生のダイジェストバイト列です。一般的な 16 進表現が必要な場合は `bin2hex` と組み合わせて使用します。

```cpp
const auto digest = xer::sha256(bytes);
const auto hex = xer::bin2hex(digest.begin(), digest.end());
```

## MeCab, Romaji, and Braille Utilities

MeCab 連携、日本語分かち書き、ローマ字変換、点字変換まわりの機能とドキュメントを拡充しました。

主な強化点は次のとおりです。

- MeCab の読みを利用した仮名分かち書き
- MeCab の読みを利用したローマ字分かち書き
- 文節風の分割補助
- 記号範囲を考慮した実用的な分割
- 点字向け分かち書き
- MeCab 連携による点字変換
- 点字関連のテストとドキュメント整備

点字関連機能は、実用的な部品群として提供します。完全な点訳エンジンを名乗るものではありません。MeCab 連携の変換結果は、外部 MeCab 辞書が返す読みに依存します。

## Documentation

次のドキュメントを中心に、v0.5.0 の実態に合わせて更新しました。

- `README.md`
- `README.ja.md`
- `docs/public_headers.md`
- `docs/policy_project_outline.md`
- `docs/policy_external_components.md`
- `docs/policy_unicode_normalize.md`
- `docs/bits/header_unicode.md`
- `docs/bits/header_ja.md`
- `docs/bits/header_binary.md`
- `docs/bits/header_ctype.md`
- `docs/bits/header_mecab.md`
- `docs/bits/header_braille.md`

リファレンスマニュアルも v0.5.0 に合わせて再生成対象です。

## Supported Environments

v0.5.0 時点の主なサポート対象およびテスト対象環境は次のとおりです。

- Ubuntu
- MSYS2 UCRT64

対象プラットフォームの範囲は次のとおりです。

- Ubuntu による Linux
- MSYS2 UCRT64 による Windows

Windows の対象バージョンは Windows 11 以降です。

次の環境は、現在および今後の予定されているテスト対象には含めません。

- MSYS2 MSYS
- MSYS2 MINGW64

将来あらためて必要性が明確になった場合は、その時点で再検討します。

## External Dependencies

xer はヘッダオンリライブラリですが、一部機能は外部コンポーネントに依存します。

- NFC 正規化: ICU C API
- MeCab 連携: MeCab 実行環境
- Tcl/Tk 連携: Tcl/Tk 開発環境およびリンク設定
- ソケット関連: 対象環境に応じたシステムライブラリ

外部コンポーネントが必要な機能は、開発用テストスクリプト側でも環境に応じて扱います。

## Compatibility Notes

### `<xer/unicode.h>` Requires ICU Headers

`<xer/unicode.h>` は NFC 正規化 API を含むため、現時点では ICU C API のヘッダが必要です。Unicode のコードポイント走査や書記素クラスタ走査のみを使う場合でも、公開ヘッダ単位では ICU ヘッダを要求します。

将来的には、ICU 依存部分をより細かく分離する可能性があります。

### Fullwidth/Halfwidth Conversion Semantics Were Clarified

全角・半角変換は、数字・英字・句読点・空白に加えて、実用的な日本語テキスト処理を意識してカタカナも扱います。

ただし、`toctrans` は 1 入力コードポイントから 1 出力コードポイントを返すため、複数コードポイントが必要な変換には制約があります。

### MSYS2 MSYS and MSYS2 MINGW64 Are Not Supported Targets

MSYS2 MSYS および MSYS2 MINGW64 は、v0.5.0 時点のサポート対象外です。MSYS2 環境では UCRT64 を対象とします。

## Notes for Release Preparation

正式版としてタグ付けする前に、`xer/version.h` のバージョン接尾辞を正式版用に更新してください。

```cpp
#define XER_VERSION_SUFFIX ""
#define XER_VERSION_STRING "0.5.0"
```

リファレンスマニュアルの対象バージョン表記も `v0.5.0` に更新してから再生成してください。

## Summary

v0.5.0 は、xer の Unicode・日本語処理まわりを大きく前進させるリリースです。

特に、コードポイント、書記素クラスタ、絵文字、NFC 正規化、仮名処理、漢字分類、16 進変換、ハッシュ計算が加わったことで、通常の C/C++ ユーティリティの範囲を超え、実用的なテキスト処理ライブラリとしての性格がより明確になりました。
