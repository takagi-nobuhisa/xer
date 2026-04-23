# Policy for Circular Values: `cyclic`

## Overview

XER provides the `cyclic` type and related functionality in order to handle **circular values** such as angles, phases, directions, and times of day.

This facility is intended not merely to support modular arithmetic, but also to make the notions of **clockwise** (`cw`) and **counterclockwise** (`ccw`) explicit.

The internal representation is a **normalized representation in which one full turn is `1`**.
Conversion to and from external representations such as degrees and radians is handled separately.

---

## Basic Policy

### Scope

`cyclic` is used to represent at least the following kinds of values:

- angles
- phases
- directions
- periodic positions
- arbitrary circular quantities defined relative to one full turn

### Internal Representation

The internal representation of `cyclic` is the interval **`[0, 1)`** using a floating-point value `T`.

- `0` represents the reference position
- `1` is regarded as identical to `0`, and is therefore not stored as an internal value
- values are always normalized so that `0 <= value < 1`

Here, `1` means **one full turn** as a dimensionless value.

### Type

`cyclic` is a class template parameterized by a floating-point type.

```cpp
template <std::floating_point T>
class cyclic;
````

The main intended template arguments are:

* `float`
* `double`
* `long double`

Integer types are not accepted.

### Main Intended Uses

Practical target uses of `cyclic` include at least the following:

* pan / tilt
* hue
* direction or hue control in lighting, video, and UI systems
* general real-time control use cases

For these purposes, `float` is often sufficient in practice, so `cyclic` may primarily assume use with `float`.
At the same time, `double` and `long double` should also be usable when needed.

---

## Why the Internal Representation Uses `[0, 1)`

By unifying the internal representation to `[0, 1)`, circularity itself can be handled independently of external units.

For example, for angles this can be interpreted as follows:

* degrees: `value * 360`
* radians: `value * 2π`

This policy allows the same arithmetic rules to be used internally at all times, while unit conversion is separated to input/output boundaries.

---

## Normalization Policy

### Basic Policy

Whenever a `cyclic` value is constructed or updated from an arbitrary floating-point value, the value is normalized into **`[0, 1)`**.

### Meaning of Normalization

Conceptually, normalization is **the operation of mapping `x mod 1` into `[0, 1)`**.

### Implementation Policy

In implementation terms, an operation equivalent to `x - floor(x)` using `std::floor` is sufficient.

Conceptually, examples are as follows:

* `0.3` → `0.3`
* `1.3` → `0.3`
* `-0.2` → `0.8`
* `-1.2` → `0.8`

### Why `std::modf` Is Not Mandated

Normalization can be understood as extracting the fractional part, but the implementation is not fixed to `std::modf`.

This is because the priority is to make the implementation easy to express in a **`constexpr`-friendly form**.

### Invariant

The internal value of `cyclic` always satisfies:

```text
0 <= value < 1
```

This invariant must be maintained by every operation that can modify the internal value, such as construction, assignment, and arithmetic updates.

---

## Rotation Direction Policy

### Positive Direction

In XER, the positive direction of `cyclic` follows mathematical convention and is **counterclockwise**.

* positive direction: `ccw`
* negative direction: `cw`

### Naming

Function names should remain short in order to avoid unnecessary verbosity.

* `cw`
* `ccw`

The longer names `clockwise` and `counterclockwise` are not adopted.

---

## Nature of the `cyclic` Type

`cyclic` is designed as a lightweight value type that stores a single floating-point value.

* it has value semantics
* it is copyable
* it is movable
* it should be handled as a small value type

For this reason, arguments of member functions and operators should, as a rule, be passed **by value rather than by reference**.

Especially when `float` is used, passing by value is considered lighter and more natural.

---

## Default Tolerance

The default tolerance for equality testing in `cyclic` is stored as a static member of the `cyclic` type itself.

```cpp
static constexpr T default_epsilon =
    std::numeric_limits<T>::epsilon() * static_cast<T>(16);
```

The purpose of this is:

* to avoid type switching
* to follow `float`, `double`, and `long double` naturally
* to provide a more practical width than `std::numeric_limits<T>::epsilon()` alone

---

## Member Functions

The member functions of `cyclic` are unified in the direction where `this` is **from** and the argument is **to**.

### Retrieving the Value

```cpp
auto value() const noexcept -> T;
```

* returns the internal representation itself
* the returned value is always within `[0, 1)`

### Clockwise Distance

```cpp
auto cw(cyclic to) const noexcept -> T;
```

* returns the distance from `this` to `to` in the **clockwise** direction
* the return value is in the range `0` inclusive to `1` exclusive

### Counterclockwise Distance

```cpp
auto ccw(cyclic to) const noexcept -> T;
```

* returns the distance from `this` to `to` in the **counterclockwise** direction
* the return value is in the range `0` inclusive to `1` exclusive

### Shortest Signed Difference

```cpp
auto diff(cyclic to) const noexcept -> T;
```

* returns the **shortest signed difference** from `this` to `to`
* positive means `ccw`
* negative means `cw`

The return value range is the following half-open interval:

```text
[-0.5, 0.5)
```

Accordingly, when the difference is exactly a half turn, it is normalized to the `-0.5` side.

### Equality Testing

Equality testing for `cyclic` is not strict bitwise equality, but **practical equality with tolerance**.

For that reason, `==` and `!=` are not adopted.
Instead, member functions `eq` and `ne` are provided.

```cpp
auto eq(cyclic to) const noexcept -> bool;
auto ne(cyclic to) const noexcept -> bool;

auto eq(cyclic to, T epsilon) const noexcept -> bool;
auto ne(cyclic to, T epsilon) const noexcept -> bool;
```

#### Method of Comparison

The comparison is based not on a simple difference between representative values, but on the **shortest difference on the circle**.

Conceptually, the rule is:

```text
abs(diff(to)) <= epsilon
```

#### Why `==` and `!=` Are Not Adopted

If `==` and `!=` are defined as approximate comparison, they are easy to misunderstand as strict comparison.
Also, the fact that a tolerance is involved becomes difficult to read from the code.

For that reason, `cyclic` adopts explicit member functions `eq` and `ne`.

---

## Operators

### Operators That May Be Provided

At least the following operators may be provided for `cyclic`:

* unary `+`
* unary `-`
* binary `+`
* binary `-`
* `+=`
* `-=`

### Meaning

These operators are treated not as ordinary real-number arithmetic, but as **arithmetic on a circle**.

* the result after addition or subtraction is always normalized into `[0, 1)`
* `a + b` means the result of moving forward by `b` on the circle
* `a - b` means the result of moving backward by `b` on the circle

### Comparison Operators

`==`, `!=`, `<`, `<=`, `>`, `>=`, and `<=>` are not provided.

The reasons are as follows:

* `==` and `!=` are too easily misunderstood as strict comparison
* equality testing for `cyclic` is tolerance-based
* order comparison is not intrinsic to circular values

---

## Relationship to Mathematical Constants

### Basic Policy

Radian conversion for `cyclic` uses a mathematical constant representing π.

However, the π constant is not stored as a member of `cyclic`, but is separated into a dedicated header under `xer/bits`.

### Dedicated Header

At minimum, a dedicated header such as the following may be provided:

```text
xer/bits/math_constants.h
```

This header may define π for the time being, and may later be extended with additional mathematical constants if necessary.

### Constant Names

At least the following constants may be defined:

```cpp
template <std::floating_point T>
inline constexpr T pi_v = ...;

template <std::floating_point T>
inline constexpr T 𝜋 = pi_v<T>;
```

### Why `𝜋` Is Defined as an Alias

A one-character global identifier is often avoided in general, but `𝜋` is a widely known mathematical symbol with a clear meaning.
Also, since `pi_v` remains available as the formal name, readability and maintainability still have an escape route.

For that reason, `𝜋` may be formally allowed as an **alias in the role of a mathematical symbol**.

---

## Unit Conversion by Free Functions

### Basic Policy

Conversion to and from degrees and radians is not the responsibility of `cyclic` itself, but is provided as **free functions**.

### Reason

* `cyclic` itself is a unitless internal representation
* unit conversion is more naturally performed at input/output boundaries
* this also makes it easier to handle other one-turn-based quantities besides degrees and radians in the future

### Examples

At least the following functions may be assumed:

```cpp
template <std::floating_point T>
auto from_degree(T value) noexcept -> cyclic<T>;

template <std::floating_point T>
auto to_degree(cyclic<T> value) noexcept -> T;

template <std::floating_point T>
auto from_radian(T value) noexcept -> cyclic<T>;

template <std::floating_point T>
auto to_radian(cyclic<T> value) noexcept -> T;
```

The exact function names may be reconsidered during implementation, but they should not be member functions.

---

## Typical Interpretations

### Example: `cw` / `ccw`

If the internal values are:

* `a = 0.1`
* `b = 0.3`

then they are interpreted as follows:

* `a.ccw(b)` is `0.2`
* `a.cw(b)` is `0.8`

Here, `a` is from and `b` is to.

### Example: `diff`

* if `a.diff(b)` is positive, it means `ccw`
* if `a.diff(b)` is negative, it means `cw`

For example, the shortest difference from `0.9` to `0.1` is treated as `+0.2`.

---

## Candidates for Future Extensions

In the future, at least the following extensions may be considered:

* a thin wrapper type specialized for azimuth
* a thin wrapper type specialized for times of day
* a discrete circular value type based on a fixed division count of one full turn
* a circular interval type such as arcs or sectors
* helper functions for integration with `sin`, `cos`, `atan2`, and similar functions
* unit conversion functions beyond degrees and radians

However, in the initial stage the priority is not to expand that far, but first to establish the basic design of `cyclic<T>` itself.

---

## Tentative API List

The currently envisioned tentative API is as follows:

```cpp
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

template <std::floating_point T>
constexpr auto operator+(cyclic<T> left, cyclic<T> right) noexcept -> cyclic<T>;

template <std::floating_point T>
constexpr auto operator-(cyclic<T> left, cyclic<T> right) noexcept -> cyclic<T>;

template <std::floating_point T>
constexpr auto from_degree(T value) noexcept -> cyclic<T>;

template <std::floating_point T>
constexpr auto to_degree(cyclic<T> value) noexcept -> T;

template <std::floating_point T>
constexpr auto from_radian(T value) noexcept -> cyclic<T>;

template <std::floating_point T>
constexpr auto to_radian(cyclic<T> value) noexcept -> T;
```

---

## Summary

* `cyclic` is a lightweight type for handling circular values such as angles, phases, directions, and times of day
* its internal representation is normalized to the half-open interval `[0, 1)`
* one full turn is represented by `1`
* the positive direction is counterclockwise
* clockwise and counterclockwise distance are made explicit through `cw` and `ccw`
* the shortest signed difference is represented by `diff`
* equality is approximate and is expressed by `eq` and `ne` rather than by `==` and `!=`
* ordering operators are not provided
* conversion to and from degrees and radians is handled by free functions
* mathematical constants such as π are separated into a dedicated header
* the initial priority is to establish the basic design of `cyclic<T>` itself
