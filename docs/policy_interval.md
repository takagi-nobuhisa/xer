# Policy for Interval Values: `interval`

## Overview

XER provides the `interval` type in order to handle real values that must stay within a fixed closed interval.

This facility is intended for values such as color components, alpha values, gain values, normalized ratios, grayscale values, and other bounded scalar values.

The `interval` type is a counterpart to `cyclic`.

- `cyclic` represents circular values normalized into `[0, 1)`.
- `interval` represents linear bounded values clamped into `[Min, Max]`.

The purpose is not to provide a mathematically complete interval-arithmetic system.
Instead, `interval` is a lightweight value type that maintains a simple invariant: the stored value is always a finite value inside the specified closed interval.

---

## Basic Policy

### Scope

`interval` is used to represent bounded scalar values.

Practical target uses include at least the following:

- RGB and alpha color components
- grayscale values
- normalized ratios
- gain and opacity values
- UI slider values
- bounded control parameters
- intermediate values used in color-space conversion

The initial design focuses on practical scalar values rather than general interval arithmetic.

### Internal Representation

The internal representation of `interval<T, Min, Max>` is a single floating-point value `T`.

The value is always kept inside the closed interval:

```text
Min <= value <= Max
```

The default interval is `[0, 1]`.

```cpp
template <
    std::floating_point T,
    T Min = static_cast<T>(0),
    T Max = static_cast<T>(1)>
class interval;
```

### Type

`interval` is a class template parameterized by a floating-point type and two compile-time bounds.

```cpp
template <std::floating_point T, T Min, T Max>
class interval;
```

The main intended template arguments for `T` are:

- `float`
- `double`
- `long double`

Integer types are not accepted.

### Bounds

The bounds are specified as non-type template parameters.

The type requires:

```cpp
static_assert(Min < Max);
```

An empty interval or reversed interval is not accepted.

---

## Relationship to `cyclic`

`interval` and `cyclic` both store normalized or constrained scalar values, but their meanings are different.

### `cyclic`

`cyclic<T>` represents a circular value.

- internal range: `[0, 1)`
- `1` is equivalent to `0`
- values wrap around
- ordering is not intrinsic
- equality is tolerance-based
- comparison operators are not provided

### `interval`

`interval<T, Min, Max>` represents a linear bounded value.

- internal range: `[Min, Max]`
- `Min` and `Max` are distinct values
- values do not wrap around
- out-of-range finite values are clamped
- ordering is intrinsic
- comparison operators are provided

This distinction is important for color handling.

For example:

- hue is naturally represented by `cyclic<float>`
- red, green, blue, alpha, and grayscale components are naturally represented by `interval<float>`

---

## Construction

### Default Construction

Default construction creates an interval value initialized to `Min`.

```cpp
interval<float> value; // value() == 0.0f
```

For a custom interval:

```cpp
interval<float, -1.0f, 1.0f> value; // value() == -1.0f
```

### Construction from a Raw Value

Construction from a raw value is explicit.

```cpp
explicit constexpr interval(T value);
```

If `value` is finite, it is clamped into `[Min, Max]`.

Examples for `interval<float>`:

- `interval<float>(0.5f)` stores `0.5f`
- `interval<float>(-0.2f)` stores `0.0f`
- `interval<float>(1.2f)` stores `1.0f`

### Invalid Values

`NaN`, positive infinity, and negative infinity are not valid interval values.

If such a value is passed to construction or assignment, an exception is thrown.

This is intentional.
Silently clamping `NaN` or infinity would hide a serious error and could easily lead to incorrect results.

---

## Exception Policy

### Invalid Floating-Point Values

`NaN` and infinity are treated as invalid values.

The implementation may throw a standard exception such as `std::domain_error`.

Examples of invalid values:

- `std::numeric_limits<T>::quiet_NaN()`
- `std::numeric_limits<T>::infinity()`
- `-std::numeric_limits<T>::infinity()`

### Division by Zero

Division by zero is also invalid.

The implementation may throw `std::domain_error`.

### Rationale

XER normally represents ordinary recoverable failures with `xer::result`.

However, for `interval`, `NaN`, infinity, and division by zero are treated as exceptional cases that break the numeric invariant of the value type.
They do not normally occur unless the caller explicitly produces such values or an earlier calculation has already gone wrong.

For that reason, exception reporting is acceptable in this facility.

---

## Value Access

### `value`

```cpp
constexpr auto value() const noexcept -> T;
```

Returns the stored value.

The returned value always satisfies:

```text
Min <= value <= Max
```

The returned value is finite.

### `assign`

```cpp
constexpr auto assign(T value) -> void;
```

Assigns a raw value after validating and clamping it.

If the input is finite, it is clamped into `[Min, Max]`.

If the input is `NaN` or infinity, an exception is thrown.

### Assignment Operator

Assignment from a raw value is provided.

```cpp
constexpr auto operator=(T value) -> interval&;
```

It behaves the same as `assign`.

Assignment from another `interval` of the same type uses ordinary value assignment.

---

## Ratio Conversion

### `ratio`

```cpp
constexpr auto ratio() const noexcept -> T;
```

Returns the relative position of the current value in the interval as a value in `[0, 1]`.

The conceptual formula is:

```text
(value - Min) / (Max - Min)
```

Examples for `interval<float, 10.0f, 20.0f>`:

- value `10.0f` -> ratio `0.0f`
- value `15.0f` -> ratio `0.5f`
- value `20.0f` -> ratio `1.0f`

### `from_ratio`

```cpp
static constexpr auto from_ratio(T ratio) -> interval;
```

Creates an interval value from a relative position.

The input ratio is handled like an `interval<T>` value:

- finite values are clamped into `[0, 1]`
- `NaN` and infinity are rejected by exception

The conceptual formula is:

```text
Min + ratio * (Max - Min)
```

Examples for `interval<float, 10.0f, 20.0f>`:

- ratio `0.0f` -> value `10.0f`
- ratio `0.5f` -> value `15.0f`
- ratio `1.0f` -> value `20.0f`

---

## Comparison

`interval` represents a linear value, so comparison operators are provided.

At minimum, the following are available:

```cpp
operator==
operator<=>
```

The remaining comparison operators may be generated from these.

Comparison is based on the stored value.

Unlike `cyclic`, `interval` does not use tolerance-based equality by default.
The stored value is a regular bounded scalar, and linear order is intrinsic to the type.

---

## Arithmetic

### Basic Policy

Arithmetic operators are provided for practical use.

The result of arithmetic is always normalized back into the same interval type.

That means:

- finite results are clamped into `[Min, Max]`
- `NaN` and infinity cause an exception
- division by zero causes an exception

Arithmetic is not intended to implement mathematical interval arithmetic.
It is ordinary scalar arithmetic followed by validation and clamping.

### Arithmetic Between `interval` Values

The following operators are provided between values of the same `interval` type:

```cpp
operator+
operator-
operator*
operator/
```

Each operator computes using the stored values and constructs a new `interval` from the result.

Examples for `interval<float>`:

- `interval<float>(0.8f) + interval<float>(0.5f)` -> `1.0f`
- `interval<float>(0.2f) - interval<float>(0.5f)` -> `0.0f`
- `interval<float>(0.5f) * interval<float>(0.5f)` -> `0.25f`
- `interval<float>(0.5f) / interval<float>(2.0f)` -> `0.25f`, if such a value exists in the same interval type

For the default `[0, 1]` interval, division by another interval value whose stored value is `0` throws an exception.

### Arithmetic with Right-Hand Scalar Values

For incrementing, decrementing, scaling, and dividing a bounded value, right-hand scalar operands are supported.

```cpp
interval + T
interval - T
interval * T
interval / T
```

Examples:

```cpp
auto x = interval<float>(0.5f);
x = x + 0.1f; // 0.6f
x = x - 1.0f; // 0.0f
x = x * 3.0f; // 0.0f if starting from 0.0f
```

Right-hand scalar values are validated in the same way as construction inputs.
If the scalar is `NaN` or infinity, an exception is thrown.

Division by zero throws an exception.

### Left-Hand Scalar Multiplication

Scalar multiplication is also provided in the left-hand form:

```cpp
T * interval
```

This is allowed because multiplication remains natural and useful in either order.

Example:

```cpp
auto x = 0.5f * interval<float>(0.8f); // 0.4f
```

### Left-Hand Scalar Addition and Subtraction

The following forms are not provided:

```cpp
T + interval
T - interval
```

The main intended use of scalar addition and subtraction is to increase or decrease an interval value.

Therefore, the `interval` value should appear on the left-hand side:

```cpp
x + delta
x - delta
```

This keeps the expression easy to read as an update or adjustment of a bounded value.

### Left-Hand Scalar Division

The following form is not provided:

```cpp
T / interval
```

This expression has a reciprocal-like meaning and is less natural for bounded scalar values such as color components or ratios.

If such behavior is needed, the caller should write the operation explicitly using `value()`.

---

## Compound Assignment

Compound assignment operators are provided.

### With Another `interval`

```cpp
operator+=
operator-=
operator*=
operator/=
```

These take another value of the same `interval` type.

### With a Right-Hand Scalar

```cpp
operator+=
operator-=
operator*=
operator/=
```

These take a raw scalar value `T`.

The right-hand scalar is validated in the same way as construction input.

Examples:

```cpp
auto brightness = interval<float>(0.5f);

brightness += 0.1f; // 0.6f
brightness -= 1.0f; // 0.0f
brightness = 0.7f;
brightness *= 2.0f; // 1.0f
```

Each compound assignment preserves the interval invariant.

---

## Unary Operators

Unary plus and unary minus may be provided.

```cpp
operator+
operator-
```

Unary plus returns the value unchanged.

Unary minus constructs a new interval value from the negated stored value.
The result is then clamped into the same interval.

For the default `[0, 1]` interval, unary minus usually clamps to `0` except when the stored value is already `0`.

---

## Type Aliases

The initial implementation may provide convenient aliases when they become useful.

Possible aliases include:

```cpp
using intervalf = interval<float>;
using intervald = interval<double>;
using intervall = interval<long double>;
```

However, such aliases are optional.
The basic template itself is the main public API.

---

## Public Header

`interval` should be provided through an independent public header.

```text
xer/interval.h
```

The implementation may be placed in:

```text
xer/bits/interval.h
```

This follows the general XER header organization in which public headers are placed directly under `xer/`, while subdivided implementation headers are placed under `xer/bits/`.

`interval` belongs to the numeric and arithmetic-related group, but it is not merely an arithmetic helper.
It is a small value type with its own invariant, so an independent public header is appropriate.

---

## Documentation and Examples

Executable examples should be placed under `examples/`.

Good example topics include:

- basic clamping behavior
- color component values
- brightness or gain adjustment
- conversion between custom intervals and normalized ratios
- comparison and arithmetic between interval values

Examples should use public headers only and should avoid test-style exhaustive boundary checks.

---

## Deferred Items

At least the following are deferred:

- mathematical interval arithmetic
- open, half-open, or dynamically bounded intervals
- integer interval values
- mixed-bound arithmetic between different `interval<T, Min, Max>` types
- tolerance-based equality
- integration with physical quantity dimensions
- color-space-specific aliases and structures
- special policies for `NaN` other than throwing exceptions

---

## Summary

- `interval<T, Min, Max>` represents a finite value in the closed interval `[Min, Max]`
- the default interval is `[0, 1]`
- `T` must be a floating-point type
- `Min` and `Max` are compile-time bounds
- `Min < Max` is required
- finite out-of-range values are clamped
- `NaN` and infinity are rejected by exception
- division by zero is rejected by exception
- comparison operators are provided
- arithmetic operators are provided as scalar arithmetic followed by clamping
- compound assignment operators are provided
- right-hand scalar addition, subtraction, multiplication, and division are supported
- left-hand scalar multiplication is supported
- left-hand scalar addition, subtraction, and division are not provided
- `interval` is distinct from `cyclic`: it is linear, closed, and non-wrapping

---

## Relationship to `cyclic` and Explicit Conversion

`interval` and `cyclic` intentionally do not provide implicit conversion constructors.

The two types both use normalized scalar values, but they differ at the upper endpoint.

- `interval<T>` uses the closed interval `[0, 1]`.
- `cyclic<T>` uses the half-open interval `[0, 1)`.

Therefore, an interval value at the upper endpoint maps to the zero position when converted to a cyclic value.
This behavior is useful and correct, but it should be visible at the call site.

The following explicit helper functions are provided:

```cpp
template <std::floating_point T, T Min, T Max>
constexpr auto to_cyclic(interval<T, Min, Max> value) noexcept -> cyclic<T>;

template <std::floating_point T>
constexpr auto to_interval(cyclic<T> value) -> interval<T>;
```

`to_cyclic(interval)` converts through `value.ratio()`.

Examples:

```cpp
using level = xer::interval<float, 10.0f, 20.0f>;

auto a = xer::to_cyclic(level(10.0f)); // 0.0f
auto b = xer::to_cyclic(level(15.0f)); // 0.5f
auto c = xer::to_cyclic(level(20.0f)); // 0.0f
```

`to_interval(cyclic)` converts to the default interval `[0, 1]`.

For a custom interval, use `from_ratio` explicitly.

```cpp
using gain = xer::interval<float, -1.0f, 1.0f>;

auto x = gain::from_ratio(hue.ratio());
```
