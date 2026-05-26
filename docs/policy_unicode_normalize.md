# Policy for Unicode Utilities

## Overview

Unicode utilities in XER are provided through `<xer/unicode.h>`.

The current public scope is intentionally incremental:

```text
code point traversal for UTF-8, UTF-16, and wide string views
extended grapheme cluster traversal for UTF-8, UTF-16, and wide string views
grapheme-cluster-based string operations for UTF-8, UTF-16, and wide string views
NFC normalization for UTF-8 text
```

Code point traversal is a small table-free layer. Grapheme cluster traversal is built on that layer and uses compact rule helpers for practical extended grapheme cluster boundaries. Unicode normalization is provided as a practical external-component feature based on the ICU C API.

This gives XER a useful and standards-based normalization facility without embedding large Unicode normalization tables into the header-only library.

---

## Language Scope

XER primarily targets practical text handling for English and Japanese.

The Unicode APIs should therefore provide reliable building blocks for:

- ASCII and ordinary English text
- UTF-8 Japanese text
- Japanese text that needs NFC normalization, such as kana with combining dakuten or handakuten
- common emoji and symbol sequences that appear in Japanese or English user-facing text

The grapheme cluster implementation is not intended to be a complete implementation of every language-specific writing-system rule in Unicode. In particular, complex script behavior outside the ordinary English/Japanese scope, such as Indic conjunct handling and other script-specific tailoring, is outside XER's default scope unless a clear project need appears.

Users who need more complete handling for other languages or scripts may extend XER, add generated Unicode property tables, or use ICU text-boundary services directly in their own code. XER is publicly available as source code, so such extensions should remain possible without forcing the base header-only library to embed large Unicode data tables.

---

## Public Header

Unicode utilities are provided through:

```text
xer/unicode.h
```

The internal implementation may be placed under:

```text
xer/bits/unicode_code_point.h
xer/bits/unicode_grapheme_cluster.h
xer/bits/unicode_grapheme_string.h
xer/bits/unicode_normalize.h
```

The feature is not absorbed into `<xer/string.h>` or `<xer/ctype.h>`. Code point traversal, grapheme cluster traversal, and grapheme-cluster-based string operations are Unicode-specific text facilities, and normalization depends on ICU and is much heavier than ordinary string utilities or character classification.

---

## External Dependency

`<xer/unicode.h>` depends on the ICU C API.

Required headers include:

```cpp
#include <unicode/utypes.h>
#include <unicode/ustring.h>
#include <unicode/unorm2.h>
```

If these headers are not available, including `<xer/unicode.h>` should fail at compile time with `#error`.

This is intentional. A program that explicitly includes this header is requesting ICU-based Unicode normalization, so missing ICU development headers should be detected statically.

Link settings are the user's build-system responsibility. XER may document typical link options, and XER's own test runner should add the required libraries for known environments.

---

## C API Only

XER uses ICU's C API for this feature.

The public API must not expose ICU C++ API types.
It also should not expose ICU C API handles unless a future low-level advanced API has a clear need for them.

Reasons:

- The C API is stable and fits XER's C-oriented design.
- Some platforms expose ICU through the C API only.
- Public XER APIs should remain simple UTF-8-based functions returning `xer::result`.

---

## Public API Shape

The code point traversal API is:

```cpp
struct code_point {
    std::size_t offset;
    std::size_t size;
    char32_t value;
};

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

auto code_points(std::u8string_view text)
    -> xer::code_point_range<char8_t>;

auto code_points(std::u16string_view text)
    -> xer::code_point_range<char16_t>;

auto code_points(std::wstring_view text)
    -> xer::code_point_range<wchar_t>;
```

The dereferenced range element is `xer::result<xer::code_point>` so malformed input remains explicit during traversal.

The grapheme cluster traversal API is:

```cpp
struct grapheme_cluster {
    std::size_t offset;
    std::size_t size;
};

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

auto grapheme_clusters(std::u8string_view text)
    -> xer::grapheme_cluster_range<char8_t>;

auto grapheme_clusters(std::u16string_view text)
    -> xer::grapheme_cluster_range<char16_t>;

auto grapheme_clusters(std::wstring_view text)
    -> xer::grapheme_cluster_range<wchar_t>;
```

The dereferenced range element is `xer::result<xer::grapheme_cluster>` so malformed input remains explicit during traversal.

`xer::grapheme_cluster` records only source `offset` and `size`. It does not own or copy the underlying text, and it does not pretend that a user-visible character can always be represented by one `char32_t`.

The default grapheme cluster rules are practical rules for English/Japanese-oriented text processing. They are not a promise of full UAX #29 conformance for every script, nor a replacement for language-specific boundary tailoring.


The grapheme-cluster-based string operation API is:

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

`std::u16string_view` and `std::wstring_view` overloads are also provided.

These functions take grapheme cluster counts, not source code-unit counts. They return views into the original text and do not allocate.

The normalization API is:

```cpp
auto normalize_nfc(std::u8string_view text)
    -> xer::result<std::u8string>;

auto is_normalized_nfc(std::u8string_view text)
    -> xer::result<bool>;
```

All return types follow the ordinary XER error policy.
Code point decoding can fail when the offset is outside the view or when the input is malformed.
Grapheme cluster traversal can fail when the offset is outside the view, when `prev_grapheme_cluster` is given a non-boundary offset, or when the input is malformed. Grapheme-cluster-based string operations can fail when the requested offset is outside the grapheme cluster length or when malformed input is encountered while traversing the required part of the source view.
The normalization functions can fail when the input is invalid UTF-8, when a size is outside ICU's supported range, or when ICU reports an error.

Function return types should use trailing return type syntax.

---

## Text Representation

The primary text representation remains UTF-8.

For code point traversal, the supported source views are:

- `std::u8string_view`
- `std::u16string_view`
- `std::wstring_view`

The `offset` and `size` fields of `xer::code_point` and `xer::grapheme_cluster` are expressed in source code units. Grapheme string operation offsets and counts are expressed in grapheme clusters.

For normalization, the public input and output representation is UTF-8:

- input: `std::u8string_view`
- output: `std::u8string`

Internally, the implementation may convert UTF-8 to ICU's UTF-16 representation, normalize it, and convert the result back to UTF-8.

A typical internal flow is:

```text
std::u8string_view
  -> ICU UTF-16 buffer
  -> ICU NFC normalization
  -> std::u8string
```

The internal use of UTF-16 does not change XER's public UTF-8-first policy.

---

## Initial Scope: NFC Only

The first supported normalization form is NFC.

NFC is the most practical initial form for XER because it is widely useful for:

- file names
- search keys
- dictionary inputs
- Japanese text containing combining dakuten or handakuten
- text cleanup before comparison or storage

XER does not initially expose NFD, NFKC, or NFKD.
Those may be added later if there is a clear need.

---

## What NFC Means

NFC is Unicode Normalization Form C.
It performs canonical decomposition followed by canonical composition.

Examples:

```text
U+304B HIRAGANA LETTER KA
U+3099 COMBINING KATAKANA-HIRAGANA VOICED SOUND MARK
```

may normalize to:

```text
U+304C HIRAGANA LETTER GA
```

and:

```text
U+0041 LATIN CAPITAL LETTER A
U+030A COMBINING RING ABOVE
```

may normalize to:

```text
U+00C5 LATIN CAPITAL LETTER A WITH RING ABOVE
```

XER delegates the exact Unicode normalization behavior to ICU.

---

## Error Handling

Malformed UTF-8 and malformed UTF-16 surrogate pairs should be reported as encoding errors.

Offsets outside the source view should be reported as out-of-range errors.

Invalid UTF-8 passed to ICU-based normalization should be reported as an encoding error.

Input or output sizes that cannot be represented by the ICU C API length type should be reported as a length error.

Other ICU failures should be reported as runtime errors unless a more precise existing XER error category is appropriate.

The API should not throw exceptions for ordinary ICU failures.
It should return `xer::result` errors.

---

## No Fallback Normalizer

XER should not maintain a partial fallback NFC implementation.

A partial or outdated Unicode normalization table would be worse than an explicit dependency because it could silently produce incorrect results.
For this feature, ICU is the normalization engine.

If ICU is unavailable, the feature is unavailable.
The failure should occur at compile time when the header is included.

---

## Relationship to Other Unicode Features

Unicode code point traversal and normalization are separate from:

- character classification in `<xer/ctype.h>`
- low-level string operations in `<xer/string.h>`
- encoding conversion in `<xer/stdlib.h>`
- Japanese morphological processing in `<xer/mecab.h>`
- furigana formatting in `<xer/furigana.h>`
- braille conversion in `<xer/braille.h>`

These features may use code point traversal or normalized text where appropriate, but they should not become implicitly dependent on ICU unless the dependency is explicitly accepted.

---

## Future Expansion

Possible future normalization additions include:

```cpp
auto normalize_nfd(std::u8string_view text)
    -> xer::result<std::u8string>;

auto normalize_nfkc(std::u8string_view text)
    -> xer::result<std::u8string>;

auto normalize_nfkd(std::u8string_view text)
    -> xer::result<std::u8string>;
```

Other ICU-based text facilities, such as case folding, collation, transliteration, or locale-sensitive processing, should not be added automatically just because ICU is available.
Each should have its own practical need and API policy.

---

## Testing Policy

Tests should cover at least:

- UTF-8 code point traversal for ASCII, BMP, and supplementary-plane characters
- UTF-16 code point traversal including surrogate pairs
- wide string traversal on the host platform
- malformed UTF-8
- malformed UTF-16 surrogate pairs
- ASCII text that is already NFC
- Japanese text that is already NFC
- Japanese decomposed dakuten input such as `か` + U+3099
- Latin decomposed input such as `A` + U+030A
- `is_normalized_nfc` for both normalized and non-normalized inputs
- invalid UTF-8 input

The PHP test runner should link ICU for known supported environments.
If ICU is not available in the test environment, ICU-dependent tests may be skipped.

---

## Documentation Requirements

The reference fragment for `<xer/unicode.h>` should document:

- code point traversal APIs
- source code unit offset and size semantics
- malformed input handling
- the ICU dependency
- required ICU headers
- typical link options
- the public API
- valid UTF-8 input requirements
- NFC-only scope
- failure conditions
- future limitations

Examples should remain small and should not introduce unrelated ICU features.

---

## Summary

- Unicode utilities are provided through `<xer/unicode.h>`.
- Code point traversal supports `std::u8string_view`, `std::u16string_view`, and `std::wstring_view`.
- The normalization implementation uses ICU C API only.
- The current normalization scope is NFC and `is_normalized_nfc`.
- Public input and output are UTF-8.
- Missing ICU headers are compile-time errors.
- User link settings are outside XER's responsibility.
- XER does not provide a partial fallback normalizer.
- Future ICU-based features should be added only with explicit need and separate design consideration.
