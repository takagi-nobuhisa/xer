# Conversion Policy

## Purpose

This document describes the policy behind `xer::to<T>` in `<xer/convert.h>`.

`xer::to<T>` is a practical value conversion function. It is not a C++ cast expression and should not be treated as a synonym for `static_cast`, `reinterpret_cast`, or any other cast form.

Because conversion may involve parsing, formatting, validation, transcoding, or path normalization, `xer::to<T>` returns `xer::result<T>`.

---

## Name

The function is named `to` rather than `cast` or `lexical_cast`.

The name `cast` suggests a C++ cast-like operation. That would be misleading because this function may fail and may perform higher-level conversion work.

The name `to` emphasizes the destination type:

```cpp
auto n = xer::to<int>(u8"123");
auto s = xer::to<std::u8string>(123);
```

---

## Numeric Safety

Arithmetic-to-arithmetic conversion is range-checked.

`xer::to<T>` uses `xer::in_range<T>(value)` before converting numeric values. If the value is not representable by the destination type, the conversion fails with `error_t::range`.

This prevents silent narrowing such as:

```cpp
xer::to<unsigned char>(256);
xer::to<unsigned>(-1);
```

---

## Character Policy

`char` is treated as a character type, not as a small integer type.

`signed char` and `unsigned char` are treated as integer types. This distinction is intentional because `std::int8_t` and `std::uint8_t` are commonly aliases of signed and unsigned character types.

Examples:

```cpp
xer::to<std::u8string>('A');                  // character conversion
xer::to<int>('A');                            // rejected
xer::to<int>(static_cast<signed char>(-5));   // integer conversion
xer::to<int>(static_cast<unsigned char>(250));// integer conversion
```

---

## Text Encoding Policy

`xer::to<T>` accepts explicitly encoded character strings based on:

- `char8_t`
- `char16_t`
- `char32_t`
- `wchar_t`

It intentionally does not interpret narrow `char` strings as text.

Narrow `char` strings may mean UTF-8, the current locale, a Windows ANSI code page, raw bytes, or some application-specific encoding. Choosing one interpretation inside `xer::to<T>` would be unsafe, especially for paths.

Therefore these conversions are rejected:

```cpp
xer::to<int>("123");
xer::to<std::u8string>("abc");
xer::to<xer::path>("dir/file.txt");
```

Use an explicitly encoded string instead:

```cpp
xer::to<int>(u8"123");
xer::to<std::u8string>(u8"abc");
xer::to<xer::path>(u8"dir/file.txt");
```

---

## Wide Character Policy

`wchar_t` is implementation-dependent, but xer targets environments where this is predictable:

- on MSYS2 UCRT64 / MinGW-w64, `wchar_t` is treated as UTF-16
- on Ubuntu and similar Unix-like environments, `wchar_t` is treated as UTF-32

The implementation follows `sizeof(wchar_t)` when converting wide strings.

---

## Path Policy

`xer::to<xer::path>` accepts explicitly encoded text and constructs `xer::path` from normalized UTF-8 text.

It does not accept narrow `char` strings because narrow path strings are platform-sensitive. In particular, Windows narrow paths are normally interpreted according to an ANSI code page, not automatically as UTF-8.

Native or locale-dependent path conversions should be separate explicit APIs if they become necessary.

---

## Relationship with `sprintf` and `sscanf`

`xer::to<T>` uses existing formatting and scanning facilities where appropriate, but its public rule is stricter than general diagnostic formatting.

In particular, `printf`-style `%@` may continue to support existing diagnostic behavior for narrow strings, while `xer::to<T>` does not interpret `char` strings as text.

This separation keeps compatibility for formatting while keeping generic conversion safe.
