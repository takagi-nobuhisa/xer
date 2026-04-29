# Policy for Color Systems

## Overview

XER provides color-system facilities in order to handle practical color values and color-space conversion in a lightweight and C++-friendly form.

The purpose of this facility is not to cover every historical or perceptual color classification system.
Instead, XER focuses on color systems that can be handled as numeric value types and converted through clear formulas.

The initial supported color systems are:

- RGB
- Grayscale
- CMY
- HSV
- CIE 1931 XYZ
- CIE 1976 L*a*b*
- CIE 1976 L*u*v*

The following systems are outside the scope of XER, unless there is a major reason to reconsider in the future:

- Munsell color system
- PCCS
- Ostwald color system
- NCS
- ABC tone system

---

## Basic Policy

### Purpose

The color facility should make it possible to represent common color values and perform practical conversion between supported color systems.

At minimum, the facility should support:

- representation of RGB color values
- conversion between RGB and CMY
- conversion between RGB and HSV
- conversion between RGB and XYZ
- conversion between XYZ and CIE L*a*b*
- conversion between XYZ and CIE L*u*v*

### Scope

The initial scope is numeric color representation and conversion.

The following are not the purpose of this facility:

- color palette management
- color naming
- color harmony theory
- color appearance models
- printer-profile management
- ICC profile handling
- spectral color representation
- complete color management system functionality

XER's color facility is intended to remain small and practical.

---

## Public Header

Color facilities should be provided through an independent public header:

```text
xer/color.h
```

The implementation may be placed in:

```text
xer/bits/color.h
```

Color values are not merely arithmetic helpers.
They are small value types with clear domain meaning.
For that reason, they should not be absorbed into `xer/arithmetic.h`.

---

## Naming Policy

### Type Names

The basic class templates use the `basic_` prefix and the color-system name.

```cpp
template <std::floating_point T>
struct basic_rgb;

template <std::floating_point T>
struct basic_gray;

template <std::floating_point T>
struct basic_cmy;

template <std::floating_point T>
struct basic_hsv;

template <std::floating_point T>
struct basic_xyz;

template <std::floating_point T>
struct basic_lab;

template <std::floating_point T>
struct basic_luv;
```

The ordinary aliases use `float`.

```cpp
using rgb = basic_rgb<float>;
using gray = basic_gray<float>;
using cmy = basic_cmy<float>;
using hsv = basic_hsv<float>;
using xyz = basic_xyz<float>;
using lab = basic_lab<float>;
using luv = basic_luv<float>;
```

### Why `float` Aliases Are Provided

Although `float`, `double`, and `long double` should be usable as internal representation types, practical color handling will usually use `float`.

For ordinary use, users should not need to write `basic_rgb<float>` repeatedly.

Therefore, `rgb`, `cmy`, `hsv`, `xyz`, `lab`, and `luv` are provided as `float` aliases.

### RGB Naming

In color science, RGB-to-XYZ conversion depends on the specific RGB color space.

XER's RGB-to-XYZ conversion assumes sRGB with the D65 white point.

However, in the public API, the code notation is simply `rgb` and `basic_rgb`.
The names `srgb` and `basic_srgb` are not used in the initial API.

This is intentional.

The documentation must clearly state that conversion between RGB and XYZ uses the sRGB/D65 interpretation, even though the type name is `rgb`.

---

## Supported Color Systems

## RGB

### Meaning

`basic_rgb<T>` represents an RGB color using normalized red, green, and blue components.

In XER, RGB is treated as sRGB when converting to or from XYZ.

### Components

```cpp
template <std::floating_point T>
struct basic_rgb {
    using value_type = T;
    using component_type = interval<T>;

    component_type r;
    component_type g;
    component_type b;
};
```

Each component is a bounded value in `[0, 1]`.

### Default Alias

```cpp
using rgb = basic_rgb<float>;
```

### Alpha

Alpha is not part of `basic_rgb`.

Alpha is mainly useful for graphics, compositing, and image processing.
It is not a general component of color itself and is unnecessary for areas such as printing, coating, lighting, and colorimetry.

If alpha support becomes necessary, it should be provided separately as a different type such as `basic_rgba<T>`.
It should not be mixed into `basic_rgb<T>`.

---

## Grayscale

`basic_gray<T>` represents a display-oriented grayscale value. Its component is `interval<T>` in `[0, 1]`.

```cpp
template <std::floating_point T>
struct basic_gray {
    using value_type = T;
    using component_type = interval<T>;

    component_type y;
};

using gray = basic_gray<float>;
```

XER provides both simple luma grayscale and luminance-based grayscale.

- `to_luma_gray(rgb)` computes `0.2126 R' + 0.7152 G' + 0.0722 B'` directly from nonlinear sRGB components.
- `to_luminance_gray(rgb)` decodes sRGB to linear RGB, computes relative luminance, and encodes the result back to a nonlinear display grayscale value.
- `to_gray(rgb)` is the ordinary convenience function and is an alias for `to_luma_gray`.
- `to_rgb(gray)` duplicates the grayscale component into RGB.

## CMY

### Meaning

`basic_cmy<T>` represents a subtractive CMY color using normalized cyan, magenta, and yellow components.

CMY is useful as a simple counterpart to RGB.

### Components

```cpp
template <std::floating_point T>
struct basic_cmy {
    using value_type = T;
    using component_type = interval<T>;

    component_type c;
    component_type m;
    component_type y;
};
```

Each component is a bounded value in `[0, 1]`.

### Default Alias

```cpp
using cmy = basic_cmy<float>;
```

### Relationship to RGB

The initial CMY model is the simple normalized complement model.

Conceptually:

```text
C = 1 - R
M = 1 - G
Y = 1 - B
```

and:

```text
R = 1 - C
G = 1 - M
B = 1 - Y
```

This is not a complete printer color-management model.
It does not cover CMYK, ink behavior, ICC profiles, or device-specific calibration.

---

## HSV

### Meaning

`basic_hsv<T>` represents hue, saturation, and value.

HSV is useful for UI, color picking, simple hue rotation, and intuitive adjustment.

### Components

```cpp
template <std::floating_point T>
struct basic_hsv {
    using value_type = T;
    using hue_type = cyclic<T>;
    using component_type = interval<T>;

    hue_type h;
    component_type s;
    component_type v;
};
```

### Component Meaning

- `h`: hue, represented by `cyclic<T>` in `[0, 1)`
- `s`: saturation, represented by `interval<T>` in `[0, 1]`
- `v`: value, represented by `interval<T>` in `[0, 1]`

### Default Alias

```cpp
using hsv = basic_hsv<float>;
```

### Hue

Hue is circular.
For that reason, it should use `cyclic<T>` rather than `interval<T>`.

This keeps wraparound behavior explicit and matches the existing XER model for circular values.

---

## CIE 1931 XYZ

### Meaning

`basic_xyz<T>` represents CIE 1931 XYZ tristimulus values.

XYZ is the central connection point between RGB and CIE Lab/Luv.

### Components

```cpp
template <std::floating_point T>
struct basic_xyz {
    using value_type = T;

    T x;
    T y;
    T z;
};
```

XYZ components are not represented by `interval<T>`.

The values are colorimetric quantities and are not simply normalized UI components.
They may exceed the range `[0, 1]` depending on scaling, conversion, and white point handling.

### Default Alias

```cpp
using xyz = basic_xyz<float>;
```

### White Point

RGB-to-XYZ and XYZ-to-RGB conversion assumes sRGB with the D65 white point.

Lab and Luv conversion also require a reference white.
The initial implementation should use D65 consistently unless a later API explicitly adds configurable white points.

---

## CIE 1976 L*a*b*

### Meaning

`basic_lab<T>` represents the CIE 1976 L*a*b* color space.

It is derived from XYZ with a reference white point.

### Components

```cpp
template <std::floating_point T>
struct basic_lab {
    using value_type = T;

    T l;
    T a;
    T b;
};
```

### Naming of Members

The member names are lowercase ASCII identifiers:

- `l`
- `a`
- `b`

The public API does not use identifiers such as `L*`, `a*`, or `b*`.

### Default Alias

```cpp
using lab = basic_lab<float>;
```

### Component Ranges

Lab components are not represented by `interval<T>`.

Although `L*` is commonly in `[0, 100]`, practical conversion and intermediate processing should not force all components into interval wrappers at this stage.

The `a*` and `b*` components are signed and have no simple universal fixed range suitable for `interval<T>`.

---

## CIE 1976 L*u*v*

### Meaning

`basic_luv<T>` represents the CIE 1976 L*u*v* color space.

It is derived from XYZ with a reference white point.

### Components

```cpp
template <std::floating_point T>
struct basic_luv {
    using value_type = T;

    T l;
    T u;
    T v;
};
```

### Naming of Members

The member names are lowercase ASCII identifiers:

- `l`
- `u`
- `v`

The public API does not use identifiers such as `L*`, `u*`, or `v*`.

### Default Alias

```cpp
using luv = basic_luv<float>;
```

### Component Ranges

Luv components are not represented by `interval<T>`.

As with Lab, the signed chromatic components do not have a simple fixed range appropriate for `interval<T>` in the initial design.

---

## Conversion Policy

### General Form

Conversions should be provided as ordinary free functions.

The basic naming style is:

```cpp
auto to_rgb(cmy value) -> rgb;
auto to_cmy(rgb value) -> cmy;
auto to_rgb(hsv value) -> rgb;
auto to_hsv(rgb value) -> hsv;
auto to_xyz(rgb value) -> xyz;
auto to_rgb(xyz value) -> rgb;
auto to_lab(xyz value) -> lab;
auto to_xyz(lab value) -> xyz;
auto to_luv(xyz value) -> luv;
auto to_xyz(luv value) -> xyz;
```

For templates, the return type should preserve the source value type where practical.

Example:

```cpp
template <std::floating_point T>
constexpr auto to_cmy(basic_rgb<T> value) -> basic_cmy<T>;
```

### RGB and CMY

RGB and CMY conversion is a simple component complement conversion.

This conversion should be exact apart from ordinary floating-point rounding.

### RGB and HSV

RGB and HSV conversion should follow the common normalized HSV model:

- RGB components are in `[0, 1]`
- hue is normalized to `[0, 1)`
- saturation is in `[0, 1]`
- value is in `[0, 1]`

Hue should be represented through `cyclic<T>`.

For grayscale colors where saturation is zero, hue is not meaningful.
In such cases, the initial implementation may set hue to zero.

### RGB and XYZ

RGB-to-XYZ conversion assumes sRGB with the D65 white point.

The conversion must include:

- sRGB transfer-function decoding from nonlinear RGB to linear RGB
- matrix conversion from linear RGB to XYZ

XYZ-to-RGB conversion must include:

- matrix conversion from XYZ to linear RGB
- sRGB transfer-function encoding
- clamping into RGB component intervals

The public names remain `to_xyz(rgb)` and `to_rgb(xyz)`.
Documentation must explicitly state the sRGB/D65 assumption.

### XYZ and Lab

XYZ-to-Lab and Lab-to-XYZ conversion use the CIE 1976 L*a*b* formulas.

The initial implementation should use D65 as the reference white unless a later API introduces configurable white points.

### XYZ and Luv

XYZ-to-Luv and Luv-to-XYZ conversion use the CIE 1976 L*u*v* formulas.

The initial implementation should use D65 as the reference white unless a later API introduces configurable white points.

### Direct Conversion Between Non-Adjacent Systems

The initial API does not need to provide every possible direct conversion.

For example, these can be composed by the caller:

```cpp
auto x = to_xyz(color);
auto y = to_lab(x);
```

This keeps the public API small and avoids unnecessary duplication.

If a direct conversion becomes clearly useful later, it may be added.

---

## White Point Policy

### Initial Policy

The initial implementation uses D65 as the reference white.

This applies to:

- RGB <-> XYZ
- XYZ <-> Lab
- XYZ <-> Luv

### Future Extension

Configurable white points may be considered later.

However, the initial API should not be complicated with white-point parameters unless actual need arises.

Possible future types may include:

```cpp
struct white_point;
```

or predefined constants such as:

```cpp
inline constexpr white_point d65;
inline constexpr white_point d50;
```

These are deferred.

---

## Gamma and Transfer Function Policy

RGB values in `basic_rgb<T>` are treated as ordinary nonlinear sRGB component values when converting to and from XYZ.

Therefore:

- `to_xyz(rgb)` decodes sRGB components to linear RGB before matrix conversion
- `to_rgb(xyz)` applies the sRGB transfer function after matrix conversion

XER does not introduce a separate linear RGB type in the initial design.

If linear RGB becomes necessary, it should be introduced as a separate type rather than overloading the meaning of `rgb`.

Possible future type:

```cpp
template <std::floating_point T>
struct basic_linear_rgb;
```

This is deferred.

---

## Use of `interval` and `cyclic`

### `interval`

`interval<T>` should be used for normalized bounded components.

This includes:

- RGB components
- grayscale component
- CMY components
- HSV saturation
- HSV value

These values naturally stay in `[0, 1]`.

### `cyclic`

`cyclic<T>` should be used for hue.

Hue is circular rather than linear.
For that reason, it should not be represented as an `interval<T>`.

### Raw Floating-Point Values

Raw `T` members should be used for colorimetric spaces whose values are not naturally bounded to `[0, 1]`.

This includes:

- XYZ
- Lab
- Luv

---

## Constructors

Each color type should provide at least:

- default construction
- construction from component values

For component-based types, construction from raw `T` values should be allowed where natural.

Example:

```cpp
constexpr basic_rgb(T r, T g, T b);
```

This constructor should initialize interval components and therefore clamp finite out-of-range values.

For types with raw floating-point members such as XYZ, Lab, and Luv, construction may store the values directly.

If NaN or infinity validation is considered necessary for raw colorimetric types, it should be considered separately.
The first implementation may keep those types as simple aggregate-like values.

---

## Public Data Members

Color value types should be simple structs with public data members.

This follows XER's general preference for simple, understandable value types.

Examples:

```cpp
color.r
color.g
color.b
```

This is more readable and practical than requiring accessor functions for every component.

---

## Exception and Error Policy

Color conversion functions should generally avoid `xer::result` unless there is a normal, expected failure mode.

Most supported color conversions are deterministic arithmetic operations.

Therefore, they should normally return the destination color value directly.

For component types based on `interval<T>`, invalid values such as NaN or infinity may throw according to `interval`'s own policy.

For raw colorimetric types such as XYZ, Lab, and Luv, the initial implementation may simply store raw values and rely on ordinary floating-point behavior unless a specific validation policy is introduced later.

---

## Non-Supported Color Systems

The following systems are not supported by XER's color facility.

This is not merely a temporary omission.
Unless there is a major reason to reconsider, they should remain outside the scope of XER.

### Munsell Color System

The Munsell color system is a color-order system based on hue, value, and chroma.

It is useful as a color notation and color-ordering system, but practical conversion to numeric RGB or XYZ values depends on color tables, interpolation, and reference data.

XER does not include it in the initial or planned scope.

### PCCS

PCCS is the Practical Color Co-ordinate System / Japanese Color Research Institute color system.

It is useful for hue-tone organization and color planning, but it is not a simple formula-based color space suitable for XER's lightweight numeric conversion API.

XER does not include it in the initial or planned scope.

### Ostwald Color System

The Ostwald color system is a historical color-order system based on pure color, white, and black components.

It is not adopted as part of XER's practical numeric color conversion layer.

### NCS

NCS is a perceptual natural color system.

It is useful as a color notation and specification system, but it is not adopted as part of XER's lightweight formula-based color facility.

### ABC Tone System

The ABC tone system is outside the scope of XER.

It appears closer to tone classification or applied color planning than to a numeric color space suitable for the core library.

---

## Deferred Items

The following items are deferred:

- alpha / RGBA
- CMYK
- HSL
- HWB
- linear RGB
- configurable white points
- chromatic adaptation
- ICC profile handling
- color appearance models
- spectral color representation
- color difference formulas such as Delta E
- color temperature
- named colors
- palette utilities
- direct conversion functions for every pair of supported spaces

Some of these may be useful later, but they are not needed for the initial color-system facility.

---

## Implementation Order

A practical implementation order is:

1. `basic_rgb<T>` and `rgb`
2. `basic_cmy<T>` and `cmy`
3. `basic_hsv<T>` and `hsv`
4. RGB <-> CMY conversion
5. RGB <-> HSV conversion
6. `basic_xyz<T>` and `xyz`
7. RGB <-> XYZ conversion using sRGB/D65
8. `basic_lab<T>` and `lab`
9. XYZ <-> Lab conversion using D65
10. `basic_luv<T>` and `luv`
11. XYZ <-> Luv conversion using D65

This order keeps early implementation simple while preparing for CIE-based conversion.

---

## Documentation Notes

Documentation should explicitly state the following:

- RGB in code is written as `rgb`, not `srgb`
- RGB-to-XYZ conversion assumes sRGB/D65
- RGB, CMY, and HSV normalized components use `interval<T>`
- HSV hue uses `cyclic<T>`
- XYZ, Lab, and Luv use raw floating-point members
- alpha is not part of RGB
- unsupported color systems are outside the scope of XER
- XER's color facility is not a full color management system

---

## Summary

- XER supports RGB, grayscale, CMY, HSV, XYZ, Lab, and Luv color values
- ordinary aliases use `float`
- `rgb` means the practical sRGB-based RGB model for conversion purposes
- code uses `rgb`, not `srgb`
- RGB components are `interval<T>` values in `[0, 1]`
- CMY components are `interval<T>` values in `[0, 1]`
- HSV hue is `cyclic<T>`
- HSV saturation and value are `interval<T>` values in `[0, 1]`
- XYZ, Lab, and Luv use raw floating-point components
- RGB <-> XYZ conversion assumes sRGB/D65
- Lab and Luv conversion initially use D65
- alpha is not part of RGB
- Munsell, PCCS, Ostwald, NCS, and ABC tone system are not supported
- the facility remains lightweight and formula-based
