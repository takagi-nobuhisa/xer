# Policy for Physical Quantities and Units

## Overview

XER provides a quantity system with units and dimensions in order to handle physical quantities in a type-safe manner.

The purpose of this facility is not to follow existing C++ libraries or future standardization proposals as they are.
Instead, in line with XER's overall policy, it aims to provide **physical quantity and unit facilities that are lightweight, easy to understand, and convenient for everyday use**.

XER already adopts original designs for time handling and circular values rather than matching the standard library exactly.
Physical quantities and units follow the same approach and adopt a concise design suited to XER.

This document is the **initial policy document** for physical quantity and unit facilities.
Implementation details will be examined separately and fed back into this document when necessary.

---

## Basic Policy

### Purpose

At minimum, the physical quantity and unit facility should achieve the following:

- prevent incorrect addition and comparison between quantities of different dimensions
- make unit conversion explicit and safe
- allow base units and derived units to be handled in natural notation
- focus on practically common use cases and avoid excessive generalization

### Internal Representation

A physical quantity is stored internally as a **real value normalized to the base unit system**.

In the initial stage, the base unit system consists of the following four units:

- length: meter
- mass: kilogram
- time: second
- electric current: ampere

That is, the internal system is effectively **MKSA**.

For example:

- `1 km` is stored internally as `1000 m`
- `1 g` is stored internally as `0.001 kg`
- `1 msec` is stored internally as `0.001 sec`

By always storing internal values in base units, addition, subtraction, comparison, multiplication, division, and unit conversion are simplified.

---

## Handling of Dimensions

### Base Dimensions

In the initial stage, the following four base dimensions are handled:

- length
- mass
- time
- electric current

Temperature, amount of substance, luminous intensity, and similar dimensions are not handled in the initial stage.

### Dimension Representation

A dimension is represented by a type whose template arguments are the exponents for each base dimension.

```cpp
template <int L, int M, int T, int I>
struct dimension;
````

The meaning of each argument is as follows:

* `L`: length
* `M`: mass
* `T`: time
* `I`: electric current

### Examples

* length: `dimension<1, 0, 0, 0>`
* mass: `dimension<0, 1, 0, 0>`
* time: `dimension<0, 0, 1, 0>`
* electric current: `dimension<0, 0, 0, 1>`
* velocity: `dimension<1, 0, -1, 0>`
* force: `dimension<1, 1, -2, 0>`
* voltage: `dimension<2, 1, -3, -1>`

### Dimensionless

A dimensionless quantity is represented by the dimension whose exponents are all zero.

```cpp
dimension<0, 0, 0, 0>
```

If needed, an alias such as the following may be provided:

```cpp
using dimensionless = dimension<0, 0, 0, 0>;
```

---

## Quantity Type

### Basic Policy

A physical quantity is represented as a value type consisting of a **numeric value** and a **dimension**.

```cpp
template <typename T, typename Dim>
class quantity;
```

Here:

* `T` is the stored value type
* `Dim` is the dimension type

### Stored Value Type

At least in the initial stage, the stored value type of `quantity` is restricted to **floating-point types**.

The main intended types are:

* `float`
* `double`
* `long double`

Integer types are not adopted as the stored value type of `quantity`, at least in the initial stage.

The reasons are as follows:

* unit conversion naturally produces fractional values
* units such as `rad` involve non-rational scale factors
* normalized internal values in base units are not limited to integers
* in the initial stage, priority is given to keeping the design simple

### Stored Value

The value held by `quantity<T, Dim>` is **always a value already normalized to base units**.

For example, if `quantity<float, time_dim>` contains `0.001f`, that means `0.001 sec`.

### Construction and Value Retrieval

`quantity` should be handled according to at least the following ideas:

* `quantity(T value)` constructs directly from a value in **base units**
* `T * unit` constructs a `quantity` from a specified unit
* `value()` returns the value in base units
* `value(unit)` returns the value converted to the specified unit

For example:

```cpp
using namespace xer::units;

auto x = 1.5 * km;
auto y = x.value();    // 1500.0 (m)
auto z = x.value(km);  // 1.5
```

### Retrieving a Raw Value from a Dimensionless Quantity

Even for a dimensionless quantity, there are practical situations where the raw numeric value should be retrievable.

For that reason, a means of retrieving the raw value from a dimensionless quantity is provided.

However, implicit conversion from a dimensionless quantity to a raw scalar tends to weaken type safety.
Accordingly, at least in the initial stage, conversion from a dimensionless quantity to a raw value should be **explicit**.

---

## Unit Type

### Basic Policy

A unit is provided as something that represents the **scale relative to the base unit** and the **dimension**.

It has at least the following two elements:

* the scale relative to the base unit
* the dimension

Here, the base units are, in the initial stage:

* length: `m`
* mass: `kg`
* time: `sec`
* electric current: `A`

### Nature of `unit`

`unit` should, as far as possible, be something that represents unit information **as a type**, and should prioritize a design that does not hold runtime data members that need to exist materially.

That is, unit information should in principle be represented by template arguments, and `unit` objects themselves should ideally be usable as empty value types.

The reasons are as follows:

* units are naturally treated as type information attached to a quantity rather than as values themselves
* multiplication, division, and composition of units should be resolved at compile time as much as possible
* unit objects under `xer::units` should remain lightweight
* unnecessary memory usage and runtime processing should be avoided

Accordingly, at least in the initial stage, `unit` should prioritize a form that **represents only the information specified by template arguments and has no additional data members**.

### Handling of Scale

A unit scale is **not limited to what can be represented as a rational number**.

For example, the following units are naturally expressible by rational scale factors:

* `mm`
* `cm`
* `km`
* `g`
* `mg`
* `nsec`
* `microsec`
* `mA`
* `kHz`
* `GHz`
* `hPa`

On the other hand, when the angular unit `rad` is handled relative to `taurad`, its scale is `1 / τ`.
This cannot be represented as a rational number.

Accordingly, XER's unit system does **not** fix unit scales to a rational-number representation equivalent to `std::ratio`.

### Scale Representation

At least in the initial stage, the representation of unit scales should use a **`std::ratio`-like rational representation as the default**.

At the same time, there are units such as `rad`, handled relative to `taurad`, whose scales cannot be represented rationally.
For that reason, XER should use a design that **treats `std::ratio` as the default while also allowing floating-point-based scale representations when necessary**.

At minimum, the initial organization is:

* ordinary units use a `std::ratio`-like representation
* special units use a scale representation equivalent to `long double`

This means that `std::ratio` should be understood as the default representation strategy, not as a hard limit on what kinds of units the library may represent.

### Scale in Unit Arithmetic

When units are multiplied or divided, not only the dimensions but also the scales are composed.

* multiplication: multiply the scales
* division: divide the scales

For results of arithmetic between rational units, rational representations may be preserved as much as possible.
If the result cannot be represented rationally, it may move to a floating-point-based scale representation.

---

## Unit Objects

### How They Are Provided

Units used in everyday practice are provided as unit objects in the `xer::units` namespace.

```cpp
namespace xer::units {
    inline constexpr /* ... */ m = /* ... */;
    inline constexpr /* ... */ kg = /* ... */;
    inline constexpr /* ... */ sec = /* ... */;
    inline constexpr /* ... */ A = /* ... */;
}
```

Unit objects should be `inline constexpr` so that they fit naturally with a header-only structure.

### Namespace

Units are not placed directly under `xer`, but are grouped into the dedicated namespace `xer::units`.

This makes it possible to:

* separate unit names from ordinary APIs
* import them only where needed via `using namespace xer::units;`
* avoid making `xer::msec`-style notation the normal form

### Example Usage

```cpp
using namespace xer::units;

auto t = 1.23f * msec;
auto x = 10.0 * m;
auto i = 20.0 * mA;
```

If needed, fully qualified names may also be used.

```cpp
auto t = 1.23f * xer::units::msec;
```

---

## Policy for Unit Names

### Basic Policy

Unit names should use the **common symbolic forms that people actually use** whenever possible.

For example, the following symbolic names are preferred:

* `m`
* `kg`
* `sec`
* `A`
* `V`

Long English words such as `meter`, `second`, `kilogram`, and `volt` are not adopted at least as unit object names.

### Seconds

For the second, the unit object name should be `sec` rather than `s`.

The purpose is:

* to improve visual readability
* to make it easier to distinguish from names such as `ms`
* to avoid collisions with ordinary identifiers and reduce ambiguity in code

### Prefixes

For the SI prefix micro, `u` is not used.

* `micro` is used as the formal name
* if needed, `μ` may be provided as an alias

Accordingly, if microseconds or microamperes are introduced, the following forms are preferred at minimum:

* `microsec`
* `microA`

The following aliases may also be provided when needed:

* `μsec`
* `μA`

Although `u` is often used as an ASCII substitute, XER does not adopt that as the formal notation.

### Representation of `L`

For the volume unit `L`, `ℓ` is not adopted as an alias.

---

## User-Defined Literals

### Policy

User-defined literals are not defined.

As a result of consideration, notation such as the following is judged to be more natural:

```cpp
1.23f * msec
```

Accordingly, the physical quantity and unit facility:

* uses multiplication with unit objects as the basic notation
* does not introduce user-defined literals

---

## Quantity Arithmetic

### Addition and Subtraction

Addition and subtraction are allowed **only between quantities of the same dimension**.

* `length + length` is allowed
* `length - length` is allowed
* `length + time` is not allowed

### Comparison

Comparison is also allowed **only between quantities of the same dimension**.

* `length < length` is allowed
* `length == length` is allowed
* `length < time` is not allowed

### Multiplication and Division

Multiplication and division are allowed.

In this case, the resulting dimension is obtained by addition or subtraction of dimension exponents.

* multiplication: add the exponents
* division: subtract the exponents

### Relation to Raw Scalars

Operations such as `quantity * scalar`, `scalar * quantity`, and `quantity / scalar` may be allowed.

However, there is no implicit conversion between dimensioned quantities and raw scalars.

### Examples

* `m * m` → area
* `m / sec` → velocity
* `m / (sec * sec)` → acceleration
* `kg * m / (sec * sec)` → force

---

## Unit Arithmetic

### Basic Policy

Multiplication and division between units are allowed.

Accordingly, derived units need not all be provided as large numbers of predeclared names.

### Examples

```cpp
using namespace xer::units;

auto v = 3.0 * m / sec;
auto a = 9.8 * m / (sec * sec);
auto f = 2.0 * kg * m / (sec * sec);
```

This avoids the need to define large numbers of composed unit names such as `m_sec` or `m_per_sec`.

### Mixed Arithmetic Between Quantities and Units

At the implementation level, notation such as the following may be allowed:

* `quantity / unit`
* `scalar * unit`

This makes it possible to write expressions such as `10.0 * m / sec` naturally.

However, the meaning must always be understood as **constructing or converting a quantity normalized to base units**.

---

## Criteria for Choosing Units in the Initial Stage

Units provided by default in the initial stage should prioritize those that satisfy at least one of the following:

* units learned by the end of Japanese high school education
* units frequently used in software and electronic engineering
* units frequently appearing in ordinary books, news, and practical technical documents

On the other hand, at least in the initial stage, XER does not rush to provide unit groups such as the following:

* units strongly tied to specific fields
* units whose user base is limited
* units whose symbolic names are short and prone to collision with other identifiers
* unit groups that become awkward unless the whole system is prepared consistently

For example, XER recognizes that CGS units are used in some fields, but at least in the initial stage they should not be broadly incorporated into XER itself.
Instead, they should be defined by users or by domain-specific extensions when needed.

---

## Units Provided in the Initial Stage

### Base Units

At minimum, the following base units are provided in the initial stage:

* length: `m`
* mass: `kg`
* time: `sec`
* electric current: `A`

### Prefixed Units

At minimum, the following prefixed units are provided in the initial stage.

#### Length

* `mm`
* `cm`
* `km`
* `microm`
* `nm`

#### Mass

* `g`
* `mg`

#### Time

* `nsec`
* `microsec`
* `msec`

#### Electric Current

* `mA`

#### Frequency

* `kHz`
* `GHz`

#### Pressure

* `hPa`

### Derived Units

At minimum, the following derived units are provided in the initial stage:

* `Hz`
* `N`
* `J`
* `W`
* `V`
* `Pa`

### Conventional Units for Area and Volume

Considering everyday use and practicality, at minimum the following units are provided in the initial stage.

#### Area

* `ha`

#### Volume

* `mL`
* `dL`
* `L`
* `kL`

### Conventional Units for Heat Quantity

Considering their frequent use in books, news, and nutrition labeling, at minimum the following units are provided in the initial stage:

* `cal`
* `kcal`

Although these are dimensionally the same as energy, at least in the initial stage they are positioned as **conventional units for heat quantity**.

### Angular Units

At minimum, the following angular units are provided in the initial stage:

* `taurad`
* `τrad`
* `rad`

Here:

* `taurad` is the base unit for angle
* `τrad` is an alias of `taurad`
* `rad` is a dimensionless unit for angles

### Units Provided as Aliases

At minimum, the following aliases may be provided in the initial stage:

* `μm` = `microm`
* `μsec` = `microsec`
* `cc` = `mL`

---

## Units Deferred in the Initial Stage

At least the following units are deferred in the initial stage.

### Area

* `a`

### Length / Astronomy

* `AU`
* light-year

### Energy / Particle Physics

* `eV`

### Calendar / Everyday Time Units

* year
* month
* day
* hour
* minute
* week

### Others

* `ℓ`
* `Å`

---

## Handling of Angles

### Basic Policy

For angles, the roles of ordinary physical quantities and circular values are separated.

* ordinary angular quantities including whole turns are handled as **simple real values plus units**
* circular operations are handled by **`cyclic`** when needed

XER's `cyclic` is a type for circular values where one full turn is `1`.
Accordingly, it is more natural **not** to store every angle quantity directly as `cyclic`, but to use `cyclic` only when circular operations are required.

### Examples

Quantities such as the following should preferably be representable as ordinary quantities including turn counts:

* `2.25` turns
* `-0.1` turns
* `10.0` turns

On the other hand, concepts such as:

* the position within one turn
* clockwise distance
* counterclockwise distance
* shortest difference

should use `cyclic`.

---

## Angular Units

### Handling of `rad`

`rad` is dimensionless in terms of dimension analysis.

Accordingly, when `rad` is introduced in the physical quantity and unit framework, it is treated not as a new dimension, but as a **dimensionless unit that differs only in scale**.

### `taurad` / `τrad`

Since `cyclic` in XER uses the design where one full turn is `1`, XER adopts **`taurad`** as the base unit for angles and provides **`τrad`** as an alias.

* `1 taurad` = 1 turn
* `0.5 taurad` = half a turn
* `0.25 taurad` = quarter turn

This unit corresponds very clearly to `cyclic`.

### Policy

At minimum, angular units are handled as follows:

* `taurad` as the base unit
* `τrad` as an alias

`rad` is provided from the initial stage as a **dimensionless unit for angles**.

Because `taurad` is the base unit, `rad` is positioned as a conversion unit relative to `taurad`.

### Angular Quantities Including Turn Counts

Angular quantities including turn counts are not stored internally as `cyclic`.
Instead, they are stored as **simple real values**, and converted to `cyclic` only when needed.

This makes it possible to handle naturally:

* ordinary addition and subtraction of angles
* angle calculations from angular velocity and time
* retention of turn counts

---

## Relationship to `cyclic`

`cyclic` is positioned not as the general representation of angles themselves, but as an **auxiliary type for circular operations**.

Accordingly, the relationship between angular quantities and `cyclic` is organized as follows:

* angular quantities are stored as ordinary quantities
* they are converted to `cyclic` when circular comparison or shortest-difference calculation is needed
* the design does not directly multiply units onto `cyclic`

In the initial stage, a dedicated high-level bridge API between physical quantities and `cyclic` is not mandatory.

At minimum in the initial stage, it is sufficient that:

* `quantity` can explicitly provide its raw value
* a raw value can explicitly construct a `quantity`
* `cyclic` can likewise exchange values through raw scalars

This allows users to write explicitly, when needed:

* extract a raw value from an angular quantity
* pass that value to `cyclic`
* convert the value obtained from `cyclic` back into an angular quantity

This is less type-safe than a dedicated API, but in the initial stage priority is given to avoiding excessive design complexity.

---

## Relationship to the Standard Library

The current standard library provides time-interval facilities such as `std::chrono::duration`, but a general-purpose physical quantity and unit library is not yet established as a standard facility.

At the same time, standardization proposals for quantity and unit libraries are progressing.

However, XER does not simply follow them wholesale.
Instead, in the initial stage it prioritizes the following:

* lightweight design
* ease of understanding
* consistency with XER's overall design
* avoiding the introduction of an excessively large metamodel

Accordingly, advanced concepts such as the following may be deferred in the initial stage:

* quantity kind
* quantity point
* origin-based quantities such as temperature
* generalized complex unit systems

---

## Tentative API Direction

The following API shape may be assumed at present as a **provisional and deliberately simplified sketch**:

```cpp
template <int L, int M, int T, int I>
struct dimension;

using dimensionless = dimension<0, 0, 0, 0>;

template <typename Dim, typename Scale = std::ratio<1>>
class unit;

template <std::floating_point T, typename Dim>
class quantity;
```

Here, `Scale = std::ratio<1>` should be understood only as a convenient default for ordinary rational-scale units.

It is **not** intended to mean that XER permanently restricts all unit scales to `std::ratio`-expressible values.
Special units such as `rad`, whose scale relative to the base unit is non-rational, may require a different internal representation strategy.

In other words, this tentative API sketch shows the default direction for the common case, while the exact scale-representation mechanism remains open to refinement during implementation.

At minimum, usage such as the following should be allowed:

```cpp
using namespace xer::units;

auto x = 1.5 * km;
auto t = 2.0 * sec;
auto v = x / t;

auto base = x.value();     // base unit value
auto kilo = x.value(km);   // value in km
```

---

## Summary

* physical quantities carry a value normalized to base units and a dimension
* the base dimensions are length, mass, time, and electric current
* a dimensionless quantity is represented by all-zero exponents
* at least in the initial stage, the stored value type of `quantity` is restricted to floating-point types
* `quantity` stores base-unit values and provides `value()` and `value(unit)` for retrieval
* units are provided as `inline constexpr` objects under `xer::units`
* `unit` should prioritize type-level representation without runtime data members
* user-defined literals are not defined
* multiplication and division between units are allowed
* natural expressions such as `10.0 * m / sec` are allowed through mixed arithmetic between scalars, quantities, and units
* conversion from a dimensionless quantity to a raw scalar should be explicit
* unit scales are not limited to rational numbers
* ordinary units use `std::ratio`-like scales by default, while special units may use floating-point-based scales
* the initial unit set focuses on practical and commonly used units
* `taurad` is the base unit for angle, and `rad` is treated as a dimensionless angular unit derived from it
* `cyclic` is positioned as an auxiliary type for circular operations rather than as the general representation of all angle quantities
