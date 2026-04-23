# `<xer/ctype.h>`

## Purpose

`<xer/ctype.h>` provides character classification and character conversion facilities in XER.

This header covers two closely related areas:

- classification of characters into categories such as alphabetic, digit, space, and printable
- conversion of characters such as uppercase/lowercase conversion and related transformations

Its role is similar in spirit to C's `<ctype.h>` and `<wctype.h>`, but the design follows XER's own text model and API policy rather than reproducing the standard library structure exactly.

---

## Main Role

The main role of `<xer/ctype.h>` is to provide a simple and explicit character-handling model that fits the rest of XER.

In particular, it aims to provide:

- locale-independent basic behavior
- a clear distinction between ordinary ASCII-oriented operations and more extended operations
- a unified character argument type
- a dynamic mechanism for classification and conversion when fixed function names are not enough

This makes the header suitable both for straightforward checks and for cases where the caller wants to choose a classification or conversion kind dynamically.

---

## Basic Design Direction

### Locale Independence

The basic `is` functions and `to` functions are locale-independent.

Their behavior corresponds to the `"C"` locale rather than to environment-dependent locale rules.

This is important because XER's broader design tries to minimize dependence on locale.

### Character Type

The argument type for individual character classification and conversion functions is unified to `char32_t`.

This matches XER's general policy that individual Unicode scalar values are handled as `char32_t`.

### ASCII as the Basic Scope

The basic individual functions operate only on the ASCII range.

That means:

- classification functions return `false` for non-ASCII input
- conversion functions return failure for non-ASCII input

This is a deliberate design choice rather than a temporary limitation.

---

## Individual Classification Functions

At minimum, `<xer/ctype.h>` provides individual classification functions such as the following:

```cpp
isalpha
isdigit
isalnum
islower
isupper
isspace
isblank
iscntrl
isprint
isgraph
ispunct
isxdigit
isascii
isoctal
isbinary
```

### Role of These Functions

These functions provide direct and readable checks for common categories.

They are intended for the ordinary case where the caller already knows which category should be tested.

### Return Type

Individual classification functions return:

```cpp
bool
```

### Behavior for Non-ASCII Input

These functions classify only ASCII characters.

If a non-ASCII character is passed, they return `false`.

This keeps the meaning of each function simple and predictable.

---

## Individual Conversion Functions

At minimum, `<xer/ctype.h>` provides individual conversion functions such as:

```cpp
tolower
toupper
```

### Role of These Functions

These functions provide direct conversion for common character transformations.

They are intended for the ordinary case where the caller wants a fixed known transformation.

### Return Type

Individual conversion functions return:

```cpp
xer::result<char32_t>
```

### Behavior

These functions operate only on the ASCII range.

Their behavior is:

* if the input is a conversion target, return the converted character
* if the input is not a conversion target, return the original character as a successful result
* if the input is non-ASCII, return failure

### About `toascii`

A traditional C-family `toascii` is not adopted at present.

This is because `toascii` is often understood as a low-level bit-masking operation that extracts the lower 7 bits, and that does not fit naturally with XER's character-handling policy.

---

## Dynamic Character Classification

In addition to fixed `is` functions, XER provides a dynamic classifier:

```cpp
enum class ctype_id;
auto isctype(char32_t c, ctype_id id) noexcept -> bool;
```

### Role of `isctype`

`isctype` allows the classification kind to be chosen dynamically through a value of `ctype_id`.

This is useful when:

* the classification kind is determined at runtime
* a single function should support multiple categories
* code should not branch manually over many fixed `is...` functions

### Design Direction

`ctype_id` may contain both:

* ASCII-limited categories
* extended categories that also cover non-ASCII characters

This allows one unified API to cover both the basic and extended cases.

### Behavior

When an ASCII-limited category is specified, `isctype` behaves like the corresponding individual `is` function.

When an extended category is specified, it may classify non-ASCII characters as well.

This makes it possible, for example, to keep `isdigit` simple while still supporting richer classification through `isctype` when explicitly requested.

---

## Dynamic Character Conversion

XER also provides a dynamic conversion function:

```cpp
enum class ctrans_id;
auto toctrans(char32_t c, ctrans_id id) -> xer::result<char32_t>;
```

### Role of `toctrans`

`toctrans` allows the conversion kind to be selected dynamically through `ctrans_id`.

This is useful when:

* the desired transformation is chosen at runtime
* a single API should support multiple conversion kinds
* extended conversion categories should be available without creating a large number of fixed function names

### Behavior

When an ASCII-limited category is specified, `toctrans` behaves like the corresponding individual conversion function.

When an extended category is specified, it may perform non-ASCII conversion as well.

---

## Extended Conversion Areas

Extended `toctrans` categories may cover at least the following areas:

* fullwidth/halfwidth conversion for Japanese use
* conversion between Hiragana and Katakana
* uppercase/lowercase conversion for Greek and Cyrillic letters

These are intentionally treated as explicit extended features rather than as behavior automatically implied by the basic ASCII functions.

---

## Fullwidth/Halfwidth Conversion

Fullwidth/halfwidth conversion in `toctrans` is intended mainly for Japanese use.

### Target Characters

The target set may include at least:

* ASCII-compatible letters, digits, symbols, and space
* halfwidth Katakana
* fullwidth Katakana
* punctuation related to Kana, dakuten, and handakuten

### Exclusions

Hiragana is excluded from fullwidth/halfwidth conversion.

### Single-Character Limitation

Because `toctrans` returns a single `char32_t`, some information may be dropped in conversions that would otherwise require multiple output characters.

For example, converting a fullwidth Katakana character with dakuten or handakuten into a halfwidth form may drop the mark when a one-character result is required.

---

## Kana Conversion

`toctrans` also supports conversion between fullwidth Hiragana and fullwidth Katakana.

Typical categories include:

* `katakana`
* `hiragana`

### Intended Meaning

* `katakana`: convert fullwidth Hiragana to the corresponding fullwidth Katakana
* `hiragana`: convert fullwidth Katakana to the corresponding fullwidth Hiragana

### Exclusions

Halfwidth Katakana is excluded from Kana conversion.

This keeps the transformation model simpler and avoids mixing script conversion with width conversion.

---

## Latin-1 and Extended Classification

`<xer/ctype.h>` may also provide extended classification helpers for Latin-1-related cases.

These can include functions such as:

```cpp
islatin1_upper
islatin1_lower
islatin1_alpha
islatin1_alnum
islatin1_print
islatin1_graph
```

### Role of These Functions

They provide an intermediate step between:

* strict ASCII-only individual functions
* fully dynamic or more ambitious Unicode-wide classification

This is useful where the project wants a small, explicit expansion of practical character handling without turning the basic API into a fully locale-driven or fully Unicode-property-driven system.

---

## Relationship to Other Policies

`<xer/ctype.h>` should be understood together with the following documents:

* `policy_project_outline.md`
* `policy_encoding.md`

It is also related to:

* `header_string.md`
* `header_stdlib.md`

The rough boundary is:

* `<xer/ctype.h>` handles classification and character transformation
* `<xer/string.h>` handles string and memory utilities
* `<xer/stdlib.h>` handles multibyte conversion and related facilities
* encoding policy defines the broader model within which these character operations make sense

---

## Documentation Notes

When this header is used in generated documentation, it is usually enough to explain:

* that the basic individual functions are ASCII-oriented and locale-independent
* that `char32_t` is the standard argument type
* that `isctype` and `toctrans` provide dynamic classification and conversion
* that extended categories exist explicitly rather than being implied automatically by the basic API

Detailed per-category semantics should be described in the reference manual or generated API sections.

---

## Example Topics Commonly Worth Showing

The following kinds of examples are especially suitable for this header:

* testing a character with `isalpha` or `isdigit`
* converting a character with `tolower` or `toupper`
* performing dynamic classification with `isctype`
* using `toctrans` for Kana or width conversion

These are good candidates for executable examples in `examples/`.

---

## Example

```cpp
#include <xer/ctype.h>

auto main() -> int
{
    const auto lower = xer::tolower(U'A');
    if (!lower.has_value()) {
        return 1;
    }

    if (*lower != U'a') {
        return 1;
    }

    if (!xer::isdigit(U'7')) {
        return 1;
    }

    return 0;
}
```

This example shows the normal style:

* use `char32_t` for individual character values
* call basic classification directly
* check `xer::result` explicitly for conversion

---

## See Also

* `policy_project_outline.md`
* `policy_encoding.md`
* `header_string.md`
* `header_stdlib.md`
