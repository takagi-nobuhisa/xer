# xer C++ Utility Library v0.5.0 Release Notes

xer C++ Utility Library v0.5.0 expands the library mainly in the areas of Unicode processing, Japanese text processing, and binary data utilities.

This release adds code point traversal for UTF-8, UTF-16, and wide strings; extended grapheme cluster traversal; grapheme-cluster-based string operations; practical emoji detection; and NFC normalization using ICU. It also adds Japanese character classification, kana conversion, kana normalization, fullwidth/halfwidth conversion, binary-to-hex and hex-to-binary conversion, MD5, SHA-1, and SHA-256 utilities.

## Highlights

- Added `<xer/unicode.h>` for practical Unicode processing.
- Added `<xer/ja.h>` as a convenience header for Japanese-specific APIs under the `xer::ja` namespace.
- Added `bin2hex`, `hex2bin`, `md5`, `sha1`, and `sha256` to `<xer/binary.h>`.
- Extended `xer::ctype_id` and `xer::ctrans_id` with fullwidth, halfwidth, and kana-related character classification and conversion identifiers.
- Improved the practical usability of MeCab integration, Japanese word segmentation, romaji conversion, and braille-related utilities.
- Updated README files, the public header list, reference manual fragments, and policy documents to match the current library state.
- Clarified supported environments and explicitly excluded MSYS2 MSYS and MSYS2 MINGW64 from supported targets.

## New Public Header

### `<xer/unicode.h>`

A new public header, `<xer/unicode.h>`, has been added.

The main features are:

- Unicode code point traversal
- Extended grapheme cluster traversal
- Grapheme-cluster-based string length
- Grapheme-cluster-based substring operations
- Practical emoji detection
- NFC normalization for UTF-8 text
- NFC normalization checks for UTF-8 text

Code point traversal, grapheme cluster traversal, grapheme-cluster-based string operations, and emoji detection do not depend on ICU. NFC normalization and NFC checks use the ICU C API.

At this point, `<xer/unicode.h>` includes the NFC normalization APIs, so using this public header requires ICU development headers and the corresponding link settings.

## Unicode Utilities

### Code Point Traversal

Code point traversal has been added for `std::u8string_view`, `std::u16string_view`, and `std::wstring_view`.

The main APIs are:

```cpp
auto next_code_point(std::u8string_view text, std::size_t offset = 0)
    -> xer::result<xer::code_point>;

auto prev_code_point(std::u8string_view text, std::size_t offset)
    -> xer::result<xer::code_point>;

auto code_points(std::u8string_view text)
    -> xer::code_point_range<char8_t>;
```

Equivalent overloads are provided for `std::u16string_view` and `std::wstring_view`.

Invalid UTF-8, UTF-16, and wide string input is handled as an explicit error through `xer::result`.

### Grapheme Cluster Traversal

Traversal by extended grapheme cluster has been added.

The main APIs are:

```cpp
auto next_grapheme_cluster(std::u8string_view text, std::size_t offset = 0)
    -> xer::result<xer::grapheme_cluster>;

auto prev_grapheme_cluster(std::u8string_view text, std::size_t offset)
    -> xer::result<xer::grapheme_cluster>;

auto grapheme_clusters(std::u8string_view text)
    -> xer::grapheme_cluster_range<char8_t>;
```

Equivalent overloads are provided for `std::u16string_view` and `std::wstring_view`.

### Grapheme-Cluster-Based String Operations

String operations based on grapheme clusters have been added.

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

Equivalent overloads are provided for `std::u16string_view` and `std::wstring_view`.

### Emoji Detection

Practical emoji detection has been added.

```cpp
auto is_emoji(char32_t value) noexcept -> bool;

auto is_emoji(std::u8string_view text)
    -> xer::result<bool>;
```

The string overload returns `true` when the entire input is exactly one practical emoji grapheme cluster. An empty string returns `false`.

### NFC Normalization

NFC normalization and NFC normalization checks using the ICU C API have been added.

```cpp
auto normalize_nfc(std::u8string_view text)
    -> xer::result<std::u8string>;

auto is_normalized_nfc(std::u8string_view text)
    -> xer::result<bool>;
```

At this point, only NFC is provided because it is the most commonly needed normalization form in practical use. NFD, NFKC, and NFKD are possible future extensions.

## Japanese Text Utilities

### `<xer/ja.h>`

`<xer/ja.h>` has been added as a convenience header for Japanese-specific APIs.

`<xer/ja.h>` collects the following feature groups:

- Furigana formatting
- Kanji numeral conversion
- MeCab integration
- Japanese character classification
- Japanese kanji classification
- Kana conversion
- Kana normalization

Japanese-specific APIs are placed in the `xer::ja` namespace.

### Japanese Character Classification

Japanese character classification has been added and expanded.

The main APIs include:

```cpp
xer::ja::is_hiragana(U'あ');
xer::ja::is_katakana(U'ア');
xer::ja::is_kana(U'ｱ');
xer::ja::is_kanji(U'漢');
xer::ja::is_japanese_punctuation(U'。');
xer::ja::is_japanese(U'日');
```

The `contains_*` APIs check whether a UTF-8 string contains at least one character of the corresponding kind.

```cpp
xer::ja::contains_hiragana(u8"abcあ");
xer::ja::contains_katakana(u8"abcア");
xer::ja::contains_kana(u8"abcｱ");
xer::ja::contains_kanji(u8"abc漢");
xer::ja::contains_japanese(u8"hello日本語");
```

The `is_all_*` APIs check whether the whole string consists of hiragana, katakana, or kana characters.

```cpp
xer::ja::is_all_hiragana(u8"こんにちはー");
xer::ja::is_all_katakana(u8"コンニチハー");
xer::ja::is_all_kana(u8"こんにちはコンニチハー");
```

These string APIs return `false` for an empty string. Invalid UTF-8 input is reported as an `encoding_error`.

### Kanji Classification

Japanese kanji classification tables have been added.

The main categories are:

- Jinmeiyo kanji
- Joyo kanji
- Kyoiku kanji
- JIS Level 1 through Level 4 kanji

The main APIs include:

```cpp
xer::ja::is_name_kanji(U'凜');
xer::ja::is_jouyou_kanji(U'鬱');
xer::ja::is_kyouiku_kanji(U'日');
xer::ja::jis_kanji_level_of(U'亜');
```

### Kana Conversion and Normalization

Hiragana/katakana conversion and practical kana normalization have been added.

```cpp
auto to_hiragana(std::u8string_view text)
    -> xer::result<std::u8string>;

auto to_katakana(std::u8string_view text)
    -> xer::result<std::u8string>;

auto normalize_kana(std::u8string_view text)
    -> xer::result<std::u8string>;
```

`normalize_kana` converts halfwidth katakana to fullwidth katakana and composes dakuten and handakuten marks when composition is possible. It preserves the distinction between hiragana and katakana where practical.

## Character Classification and Conversion

Character classification and conversion in `<xer/ctype.h>` have been extended.

### Fullwidth and Halfwidth Classification

Fullwidth and halfwidth classifications have been added to `ctype_id`.

The main classifications are:

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

Fullwidth/halfwidth conversion and hiragana/katakana conversion identifiers have been added to `ctrans_id`.

The main conversion identifiers are:

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

Fullwidth/halfwidth conversion is primarily intended for practical Japanese text processing. Hiragana is not converted by the fullwidth/halfwidth conversions.

The current `toctrans` model maps one input code point to one output code point. Therefore, when converting fullwidth katakana with dakuten or handakuten to halfwidth katakana, information that cannot be represented as a single character may not be preserved. Use `strtoctrans` or `xer::ja::normalize_kana` when string-level handling is required.

## Binary Data Utilities

Binary data utilities in `<xer/binary.h>` have been expanded.

### Hex Conversion

Conversion between binary data and hexadecimal strings has been added.

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

`bin2hex` returns lowercase hexadecimal strings.

`hex2bin` accepts `0` through `9`, `a` through `f`, and `A` through `F`. The input length must be even.

### Hash Functions

MD5, SHA-1, and SHA-256 have been added.

```cpp
auto md5(std::span<const std::byte> bytes) noexcept
    -> std::array<std::byte, 16>;

auto sha1(std::span<const std::byte> bytes) noexcept
    -> std::array<std::byte, 20>;

auto sha256(std::span<const std::byte> bytes) noexcept
    -> std::array<std::byte, 32>;
```

Overloads are also provided for `const void*` plus size, iterator ranges, and file paths.

The return values are raw digest byte arrays, not hexadecimal strings. Use `bin2hex` when a conventional hexadecimal representation is needed.

```cpp
const auto digest = xer::sha256(bytes);
const auto hex = xer::bin2hex(digest.begin(), digest.end());
```

## MeCab, Romaji, and Braille Utilities

MeCab integration, Japanese segmentation, romaji conversion, and braille-related utilities and documentation have been improved.

The main improvements are:

- Kana segmentation using readings from MeCab
- Romaji segmentation using readings from MeCab
- Bunsetsu-like segmentation helpers
- Practical segmentation that accounts for symbol ranges
- Segmentation for braille conversion
- Braille conversion using MeCab integration
- Improved tests and documentation for braille-related features

Braille-related features are provided as practical building blocks. They are not intended to claim full coverage as a complete braille transcription engine. Conversion results that use MeCab integration depend on the readings returned by the external MeCab dictionary.

## Documentation

The following documents have been updated to match the state of v0.5.0:

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

The reference manual should also be regenerated for v0.5.0.

## Supported Environments

The main supported and tested environments as of v0.5.0 are:

- Ubuntu
- MSYS2 UCRT64

The target platform scope is:

- Linux through Ubuntu
- Windows through MSYS2 UCRT64

The target Windows version is Windows 11 or later.

The following environments are not included in the current or planned test targets:

- MSYS2 MSYS
- MSYS2 MINGW64

If a clear need arises in the future, support for those environments may be reconsidered at that time.

## External Dependencies

xer is a header-only library, but some features depend on external components.

- NFC normalization: ICU C API
- MeCab integration: MeCab runtime environment
- Tcl/Tk integration: Tcl/Tk development environment and link settings
- Socket-related features: system libraries appropriate to the target environment

Features that require external components are also handled according to the current environment by the development test scripts.

## Compatibility Notes

### `<xer/unicode.h>` Requires ICU Headers

`<xer/unicode.h>` includes the NFC normalization APIs, so it currently requires ICU C API headers. This is true even when using only code point traversal or grapheme cluster traversal from the public header.

The ICU-dependent parts may be split more finely in a future release.

### Fullwidth/Halfwidth Conversion Semantics Were Clarified

Fullwidth/halfwidth conversion handles katakana in addition to digits, alphabetic characters, punctuation, and spaces, reflecting the practical needs of Japanese text processing.

However, `toctrans` maps one input code point to one output code point, so conversions that require multiple code points have inherent limitations.

### MSYS2 MSYS and MSYS2 MINGW64 Are Not Supported Targets

MSYS2 MSYS and MSYS2 MINGW64 are not supported targets as of v0.5.0. Use UCRT64 when using xer with MSYS2.

## Notes for Release Preparation

Before tagging the official release, update the version suffix in `xer/version.h` for the final release.

```cpp
#define XER_VERSION_SUFFIX ""
#define XER_VERSION_STRING "0.5.0"
```

Also update the target version notation in the reference manual to `v0.5.0` before regenerating it.

## Summary

v0.5.0 is a major step forward for Unicode and Japanese text processing in xer.

With code point traversal, grapheme cluster handling, emoji detection, NFC normalization, kana utilities, kanji classification, hexadecimal conversion, and hash calculation, xer now has a clearer role as a practical text-processing utility library that goes beyond ordinary C/C++ helper routines.
