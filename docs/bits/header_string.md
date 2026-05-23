# `<xer/string.h>`

## Purpose

`<xer/string.h>` provides string-related utilities in XER.

This header brings together several kinds of functionality:

- C-style string operations
- UTF-aware character and substring search helpers
- PHP-inspired utility functions such as split/join and trim
- raw memory helpers grouped with string-oriented facilities
- prefix/suffix checks
- case conversion and dynamic string transformation
- error text helpers

The goal is not to reproduce the C standard library exactly.
Instead, this header reorganizes practical string-related functionality into a form that fits XER's overall design.

---

## Main Role

The main role of `<xer/string.h>` is to provide a practical collection of text and memory helpers centered on XER's UTF-8-oriented public string model.

In particular, it serves the following purposes:

- provide familiar C-style names where that improves approachability
- support UTF-8-oriented public APIs based on `char8_t`
- provide allocation-free helpers where practical
- provide higher-level utility functions that are convenient in ordinary code

This header therefore mixes low-level and high-level facilities more intentionally than the standard C library does.

---

## Main Function Groups

At a high level, `<xer/string.h>` contains the following groups of functionality:

- search and comparison
- case-insensitive search and comparison
- copy and concatenation
- split / join / trim
- string replacement
- raw memory helpers
- prefix/suffix checks
- case conversion and dynamic string transformation
- error text helpers

---

## Search and Comparison

At minimum, this header provides functions such as the following:

```cpp
strlen
strcmp
strncmp
strchr
strrchr
strstr
strrstr
strpos
strrpos
strpbrk
strspn
strcspn
```

### Role of This Group

These functions provide familiar string-search and comparison operations in a form adapted to XER's public string model.

Some are close in spirit to C standard-library functions, while others are influenced more by PHP-style utility naming.

### Notes

* not all functions are exact source-compatible reimplementations of their C or PHP namesakes
* accepted argument forms are designed according to XER's own API policy
* ordinary public APIs are documented in terms of ordinary values rather than `xer::result` arguments

---

## Case-Insensitive Search and Comparison

This header also provides case-insensitive operations such as the following:

```cpp
strcasecmp
strncasecmp
stricmp
strnicmp
strcasechr
strcaserchr
strichr
strirchr
strcasepos
strcaserpos
strcasestr
strcaserstr
stripos
strirpos
stristr
strirstr
```

### Role of This Group

These functions exist to support practical case-insensitive text handling without requiring users to build the operation manually every time.

### Notes

* these facilities are intentionally simple
* they are primarily oriented around ASCII and the normalization currently implemented by the library
* they should not be read as a promise of full locale-sensitive or Unicode-wide collation behavior

---

## Copy and Concatenation

At minimum, this header provides functions such as:

```cpp
strcpy
strncpy
strcat
strncat
```

### Role of This Group

These functions provide familiar copy and concatenation operations, but their exact behavior follows XER's design rather than attempting perfect C compatibility.

### Notes

* XER's design gives priority to natural and practical use in C++ code
* overloads may exist for arrays, pointers, and container-like targets
* when automatic capacity growth is appropriate for dynamic containers, the design may favor that behavior over stricter historical C behavior

---

## Split / Join / Trim

This header also provides higher-level helpers such as:

```cpp
explode
implode
ltrim
rtrim
trim
ltrim_view
rtrim_view
trim_view
```

### Role of This Group

This group provides convenient text-processing helpers inspired in part by PHP.

These functions are intended for cases where ordinary code benefits from a compact utility API rather than from manual loops and range manipulation.

### Trim Functions

The trimming family is provided in two forms:

```cpp
auto ltrim(std::u8string_view value, std::u8string_view characters = {})
    -> xer::result<std::u8string>;

auto rtrim(std::u8string_view value, std::u8string_view characters = {})
    -> xer::result<std::u8string>;

auto trim(std::u8string_view value, std::u8string_view characters = {})
    -> xer::result<std::u8string>;

auto ltrim_view(std::u8string_view value, std::u8string_view characters = {})
    -> xer::result<std::u8string_view>;

auto rtrim_view(std::u8string_view value, std::u8string_view characters = {})
    -> xer::result<std::u8string_view>;

auto trim_view(std::u8string_view value, std::u8string_view characters = {})
    -> xer::result<std::u8string_view>;
```

The owning forms return a new `std::u8string`.
The `*_view` forms return a view into the original storage and do not allocate.

When `characters` is empty, XER uses the PHP-compatible default trim set:

```text
space, horizontal tab, line feed, carriage return, vertical tab, NUL
```

When `characters` is not empty, it is interpreted as a byte-oriented character list.
The list supports PHP-style range notation such as `a..z` and `0..9`.

### Important Notes for UTF-8

The trimming functions operate on UTF-8 code units, not on Unicode scalar values or grapheme clusters.
They are therefore appropriate for ASCII-oriented boundary trimming and PHP-style byte character lists.

Do not use these functions as a general Unicode whitespace normalizer.

### `*_view` Variants

The `ltrim_view`, `rtrim_view`, and `trim_view` family are especially important because they provide non-allocating trimming operations around UTF-8-oriented string views.

The returned view refers to the storage of `value`.
The caller must ensure that the source storage outlives the returned view.

### Notes

* `trim_view`-style functions are intended to be lightweight and convenient
* the trimming character list is byte-oriented and supports `..` ranges
* these helpers are useful both in ordinary code and in executable examples
* code examples are expected to use these functions naturally with explicit `xer::result` checking where required

---

## String Replacement

`<xer/string.h>` provides a PHP-inspired string replacement helper:

```cpp
auto str_replace(
    std::u8string_view search,
    std::u8string_view replace,
    std::u8string_view subject,
    std::size_t* count = nullptr) -> xer::result<std::u8string>;
```

### Role of This Function

`str_replace` replaces all non-overlapping occurrences of `search` in `subject` with `replace`.

The argument order follows PHP's `str_replace` naming tradition: search string, replacement string, and subject string.

### Behavior

Replacement is performed on UTF-8 code units. The function does not attempt grapheme-cluster processing.

If `search` is empty, no replacement is performed and `subject` is returned unchanged.

If `count` is not `nullptr`, the number of performed replacements is stored there.

### Notes

This function is useful for simple text substitution. More advanced text processing, such as regular-expression replacement or locale-sensitive transformation, is outside the scope of this helper.

---

## Raw Memory Helpers

`<xer/string.h>` also groups raw memory helpers through the related memory facilities.

These include operations such as:

```cpp
memcpy
memmove
memchr
memrchr
memcmp
memset
```

### Role of This Group

Although raw memory operations are not text operations in the narrow sense, XER groups them together with string-oriented helpers for practical convenience.

This reflects the historical closeness of string and memory functions in C-style programming, while still keeping the public-header structure compact.

### Notes

* these functions are low-level helpers
* they are still part of the public surface when exposed through `<xer/string.h>`
* their semantics should be read according to XER's own design and test coverage, not assumed solely from the standard library

---

## Error Text Helpers

This header also provides helpers such as:

```cpp
strerror
get_error_name
get_errno_name
```

### Role of This Group

These helpers are intended to convert error categories into human-readable or symbolic forms.

They are useful in diagnostics, debugging output, and user-facing reporting when appropriate.

### Notes

* the exact mapping policy depends on XER's own error model
* `get_error_name` and `get_errno_name` are especially useful where symbolic names are preferable to free-form text

---

## UTF-Oriented Public String Model

`<xer/string.h>` should be understood in the context of XER's general text model.

### Basic Expectations

* public string APIs generally use `char8_t`
* owned strings generally use `std::u8string`
* non-owning text views generally use `std::u8string_view`
* individual Unicode scalar values are generally represented as `char32_t`

This is important because the header name may look similar to C's `<string.h>`, but the actual design direction is different.

### Character Search

UTF-aware character search functions may accept `char32_t` for searching UTF-8, UTF-16, or UTF-32 text.

This allows a single Unicode scalar value to be expressed clearly at the call site.

---

## Relationship to Other Policies

`<xer/string.h>` is closely related to the following policy documents:

* `policy_project_outline.md`
* `policy_result_arguments.md`
* `policy_examples.md`

It also interacts conceptually with:

* `policy_ctype.md`
* `policy_encoding.md`

The exact boundary is as follows:

* `<xer/string.h>` handles string and memory utilities
* `<xer/ctype.h>` handles classification and character transformation
* encoding policy defines the broader text model used by the library

---

## Documentation Notes

When this header is used as part of generated documentation, it is usually enough to explain:

* that it combines C-style string utilities with XER-specific UTF-8-oriented helpers
* that it includes both low-level and higher-level practical utilities
* that trim/split/join helpers are an important user-facing part of the header
* that accepted argument forms follow XER's own API policy

Detailed per-function semantics should be described in the reference manual or generated API sections.

---

## Example Topics Commonly Worth Showing

The following kinds of examples are especially suitable for this header:

* trimming a UTF-8 string with `trim_view`
* splitting and joining text with `explode` / `implode`
* replacing text with `str_replace`
* searching for a Unicode scalar value in UTF-8 text
* performing familiar C-style comparison or copying in XER style

This aligns well with the project direction that executable examples should become the canonical source for user-facing code snippets.

---

## Example

```cpp id="r4yb9n"
#include <xer/stdio.h>
#include <xer/string.h>

auto main() -> int
{
    constexpr std::u8string_view input = u8"  hello  ";

    const auto trimmed = xer::trim_view(input);
    if (!trimmed.has_value()) {
        return 1;
    }

    if (!xer::puts(*trimmed).has_value()) {
        return 1;
    }

    return 0;
}
```

This example shows a typical XER style:

* use UTF-8-oriented string input
* call an ordinary public API with an ordinary value
* check `xer::result` explicitly
* use `<xer/stdio.h>` for text output in examples

---

## See Also

* `policy_project_outline.md`
* `policy_result_arguments.md`
* `policy_examples.md`
* `header_ctype.md`
* `header_stdlib.md`


---

## Prefix and Suffix Checks

`<xer/string.h>` provides prefix and suffix helpers:

```cpp
starts_with
ends_with
```

They accept string-like arguments and are intended to cover common UTF-8 string-view and literal use cases naturally.

---

## Case Conversion and Dynamic String Transformation

The header provides string-level transformation helpers:

```cpp
strtolower
strtoupper
strtoctrans
```

`strtolower` and `strtoupper` apply character conversion to a string and return a transformed string.

`strtoctrans` applies a `ctrans_id` transformation to a string.
For ordinary one-code-point transformations, it acts as the string-level counterpart of `toctrans`.
For transformations that inherently require multiple code points or context across neighboring characters, `strtoctrans` may provide functionality that `toctrans` cannot provide.

Current transformation support includes:

- ASCII and Latin-1 case conversion
- fullwidth/halfwidth transformations
- kana-aware string transformations already implemented through `strtoctrans`
- kana-to-romaji conversion

---

## Kana-to-Romaji Conversion

`strtoctrans` supports romaji conversion from Hiragana and fullwidth Katakana.

```cpp
auto standard =
    xer::strtoctrans(u8"とうきょう", xer::ctrans_id::romaji);
// tōkyō

auto alternate =
    xer::strtoctrans(u8"とうきょう", xer::ctrans_id::romaji_alt);
// toukyou
```

### Conversion Kinds

| `ctrans_id` | Meaning |
|---|---|
| `romaji` | Romanization following the standard long-vowel notation with macrons |
| `romaji_alt` | Alternate notation that writes long vowels by lining up vowel letters according to modern kana spelling |

The romanization policy follows the 2025 Cabinet Notification **“Romanization Spelling”** adopted for Japanese romanization.
The ordinary `romaji` form uses macrons for long vowels.
The `romaji_alt` form provides the separately allowed no-macron spelling style.

### Examples

| Source | `romaji` | `romaji_alt` |
|---|---|---|
| `とうきょう` | `tōkyō` | `toukyou` |
| `おおさか` | `ōsaka` | `oosaka` |
| `えいが` | `ēga` | `eiga` |
| `コーヒー` | `kōhī` | `koohii` |

The conversion also handles ordinary romanization details such as:

- yōon combinations such as `きゃ`, `しゃ`, and `ちゃ`
- sokuon such as `っ`
- syllabic `ん`, including separator apostrophes before vowels and `y`
- long sound marks in Katakana text

### Scope

The initial romaji conversion accepts:

- Hiragana
- fullwidth Katakana
- the Katakana prolonged sound mark `ー`
- ordinary halfwidth and fullwidth spaces where they can be preserved as spacing

Inputs containing unsupported characters are reported as:

```cpp
error_t::invalid_argument
```

### Relationship to `toctrans`

`ctrans_id::romaji` and `ctrans_id::romaji_alt` are string-level transformations.

They are accepted by:

```cpp
strtoctrans
```

They are not meaningful for single-code-point conversion through:

```cpp
toctrans
```

and `toctrans` reports:

```cpp
error_t::invalid_argument
```

for these identifiers.
