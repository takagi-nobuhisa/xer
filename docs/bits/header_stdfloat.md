# `<xer/stdfloat.h>`

## Purpose

`<xer/stdfloat.h>` provides floating-point type aliases and floating-point user-defined literals in the same spirit as `<xer/stdint.h>`.

The header is intended to make floating-point width and minimum-width intent explicit while still remaining usable on implementations where C++23 `<stdfloat>` support is incomplete.

---

## Main Role

This header provides:

- fixed-width floating-point aliases where the implementation provides them
- practical fallback aliases for `float32_t` and `float64_t`
- optional aliases for 80-bit and 128-bit floating-point formats
- least-width and fast-width floating-point aliases
- optional decimal floating-point aliases when the implementation provides them
- floating-point user-defined literals under `xer::literals::floating_literals`

---

## Availability Macros

The header defines availability macros for optional types.

Examples include:

```cpp
XER_HAS_FLOAT16_T
XER_HAS_FLOAT32_T
XER_HAS_FLOAT64_T
XER_HAS_FLOAT80_T
XER_HAS_FLOAT128_T
XER_HAS_BFLOAT16_T
XER_HAS_FLOAT_LEAST80_T
XER_HAS_FLOAT_FAST80_T
XER_HAS_DECIMAL32_T
XER_HAS_DECIMAL64_T
XER_HAS_DECIMAL128_T
```

These macros allow code and tests to guard features that depend on implementation support.

---

## Binary Floating-Point Aliases

At minimum, the following aliases are provided when possible:

```cpp
float16_t
float32_t
float64_t
float80_t
float128_t
bfloat16_t
```

`float32_t` and `float64_t` are always available in XER. If the standard `<stdfloat>` aliases are not available, they fall back to `float` and `double` respectively.

`float80_t`, `float128_t`, and `bfloat16_t` are optional and are available only when the implementation provides a suitable underlying type.

---

## Least and Fast Floating-Point Aliases

The header provides least-width and fast-width aliases such as:

```cpp
float_least16_t
float_least32_t
float_least64_t
float_least80_t
float_least128_t

float_fast16_t
float_fast32_t
float_fast64_t
float_fast80_t
float_fast128_t
```

`float_least80_t` uses `float80_t` when available, and otherwise uses `float128_t` when that is available.

---

## Maximum Floating-Point Alias

```cpp
floatmax_t
```

`floatmax_t` is selected from the widest practical binary floating-point type available to XER.

---

## Decimal Floating-Point Aliases

When the implementation provides `<decimal/decimal>`, XER exposes decimal floating-point aliases such as:

```cpp
decimal32_t
decimal64_t
decimal128_t

decimal_least32_t
decimal_least64_t
decimal_least128_t

decimal_fast32_t
decimal_fast64_t
decimal_fast128_t

decimalmax_t
```

These aliases are optional and should be guarded with the corresponding `XER_HAS_DECIMAL...` macros.

---

## Floating-Point Literals

Floating-point user-defined literals are placed under:

```cpp
xer::literals::floating_literals
```

Examples include:

```cpp
_f32
_f64
_f80
_f128
_fl16
_fl32
_fl64
_fl80
_fl128
_bf16
```

Only literals whose destination type is available are provided.

---

## Notes

- This header is intentionally capability-based.
- Optional types are not promised on every compiler or target.
- Code that depends on optional formats should check the corresponding availability macro.
- The least-width literal suffixes are useful when the exact underlying available type may vary by platform.
