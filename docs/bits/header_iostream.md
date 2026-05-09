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
xer::quantity<T, Dim>
xer::matrix<T, Rows, Cols>
xer::image::point
xer::image::pointf
xer::image::size
xer::image::sizef
xer::image::rect
xer::image::rectf
xer::basic_rgb<T>
xer::basic_gray<T>
xer::basic_cmy<T>
xer::basic_hsv<T>
xer::basic_xyz<T>
xer::basic_lab<T>
xer::basic_luv<T>
```

It also provides `operator>>` for the following types where formatted extraction is straightforward:

```cpp
xer::error_t
xer::path
xer::cyclic<T>
xer::interval<T, Min, Max>
xer::quantity<T, Dim>
xer::image::point
xer::image::pointf
xer::image::size
xer::image::sizef
xer::image::rect
xer::image::rectf
```

Formatted extraction for matrices and color values is intentionally deferred because their insertion format is meant for diagnostics rather than as a stable serialized grammar.

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

## `quantity<T, Dim>`

`operator<<` for `quantity<T, Dim>` writes the stored value in the base unit system.

`operator>>` reads a scalar value and constructs a quantity of the destination dimension. The input value is interpreted as already normalized to the base unit system.

For example, if the destination type is a length quantity, the input value is interpreted as meters, not as kilometers or centimeters.

---

## `matrix<T, Rows, Cols>`

`operator<<` for `matrix<T, Rows, Cols>` writes a compact row-major diagnostic form.

Example output for a 2x2 matrix:

```text
[[1, 2], [3, 4]]
```

There is no extraction operator for matrices at this stage. Parsing a matrix would require committing the diagnostic output form to a stable input grammar, which is intentionally avoided for now.

---
## Image Geometry Types

`operator<<` and `operator>>` are provided for the image geometry helper types in `xer::image`.

The stream forms are intentionally compact:

```text
point  -> (x, y)
size   -> {width, height}
rect   -> (x, y) {width, height}
```

The floating-point variants use the same spelling:

```text
pointf -> (x, y)
sizef  -> {width, height}
rectf  -> (x, y) {width, height}
```

Extraction accepts the same forms and allows ordinary formatted-input whitespace around punctuation and values. For example, all of the following are valid when reading a `rect`:

```text
(10,20){30,40}
(10, 20) {30, 40}
( 10, 20 ) { 30, 40 }
```

The extraction grammar is intentionally strict about punctuation. A point uses parentheses, a size uses braces, and a rectangle is a point followed by a size. Forms such as `point(10, 20)`, `size(30, 40)`, and `rect(10, 20, 30, 40)` are not accepted by these operators.

These operators also make image geometry values usable through generic `%@` formatting and scanning paths that rely on stream insertion or extraction.

---

## Color Types

`operator<<` is provided for the basic color value types.

Example output:

```text
rgb(1, 0.5, 0)
gray(0.25)
cmy(0, 0.5, 1)
hsv(0.25, 0.5, 1)
xyz(0.1, 0.2, 0.3)
lab(50, 10, -20)
luv(50, 10, -20)
```

There are no extraction operators for color values at this stage. The insertion format is intended for diagnostics and `%@` formatting, not for stable serialization.

---

## Deferred Items

The following are intentionally deferred from the current implementation:

- `operator<<` and `operator>>` for `error<Detail>`
- `operator>>` for `matrix`
- `operator>>` for color types
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
- `<xer/quantity.h>`
- `<xer/matrix.h>`
- `<xer/image.h>`
- `<xer/color.h>`
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

#include <xer/color.h>
#include <xer/cyclic.h>
#include <xer/image.h>
#include <xer/interval.h>
#include <xer/iostream.h>
#include <xer/matrix.h>
#include <xer/path.h>
#include <xer/quantity.h>

auto main() -> int
{
    using namespace xer::units;

    const auto path = xer::path(u8"work/file.txt");
    const auto angle = xer::cyclic<double>(1.25);
    const auto gain = xer::interval<double>(1.25);
    const auto distance = 1.5 * km;
    const auto transform = xer::matrix<double, 2, 2>(1.0, 2.0, 3.0, 4.0);
    const auto area = xer::image::rect(
        xer::image::point(10, 20),
        xer::image::size(30, 40));
    const auto color = xer::rgb(1.0f, 0.5f, 0.0f);

    std::cout << path << '\n';
    std::cout << angle << '\n';
    std::cout << gain << '\n';
    std::cout << distance << '\n';
    std::cout << transform << '\n';
    std::cout << color << '\n';

    std::istringstream input("logs/output.txt -0.25 0.5 2.5");
    xer::path read_path;
    xer::cyclic<double> read_angle;
    xer::interval<double> read_gain;
    xer::quantity<double, xer::units::length_dim> read_distance;

    input >> read_path >> read_angle >> read_gain >> read_distance;
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
- `header_quantity.md`
- `header_matrix.md`
- `header_image.md`
- `header_color.md`
- `header_stdio.md`
