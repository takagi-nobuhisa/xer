# Policy for Floating-Point Type Aliases

## Overview

XER provides `<xer/stdfloat.h>` as a floating-point counterpart to `<xer/stdint.h>`.

The purpose is not to promise that every implementation supports every floating-point format, but to provide a clear and testable vocabulary for the formats that are available.

---

## Basic Policy

- `float32_t` and `float64_t` are always available in XER.
- Optional formats such as `float16_t`, `float80_t`, `float128_t`, and `bfloat16_t` are exposed only when supported.
- Availability is expressed through `XER_HAS_...` macros.
- Least-width and fast-width aliases are provided where meaningful.
- User-defined literals are provided under a dedicated literals namespace.

---

## Relationship to `<stdfloat>`

When the implementation provides the standard `<stdfloat>` header and the corresponding feature macros, XER reuses the standard aliases.

When the standard aliases are not available, XER provides practical fallbacks only where the meaning is clear.
For example, `float32_t` may fall back to `float`, and `float64_t` may fall back to `double`.

---

## 80-Bit Floating-Point Type

`float80_t` represents the 80-bit extended floating-point format when `long double` has that format.

The name describes the floating-point format, not necessarily the object storage size.
Some targets store the 80-bit format in a larger object representation.

If `float80_t` is unavailable but `float128_t` is available, `float_least80_t` may be defined as `float128_t`.

---

## Decimal Floating-Point Types

If the implementation provides decimal floating-point support through `<decimal/decimal>`, XER exposes decimal aliases.

Decimal floating-point support is optional and must be guarded by the corresponding availability macros.

---

## User-Defined Literals

Floating-point literal suffixes are provided in:

```cpp
xer::literals::floating_literals
```

Exact-width suffixes such as `_f32` and `_f64` are useful when the destination format is fixed.
Least-width suffixes such as `_fl32`, `_fl64`, and `_fl80` are useful when the code wants a minimum capability while allowing the implementation to choose the available type.

---

## Summary

- `<xer/stdfloat.h>` provides floating-point aliases and literals
- optional formats are exposed only when available
- availability macros are part of the public contract for conditional use
- least-width aliases improve portability across implementations with different floating-point support
