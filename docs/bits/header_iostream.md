# `<xer/iostream.h>`

## Purpose

`<xer/iostream.h>` provides formatted iostream insertion and extraction operators for selected XER value types.

This header does not make iostreams the main input/output model of XER. XER's primary I/O model remains based on `binary_stream`, `text_stream`, and the functions provided by `<xer/stdio.h>`. The role of `<xer/iostream.h>` is narrower: it provides a bridge for diagnostics, tests, examples, and the implementation of generic `%@` formatting and scanning support.

The header is intentionally opt-in. Users include it only when they want ordinary C++ iostream operators for XER value types.

---

## Main Entities

At minimum, `<xer/iostream.h>` makes `operator<<` available for the following types. The `error_t` and `error<void>` insertion operators are defined by `<xer/error.h>` and are available because this header includes it:

```cpp
xer::error_t
xer::error<void>
xer::type_info
xer::path
xer::cyclic<T>
xer::interval<T, Min, Max>
```

It also provides `operator>>` for the following types where formatted extraction is straightforward:

```cpp
xer::error_t
xer::path
xer::cyclic<T>
xer::interval<T, Min, Max>
```

The initial scope is deliberately small. More complex types, such as matrices, quantities, colors, JSON, INI, and TOML values, may be considered later.

---

## Design Role

The main design role of this header is to make XER-provided value types usable by generic stream-based formatting paths.

In particular, it supports facilities that internally rely on stream insertion or extraction for default formatting, such as enhanced `%@` handling in the printf and scanf families.

The operators are not intended to replace XER's own text I/O APIs.

---

## UTF-8 Handling

XER uses `char8_t` and `std::u8string_view` for UTF-8 text. Ordinary iostreams use `char` streams.

For this reason, `<xer/iostream.h>` writes UTF-8 text to `std::ostream` by copying the underlying UTF-8 bytes to the stream without locale-dependent conversion. This applies to types such as `path` and `type_info`, whose display strings are UTF-8-oriented.

Formatted extraction from `std::istream` reads ordinary narrow tokens and copies the bytes into UTF-8 storage where appropriate.

---

## `error_t`

`operator<<` for `error_t` writes the enumerator name returned by `get_error_name`.

Example output:

```text
invalid_argument
not_found
io_error
```

`operator>>` accepts enumerator names without the `error_t::` prefix. Unknown names set the stream fail bit.

---

## `error<void>`

`operator<<` for `error<void>` writes the compact diagnostic representation already provided by `<xer/error.h>`:

```text
xer::error{code=invalid_argument}
```

The source location stored in the error object is not written. This keeps the default representation short and suitable for assertion messages, trace output, and `%@` formatting.

There is no extraction operator for `error<void>`. Error objects are normally created by failed operations, not read from formatted input.

---

## `type_info`

`operator<<` for `type_info` writes the display name returned by `type_info::name()`.

This is intended for diagnostics. The spelling remains implementation-dependent, just like the underlying type information facility.

There is no extraction operator for `type_info`.

---

## `path`

`operator<<` for `path` writes the normalized UTF-8 path returned by `path::str()`.

`operator>>` reads one whitespace-delimited token and constructs a `path` from it. As usual for formatted extraction, paths containing whitespace are not handled by this operator. Use line-oriented input and construct `xer::path` explicitly when such paths are needed.

---

## `cyclic<T>`

`operator<<` for `cyclic<T>` writes the normalized scalar value returned by `value()`.

`operator>>` reads a scalar value and constructs `cyclic<T>` from it. The ordinary `cyclic` normalization rules apply.

Example:

```text
-0.25
```

is read as the normalized cyclic value:

```text
0.75
```

---

## `interval<T, Min, Max>`

`operator<<` for `interval<T, Min, Max>` writes the stored scalar value returned by `value()`.

`operator>>` reads a scalar value and constructs an interval value from it. The ordinary `interval` rules apply: finite out-of-range values are clamped, while invalid floating-point values such as NaN or infinity cause extraction to set the stream fail bit if interval construction rejects them.

---

## Deferred Items

The following are intentionally deferred from the initial implementation:

- `operator<<` and `operator>>` for `error<Detail>`
- `operator<<` and `operator>>` for `quantity`
- `operator<<` and `operator>>` for `matrix`
- `operator<<` and `operator>>` for color types
- `operator<<` and `operator>>` for JSON, INI, and TOML values
- stream insertion for resource handles such as `binary_stream`, `text_stream`, `process`, and `socket`

These types either need additional formatting policy or are not ordinary value types suitable for default formatted extraction.

---

## Relationship to Other Headers

`<xer/iostream.h>` is related to the following headers:

- `<xer/error.h>`
- `<xer/typeinfo.h>`
- `<xer/path.h>`
- `<xer/cyclic.h>`
- `<xer/interval.h>`
- `<xer/stdio.h>`

The rough boundary is:

- `<xer/stdio.h>` remains the ordinary XER text I/O header
- `<xer/iostream.h>` provides opt-in compatibility with standard iostream formatting
- individual value-type headers remain usable without pulling in iostream support

---

## Example

```cpp
#include <iostream>
#include <sstream>

#include <xer/cyclic.h>
#include <xer/interval.h>
#include <xer/iostream.h>
#include <xer/path.h>

auto main() -> int
{
    const auto path = xer::path(u8"work/file.txt");
    const auto angle = xer::cyclic<double>(1.25);
    const auto gain = xer::interval<double>(1.25);

    std::cout << path << '\n';
    std::cout << angle << '\n';
    std::cout << gain << '\n';

    std::istringstream input("logs/output.txt -0.25 0.5");
    xer::path read_path;
    xer::cyclic<double> read_angle;
    xer::interval<double> read_gain;

    input >> read_path >> read_angle >> read_gain;
    return input ? 0 : 1;
}
```

---

## See Also

- `header_error.md`
- `header_typeinfo.md`
- `header_path.md`
- `header_cyclic.md`
- `header_interval.md`
- `header_stdio.md`
