# `<xer/unicode.h>`

## Purpose

`<xer/unicode.h>` provides practical Unicode utilities.

The current scope includes:

- code point traversal for `std::u8string_view`
- code point traversal for `std::u16string_view`
- code point traversal for `std::wstring_view`
- NFC normalization for UTF-8 text
- NFC status checking for UTF-8 text

Code point traversal itself does not use ICU. NFC normalization uses the ICU C API.

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

XER does not manage application build-system settings. Users must provide suitable include paths and link options for their environment.

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

The XER test runner handles known environments separately.

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

Grapheme cluster APIs should be built on top of this layer later. They should return source string spans rather than a single `char32_t`, because one grapheme cluster may contain multiple code points.

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

Unicode normalization is heavier than simple UTF-8 string utilities because it requires Unicode normalization data. XER delegates this responsibility to ICU instead of embedding a large generated Unicode table in the header-only library.

The initial normalization API exposes only NFC because NFC is the most practical normalization form for many file-name, search-key, dictionary, and text-cleanup use cases. Other normalization forms can be added later without changing the basic API shape.

---
