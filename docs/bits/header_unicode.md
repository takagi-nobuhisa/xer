# `<xer/unicode.h>`

## Purpose

`<xer/unicode.h>` provides practical Unicode utilities.

The current scope includes:

- code point traversal for `std::u8string_view`
- code point traversal for `std::u16string_view`
- code point traversal for `std::wstring_view`
- extended grapheme cluster traversal for `std::u8string_view`
- extended grapheme cluster traversal for `std::u16string_view`
- extended grapheme cluster traversal for `std::wstring_view`
- grapheme-cluster-based string operations for `std::u8string_view`
- grapheme-cluster-based string operations for `std::u16string_view`
- grapheme-cluster-based string operations for `std::wstring_view`
- practical emoji detection for code points and single grapheme clusters
- NFC normalization for UTF-8 text
- NFC status checking for UTF-8 text

Code point traversal, grapheme cluster traversal, grapheme-cluster-based string operations, and emoji detection themselves do not use ICU. NFC normalization uses the ICU C API.

At present, `<xer/unicode.h>` includes the ICU-based normalization implementation, so including this public header requires the ICU development headers to be available.

---

## External Dependency

`<xer/unicode.h>` requires the ICU C API headers because it exposes NFC normalization utilities:

```cpp
#include <unicode/utypes.h>
#include <unicode/ustring.h>
#include <unicode/unorm2.h>
```

The normalization implementation uses ICU C API functions such as `u_strFromUTF8`, `u_strToUTF8`, `unorm2_getNFCInstance`, `unorm2_normalize`, and `unorm2_isNormalized`.

xer does not manage application build-system settings. Users must provide suitable include paths and link options for their environment.

For example, on many Unix-like environments, the required link options are obtained through `pkg-config`:

```bash
pkg-config --cflags --libs icu-uc
```

A direct compile command may look like this:

```bash
g++ -std=c++23 -I. example.cpp -licuuc -licudata
```

On MSYS2 environments, the ICU data library may be named `icudt` rather than `icudata`, so a typical command may look like this:

```bash
g++ -std=c++23 -I. example.cpp -licuuc -licudt
```

The xer test runner handles known environments separately. On Visual Studio 2026 with clang-cl or MSVC cl.exe, xer's tests and examples use ICU installed by vcpkg manifest mode under `vcpkg_installed\x64-windows`.

---

## Main Entities

`<xer/unicode.h>` provides the following code point traversal type:

```cpp
struct code_point {
    std::size_t offset;
    std::size_t size;
    char32_t value;
};
```

The `offset` and `size` fields are expressed in source code units:

- for `std::u8string_view`, UTF-8 code units, effectively bytes
- for `std::u16string_view`, UTF-16 code units
- for `std::wstring_view`, wide code units

The decoded `value` is a Unicode scalar value represented as `char32_t`.

The header provides these decoding functions:

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

It also provides range helpers:

```cpp
auto code_points(std::u8string_view text)
    -> xer::code_point_range<char8_t>;

auto code_points(std::u16string_view text)
    -> xer::code_point_range<char16_t>;

auto code_points(std::wstring_view text)
    -> xer::code_point_range<wchar_t>;
```

The dereferenced range element is:

```cpp
xer::result<xer::code_point>
```

This keeps malformed input explicit during traversal.

`<xer/unicode.h>` provides the following grapheme cluster traversal type:

```cpp
struct grapheme_cluster {
    std::size_t offset;
    std::size_t size;
};
```

The `offset` and `size` fields are expressed in source code units. A grapheme cluster may contain multiple code points, so `grapheme_cluster` intentionally does not contain a `char32_t` value.

The header provides these grapheme cluster functions:

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

It also provides range helpers:

```cpp
auto grapheme_clusters(std::u8string_view text)
    -> xer::grapheme_cluster_range<char8_t>;

auto grapheme_clusters(std::u16string_view text)
    -> xer::grapheme_cluster_range<char16_t>;

auto grapheme_clusters(std::wstring_view text)
    -> xer::grapheme_cluster_range<wchar_t>;
```

The dereferenced range element is:

```cpp
xer::result<xer::grapheme_cluster>
```

This keeps malformed input explicit during traversal.

The header also provides grapheme-cluster-based string operations:

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

`std::u16string_view` and `std::wstring_view` overloads are also provided. The returned substring values are views into the original text.

The header also provides practical emoji detection:

```cpp
auto is_emoji(char32_t value) noexcept -> bool;

auto is_emoji(std::u8string_view text)
    -> xer::result<bool>;

auto is_emoji(std::u16string_view text)
    -> xer::result<bool>;

auto is_emoji(std::wstring_view text)
    -> xer::result<bool>;
```

The string-view overloads return `true` only when the whole input is one practical emoji grapheme cluster. Empty input returns `false`. Malformed UTF-8, UTF-16, or wide text is reported as an error.

The header also provides NFC utilities:

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

### Purpose

`code_point` describes one decoded Unicode scalar value and the corresponding source span.

It does not own the original string. It only records where the decoded code point was found and how many source code units it occupied.

### Offset and Size

`offset` is the index where the code point begins.

`size` is the number of source code units occupied by the code point.

For example, in UTF-8 text:

```cpp
std::u8string_view text = u8"Aあ😀";
```

The code points are:

```text
offset=0 size=1 value=U+0041
offset=1 size=3 value=U+3042
offset=4 size=4 value=U+1F600
```

For UTF-16 text, a supplementary-plane character such as U+1F600 occupies two code units because it is represented as a surrogate pair.

For `std::wstring_view`, the unit depends on the platform's `wchar_t` representation. On platforms where `wchar_t` is 16 bits, supplementary-plane characters occupy two wide code units. On platforms where `wchar_t` is 32 bits, they occupy one wide code unit.

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

### Purpose

`next_code_point` decodes the code point beginning at `offset`.

### Input Model

The `offset` argument is expressed in the source string view's code units.

The input must be well-formed for the corresponding encoding:

- `std::u8string_view` must contain well-formed UTF-8
- `std::u16string_view` must contain well-formed UTF-16
- `std::wstring_view` must contain well-formed wide text according to the platform's `wchar_t` width

### Return Model

The return type is:

```cpp
xer::result<xer::code_point>
```

The function can fail if:

- `offset` is outside the string view
- `offset` is equal to the end of the string view
- the input contains malformed UTF-8
- the input contains malformed UTF-16 surrogate pairs
- the decoded value is not a Unicode scalar value

Out-of-range offsets are reported as `xer::error_t::out_of_range`.
Malformed encoded text is reported as `xer::error_t::encoding_error`.

### Example

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

### Purpose

`prev_code_point` decodes the code point immediately before `offset`.

The `offset` argument is a one-past position. Passing `text.size()` decodes the final code point.

### Return Model

The return type is:

```cpp
xer::result<xer::code_point>
```

The function can fail if:

- `offset` is outside the string view
- `offset` is `0`
- the bytes or code units immediately before `offset` do not form exactly one well-formed code point

### Example

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

### Purpose

`code_points` creates a lightweight input range that walks through the source text by Unicode code point.

### Element Type

The dereferenced iterator value is:

```cpp
const xer::result<xer::code_point>&
```

This means each element must be checked before use.

A malformed sequence appears as an error element. Incrementing the iterator after an error moves it to the end.

### Example

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

## Relationship to Grapheme Clusters

Code point traversal is not the same as grapheme cluster traversal.

For example, the following may be two code points but one user-perceived character:

```text
U+304B HIRAGANA LETTER KA
U+3099 COMBINING KATAKANA-HIRAGANA VOICED SOUND MARK
```

Code point APIs intentionally expose this as two code points.

Grapheme cluster APIs are built on top of this layer and return source string spans rather than a single `char32_t`, because one grapheme cluster may contain multiple code points.

---


## `grapheme_cluster`

```cpp
struct grapheme_cluster {
    std::size_t offset;
    std::size_t size;
};
```

### Purpose

`grapheme_cluster` describes one extended grapheme cluster and the corresponding source span.

It does not own the original string. It only records where the cluster begins and how many source code units it occupies.

Unlike `code_point`, it does not contain a `char32_t` value. A single user-visible character can consist of multiple Unicode code points, such as a base character followed by a combining mark, an emoji with a variation selector, or an emoji ZWJ sequence.

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

### Purpose

`next_grapheme_cluster` decodes the extended grapheme cluster beginning at `offset`.

The implementation is based on the code point traversal layer and groups practical extended grapheme cluster sequences, including combining marks, variation selectors, emoji modifiers, emoji ZWJ sequences, regional indicator pairs, CRLF, and Hangul syllable sequences.

### Return Model

The return type is:

```cpp
xer::result<xer::grapheme_cluster>
```

The function can fail if:

- `offset` is outside the string view
- `offset` is equal to the end of the string view
- the input contains malformed UTF-8
- the input contains malformed UTF-16 surrogate pairs
- the decoded value is not a Unicode scalar value

Out-of-range offsets are reported as `xer::error_t::out_of_range`. Malformed encoded text is reported as `xer::error_t::encoding_error`.

### Example

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

### Purpose

`prev_grapheme_cluster` decodes the extended grapheme cluster immediately before `offset`.

The `offset` argument is a one-past cluster boundary. Passing `text.size()` decodes the final grapheme cluster. Passing an offset inside a cluster is reported as `xer::error_t::encoding_error`.

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

### Purpose

`grapheme_clusters` creates a lightweight input range that walks through the source text by extended grapheme cluster.

The dereferenced element type is:

```cpp
xer::result<xer::grapheme_cluster>
```

### Example

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

### Purpose

`grapheme_length` counts extended grapheme clusters in the source string view.

This is different from `text.size()`, which counts source code units. For UTF-8 Japanese text, `text.size()` is a byte count. `grapheme_length` is intended for user-visible character counts based on xer's practical grapheme cluster rules.

### Return Model

The return type is:

```cpp
xer::result<std::size_t>
```

Malformed UTF-8 or UTF-16 input is reported as `xer::error_t::encoding_error`.

### Example

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

### Purpose

`grapheme_substr` returns a substring view selected by grapheme cluster index.

The `offset` and `count` parameters are grapheme cluster counts, not byte counts and not UTF-16 code-unit counts. The returned value is a view into the original string.

If `offset` is equal to the grapheme cluster length, the result is an empty view at the end of the input. If `offset` is greater than the grapheme cluster length, the function returns `xer::error_t::out_of_range`.

### Example

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

### Purpose

`grapheme_left` returns the first `count` grapheme clusters as a view into the original string. If `count` is greater than the grapheme cluster length, the whole input view is returned.

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

### Purpose

`grapheme_right` returns the last `count` grapheme clusters as a view into the original string. If `count` is greater than the grapheme cluster length, the whole input view is returned.

`grapheme_right` first determines the grapheme cluster length, so malformed input anywhere in the source view is reported.

### Example

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

### Purpose

`is_emoji` provides practical emoji detection for English/Japanese user-facing text.

The `char32_t` overload checks whether a Unicode scalar value is treated as an emoji base by xer. It is intended for quick code point classification.

The string-view overloads check whether the whole input is one emoji grapheme cluster. This is the overload to use for sequences such as flags, keycap emoji, emoji with variation selectors, skin-tone modifiers, and ZWJ emoji sequences.

### Input Model

The string-view overloads accept UTF-8, UTF-16, or wide text. The input must contain exactly one grapheme cluster to return `true`. Empty input returns `false`.

### Return Model

The `char32_t` overload returns `bool` and never reports an error.

The string-view overloads return:

```cpp
xer::result<bool>
```

They can fail when the input contains malformed UTF-8, UTF-16, or wide text.

### Scope

This is a compact practical detector, not a complete generated implementation of every Unicode emoji property. It reuses xer's existing grapheme cluster handling and covers common emoji used in English/Japanese text, including pictographic emoji, flags, keycap emoji, variation-selector forms, emoji modifiers, and ZWJ sequences.

### Example

```cpp
const auto face = xer::is_emoji(std::u8string_view{u8"😀"});
const auto worker = xer::is_emoji(std::u8string_view{u8"👩‍💻"});
const auto letter = xer::is_emoji(std::u8string_view{u8"A"});
```

In this example, `face` and `worker` are expected to contain `true`, while `letter` is expected to contain `false`.

---

## `normalize_nfc`

```cpp
auto normalize_nfc(std::u8string_view text)
    -> xer::result<std::u8string>;
```

### Purpose

`normalize_nfc` converts valid UTF-8 text to Unicode Normalization Form C.

NFC is the normalization form that first applies canonical decomposition and then canonical composition. It is useful for making canonically equivalent text use a consistent byte representation where composed characters are available.

For example, the sequence:

```text
U+304B HIRAGANA LETTER KA
U+3099 COMBINING KATAKANA-HIRAGANA VOICED SOUND MARK
```

can be normalized to:

```text
U+304C HIRAGANA LETTER GA
```

Similarly, the sequence:

```text
U+0041 LATIN CAPITAL LETTER A
U+030A COMBINING RING ABOVE
```

can be normalized to:

```text
U+00C5 LATIN CAPITAL LETTER A WITH RING ABOVE
```

### Input Model

The input is provided as:

```cpp
std::u8string_view
```

The input must be valid UTF-8. Invalid UTF-8 is reported as an error.

### Output Model

On success, the function returns:

```cpp
std::u8string
```

The returned string is valid UTF-8 normalized to NFC.

### Return Model

The return type is:

```cpp
xer::result<std::u8string>
```

The function can fail if:

- the input is not valid UTF-8
- the input or output size exceeds the range accepted by ICU C API length parameters
- ICU reports an internal failure

Typical invalid UTF-8 input is reported as `xer::error_t::encoding_error`.

### Example

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

### Purpose

`is_normalized_nfc` checks whether valid UTF-8 text is already normalized to Unicode NFC.

This is useful when a program wants to avoid rewriting text that is already in the desired normalization form.

### Input Model

The input is provided as:

```cpp
std::u8string_view
```

The input must be valid UTF-8. Invalid UTF-8 is reported as an error.

### Output Model

On success, the function returns:

```cpp
bool
```

The value is `true` if the input is already NFC, and `false` otherwise.

### Return Model

The return type is:

```cpp
xer::result<bool>
```

The function can fail if:

- the input is not valid UTF-8
- the input size exceeds the range accepted by ICU C API length parameters
- ICU reports a failure

Typical invalid UTF-8 input is reported as `xer::error_t::encoding_error`.

### Example

```cpp
const auto before = xer::is_normalized_nfc(u8"か\u3099");
const auto after = xer::is_normalized_nfc(u8"が");
```

In this example, `before` is expected to contain `false`, while `after` is expected to contain `true`.

---

## Design Notes

`<xer/unicode.h>` is intentionally independent from ordinary string-processing headers such as `<xer/string.h>`.

The code point traversal layer is small and table-free. It validates UTF-8 and UTF-16 structure, reports malformed input through `xer::result`, and records source spans in code units.

The grapheme cluster traversal layer is built on top of the code point layer. It is intended for practical user-visible character traversal while still returning source spans instead of copying text. The grapheme-cluster-based string operations provide convenient length and substring helpers while returning views into the original text. Emoji detection is also built on the same code point and grapheme cluster layers so that multi-code-point emoji sequences can be checked as one user-visible unit.

The default language scope of this layer is English and Japanese text. It handles common sequences needed for practical English/Japanese user-facing text, including combining marks, variation selectors, emoji modifiers, emoji ZWJ sequences, regional indicator pairs, CRLF, and Hangul syllable sequences. It is not a full Unicode text-boundary engine for every script and does not provide language-specific tailoring. Users who need broader script coverage can extend xer's public source code or use a dedicated Unicode boundary service.

Unicode normalization is heavier than simple UTF-8 string utilities because it requires Unicode normalization data. xer delegates this responsibility to ICU instead of embedding a large generated Unicode table in the header-only library.

The initial normalization API exposes only NFC because NFC is the most practical normalization form for many file-name, search-key, dictionary, and text-cleanup use cases. Other normalization forms can be added later without changing the basic API shape.

---
