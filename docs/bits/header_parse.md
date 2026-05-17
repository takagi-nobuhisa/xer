# `<xer/parse.h>`

## Purpose

`<xer/parse.h>` provides common structured detail types for parsed input errors.

The header is intentionally independent from JSON, INI, TOML, XBF bitmap-font data, and other individual parsers or loaders. A parse-failure position and a parse-failure reason are useful across multiple structured-input APIs, so XER provides a shared vocabulary rather than format-specific detail types.

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

`parse_error_reason` is a format-neutral reason code for structured input failures.

The current reason set includes:

```cpp
enum class parse_error_reason {
    none,
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
    invalid_magic,
    unsupported_version,
    invalid_header,
    invalid_range,
    invalid_offset,
    truncated_input,
};
```

The reason is a more detailed classification than `error_t`. For example:

- a text parser may use `error_t::invalid_argument` together with `parse_error_reason::duplicate_key`
- an XBF bitmap-font loader may use `error_t::invalid_argument` together with `parse_error_reason::invalid_magic`

### Binary-Structure Reasons

The following reasons are intended for binary formats or other structured inputs that have fixed headers, offsets, ranges, or finite byte spans:

- `invalid_magic`
- `unsupported_version`
- `invalid_header`
- `invalid_range`
- `invalid_offset`
- `truncated_input`

They are currently used by `xer::image::bitmap_font_load` when validating XBF bitmap-font data.

---

## `parse_error_detail`

```cpp
struct parse_error_detail {
    std::size_t offset = 0;
    std::size_t line = 0;
    std::size_t column = 0;
    parse_error_reason reason = parse_error_reason::none;
};
```

### Position Meaning

`offset` is the zero-based position from the beginning of the input.

- For UTF-8 text formats, it is counted in UTF-8 code units.
- For binary formats, it is counted in bytes.

`line` is the one-based line number when line information is available. It is `0` when line information is not available.

`column` is the one-based column number when column information is available. For UTF-8 text formats, it is counted in UTF-8 code units, not display cells, Unicode scalar values, or grapheme clusters. It is `0` when column information is not available.

This rule keeps parser diagnostics aligned with XER's `char8_t`-based text model while also allowing binary loaders to report exact byte positions without inventing fake line or column values.

---

## Intended Use

A text parser that can report structured detail may return a result such as:

```cpp
auto toml_decode(std::u8string_view text)
    -> xer::result<toml_value, parse_error_detail>;
```

A binary loader may use the same detail type:

```cpp
auto bitmap_font_load(const xer::path& filename)
    -> xer::result<xer::image::bitmap_font, parse_error_detail>;
```

When parsing or loading fails, callers can inspect both the common error code and the parse-specific detail.

```cpp
const auto result = xer::toml_decode(text);
if (!result.has_value()) {
    const auto& error = result.error();
    // error.code
    // error.offset
    // error.line
    // error.column
    // error.reason
}
```

Because `error<Detail>` exposes class-type detail naturally, `offset`, `line`, `column`, and `reason` are directly available on the returned error object.

---

## Relationship to Other Headers

`<xer/parse.h>` is related to:

- `<xer/error.h>`
- `<xer/json.h>`
- `<xer/ini.h>`
- `<xer/toml.h>`
- `<xer/image.h>`

The rough boundary is:

- `<xer/error.h>` provides the common result and error model
- `<xer/parse.h>` provides shared structured-input error detail
- data-format headers use that detail where position-aware diagnostics are useful
- `<xer/image.h>` uses that detail for XBF bitmap-font loading

---

## Documentation Notes

`parse_error_detail` is intentionally not named after TOML, JSON, INI, or XBF. It is a common structured-input detail type and should remain reusable by multiple parsers and loaders.

### No-detail State

`parse_error_reason::none` means that the parse-detail payload is intentionally unused. In that state, `offset`, `line`, and `column` are all zero.

This is used by convenience functions such as `json_load`, `ini_load`, `toml_load`, and `bitmap_font_load` when file I/O fails before format parsing begins.

When `reason` is not `none`:

- `offset` is always a zero-based input position
- `line` and `column` are one-based only when line information exists
- binary loaders keep `line` and `column` at `0`
