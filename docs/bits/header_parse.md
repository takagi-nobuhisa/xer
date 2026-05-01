# `<xer/parse.h>`

## Purpose

`<xer/parse.h>` provides common structured detail types for text parse errors.

The header is intentionally independent from JSON, INI, TOML, and other individual parsers.  A parse failure position and a parse-failure reason are useful across multiple text-oriented APIs, so XER provides a shared vocabulary rather than format-specific detail types.

---

## Main Entities

At minimum, `<xer/parse.h>` provides the following entities:

```cpp
enum class xer::parse_error_reason;

struct xer::parse_error_detail;
```

`parse_error_detail` is intended for use as the `Detail` argument of `xer::result<T, Detail>`.

---

## `parse_error_reason`

`parse_error_reason` is a format-neutral reason code for text parse failures.

The current reason set includes:

```cpp
enum class parse_error_reason {
    invalid_syntax,
    invalid_encoding,
    invalid_token,
    invalid_key,
    duplicate_key,
    duplicate_table,
    invalid_string,
    invalid_escape,
    invalid_unicode_escape,
    invalid_number,
    integer_out_of_range,
    invalid_date_time,
    invalid_array,
    invalid_table,
};
```

The reason is a more detailed classification than `error_t`.  For example, a parser may use `error_t::invalid_argument` as the common error category while using `parse_error_reason::duplicate_key` or `parse_error_reason::invalid_date_time` as the parse-specific detail.

---

## `parse_error_detail`

```cpp
struct parse_error_detail {
    std::size_t offset = 0;
    std::size_t line = 1;
    std::size_t column = 1;
    parse_error_reason reason = parse_error_reason::invalid_syntax;
};
```

### Position Meaning

`offset` is the input offset from the beginning of the text.  It is zero-based and counted in UTF-8 code units.

`line` is the line number.  It is one-based.

`column` is the column number.  It is one-based and counted in UTF-8 code units, not display cells, Unicode scalar values, or grapheme clusters.

This rule keeps parser diagnostics aligned with XER's `char8_t`-based text model and avoids introducing display-width or locale-dependent behavior into low-level parsers.

---

## Intended Use

A parser that can report structured detail may return a result such as:

```cpp
auto toml_decode(std::u8string_view text)
    -> xer::result<toml_value, parse_error_detail>;
```

When parsing fails, callers can inspect both the common error code and the parse-specific detail.

```cpp
const auto result = xer::toml_decode(text);
if (!result.has_value()) {
    const auto& error = result.error();
    // error.code
    // error.line
    // error.column
    // error.reason
}
```

Because `error<Detail>` exposes class-type detail naturally, `line`, `column`, and `reason` are directly available on the returned error object.

---

## Relationship to Other Headers

`<xer/parse.h>` is related to:

- `<xer/error.h>`
- `<xer/json.h>`
- `<xer/ini.h>`
- `<xer/toml.h>`

The rough boundary is:

- `<xer/error.h>` provides the common result and error model
- `<xer/parse.h>` provides parser-specific structured detail
- data-format headers use that detail where position-aware diagnostics are useful

---

## Documentation Notes

`parse_error_detail` is intentionally not named after TOML, JSON, or INI.  It is a common parse detail type and should remain reusable by multiple parsers.
