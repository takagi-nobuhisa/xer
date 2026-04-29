# `<xer/cyclic.h>`

## Purpose

`<xer/cyclic.h>` provides the `cyclic` type and related helpers for handling circular values in XER.

This header is intended for values such as:

- angles
- phases
- directions
- time-of-day-like circular positions
- other quantities defined relative to one full turn

Its role is not merely to provide modular arithmetic.
Instead, it provides a lightweight value type that makes circular semantics explicit, especially concepts such as clockwise and counterclockwise distance.

---

## Main Role

The main role of `<xer/cyclic.h>` is to provide a compact and explicit model for circular values that:

- are normalized to one full turn
- need wraparound behavior
- benefit from shortest-difference operations
- should expose clockwise and counterclockwise interpretation directly

This makes the header especially useful for code involving:

- angles and rotations
- periodic control values
- UI or graphics direction handling
- other one-turn-based quantities

---

## Main Entities

At minimum, `<xer/cyclic.h>` provides the following entities:

```cpp
template <std::floating_point T>
class cyclic;

template <std::floating_point T>
auto from_degree(T value) noexcept -> cyclic<T>;

template <std::floating_point T>
auto to_degree(cyclic<T> value) noexcept -> T;

template <std::floating_point T>
auto from_radian(T value) noexcept -> cyclic<T>;

template <std::floating_point T>
auto to_radian(cyclic<T> value) noexcept -> T;
```

The exact overload set may grow, but this is the essential public shape.

---

## `cyclic<T>`

`cyclic<T>` is the central type of the header.

It represents a circular value normalized to one full turn.

### Basic Shape

At minimum, the class is expected to have a form like the following:

```cpp id="0r4t03"
template <std::floating_point T>
class cyclic {
public:
    using value_type = T;

    static constexpr T default_epsilon =
        std::numeric_limits<T>::epsilon() * static_cast<T>(16);

    constexpr cyclic() noexcept;
    constexpr explicit cyclic(T value) noexcept;

    constexpr auto value() const noexcept -> T;

    constexpr auto cw(cyclic to) const noexcept -> T;
    constexpr auto ccw(cyclic to) const noexcept -> T;
    constexpr auto diff(cyclic to) const noexcept -> T;

    constexpr auto eq(cyclic to) const noexcept -> bool;
    constexpr auto ne(cyclic to) const noexcept -> bool;

    constexpr auto eq(cyclic to, T epsilon) const noexcept -> bool;
    constexpr auto ne(cyclic to, T epsilon) const noexcept -> bool;

    constexpr auto operator+() const noexcept -> cyclic;
    constexpr auto operator-() const noexcept -> cyclic;

    constexpr auto operator+=(cyclic value) noexcept -> cyclic&;
    constexpr auto operator-=(cyclic value) noexcept -> cyclic&;
};
```

This header is therefore centered on one small, value-oriented class template rather than on a large framework.

---

## Internal Representation

`cyclic<T>` uses a normalized internal representation where one full turn is `1`.

### Basic Rule

The stored value always belongs to the half-open interval:

```text
[0, 1)
```

That means:

* `0` is the reference position
* `1` is identified with `0`
* values are normalized after construction and arithmetic updates

### Why This Matters

By using `[0, 1)` internally, the circular nature of the value is separated cleanly from external units such as:

* degrees
* radians
* any other one-turn-based external scale

This makes the type compact and unit-independent.

---

## Supported Value Types

`cyclic<T>` is parameterized by a floating-point type.

The intended template arguments are:

* `float`
* `double`
* `long double`

Integer types are not accepted.

### Why Floating-Point Types

Circular values are naturally modeled as continuous values rather than discrete modular integers in the main intended use cases.

This is especially appropriate for:

* direction control
* angle interpolation
* phase-related processing
* real-time graphics and UI work

---

## Normalization

A `cyclic<T>` object always stores a normalized value.

### Meaning

Conceptually, normalization means mapping an arbitrary value into the interval `[0, 1)`.

Examples:

* `0.3` stays `0.3`
* `1.3` becomes `0.3`
* `-0.2` becomes `0.8`

### Design Direction

The exact implementation is not the public concern.
What matters is the invariant:

```text
0 <= value < 1
```

This invariant is fundamental to all operations provided by the type.

---

## `value()`

```cpp id="ke9w5i"
auto value() const noexcept -> T;
```

### Purpose

`value()` returns the internal normalized representation.

### Meaning

The returned value is always in `[0, 1)`.

This is the raw circular representation used by the type itself.

### Notes

This is not the same thing as a degree or radian value.
Those conversions are handled by separate helper functions.

---

## Clockwise and Counterclockwise Distance

One of the defining features of `cyclic<T>` is that it makes direction along the circle explicit.

At minimum, this is expressed by:

```cpp id="ca9elv"
auto cw(cyclic to) const noexcept -> T;
auto ccw(cyclic to) const noexcept -> T;
```

### `cw`

`cw(to)` returns the clockwise distance from `this` to `to`.

### `ccw`

`ccw(to)` returns the counterclockwise distance from `this` to `to`.

### Range

These distances are returned in the range:

```text
[0, 1)
```

### Why This Matters

This explicit directional model is one of the main reasons `cyclic<T>` exists as its own type rather than simply using a floating-point value with manual modulo arithmetic.

---

## `diff`

```cpp id="j4di47"
auto diff(cyclic to) const noexcept -> T;
```

### Purpose

`diff(to)` returns the shortest signed difference from `this` to `to`.

### Meaning of the Sign

* positive means counterclockwise
* negative means clockwise

### Range

The returned value lies in:

```text
[-0.5, 0.5)
```

If the difference is exactly half a turn, it is normalized to the `-0.5` side.

### Why This Matters

This operation is especially useful in practical code that wants:

* shortest-angle movement
* compact directional difference logic
* comparison on a circle without manual wraparound handling

---

## Equality Testing

`cyclic<T>` does not use strict bitwise equality as its main equality model.

Instead, it provides explicit approximate equality helpers:

```cpp id="v86flg"
auto eq(cyclic to) const noexcept -> bool;
auto ne(cyclic to) const noexcept -> bool;

auto eq(cyclic to, T epsilon) const noexcept -> bool;
auto ne(cyclic to, T epsilon) const noexcept -> bool;
```

### Why `eq` / `ne` Exist

This design makes it clear that equality is tolerance-based rather than strict.

### Why `==` and `!=` Are Not the Main API

If ordinary comparison operators were used for approximate equality, it would be too easy to misread them as strict equality.

XER therefore prefers explicit named functions.

### Default Tolerance

The default tolerance is stored in:

```cpp id="tvnkmz"
static constexpr T default_epsilon;
```

This provides a practical default width appropriate to the floating-point type.

---

## Arithmetic Operators

At minimum, `cyclic<T>` may provide the following operators:

* unary `+`
* unary `-`
* binary `+`
* binary `-`
* `+=`
* `-=`

### Meaning

These operators are interpreted as arithmetic on a circle.

This means:

* results are always normalized back into `[0, 1)`
* addition means moving forward around the circle
* subtraction means moving backward around the circle

### Important Note

These are not ordinary real-number operators in the abstract mathematical sense.
They are circular operations defined by the type's normalization rule.

---

## Comparison Operators Not Provided

Order-comparison operators such as:

* `<`
* `<=`
* `>`
* `>=`
* `<=>`

are not part of the intended model.

### Why

Order comparison is not intrinsic to circular values in the same way it is for ordinary real numbers.

Similarly, `==` and `!=` are not the preferred public equality model because approximate comparison is the intended design.

---

## Unit Conversion Helpers

`<xer/cyclic.h>` provides free functions for conversion to and from ordinary angular units.

At minimum:

```cpp id="rwmkpt"
template <std::floating_point T>
auto from_degree(T value) noexcept -> cyclic<T>;

template <std::floating_point T>
auto to_degree(cyclic<T> value) noexcept -> T;

template <std::floating_point T>
auto from_radian(T value) noexcept -> cyclic<T>;

template <std::floating_point T>
auto to_radian(cyclic<T> value) noexcept -> T;
```

### Why Free Functions

Unit conversion is not treated as the responsibility of the `cyclic` object itself.

This keeps the type unitless internally while allowing conversion at the API boundary.

### Meaning

These functions translate between:

* external degree/radian values
* the internal one-turn-based representation

---

## Relationship to Mathematical Constants

Radian conversion naturally depends on π.

In XER's design, mathematical constants such as π are not embedded directly into `cyclic<T>` as members.
Instead, they are treated as separate supporting facilities, conceptually associated with dedicated internal constant support.

This keeps `cyclic<T>` itself focused on circular value handling rather than on general constant provision.

---

## Relationship to Other Headers

`<xer/cyclic.h>` should be understood together with:

* `policy_project_outline.md`
* `policy_cyclic.md`
* `header_quantity.md`

The rough boundary is:

* `<xer/cyclic.h>` handles circular values and circular operations
* `<xer/quantity.h>` handles physical quantities and units
* angular quantities may be represented as ordinary quantities, while `cyclic` is used when circular semantics are needed explicitly

This distinction is important in XER's design.

---

## Relationship to Angle Quantities

A central point in XER's design is that `cyclic<T>` is **not** the universal storage model for all angle quantities.

### Meaning

* ordinary angle quantities, including turn counts, are better modeled as quantities with units
* `cyclic<T>` is for circular interpretation
* conversion into `cyclic<T>` happens when shortest-difference, clockwise distance, or counterclockwise distance is the real concern

This makes `cyclic<T>` a focused and practical tool rather than a universal replacement for every angle-like value.

---

## Documentation Notes

When this header is used in generated documentation, it is usually enough to explain:

* that `cyclic<T>` stores values normalized to one turn
* that clockwise and counterclockwise distance are explicit operations
* that shortest signed difference is provided by `diff`
* that equality is approximate and expressed by `eq` / `ne`
* that degree/radian conversion is handled by free functions

Detailed numeric edge cases belong in the detailed reference or generated API sections.

---

## Example Topics Commonly Worth Showing

The following kinds of examples are especially suitable for this header:

* constructing a `cyclic<float>` from a raw turn-based value
* converting from degrees with `from_degree`
* converting to degrees with `to_degree`
* measuring clockwise and counterclockwise distance
* computing the shortest difference with `diff`
* comparing values with `eq`

These are good candidates for executable examples in `examples/`.

---

## Example

```cpp
#include <xer/cyclic.h>

auto main() -> int
{
    const auto a = xer::from_degree(36.0);
    const auto b = xer::from_degree(108.0);

    const auto d = a.diff(b);
    if (d <= 0.0) {
        return 1;
    }

    const auto deg = xer::to_degree(a);
    if (deg != 36.0) {
        return 1;
    }

    return 0;
}
```

This example shows the normal XER style:

* create circular values through free conversion helpers
* use circular operations explicitly
* treat direction on the circle as part of the API itself

---

## See Also

* `policy_project_outline.md`
* `policy_cyclic.md`
* `header_quantity.md`

---

## Ratio Conversion

`cyclic<T>` provides ratio-oriented member functions for symmetry with `interval`.

```cpp
constexpr auto ratio() const noexcept -> T;
static constexpr auto from_ratio(T ratio) noexcept -> cyclic;
```

`ratio()` returns the normalized internal position in `[0, 1)`.
It is an alias of `value()`.

`from_ratio()` constructs a cyclic value from a turn-based ratio and applies normal cyclic normalization.

```cpp
auto a = xer::cyclic<float>::from_ratio(1.25f);
// a.value() == 0.25f
```

---

## Explicit Conversion with `interval`

Implicit conversion between `cyclic` and `interval` is not provided.
The endpoint semantics are different, so conversion should be visible in the source code.

The interval header provides explicit helpers:

```cpp
auto to_cyclic(interval<T, Min, Max> value) noexcept -> cyclic<T>;
auto to_interval(cyclic<T> value) -> interval<T>;
```

`to_cyclic` maps an interval through its ratio.
`to_interval` maps a cyclic value to the default interval `[0, 1]`.

For custom interval bounds, use `interval<T, Min, Max>::from_ratio(value.ratio())`.
