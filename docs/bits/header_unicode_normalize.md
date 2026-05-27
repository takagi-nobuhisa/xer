# `<xer/unicode_normalize.h>`

## Purpose

`<xer/unicode_normalize.h>` provides Unicode normalization utilities.

The current scope is intentionally narrow. This header provides NFC normalization and NFC status checking for UTF-8 text. The implementation uses the ICU C API.

This header depends on external ICU development headers and ICU libraries. If the required ICU headers are not available, including this header fails at compile time with `#error`.

---

## External Dependency

`<xer/unicode_normalize.h>` requires the ICU C API headers:

```cpp
#include <unicode/utypes.h>
#include <unicode/ustring.h>
#include <unicode/unorm2.h>
```

The implementation uses ICU C API functions such as `u_strFromUTF8`, `u_strToUTF8`, `unorm2_getNFCInstance`, `unorm2_normalize`, and `unorm2_isNormalized`.

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

The xer test runner handles known environments separately.

---

## Main Entities

At minimum, `<xer/unicode_normalize.h>` provides the following functions:

```cpp
auto normalize_nfc(std::u8string_view text)
    -> xer::result<std::u8string>;

auto is_normalized_nfc(std::u8string_view text)
    -> xer::result<bool>;
```

Both functions operate on UTF-8 text represented by `std::u8string_view`.

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
- ICU reports an internal failure

Typical invalid UTF-8 input is reported as `xer::error_t::encoding_error`.

### Example

```cpp
const auto before = xer::is_normalized_nfc(u8"か\u3099");
const auto after = xer::is_normalized_nfc(u8"が");
```

In this example, `before` is expected to contain `false`, while `after` is expected to contain `true`.

---

## Design Notes

`<xer/unicode_normalize.h>` is intentionally independent from ordinary string-processing headers such as `<xer/string.h>`.

Unicode normalization is heavier than simple UTF-8 string utilities because it requires Unicode normalization data. xer delegates this responsibility to ICU instead of embedding a large generated Unicode table in the header-only library.

The initial API exposes only NFC because NFC is the most practical normalization form for many file-name, search-key, dictionary, and text-cleanup use cases. Other normalization forms can be added later without changing the basic API shape.

---

## Limitations

The current implementation provides only NFC-related utilities:

- `normalize_nfc`
- `is_normalized_nfc`

It does not currently provide NFD, NFKC, NFKD, case folding, collation, locale-sensitive comparison, transliteration, or other ICU-based text services.

The header requires ICU at compile and link time. It is not a fallback implementation and does not attempt to normalize text without ICU.
