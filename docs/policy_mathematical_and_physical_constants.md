# Policy for Mathematical and Physical Constants

## Overview

This document defines the naming policy for mathematical and physical constants in xer.

Mathematical and physical constants often have short symbolic names such as π, τ, e, c, h, and k.
However, ASCII one-letter identifiers such as `e`, `c`, `h`, and `k` are also common variable names in ordinary C++ programs.
Providing such identifiers as public API names would make accidental name conflicts too likely, especially when users write `using namespace xer;` or when several libraries are used together.

Therefore, xer separates **descriptive primary identifiers** from **symbolic Unicode aliases**.

---

## Basic Policy

### Descriptive Names Are Primary

Every mathematical or physical constant provided by xer shall have a descriptive ASCII name as its primary identifier.

Examples:

```cpp
napier
speed_of_light
planck_constant
boltzmann_constant
```

The descriptive name is the stable, recommended, and easiest-to-type API name.
It should be usable in all environments where xer itself can be used.

### ASCII One-Letter Aliases Are Not Provided

xer shall not provide ASCII one-letter aliases for mathematical or physical constants.

Examples of names that should not be provided:

```cpp
e
c
h
k
```

These names are too likely to conflict with ordinary local variables, loop variables, template parameters, or identifiers from other libraries.

This rule applies even when the corresponding mathematical symbol is well known.
For example, Euler's number may have a symbolic alias `𝑒`, but it should not have an ASCII alias `e`.

### Unicode Symbolic Aliases May Be Provided

When a constant has a well-known mathematical or physical symbol, xer may provide a Unicode symbolic alias.

Examples:

```cpp
𝜋
τ
𝑒
𝑐
ℎ
```

Unicode symbolic aliases are optional conveniences.
They are intended for users who want code that visually resembles mathematical notation.
Users who prefer plain ASCII, or who cannot easily input such characters, can use the descriptive primary names instead.

---

## Latin Letter Symbols

### Use Mathematical Italic Letters

When a symbolic alias uses a Latin letter, xer should use the corresponding Mathematical Italic character rather than an ordinary ASCII letter.

Examples:

```cpp
𝑒 // U+1D452 MATHEMATICAL ITALIC SMALL E
𝑐 // U+1D450 MATHEMATICAL ITALIC SMALL C
𝑘 // U+1D458 MATHEMATICAL ITALIC SMALL K
```

This keeps symbolic aliases visually distinct from ordinary C++ identifiers.
It also makes it clear that the identifier is a mathematical symbol, not a normal variable name.

### Prefer Dedicated Unicode Symbols When Appropriate

If Unicode provides a dedicated symbol for a specific mathematical or physical constant, xer may prefer that symbol over a generic Mathematical Italic letter.

For example, the Planck constant should use:

```cpp
ℎ // U+210E PLANCK CONSTANT
```

rather than an ordinary ASCII `h` or a generic mathematical italic spelling.

---

## Greek Letter Symbols

Greek-letter constants may continue to use the corresponding Greek or mathematical Greek symbol, as already done by xer.

Examples:

```cpp
𝜋
τ
ω
ω²
```

The ASCII descriptive name remains the primary identifier, while the Greek symbol is a symbolic alias.

---

## Ambiguous Symbols

Some symbols are used for many different concepts depending on the field.
For such constants, xer should provide a Unicode symbolic alias only when the meaning is sufficiently clear in context.

For example:

- `𝑐` is acceptable as an alias for `speed_of_light`, because `c` is a very common symbol for the speed of light in physics.
- `𝑘` may be ambiguous because it can mean a spring constant, a wave number, Boltzmann's constant, or other quantities depending on context.
- when ambiguity is high, the descriptive name should be preferred and the symbolic alias may be omitted.

Symbolic aliases should not be added merely because a short notation exists somewhere.
They should be added when the notation is widely recognized and likely to improve readability.

---

## Versioning and Stability

Descriptive primary names are the stable API surface.
Once provided, they should be treated as ordinary public API names.

Unicode symbolic aliases are also public API once released, but they should be regarded as notation-oriented conveniences.
Before adding a new symbolic alias, xer should verify the exact Unicode character and document it in the implementation comments or reference documentation.

---

## Recommended Implementation Style

A constant should generally be defined once under its descriptive name, and the symbolic alias should refer to that definition.

Example:

```cpp
inline constexpr auto napier = /* ... */;
inline constexpr auto 𝑒 = napier;

inline constexpr auto speed_of_light = /* ... */;
inline constexpr auto 𝑐 = speed_of_light;

inline constexpr auto planck_constant = /* ... */;
inline constexpr auto ℎ = planck_constant;
```

This avoids duplicated definitions and keeps the descriptive identifier as the canonical implementation point.

If type-generic constants are provided, the same principle applies:

```cpp
template <std::floating_point T>
inline constexpr T napier_v = /* ... */;

template <std::floating_point T>
inline constexpr T 𝑒_v = napier_v<T>;
```

The exact naming form should follow the conventions of the corresponding header.

---

## Summary

- Use descriptive ASCII names as the primary identifiers for mathematical and physical constants.
- Do not provide ASCII one-letter aliases such as `e`, `c`, `h`, or `k`.
- Provide Unicode symbolic aliases only when the notation is well known and useful.
- Use Mathematical Italic characters for Latin-letter symbolic aliases.
- Prefer dedicated Unicode symbols such as `ℎ` for Planck's constant when appropriate.
- Keep symbolic aliases as aliases of descriptive primary names, not as separate definitions.
