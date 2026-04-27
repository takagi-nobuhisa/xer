# Policy for Multibyte Character Conversion in `stdlib.h`

## Overview

The multibyte character conversion facilities in XER's `stdlib.h` are not intended to reproduce the locale-dependent mechanisms of the C standard library as they are.
Instead, they are redesigned in accordance with XER's overall character encoding policy.

The supported character encodings are limited to the following four:

- CP932
- UTF-8
- UTF-16
- UTF-32

State-dependent encodings with escape sequences or shift states are not supported.

---

## Basic Policy

In XER's multibyte character conversion facilities, `char32_t` and `char16_t` are used as the internal reference character types.

`char` and `wchar_t` are treated as interfaces to existing libraries and platform APIs, but they are not used as XER's internal reference types.

The default interpretation of `char` is defined per platform:

- on Windows, the default is CP932
- on Linux, the default is UTF-8

If necessary, this default may be made switchable by a macro.

`wchar_t` is treated as an interface to existing libraries.

- on Windows, it is effectively an interface to UTF-16
- on Linux, it is effectively an interface to UTF-32

However, in either case, `wchar_t` itself is not used as XER's internal reference type.

The following types are used as explicit multibyte input and output types:

- `char`
- `unsigned char`
- `char8_t`

Their meanings are as follows:

- `char`
  - used for interoperation with existing libraries
  - interpreted according to the OS default or an explicit setting
- `unsigned char`
  - fixed to CP932
- `char8_t`
  - fixed to UTF-8

The following types are used as the destination or source character types for conversion:

- `wchar_t`
- `char16_t`
- `char32_t`

---

## Function Set

The following functions are provided for handling a single character:

- `mblen`
- `mbtotc`
- `tctomb`

The following functions are provided for handling strings:

- `mbstotcs`
- `tcstombs`

The meanings of these names are as follows:

- `mb`
  - multibyte
- `tc`
  - text character
- `tcs`
  - text character sequence

`tc` is an abstract name covering the following character types:

- `wchar_t`
- `char16_t`
- `char32_t`

The functionality corresponding to `mbtowc`, `wctomb`, `mbstowcs`, and `wcstombs` in the C standard library is provided under the naming system above.

---

## Overload Policy

These functions are provided as overload sets rather than as templates.

`mbtotc` extracts one character from a multibyte sequence.
It accepts the following input types:

- `const char*`
- `const unsigned char*`
- `const char8_t*`

It accepts the following output types:

- `wchar_t*`
- `char16_t*`
- `char32_t*`

`tctomb` converts one character into a multibyte sequence.
It accepts the following input types:

- `wchar_t`
- `char16_t`
- `char32_t`

It accepts the following output types:

- `char*`
- `unsigned char*`
- `char8_t*`

`mbstotcs` converts a multibyte string into a character string.
It accepts the following input types:

- `const char*`
- `const unsigned char*`
- `const char8_t*`

It accepts the following output types:

- `wchar_t*`
- `char16_t*`
- `char32_t*`

`tcstombs` converts a character string into a multibyte string.
It accepts the following input types:

- `const wchar_t*`
- `const char16_t*`
- `const char32_t*`

It accepts the following output types:

- `char*`
- `unsigned char*`
- `char8_t*`

---

## Stateful Conversion

Dedicated function names corresponding to `mbrlen`, `mbrtowc`, `wcrtomb`, `mbsrtowcs`, and `wcsrtombs` in the C standard library are not provided.

Instead, overloads of `mblen`, `mbtotc`, `tctomb`, `mbstotcs`, and `tcstombs` that take an additional `xer::mbstate_t*` argument are provided.
This expresses stateful conversion.

Overloads that do not take a state argument perform independent conversion on each call.

Overloads that do take a state argument continue conversion using the state stored in `xer::mbstate_t`.

No internal static state is used.

---

## `xer::mbstate_t`

`xer::mbstate_t` is a type that stores the state for stateful conversion.

This type stores at least the following information:

- the encoding kind
- an incomplete byte sequence
- the number of retained bytes

Because an incomplete byte sequence must be interpreted under a specific encoding, the state object must also store the encoding kind.

Even when `char` is accepted, the actual encoding kind is fixed at the point where data is stored into the state.
For example, `char` input on Windows is stored in the state as CP932, while `char` input on Linux is stored as UTF-8.

`xer::mbstate_t` provides a default constructor so that an unused initial state can be created.

A reinitialization member such as `reset()` is not provided.
If the state must be returned to its initial condition, a new `xer::mbstate_t` should be created instead.

---

## Return Type

As a rule, the return type of these functions is:

```cpp
xer::result<std::size_t>
```

The returned size represents the number of source elements consumed or destination elements written, depending on the function.

On failure, these functions return an error through `xer::result`. Encoding failures detected by XER's own conversion logic may use `error_t::encoding_error`. Failures that directly reflect an external implementation's `EILSEQ` may use `error_t::ilseq`.

---

## Summary

- XER multibyte conversion is locale-independent.
- Supported encodings are limited to CP932, UTF-8, UTF-16, and UTF-32.
- Conversion state is explicit through `xer::mbstate_t`.
- No hidden internal static conversion state is used.
- Ordinary conversion failure is reported through `xer::result`.
