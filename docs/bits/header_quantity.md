# `<xer/quantity.h>`

## Purpose

`<xer/quantity.h>` provides physical quantity and unit facilities in XER.

Its purpose is to allow quantities with dimensions to be handled in a type-safe and practical way.
This includes:

- preventing meaningless arithmetic between different dimensions
- making unit conversion explicit
- allowing natural notation with unit objects
- keeping the design lightweight and easy to understand

This header is not intended to reproduce an existing quantity library as it is.
Instead, it follows XER's own design priorities.

---

## Main Role

The main role of `<xer/quantity.h>` is to provide a compact framework for:

- dimensions
- units
- quantities
- practical predefined units under `xer::units`

This makes it possible to write code such as:

```cpp
using namespace xer::units;

auto x = 1.5 * km;
auto t = 2.0 * sec;
auto v = x / t;
```

while preserving dimension safety and explicit conversion rules.

---

## Main Entities

At minimum, `<xer/quantity.h>` provides the following entities:

```cpp
template <int L, int M, int T, int I>
struct dimension;

using dimensionless = dimension<0, 0, 0, 0>;

template <typename Dim, typename Scale = std::ratio<1>>
class unit;

template <std::floating_point T, typename Dim>
class quantity;

sq
cb
```

In addition, it provides predefined unit objects under the `xer::units` namespace.

---

## `dimension`

`dimension` represents the dimension of a physical quantity.

### Basic Shape

```cpp
template <int L, int M, int T, int I>
struct dimension;
```

### Meaning of the Parameters

The template arguments represent exponents of the base dimensions:

* `L`: length
* `M`: mass
* `T`: time
* `I`: electric current

### Examples

Typical examples include:

```cpp
dimension<1, 0, 0, 0>   // length
dimension<0, 1, 0, 0>   // mass
dimension<0, 0, 1, 0>   // time
dimension<0, 0, 0, 1>   // electric current
dimension<1, 0, -1, 0>  // velocity
dimension<1, 1, -2, 0>  // force
```

### Role

`dimension` exists to make dimensional correctness part of the type system.

This prevents invalid combinations such as:

* adding length and time
* comparing mass and electric current directly

---

## `dimensionless`

```cpp
using dimensionless = dimension<0, 0, 0, 0>;
```

### Role

`dimensionless` represents a quantity with no physical dimension.

This is useful for values such as:

* pure ratios
* normalized coefficients
* angular-unit-related scale values when treated dimensionlessly

### Notes

Even dimensionless quantities remain quantities in the type system.
They are not automatically the same thing as raw scalar values.

---

## `quantity<T, Dim>`

`quantity<T, Dim>` is the central value type of the header.

It represents a numeric value together with a dimension.

### Basic Shape

```cpp
template <std::floating_point T, typename Dim>
class quantity;
```

### Role

A `quantity<T, Dim>` stores:

* a numeric value
* a physical dimension

This allows arithmetic to preserve dimensional correctness.

### Stored Value Type

At least in the current design direction, `T` is restricted to floating-point types such as:

* `float`
* `double`
* `long double`

Integer storage is not the primary model.

### Why Floating-Point Storage

This is because:

* unit conversion naturally produces fractional values
* some unit scales are non-rational
* internal normalization to base units is not generally integral
* keeping the design simple is more important than supporting every numeric form initially

---

## Internal Quantity Representation

A `quantity<T, Dim>` stores its value normalized to the base unit system.

### Base Unit System

At minimum, the base dimensions are:

* meter
* kilogram
* second
* ampere

This means the internal system is effectively MKSA.

### Examples

Conceptually:

* `1 km` is stored as `1000 m`
* `1 g` is stored as `0.001 kg`
* `1 msec` is stored as `0.001 sec`

### Why This Matters

Normalizing stored values to base units simplifies:

* arithmetic
* comparison
* conversion between units
* reasoning about mixed units of the same dimension

---

## Construction and Value Retrieval

The intended usage model includes at least the following ideas:

* constructing directly from a base-unit value
* constructing from a scalar multiplied by a unit object
* retrieving the normalized base-unit value
* retrieving the value converted to a specified unit

Typical forms include:

```cpp
auto value() const noexcept -> T;
auto value(unit_type u) const noexcept -> T;
```

The exact signatures may vary, but this is the intended public direction.

### Example

```cpp
using namespace xer::units;

auto x = 1.5 * km;
auto a = x.value();    // base-unit value
auto b = x.value(km);  // value expressed in km
```

---

## Raw Value Access for Dimensionless Quantities

Dimensionless quantities sometimes need to be converted back to raw scalars.

### Design Direction

This should be possible, but it should remain explicit.

### Why Explicit

Implicit conversion from a dimensionless quantity to a raw scalar weakens the type system and makes code less clear.

For that reason, explicit conversion or explicit value retrieval is preferred.

---

## `unit<Dim, Scale>`

`unit<Dim, Scale>` represents a unit.

### Basic Shape

```cpp
template <typename Dim, typename Scale = std::ratio<1>>
class unit;
```

### Role

A unit represents:

* a dimension
* a scale relative to the base unit of that dimension

### Nature of `unit`

The intended design is that `unit` should be primarily type-level information.

That means:

* unit information should ideally be carried by template arguments
* unit objects should remain lightweight
* unnecessary runtime data members should be avoided

In practice, predefined unit objects should behave like empty or near-empty compile-time objects.

---

## Scale Representation

One important design point is that unit scales are **not** limited to rational values.

### Rational-Scale Examples

Many units can be represented naturally with rational scales:

* `mm`
* `cm`
* `km`
* `g`
* `mg`
* `msec`
* `kHz`
* `hPa`

### Non-Rational-Scale Example

Some units, such as `rad` relative to `taurad`, are not naturally representable by a purely rational scale.

### Design Direction

Therefore:

* `std::ratio`-like rational scales are the default
* floating-point-based scale representation may also be necessary for some units
* the template default should not be interpreted as a permanent restriction of the entire design

This point is especially important for understanding `unit<Dim, Scale>` correctly.

---

## Unit Arithmetic

Units may be multiplied and divided.

### Meaning

When units are multiplied or divided:

* dimensions are combined
* scales are combined

This allows natural construction of derived quantities such as:

```cpp
using namespace xer::units;

auto v = 3.0 * m / sec;
auto a = 9.8 * m / sq(sec);
auto f = 2.0 * kg * m / sq(sec);
```

### Why This Matters

This avoids the need to define every composite unit as a separate fixed name.

---

## Square and Cube Helpers for Units and Quantities

`<xer/quantity.h>` provides `sq` and `cb` for units and quantities.

```cpp
sq(unit)
cb(unit)
sq(quantity)
cb(quantity)
```

### Role

These helpers make repeated multiplication easier to read in unit expressions.
For example:

```cpp
using namespace xer::units;

auto acceleration = 9.8 * m / sq(sec);
```

This is equivalent in meaning to:

```cpp
auto acceleration = 9.8 * m / (sec * sec);
```

For quantities, `sq` and `cb` multiply the stored value and combine the dimension exponents accordingly.

### Symbolic Unit Aliases

For common base units, symbolic aliases are also provided under `xer::units`:

```cpp
m²
m³
sec²
sec³
```

These are aliases for the corresponding square or cube unit expressions:

```cpp
m²   // sq(m)
m³   // cb(m)
sec² // sq(sec)
sec³ // cb(sec)
```

They are intended as readable symbolic notation.
The ASCII forms `sq(m)`, `cb(m)`, `sq(sec)`, and `cb(sec)` remain available as the portable spelling.
---

## `xer::units`

Predefined unit objects are provided under the `xer::units` namespace.

### Role

This namespace groups common unit names in one predictable place.

### Basic Direction

Units are intentionally **not** placed directly under `xer`.

This makes it possible to write:

```cpp
using namespace xer::units;
```

only where needed, without polluting the main namespace.

### Examples of Base Units

At minimum, the following base units are provided:

```cpp
xer::units::m
xer::units::kg
xer::units::sec
xer::units::A
```

---

## Predefined Units

The header is expected to provide a practical set of common units.

### Base Units

At minimum:

* `m`
* `kg`
* `sec`
* `A`

### Squared and Cubed Base Units

At minimum:

* `m²`
* `m³`
* `sec²`
* `sec³`

### Selected Prefixed Units

Examples include:

* `mm`
* `cm`
* `km`
* `microm`
* `nm`
* `g`
* `mg`
* `nsec`
* `microsec`
* `msec`
* `mA`
* `kHz`
* `GHz`
* `hPa`

### Selected Derived Units

Examples include:

* `Hz`
* `N`
* `J`
* `W`
* `V`
* `Pa`

### Conventional Units

Examples include:

* `ha`
* `mL`
* `dL`
* `L`
* `kL`
* `cal`
* `kcal`

### Aliases

Examples may include:

* `μm`
* `μsec`
* `cc`

The exact set belongs to the detailed unit reference, but these are the main intended categories.

---

## Angular Units

`<xer/quantity.h>` also covers angle-related units in coordination with the broader XER design.

### Important Units

At minimum:

* `taurad`
* `τrad`
* `rad`

### Design Meaning

* `taurad` is the base unit for angle
* `τrad` is an alias of `taurad`
* `rad` is treated as a dimensionless unit for angle

### Why This Matters

This keeps angle quantities compatible with XER's design where one full turn corresponds naturally to the `cyclic` model.

---

## Relationship to `cyclic`

A key design point is that angle quantities and circular values are **not identical concepts**.

### Quantity Side

`quantity` handles ordinary angle quantities, including values such as:

* multiple turns
* negative turns
* values used in ordinary arithmetic

### `cyclic` Side

`cyclic` handles explicitly circular semantics such as:

* clockwise distance
* counterclockwise distance
* shortest difference on a circle

### Design Boundary

So the design direction is:

* store ordinary angles as quantities
* convert to `cyclic` when circular behavior is actually needed

This keeps both abstractions focused.

---

## User-Defined Literals

User-defined literals are not the primary notation model here.

### Preferred Style

The intended style is:

```cpp
1.23f * msec
```

rather than unit suffix literals.

### Why

This keeps the notation:

* explicit
* easy to read
* consistent with the rest of XER
* easier to extend without creating many special literal forms

---

## Relationship to Other Headers

`<xer/quantity.h>` should be understood together with:

* `policy_project_outline.md`
* `policy_quantity.md`
* `header_cyclic.md`
* `header_arithmetic.md`

The rough boundary is:

* `<xer/quantity.h>` handles units, dimensions, and quantities
* `<xer/cyclic.h>` handles explicitly circular values
* `<xer/arithmetic.h>` handles general arithmetic/comparison helpers not specific to physical dimensions

---

## Documentation Notes

When this header is used in generated documentation, it is usually enough to explain:

* that it provides dimensions, units, and quantities
* that quantities are normalized internally to base units
* that unit objects live under `xer::units`
* that scale is not conceptually restricted to rational representation only
* that ordinary angular quantities and `cyclic` values are distinct concepts

Detailed unit catalogs and per-operator rules belong in the detailed reference or generated API sections.

---

## Example Topics Commonly Worth Showing

The following kinds of examples are especially suitable for this header:

* constructing a quantity from a scalar and a unit
* converting a quantity to base units and to another unit
* dividing distance by time to obtain velocity\n* using `sq`, `cb`, `m²`, `m³`, and `sec²` in unit expressions
* using predefined units from `xer::units`
* handling angle quantities with `taurad` or `rad`

These are good candidates for executable examples in `examples/`.

---

## Example

```cpp
#include <xer/quantity.h>

using namespace xer::units;

auto main() -> int
{
    const auto distance = 1.5 * km;
    const auto seconds = 30.0 * sec;
    const auto speed = distance / seconds;

    const auto meters = distance.value();
    if (meters <= 0.0) {
        return 1;
    }

    const auto kilometers = distance.value(km);
    if (kilometers != 1.5) {
        return 1;
    }

    static_cast<void>(speed);
    return 0;
}
```

This example shows the normal XER style:

* use unit objects from `xer::units`
* construct quantities with scalar × unit
* retrieve values explicitly
* keep dimensional meaning in the type system

---

## See Also

* `policy_project_outline.md`
* `policy_quantity.md`
* `header_cyclic.md`
* `header_arithmetic.md`
