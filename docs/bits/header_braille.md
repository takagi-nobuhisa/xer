# `<xer/braille.h>`

## Purpose

`<xer/braille.h>` provides low-level braille-related building blocks for XER.

At the current stage, this header only provides common braille sign constants as UTF-8 string views.
It does not yet perform text-to-braille conversion, Japanese braille translation, English braille translation, contraction handling, or wakachi-gaki.

This keeps the first braille layer small and stable: callers can use the provided constants as parts of larger braille strings while higher-level conversion APIs are added later.

---

## Main Role

The main role of `<xer/braille.h>` is to expose reusable braille prefix signs in a form that can be concatenated with ordinary `std::u8string` values.

The current implementation provides:

- Japanese braille numeric and alphabetic indicators
- Japanese braille capital indicators
- information-processing braille indicators

All constants are represented as:

```cpp
std::u8string_view
```

This avoids allocation for the constants themselves and makes them easy to append to `std::u8string` output buffers.

---

## Main Entities

`<xer/braille.h>` currently provides the following constants:

```cpp
namespace xer::braille {

inline constexpr std::u8string_view numeric_indicator;
inline constexpr std::u8string_view alphabetic_indicator;
inline constexpr std::u8string_view capital_indicator;
inline constexpr std::u8string_view double_capital_indicator;

namespace information_processing {

inline constexpr std::u8string_view lowercase_indicator;
inline constexpr std::u8string_view uppercase_indicator;
inline constexpr std::u8string_view single_uppercase_indicator;
inline constexpr std::u8string_view double_uppercase_indicator;
inline constexpr std::u8string_view numeric_indicator;

} // namespace information_processing

} // namespace xer::braille
```

---

## Japanese Braille Indicators

### `numeric_indicator`

```cpp
inline constexpr std::u8string_view numeric_indicator = u8"⠼";
```

`numeric_indicator` is the Japanese braille numeric indicator.

It is the Unicode braille pattern for dots 3-4-5-6.

Example:

```cpp
std::u8string text;
text += xer::braille::numeric_indicator;
text += u8"⠁⠃⠉";
```

---

### `alphabetic_indicator`

```cpp
inline constexpr std::u8string_view alphabetic_indicator = u8"⠰";
```

`alphabetic_indicator` is the Japanese braille alphabetic indicator.

It is the Unicode braille pattern for dots 5-6.

Example:

```cpp
std::u8string text;
text += xer::braille::alphabetic_indicator;
text += u8"⠁⠃⠉";
```

---

### `capital_indicator`

```cpp
inline constexpr std::u8string_view capital_indicator = u8"⠠";
```

`capital_indicator` is the Japanese braille capital indicator.

It is the Unicode braille pattern for dot 6.

Example:

```cpp
std::u8string text;
text += xer::braille::capital_indicator;
text += u8"⠁";
```

---

### `double_capital_indicator`

```cpp
inline constexpr std::u8string_view double_capital_indicator = u8"⠠⠠";
```

`double_capital_indicator` represents two consecutive capital indicators.

It is useful when constructing a braille string that needs a double-capital prefix.

Example:

```cpp
std::u8string text;
text += xer::braille::double_capital_indicator;
text += u8"⠁⠃⠉";
```

---

## Information-Processing Braille Indicators

Information-processing braille indicators are placed under:

```cpp
namespace xer::braille::information_processing
```

This keeps them separate from the ordinary Japanese braille indicators while still making them available through the same public header.

### `lowercase_indicator`

```cpp
inline constexpr std::u8string_view lowercase_indicator = u8"⠰";
```

`lowercase_indicator` is the information-processing braille lowercase indicator.

It is the Unicode braille pattern for dots 5-6.

---

### `uppercase_indicator`

```cpp
inline constexpr std::u8string_view uppercase_indicator = u8"⠠";
```

`uppercase_indicator` is the information-processing braille uppercase indicator.

It is the Unicode braille pattern for dot 6.

---

### `single_uppercase_indicator`

```cpp
inline constexpr std::u8string_view single_uppercase_indicator = u8"⠠";
```

`single_uppercase_indicator` represents the single-uppercase indicator.

At the current stage, it has the same cell as `uppercase_indicator`.

---

### `double_uppercase_indicator`

```cpp
inline constexpr std::u8string_view double_uppercase_indicator = u8"⠠⠠";
```

`double_uppercase_indicator` represents two consecutive uppercase indicators.

---

### `numeric_indicator`

```cpp
inline constexpr std::u8string_view numeric_indicator = u8"⠼";
```

`numeric_indicator` is the information-processing braille numeric indicator.

It is the Unicode braille pattern for dots 3-4-5-6.

---

## Design Notes

### Constants Are UTF-8 String Views

The constants are `std::u8string_view`, not `char32_t` or a dedicated cell type.

This is intentional for the first layer because these constants represent output text fragments rather than abstract braille cells.
They can be appended directly to `std::u8string` without conversion.

### No Translation Semantics Yet

These constants do not perform any automatic state transition, validation, or translation.
For example, appending `numeric_indicator` does not check whether the following cells are valid numeric braille cells.

Higher-level APIs may later use these constants internally, but this header currently exposes only reusable low-level parts.

### Relation to `isctype`

`<xer/ctype.h>` provides `isctype(c, ctype_id::braille)` for checking whether a Unicode scalar value belongs to the Unicode Braille Patterns block.

`<xer/braille.h>` is different: it provides actual UTF-8 braille sign constants that can be used when constructing braille strings.

---

## Example

```cpp
#include <xer/braille.h>

#include <string>

int main()
{
    std::u8string text;

    text += xer::braille::numeric_indicator;
    text += u8"⠁⠃⠉";

    return 0;
}
```
