# `<xer/convert.h>`

## Purpose

`<xer/convert.h>` provides the generic `xer::to<T>` conversion function.

Unlike a C++ cast expression, `xer::to<T>` may parse text, format values, validate numeric ranges, normalize selected xer value types, and fail. Therefore it returns `xer::result<T>`.

---

## Main Role

This header provides a single practical conversion entry point for cases where the requested conversion is more than a simple C++ cast:

```cpp
template <class To, class From>
auto to(From&& from) -> xer::result<To>;
```

Typical uses include:

```cpp
auto n = xer::to<int>(u8"123");
auto s = xer::to<std::u8string>(123);
auto p = xer::to<xer::path>(u8"dir\\file.txt");
```

---

## Basic Rules

`xer::to<T>` applies the following rules in order:

1. If the source and destination types are the same value type, the value is returned directly.
2. Numeric-to-numeric conversion is range-checked with `xer::in_range` before conversion.
3. `signed char` and `unsigned char` are treated as integer types.
4. `char` is treated as a character type, not as a numeric type.
5. Explicitly encoded text strings may be parsed, formatted, or transcoded.
6. Ambiguous narrow `char` strings are not interpreted as text.
7. Unsupported or unsafe conversions return `error_t::invalid_argument`.

---

## Numeric Conversion

```cpp
auto a = xer::to<int>(u8"123");
auto b = xer::to<unsigned char>(u8"255");
auto c = xer::to<double>(u8"3.5");
```

String-to-number conversion accepts explicitly encoded character strings based on these character types:

```cpp
char8_t
char16_t
char32_t
wchar_t
```

Examples:

```cpp
xer::to<int>(u8"123");
xer::to<int>(u"123");
xer::to<int>(U"123");
xer::to<int>(L"123");
```

The entire input must be consumed by the numeric conversion. For example, `u8"123x"` is rejected.

Numeric-to-numeric conversion checks whether the source value is within the destination type's representable range:

```cpp
xer::to<unsigned char>(255); // success
xer::to<unsigned char>(256); // error_t::range
xer::to<unsigned>(-1);       // error_t::range
```

---

## Character Conversion

`char` is treated as a character type.

```cpp
auto s = xer::to<std::u8string>('A'); // u8"A"
auto c = xer::to<char>(u8"A");       // 'A'
```

`char` is not treated as a numeric value:

```cpp
xer::to<int>('A'); // error_t::invalid_argument
```

`signed char` and `unsigned char` are numeric:

```cpp
xer::to<int>(static_cast<signed char>(-5));
xer::to<int>(static_cast<unsigned char>(250));
```

---

## Text Conversion

The following owning destination string types are supported:

```cpp
std::u8string
std::u16string
std::u32string
std::wstring
```

Examples:

```cpp
auto u8  = xer::to<std::u8string>(123);
auto u16 = xer::to<std::u16string>(u8"あ");
auto u32 = xer::to<std::u32string>(u8"あ");
auto w   = xer::to<std::wstring>(u8"あ");
```

`wchar_t` strings are handled according to the target platforms supported by xer:

- 16-bit `wchar_t` is treated as UTF-16.
- 32-bit `wchar_t` is treated as UTF-32.

---

## Narrow `char` Strings

`char` strings are intentionally not interpreted as text by `xer::to<T>`.

```cpp
xer::to<int>("123");                 // error_t::invalid_argument
xer::to<std::u8string>("abc");       // error_t::invalid_argument
xer::to<xer::path>("dir/file.txt");  // error_t::invalid_argument
```

This avoids silently treating platform-dependent narrow strings as UTF-8, locale text, or native path text.

Use an explicitly encoded string instead:

```cpp
xer::to<int>(u8"123");
xer::to<std::u8string>(u8"abc");
xer::to<xer::path>(u8"dir/file.txt");
```

---

## Path Conversion

`xer::path` can be created from explicitly encoded text:

```cpp
auto p1 = xer::to<xer::path>(u8"dir\\file.txt");
auto p2 = xer::to<xer::path>(u"dir\\file.txt");
auto p3 = xer::to<xer::path>(U"dir\\file.txt");
auto p4 = xer::to<xer::path>(L"dir\\file.txt");
```

The resulting `xer::path` stores normalized UTF-8 text and normalizes backslashes to slashes through the ordinary `xer::path` constructor.

---

## Example

```cpp
#include <xer/convert.h>
#include <xer/path.h>
#include <xer/stdio.h>

int main()
{
    auto count = xer::to<int>(u8"42");
    auto path = xer::to<xer::path>(u8"data\\out.txt");
    auto text = xer::to<std::u8string>(3.5);

    if (!count || !path || !text) {
        return 1;
    }

    xer::printf(u8"count = %@\n", *count);
    xer::printf(u8"path = %s\n", path->str().data());
    xer::printf(u8"text = %s\n", text->c_str());
    return 0;
}
```
