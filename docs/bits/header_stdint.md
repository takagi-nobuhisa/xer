# `<xer/stdint.h>`

## Purpose

`<xer/stdint.h>` provides fixed-width integer facilities and closely related numeric utilities in XER.

Its role is similar in spirit to the C standard library `<stdint.h>`, but it is not limited to merely re-exporting integer typedefs.
Instead, it also serves as the home for practical integer-oriented helpers that fit XER's overall design.

This header is especially important because it provides:

- fixed-width integer type aliases
- pointer-sized integer types
- optional 128-bit integer aliases where supported
- compile-time numeric helper constants
- integer literal suffixes for convenient typed integer constants

---

## Main Role

The main role of `<xer/stdint.h>` is to provide a stable and explicit integer vocabulary for the rest of XER.

In particular, it exists to make the following easy and clear:

- writing code with explicitly sized integer types
- referring to implementation-sized integer types such as pointer-sized integers
- expressing integer limits and bit widths in a unified XER style
- writing typed integer literals directly in source code

This makes the header useful both as a foundational type header and as a practical utility header for integer-heavy code.

---

## Main Entities

At minimum, `<xer/stdint.h>` provides the following kinds of entities:

- fixed-width signed integer types
- fixed-width unsigned integer types
- least-width and fast-width integer types where appropriate
- pointer-sized integer types
- maximum-width integer types
- optional 128-bit integer types where available
- compile-time helper constants
- user-defined integer literal suffixes

The exact exposed set follows the implementation and project policy, but these are the intended public categories.

---

## Fixed-Width Integer Types

`<xer/stdint.h>` provides the familiar fixed-width integer types, including at least forms such as:

```cpp
int8_t
int16_t
int32_t
int64_t

uint8_t
uint16_t
uint32_t
uint64_t
```

### Role of These Types

These types are used when the width of the integer value matters explicitly.

Typical use cases include:

* binary formats
* protocol definitions
* packed data structures
* arithmetic with explicit size expectations
* cross-platform code that should not depend on implementation-defined plain `int` width

### Notes

These names are meant to be straightforward and familiar to users coming from C and C++.

---

## Pointer-Sized and Maximum-Width Integer Types

This header also provides integer types associated with pointer size or maximum practical width, such as:

```cpp
intptr_t
uintptr_t
intmax_t
uintmax_t
```

### Role of These Types

They are useful when code needs to:

* convert pointers to integer form safely where appropriate
* reason about the widest practical integer category
* write generic numeric code that should adapt to the implementation

### Notes

These types are especially useful in low-level code and implementation-support code.

---

## Optional 128-Bit Integer Types

Where the implementation supports `__int128`, XER may provide:

```cpp
int128_t
uint128_t
```

### Role of These Types

These types are useful when:

* 64-bit range is insufficient
* intermediate arithmetic should avoid overflow
* larger integer constants should remain explicit in type

### Availability

These types are implementation-dependent.

They are available only where the compiler and target support the necessary underlying integer type.

Documentation should therefore describe them as optional rather than universally guaranteed.

---

## Compile-Time Numeric Helpers

`<xer/stdint.h>` may also provide helper constants such as:

```cpp
min_of<T>
max_of<T>
bit_width_of<T>
```

### `min_of<T>`

`min_of<T>` represents the minimum value of the integer type `T`.

### `max_of<T>`

`max_of<T>` represents the maximum value of the integer type `T`.

### `bit_width_of<T>`

`bit_width_of<T>` represents the bit width of `T`.

### Role of These Helpers

These helpers exist so that integer-type metadata can be referred to in a compact, readable, and XER-consistent way.

They are especially useful in:

* compile-time checks
* generic numeric utilities
* range-sensitive code
* documentation examples

### Design Direction

These helpers are intended to be simple compile-time facilities, not large abstraction layers.

---

## Integer Literal Suffixes

One of the most visible user-facing features of `<xer/stdint.h>` is the integer literal suffix set.

At minimum, XER may provide literal suffixes such as:

```cpp
_i8   _i16   _i32   _i64
_u8   _u16   _u32   _u64
_i128 _u128
```

typically under:

```cpp
xer::literals::integer_literals
```

### Role of These Suffixes

These suffixes make it possible to write typed integer constants directly and readably.

For example:

```cpp
using namespace xer::literals::integer_literals;

constexpr auto x = 123_i32;
constexpr auto y = 255_u8;
```

This improves clarity in code where the intended integer type matters.

### Design Direction

These literal suffixes are intended to be:

* explicit
* readable
* convenient in tests and examples
* useful in compile-time contexts

They are especially attractive in a project like XER, which emphasizes explicitness and type clarity.

---

## Supported Literal Forms

The literal-parsing facilities behind the suffixes may support at least the following textual forms:

* decimal
* octal
* hexadecimal
* binary with `0b...`
* digit separators using `'`

### Examples

```cpp
using namespace xer::literals::integer_literals;

constexpr auto a = 123_i32;
constexpr auto b = 0xff_u32;
constexpr auto c = 0b1010_u8;
constexpr auto d = 1'000'000_i64;
```

### Notes

These features are especially useful for:

* tests
* binary and bit-oriented code
* examples where the intended numeric type should remain obvious

---

## Range Checking

An important design point of the integer literal facilities is that range checking is performed explicitly.

### Meaning

When a literal suffix requests a specific destination type, the literal should fit in that type.

If it does not fit, the program should fail to compile rather than silently narrowing the value.

### Why This Matters

This makes typed integer literals trustworthy and avoids hidden truncation.

It also aligns with XER's broader design preference for explicit failure over surprising implicit behavior.

---

## Relationship to Other Headers

`<xer/stdint.h>` should be understood together with:

* `policy_project_outline.md`
* `header_arithmetic.md`

The rough boundary is:

* `<xer/stdint.h>` provides integer types, numeric limits/helpers, and typed integer literals
* `<xer/arithmetic.h>` provides arithmetic and comparison helpers built on top of explicit numeric types

This makes `<xer/stdint.h>` foundational, while `<xer/arithmetic.h>` handles higher-level numeric operations.

---

## Relationship to XER's Numeric Design

Although `<xer/stdint.h>` looks like a basic type header, it plays an important role in XER's numeric design.

In particular, it helps make the following explicit:

* the exact type of integer values
* the intended width of constants
* the range assumptions of numeric code
* implementation-dependent availability of wider integer types

This is especially valuable in a library that also provides mixed-type arithmetic helpers and explicit range checking.

---

## Documentation Notes

When this header is used in generated documentation, it is usually enough to explain:

* that it provides fixed-width integer types and related aliases
* that it may provide optional 128-bit aliases where supported
* that it includes simple compile-time integer helpers
* that it provides user-defined literal suffixes for typed integer constants

Detailed parsing rules and edge cases for literal suffixes belong in the detailed reference or generated API sections.

---

## Example Topics Commonly Worth Showing

The following kinds of examples are especially suitable for this header:

* declaring values with fixed-width integer types
* using `bit_width_of<T>`
* writing typed integer literals with `_i32` or `_u64`
* showing compile-time range-safe constant expression use

These are good candidates for executable examples in `examples/`.

---

## Example

```cpp
#include <xer/stdint.h>

using namespace xer::literals::integer_literals;

auto main() -> int
{
    constexpr auto x = 123_i32;
    constexpr auto y = 255_u16;

    static_assert(std::same_as<decltype(x), const xer::int32_t>);
    static_assert(std::same_as<decltype(y), const xer::uint16_t>);

    return 0;
}
```

This example shows the normal style:

* use explicit XER integer types
* use typed integer literal suffixes
* keep integer width visible in the code itself

---

## See Also

* `policy_project_outline.md`
* `header_arithmetic.md`


---

## Integer Literal Suffixes

Integer literal suffixes are provided in:

```cpp
xer::literals::integer_literals
```

The fixed-width suffixes include:

```cpp
_i8   _i16   _i32   _i64
_u8   _u16   _u32   _u64
_i128 _u128  // when supported
```

The least-width suffixes include:

```cpp
_il8   _il16   _il32   _il64
_ul8   _ul16   _ul32   _ul64
```

The least-width suffixes produce the corresponding `int_leastN_t` or `uint_leastN_t` type and are useful when exact storage width is less important than a guaranteed minimum range.

---

## Numeric Limit Helpers

`min_of<T>` and `max_of<T>` are implemented through a shared numeric-limits helper so that they can also be reused by floating-point facilities such as `<xer/stdfloat.h>`.

`bit_width_of<T>` remains available from `<xer/stdint.h>` for integer-oriented bit width queries.
