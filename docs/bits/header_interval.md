# `<xer/interval.h>`

## Purpose

`<xer/interval.h>` provides bounded floating-point value types.

The main entity is `xer::interval<T, Min, Max>`, a lightweight value type that stores a finite scalar value constrained to a fixed closed interval.

The default interval is `[0, 1]`, which is useful for values such as color components, alpha values, normalized ratios, opacity, brightness, gain, and other bounded control values.

---

## Main Entity

At minimum, `<xer/interval.h>` provides the following entity:

```cpp
template <
    std::floating_point T,
    T Min = static_cast<T>(0),
    T Max = static_cast<T>(1)>
class interval;
```

The implementation is provided through the corresponding internal header:

```cpp
#include <xer/bits/interval.h>
```

Users should normally include the public header:

```cpp
#include <xer/interval.h>
```

---

## Design Role

`interval` is a small numeric value type whose main purpose is to preserve an invariant.

For `interval<T, Min, Max>`, the stored value always satisfies:

```text
Min <= value() <= Max
```

The stored value is always finite.

Finite out-of-range values are clamped to the nearest bound.
Invalid floating-point values such as `NaN` and infinity are rejected by throwing an exception.

This makes `interval` useful for values that should never escape a known range during ordinary use.

---

## Relationship to `cyclic`

`interval` is closely related to `cyclic`, but the two types represent different concepts.

`cyclic<T>` represents a circular value normalized into `[0, 1)`.

Typical examples include:

- hue
- angle
- phase
- direction

`interval<T, Min, Max>` represents a linear bounded value in `[Min, Max]`.

Typical examples include:

- red, green, and blue components
- alpha values
- grayscale values
- brightness
- gain
- opacity
- normalized ratios

This distinction is especially important in color handling.
Hue naturally wraps around, while color components do not.

---

## Template Parameters

```cpp
template <
    std::floating_point T,
    T Min = static_cast<T>(0),
    T Max = static_cast<T>(1)>
class interval;
```

### `T`

`T` is the stored floating-point type.

The main intended types are:

- `float`
- `double`
- `long double`

Integer types are not accepted.

### `Min`

`Min` is the inclusive lower bound.

### `Max`

`Max` is the inclusive upper bound.

The type requires:

```cpp
Min < Max
```

An empty interval or reversed interval is not accepted.

---

## Default Interval

The default form:

```cpp
xer::interval<float>
```

means:

```cpp
xer::interval<float, 0.0f, 1.0f>
```

This is the most common form for normalized values.

Example:

```cpp
using component = xer::interval<float>;

auto r = component(1.25f);  // stored as 1.0f
auto g = component(0.5f);   // stored as 0.5f
auto b = component(-0.25f); // stored as 0.0f
```

---

## Custom Intervals

Custom bounds can be specified as floating-point non-type template parameters.

Example:

```cpp
using gain = xer::interval<float, -1.0f, 1.0f>;

auto center = gain(0.0f);
auto upper = gain(2.0f);   // clamped to 1.0f
auto lower = gain(-2.0f);  // clamped to -1.0f
```

This is useful when a value has a natural range other than `[0, 1]`.

---

## Construction

### Default Construction

Default construction initializes the stored value to `Min`.

```cpp
xer::interval<float> x;
// x.value() == 0.0f

xer::interval<float, -1.0f, 1.0f> y;
// y.value() == -1.0f
```

### Construction from a Raw Value

Construction from a raw scalar is explicit.

```cpp
explicit constexpr interval(T value);
```

Finite values are accepted and clamped into the interval.

For `xer::interval<float>`:

```cpp
auto a = xer::interval<float>(0.5f);   // 0.5f
auto b = xer::interval<float>(-0.5f);  // 0.0f
auto c = xer::interval<float>(1.5f);   // 1.0f
```

`NaN` and infinity are rejected by exception.

---

## Exception Policy

`interval` throws `std::domain_error` for values that cannot be represented as valid finite interval values.

At minimum, the following cases throw:

- construction from `NaN`
- construction from positive infinity
- construction from negative infinity
- assignment from `NaN`
- assignment from infinity
- arithmetic that produces `NaN`
- arithmetic that produces infinity
- division by zero

This is intentional.
Silently clamping `NaN` or infinity would hide a serious numeric error.

---

## Member Types and Constants

`interval` provides the following public members:

```cpp
using value_type = T;

static constexpr T min_value = Min;
static constexpr T max_value = Max;
```

`value_type` is the stored floating-point type.

`min_value` and `max_value` expose the compile-time interval bounds.

---

## Value Access

### `value`

```cpp
constexpr auto value() const noexcept -> T;
```

Returns the stored scalar value.

The returned value is always finite and always inside `[Min, Max]`.

---

## Assignment

### `assign`

```cpp
constexpr auto assign(T value) -> void;
```

Assigns a raw scalar value.

Finite values are clamped into `[Min, Max]`.
`NaN` and infinity throw `std::domain_error`.

### Assignment from `T`

```cpp
constexpr auto operator=(T value) -> interval&;
```

Assigns a raw scalar value and returns `*this`.

This behaves the same as `assign`.

Example:

```cpp
auto x = xer::interval<float>();

x = 0.75f; // stored as 0.75f
x = 2.0f;  // stored as 1.0f
```

---

## Ratio Conversion

### `ratio`

```cpp
constexpr auto ratio() const noexcept -> T;
```

Returns the relative position of the stored value in the interval.

The result is in `[0, 1]`.

Conceptually:

```text
(value() - Min) / (Max - Min)
```

Example:

```cpp
using level = xer::interval<float, 10.0f, 20.0f>;

auto x = level(15.0f);
auto r = x.ratio(); // 0.5f
```

### `from_ratio`

```cpp
static constexpr auto from_ratio(T ratio) -> interval;
```

Creates an interval value from a relative position.

The input ratio is treated as a bounded value in `[0, 1]`.

Finite input is clamped into `[0, 1]`.
`NaN` and infinity throw `std::domain_error`.

Conceptually:

```text
Min + ratio * (Max - Min)
```

Example:

```cpp
using gain = xer::interval<float, -1.0f, 1.0f>;

auto center = gain::from_ratio(0.5f);
// center.value() == 0.0f
```

---

## Comparison

`interval` represents a linear bounded value, so comparison operators are provided.

At minimum, the type supports:

```cpp
operator==
operator<=>
```

The remaining comparison operators are available through ordinary C++ comparison rewriting.

Comparison is based on the stored scalar value.

Unlike `cyclic`, `interval` does not use tolerance-based equality.
Since `interval` rejects `NaN`, ordinary linear ordering is meaningful.

Example:

```cpp
auto a = xer::interval<float>(0.25f);
auto b = xer::interval<float>(0.75f);

if (a < b) {
    // true
}
```

---

## Arithmetic Between Interval Values

Arithmetic between values of the same `interval` type is supported.

```cpp
operator+
operator-
operator*
operator/
```

The operation is ordinary scalar arithmetic on the stored values, followed by validation and clamping.

Example:

```cpp
using component = xer::interval<float>;

auto a = component(0.8f);
auto b = component(0.5f);

auto sum = a + b;       // 1.0f
auto product = a * b;   // 0.4f
auto diff = b - a;      // 0.0f
```

Division by an interval value whose stored value is zero throws `std::domain_error`.

---

## Arithmetic with Right-Hand Scalar Values

The following forms are supported:

```cpp
interval + scalar
interval - scalar
interval * scalar
interval / scalar
```

They are useful for increasing, decreasing, scaling, and dividing bounded values.

Example:

```cpp
using component = xer::interval<float>;

auto brightness = component(0.5f);

brightness += 0.25f; // 0.75f
brightness *= 2.0f;  // 1.0f
brightness -= 2.0f;  // 0.0f
```

The scalar is converted to the interval's value type and then validated.

`NaN`, infinity, and division by zero throw `std::domain_error`.

---

## Left-Hand Scalar Multiplication

Scalar multiplication is also supported in the left-hand form:

```cpp
scalar * interval
```

Example:

```cpp
using component = xer::interval<float>;

auto brightness = component(0.75f);
auto dimmed = 0.5f * brightness;
// dimmed.value() == 0.375f
```

This form is provided because multiplication is natural in either order.

---

## Unsupported Left-Hand Scalar Forms

The following forms are intentionally not provided:

```cpp
scalar + interval
scalar - interval
scalar / interval
```

Scalar addition and subtraction are intended to express increasing or decreasing the interval value.
For readability, the interval value should appear on the left-hand side.

Scalar division by an interval value has a reciprocal-like meaning and is not considered a common bounded-value operation.

If such behavior is needed, callers can use `value()` explicitly.

---

## Compound Assignment

`interval` provides compound assignment operators.

With another interval value:

```cpp
operator+=
operator-=
operator*=
operator/=
```

With a right-hand scalar value:

```cpp
operator+=
operator-=
operator*=
operator/=
```

Each operation preserves the interval invariant.

Example:

```cpp
using component = xer::interval<float>;

auto x = component(0.5f);

x += 0.2f; // 0.7f
x *= 2.0f; // 1.0f
x /= 4.0f; // 0.25f
```

---

## Unary Operators

Unary plus and unary minus are provided.

```cpp
+x
-x
```

Unary plus returns the value unchanged.

Unary minus negates the stored value and then constructs a new interval value from the result.
For the default `[0, 1]` interval, this usually clamps to `0`.

Example:

```cpp
auto x = xer::interval<float>(0.25f);
auto y = -x;
// y.value() == 0.0f
```

For a symmetric interval, unary minus behaves more naturally.

```cpp
using gain = xer::interval<float, -1.0f, 1.0f>;

auto x = gain(0.25f);
auto y = -x;
// y.value() == -0.25f
```

---

## Error Handling Model

`interval` uses exceptions only for exceptional numeric conditions.

This differs from ordinary XER APIs that return `xer::result` for normal recoverable failures.

The reason is that `interval` is a value type with a simple invariant.
`NaN`, infinity, and division by zero are treated as invalid numeric states rather than ordinary input failures.

This design keeps normal arithmetic expressions readable:

```cpp
auto x = xer::interval<float>(0.5f);
auto y = x + 0.25f;
auto z = 0.5f * y;
```

---

## Typical Uses

### Color Components

```cpp
using component = xer::interval<float>;

auto r = component(1.25f);  // 1.0f
auto g = component(0.5f);   // 0.5f
auto b = component(-0.25f); // 0.0f
```

### Gain

```cpp
using gain = xer::interval<float, -1.0f, 1.0f>;

auto center = gain::from_ratio(0.5f);
// center.value() == 0.0f
```

### Brightness Adjustment

```cpp
using component = xer::interval<float>;

auto brightness = component(0.5f);
brightness += 0.25f;
brightness *= 2.0f;
```

---

## Relationship to Other Headers

`<xer/interval.h>` is related to the following headers:

- `<xer/cyclic.h>`
- `<xer/arithmetic.h>`
- `<xer/stdfloat.h>`

The rough boundary is:

- `<xer/cyclic.h>` handles circular normalized values
- `<xer/interval.h>` handles linear bounded values
- `<xer/arithmetic.h>` provides arithmetic helper functions
- `<xer/stdfloat.h>` provides floating-point type aliases and literals

`interval` is not absorbed into `<xer/arithmetic.h>` because it is a value type with an invariant, not merely an arithmetic helper function group.

---

## Documentation Notes

When documenting `interval`, it is important to make the following points explicit:

- the interval is closed
- the default interval is `[0, 1]`
- finite out-of-range values are clamped
- `NaN` and infinity throw
- division by zero throws
- arithmetic is scalar arithmetic followed by clamping
- this is not mathematical interval arithmetic
- `interval` is linear and non-wrapping, unlike `cyclic`

---

## Example

```cpp
#include <xer/interval.h>
#include <xer/stdio.h>

auto main() -> int
{
    using component = xer::interval<float>;

    const auto r = component(1.25f);
    const auto g = component(0.5f);
    const auto b = component(-0.25f);

    if (!xer::printf(u8"r = %g\n", static_cast<double>(r.value())).has_value()) {
        return 1;
    }
    if (!xer::printf(u8"g = %g\n", static_cast<double>(g.value())).has_value()) {
        return 1;
    }
    if (!xer::printf(u8"b = %g\n", static_cast<double>(b.value())).has_value()) {
        return 1;
    }

    auto brightness = component(0.5f);
    brightness += 0.25f;

    if (!xer::printf(
            u8"brightness = %g\n",
            static_cast<double>(brightness.value()))
            .has_value()) {
        return 1;
    }

    return 0;
}
```

This example shows the basic XER style:

- use the public header
- construct bounded values naturally
- let finite out-of-range input clamp
- use XER formatted output for examples
- check fallible output operations explicitly

---

## See Also

- `policy_interval.md`
- `policy_cyclic.md`
- `header_cyclic.md`
- `policy_arithmetic.md`
- `header_arithmetic.md`
