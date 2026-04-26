# XER printf Format Specifiers

## Scope

This document describes the format strings used by the XER printf family.

Target functions:

```cpp
printf
fprintf
sprintf
snprintf
```

---

## Basic Policy

XER printf-style functions are inspired by C printf, but they are not strict source-compatible reimplementations.

- format strings are UTF-8 strings
- fixed text in the format string is copied as UTF-8
- conversion specifications start with `%`
- ordinary failure is reported through `xer::result`
- XER-specific extensions may exist

A format string may contain ordinary UTF-8 text and conversion specifications.
Ordinary text is copied to the output as-is.

---

## Conversion Specification Syntax

A conversion specification begins with `%`.

The currently supported structure is:

```text
%[position$][flags][width][.precision][length]conversion
```

The positional form is optional.
When it is used, the first argument is numbered `1`.

Examples:

```cpp
xer::printf(u8"%@ %@\n", first, second);
xer::printf(u8"%2$@ %1$@\n", first, second);
```

---

## Flags

The following flags are recognized:

```text
- + space # 0
```

Their meanings follow the usual printf-style interpretation where applicable.
For conversions where a flag has no meaningful effect, it may be ignored.

---

## Width and Precision

A field width may be specified as a decimal integer or by `*`.

A precision may be specified with `.` followed by a decimal integer or by `*`.

Both width and precision may use positional arguments.

Examples:

```cpp
xer::printf(u8"%10@\n", value);
xer::printf(u8"%.*@\n", precision, value);
xer::printf(u8"%2$*1$@\n", width, value);
```

Width is counted in UTF-8 code units in the current implementation.
It is not a display-cell width calculation.

---

## Length Modifiers

The following length modifiers are parsed:

```text
hh h l ll j z t L
```

They are accepted as part of the printf-style grammar.
The actual effect depends on the conversion and on XER's internal argument normalization.

For floating-point conversions, `L` is used when constructing the intermediate narrow format passed to `std::snprintf`.

---

## Supported C-Style Conversions

The following C-style conversion specifiers are supported:

```text
%d %i
%u
%o
%x %X
%c
%s
%p
%e %E
%f %F
%g %G
%a %A
%%
```

`%%` outputs a literal percent sign and does not consume an argument.

---

## XER Generic Display Conversion: `%@`

`%@` is XER's generic display specifier.

It is intended for diagnostics, examples, tracing, and simple output where precise base, padding, or precision control is not the main concern.
When precise formatting is required, ordinary printf-style conversions should be used instead.

### Argument Conversion Rules

Arguments passed to `%@` are normalized to UTF-8 text according to the following rules:

1. `char8_t`, `char8_t*`, `std::u8string`, and `std::u8string_view` are treated directly as UTF-8.
2. `char16_t*`, `std::u16string`, and `std::u16string_view` are converted from UTF-16 to UTF-8.
3. `char32_t*`, `std::u32string`, and `std::u32string_view` are converted from UTF-32 to UTF-8.
4. `wchar_t*`, `std::wstring`, and `std::wstring_view` are converted according to the width of `wchar_t`.
5. `std::string` and `std::string_view` are treated as UTF-8 byte strings.
6. `bool` is formatted as `true` or `false`.
7. `nullptr` is formatted as `null`.
8. Other stream-insertable types are formatted through `std::ostringstream` and the resulting narrow string is treated as UTF-8 bytes.

Invalid UTF-16 or UTF-32 scalar data may be represented by the replacement character in diagnostic-oriented conversions.

### XER Types

The following XER types are intended to be printable through `%@`:

```cpp
xer::error_t
xer::error<Detail>
xer::result<T, Detail>
```

These types provide stream insertion support so that `%@` can display them through the generic stream-based route.

### Notes on `std::ostringstream`

XER does not use iostreams as its primary public I/O model.
However, `%@` may use `std::ostringstream` internally as a practical interoperability mechanism.
This keeps user-facing XER formatted I/O based on `xer::printf` and related functions while allowing types that support `operator<<` to be displayed conveniently.

---

## Error Handling

Format errors, unsupported argument kinds, missing arguments, and out-of-range width or precision arguments are reported through `xer::result`.

The exact error category may be refined as the implementation evolves, but invalid format usage is generally treated as an ordinary formatting failure rather than as undefined behavior.

---

## Implementation Notes

This document is intended to describe the user-visible printf-family behavior.
When implementation details in `xer/bits/printf_format.h` change, this document should be kept in sync.
