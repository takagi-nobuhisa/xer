# `<xer/arithmetic.h>`

## Purpose

`<xer/arithmetic.h>` provides arithmetic and comparison helper functions in XER.

Its purpose is not merely to wrap built-in operators with different names.
Instead, it provides a numeric utility layer designed to avoid common problems of ordinary C++ arithmetic, especially in cases such as:

- mixing signed and unsigned integer types
- integrating arithmetic with explicit failure handling
- making range failure visible
- expressing comparisons in a way that follows XER's own numeric rules

This header is therefore a central part of XER's numeric design.

---

## Main Role

The main role of `<xer/arithmetic.h>` is to provide arithmetic and comparison operations that are:

- explicit
- predictable
- easier to chain safely than raw built-in operators
- better aligned with XER's error model

In particular, it exists to make the following easier:

- mixed-type integer arithmetic without surprising implicit conversions
- explicit range-aware arithmetic
- explicit comparison helpers for use in generic code
- utility functions such as `min`, `max`, `clamp`, and `in_range`

---

## Main Function Groups

At a high level, `<xer/arithmetic.h>` contains the following groups of functionality:

- integer arithmetic helpers
- comparison helpers
- approximate floating-point comparison
- range and bounds helpers
- absolute-value helpers
- floating-point and complex-number support within the same general design

---

## Integer Arithmetic Helpers

At minimum, this header provides the following integer-oriented arithmetic helpers:

```cpp
add
uadd
sub
usub
mul
umul
div
udiv
mod
umod
```

### Role of This Group

These functions provide arithmetic operations whose behavior is designed explicitly rather than inherited automatically from C++'s usual arithmetic conversions.

This is especially important when signed and unsigned integer types are mixed.

### Design Direction

The design goals of this group include:

* allow mixed signed/unsigned input where that is mathematically meaningful
* return explicit failure when the result does not fit in the target result domain
* avoid silent narrowing or surprising wraparound
* make division and remainder behavior explicit

---

## `add`, `sub`, and `mul`

### Signed-Domain Variants

The following helpers conceptually operate in the signed result domain:

```cpp
add
sub
mul
```

These functions generally return:

```cpp
xer::result<std::int64_t>
```

when operating on integer values.

### Meaning

* `add(a, b)` performs addition
* `sub(a, b)` performs subtraction
* `mul(a, b)` performs multiplication

### Error Handling

If the result cannot be represented in the intended signed result domain, these functions return failure.

This makes overflow and out-of-range situations explicit.

---

## `uadd`, `usub`, and `umul`

### Unsigned-Domain Variants

The following helpers conceptually operate in the unsigned result domain:

```cpp
uadd
usub
umul
```

These functions generally return:

```cpp
xer::result<std::uint64_t>
```

when operating on integer values.

### Meaning

* `uadd(a, b)` performs addition in the unsigned result domain
* `usub(a, b)` performs subtraction in the unsigned result domain
* `umul(a, b)` performs multiplication in the unsigned result domain

### Error Handling

If the mathematically selected result cannot be represented in the intended unsigned result domain, these functions return failure.

For example, a negative result is an error for `usub` or `umul`.

---

## `div`, `udiv`, `mod`, and `umod`

This header also provides division and remainder helpers:

```cpp
div
udiv
mod
umod
```

### `div`

`div` performs division in the signed result domain.

For integer input:

* the quotient is rounded toward zero
* division by zero is an error
* out-of-range results are errors
* a remainder output may also be supported

### `udiv`

`udiv` performs division in the unsigned result domain.

For integer input:

* division by zero is an error
* out-of-range results are errors
* a remainder output may also be supported

### `mod`

`mod` returns the signed remainder according to the same rule family as `div`.

### `umod`

`umod` returns the unsigned remainder according to the same rule family as `udiv`.

### Why These Matter

These helpers make quotient/remainder behavior explicit and keep it aligned with XER's own arithmetic policy rather than leaving everything to built-in operator behavior.

---

## Comparison Helpers

At minimum, `<xer/arithmetic.h>` provides the following comparison helpers:

```cpp
eq
ne
lt
le
gt
ge
```

### Role of This Group

These functions provide explicit comparison in a way that fits XER's numeric rules.

They are especially useful when:

* mixed integer types are involved
* generic code should not depend directly on built-in operator behavior
* the project wants one consistent comparison layer across arithmetic helpers

### Return Type

Comparison helpers return:

```cpp
bool
```

They do **not** return `xer::result<bool>` in the ordinary design.

### Why `xer::result<bool>` Is Not Used

Returning `xer::result<bool>` from ordinary comparison helpers would make conditional use too awkward and too easy to misuse.

For example, code such as:

```cpp
if (eq(a, b)) {
    ...
}
```

should remain straightforward.

For that reason, comparison helpers use `bool` and instead restrict their intended argument domain appropriately.

---

## Approximate Floating-Point Comparison

`<xer/arithmetic.h>` provides an approximate comparison helper for floating-point-oriented checks:

```cpp
is_close
```

### `is_close`

```cpp
template<typename A, typename B, typename E>
constexpr auto is_close(A lhs, B rhs, E epsilon) noexcept -> bool;

template<typename A, typename B, typename E>
constexpr auto is_close(
    const result<A>& lhs,
    B rhs,
    E epsilon) noexcept -> result<bool>;

template<typename A, typename B, typename E>
constexpr auto is_close(
    A lhs,
    const result<B>& rhs,
    E epsilon) noexcept -> result<bool>;

template<typename A, typename B, typename E>
constexpr auto is_close(
    const result<A>& lhs,
    const result<B>& rhs,
    E epsilon) noexcept -> result<bool>;
```

`is_close(lhs, rhs, epsilon)` tests whether two arithmetic values are close enough under an absolute tolerance.

Conceptually, the comparison is:

```text
abs(lhs - rhs) <= epsilon
```

The comparison is inclusive. Therefore, values exactly at the specified tolerance boundary are treated as close.

### Rounding Margin

For positive tolerance values, the implementation may apply a small rounding margin based on the common arithmetic type.

This is intended to avoid rejecting values that are mathematically on the tolerance boundary but become slightly larger due to floating-point representation and subtraction rounding.

For example, a difference that is mathematically `0.05` should not be rejected merely because the computed floating-point difference is slightly greater than `0.05`.

When `epsilon` is zero, this extra rounding margin is not applied.
A zero tolerance therefore behaves as an exact comparison after conversion to the internal comparison type.

### Invalid Values

`is_close` returns `false` when any of the following apply:

- `lhs` is NaN or infinity
- `rhs` is NaN or infinity
- `epsilon` is NaN or infinity
- `epsilon` is negative
- the computed difference is not finite

This keeps approximate comparison simple and avoids treating invalid floating-point states as ordinary closeness.

### `xer::result` Arguments

As part of `<xer/arithmetic.h>`, `is_close` may accept `xer::result` operands for `lhs` and `rhs`.

If a result operand contains a success value, that value is compared.
If a result operand contains an error, the error is propagated and the return type is `xer::result<bool>`.

The tolerance argument is intentionally kept as an ordinary value.
The tolerance is normally a fixed decision made by the caller, and accepting a result for it would add little practical value.

### Naming

The name `is_close` is used instead of shorter names such as `near`.

This avoids collision with legacy platform macros while keeping the purpose of the function clear at the call site.

---

## Range and Bounds Helpers

This header also provides utility helpers such as:

```cpp
in_range
min
max
clamp
```

### `in_range`

`in_range<T>(value)` checks whether `value` can be represented as type `T`.

Its role is to make explicit range-checking available in ordinary code.

This is especially important before conversion or when generic code works across multiple numeric types.

### `min` and `max`

`min` and `max` return the smaller or larger of two values according to XER's comparison rules.

They are not intended to be mere clones of the standard library forms.
Instead, they are designed for mixed-type use under XER's own numeric policy.

### `clamp`

`clamp(value, lo, hi)` constrains a value to the closed interval `[lo, hi]`.

Its purpose is to provide an explicit and predictable clamping helper that works consistently with XER's comparison model.

---

## Absolute-Value Helpers

At minimum, this header provides:

```cpp
abs
uabs
```

### `abs`

`abs(value)` returns the absolute value in the signed-domain design.

For integer input, it generally returns:

```cpp
xer::result<std::int64_t>
```

and reports failure if the result cannot be represented.

### `uabs`

`uabs(value)` returns the nonnegative absolute value in the unsigned-domain design.

For integer input, it generally returns:

```cpp
xer::result<std::uint64_t>
```

and reports failure if the result cannot be represented.

### Why These Helpers Matter

These helpers are important because even "simple" absolute-value operations can fail in fixed-width signed integer domains.

XER therefore makes that failure explicit.

---

## Square and Cube Helpers

`<xer/arithmetic.h>` also provides small power helpers:

```cpp
sq
cb
```

### `sq`

`sq(value)` returns the square of `value`.

For integer input, it generally returns:

```cpp
xer::result<xer::int64_t>
```

and reports failure if the squared value cannot be represented in the signed result domain.

For floating-point input, it follows the floating-point arithmetic rules of this header and returns a result whose success value is `long double`.

### `cb`

`cb(value)` returns the cube of `value`.

It follows the same result and error-handling policy as `sq`.
For integer input, overflow and out-of-range results are reported explicitly.

### Chained Use

As arithmetic helpers, `sq` and `cb` may also accept `xer::result` arguments.
This allows forms such as:

```cpp
const auto value = xer::sq(xer::add(2, 3));
```

If the argument result already contains an error, that error is propagated.
---

## Acceptance of `xer::result`

One of the most important design points of `<xer/arithmetic.h>` is that arithmetic helpers may accept `xer::result` arguments.

### Why This Header Is Special

In the general XER API policy, ordinary public APIs are not supposed to take `xer::result` as an argument.

However, `<xer/arithmetic.h>` is the main exception.

This is because arithmetic chaining has clear value when intermediate failures should propagate naturally.

### Meaning

If an arithmetic helper receives a `xer::result` argument:

* if it contains a success value, that value is used
* if it contains an error, that error is propagated

This makes it easier to write chained arithmetic expressions without manually unwrapping every intermediate step.

### Important Boundary

This exception is specific to the arithmetic area.
It should not be treated as the default design for ordinary public APIs elsewhere in the library.

---

## Floating-Point Support

`<xer/arithmetic.h>` also covers floating-point arithmetic within the same general design.

### General Direction

For floating-point input:

* results may be represented in `long double`
* explicit failure handling is still used where appropriate
* non-computable cases may be treated as failure

### Why This Matters

This allows XER arithmetic helpers to remain usable across both integer and floating-point code while keeping a unified design direction.

---

## Complex-Number Support

Within a reasonable scope, this header may also support arithmetic on complex values.

### General Direction

For complex-number input:

* addition, subtraction, multiplication, and division may be supported
* the result type may be based on `std::complex<long double>`
* comparison operations are generally not provided

### Why Comparison Is Different

Order comparison is not part of the ordinary complex-number model.
For that reason, the comparison helper family does not extend mechanically to complex numbers.

---

## Relationship Between Arithmetic and Comparison

A central design principle of this header is that arithmetic helpers should use XER's comparison helpers internally where ordering matters.

### Meaning

When arithmetic utilities such as:

* `min`
* `max`
* `clamp`

need ordering, they should use `xer::lt` and related helpers rather than relying directly on built-in `<`.

### Why This Matters

This keeps the numeric model internally consistent, especially for mixed signed/unsigned cases.

---

## Relationship to Other Headers

`<xer/arithmetic.h>` should be understood together with:

* `policy_project_outline.md`
* `policy_arithmetic.md`
* `policy_result_arguments.md`
* `header_stdint.md`
* `header_error.md`

The rough boundary is:

* `<xer/stdint.h>` provides integer types, limits/helpers, and typed integer literals
* `<xer/error.h>` provides `xer::result` and error machinery
* `<xer/arithmetic.h>` provides arithmetic and comparison operations built on top of those foundations

---

## Documentation Notes

When this header is used in generated documentation, it is usually enough to explain:

* that it provides explicit arithmetic and comparison helpers
* that mixed integer types are an important design target
* that ordinary failure is represented explicitly through `xer::result`
* that this header is the main public exception to the general "no `xer::result` arguments" rule

Detailed per-function numeric rules belong in the detailed reference or generated API sections.

---

## Example Topics Commonly Worth Showing

The following kinds of examples are especially suitable for this header:

* adding signed and unsigned integers with `add`
* propagating failure through chained arithmetic helpers
* checking representability with `in_range`
* using `min`, `max`, or `clamp` with mixed numeric types
* using `abs` or `uabs` with explicit result checking
* using `sq` or `cb` with explicit result checking
* comparing floating-point values with `is_close`
* comparing floating-point values with `is_close`

These are good candidates for executable examples in `examples/`.

---

## Example

```cpp
#include <xer/arithmetic.h>

auto main() -> int
{
    const auto sum = xer::add(10u, -3);
    if (!sum.has_value()) {
        return 1;
    }

    const auto limited = xer::clamp(*sum, -5, 5);
    if (!limited.has_value()) {
        return 1;
    }

    if (*limited != 5) {
        return 1;
    }

    return 0;
}
```

This example shows the normal XER style:

* use explicit arithmetic helpers instead of raw operators where policy matters
* check `xer::result` explicitly
* use utility helpers such as `clamp` in the same model

---

## See Also

* `policy_project_outline.md`
* `policy_arithmetic.md`
* `policy_result_arguments.md`
* `header_stdint.md`
* `header_error.md`
