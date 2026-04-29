# `<xer/color.h>`

## Purpose

`<xer/color.h>` provides color-system value types and color conversion functions.

The purpose of this header is to support practical formula-based color representation and conversion in a lightweight XER style.

The initial supported color systems are:

- RGB
- Grayscale
- CMY
- HSV
- CIE 1931 XYZ
- CIE 1976 L*a*b*
- CIE 1976 L*u*v*

This header does not attempt to become a complete color management system.
It does not handle ICC profiles, chromatic adaptation, spectral data, named colors, or color palette management.

---

## Main Entities

At minimum, `<xer/color.h>` provides the following class templates:

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

It also provides ordinary `float` aliases:

```cpp
using rgb = basic_rgb<float>;
using gray = basic_gray<float>;
using cmy = basic_cmy<float>;
using hsv = basic_hsv<float>;
using xyz = basic_xyz<float>;
using lab = basic_lab<float>;
using luv = basic_luv<float>;
```

The public header is:

```cpp
#include <xer/color.h>
```

The implementation is provided through:

```cpp
#include <xer/bits/color.h>
```

Users should include the public header.

---

## Design Role

`<xer/color.h>` provides small value types and conversion functions for common color systems.

The design is based on these ideas:

- color values are simple structs with public data members
- ordinary aliases use `float`
- normalized bounded components use `xer::interval<T>`
- hue uses `xer::cyclic<T>`
- colorimetric spaces such as XYZ, Lab, and Luv use raw floating-point members
- conversion functions are free functions
- deterministic arithmetic conversions return the destination value directly

---

## Float Aliases

Although the templates can use `float`, `double`, or `long double`, practical color handling usually uses `float`.

For this reason, the ordinary aliases use `float`.

```cpp
xer::rgb color(0.25f, 0.5f, 0.75f);
```

If a different precision is needed, use the corresponding template directly.

```cpp
xer::basic_rgb<double> color(0.25, 0.5, 0.75);
```

---

## RGB

## `basic_rgb`

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

`basic_rgb<T>` represents an RGB color with normalized red, green, and blue components.

Each component is represented by `interval<T>` and is therefore kept in `[0, 1]`.

```cpp
auto color = xer::rgb(1.25f, 0.5f, -0.25f);

// color.r.value() == 1.0f
// color.g.value() == 0.5f
// color.b.value() == 0.0f
```

### sRGB Assumption

The public type name is `rgb`, not `srgb`.

However, conversion between RGB and XYZ assumes sRGB with the D65 white point.

That means:

- `to_xyz(rgb)` treats RGB components as nonlinear sRGB components
- `to_rgb(xyz)` produces nonlinear sRGB components

This assumption must be kept in mind when using RGB together with XYZ, Lab, or Luv.

### Alpha

Alpha is not part of `basic_rgb`.

Alpha is mainly useful for graphics, compositing, and image processing.
It is not a general component of color itself and is unnecessary for areas such as printing, coating, lighting, and colorimetry.

If alpha support becomes necessary later, it should be provided as a separate type such as `basic_rgba<T>`.
It should not be mixed into `basic_rgb<T>`.

---

## Grayscale

## `basic_gray`

```cpp
template <std::floating_point T>
struct basic_gray {
    using value_type = T;
    using component_type = interval<T>;

    component_type y;
};
```

`basic_gray<T>` represents a display-oriented grayscale value. The component is represented by `interval<T>` and is kept in `[0, 1]`.

`to_luma_gray` computes simple luma directly from nonlinear sRGB components. `to_luminance_gray` computes relative luminance after sRGB decoding and then encodes it back to a display grayscale value. `to_gray` is an alias for `to_luma_gray`. `to_rgb(gray)` duplicates the grayscale component into RGB.

## CMY

## `basic_cmy`

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

`basic_cmy<T>` represents a simple normalized CMY color.

Each component is represented by `interval<T>` and is therefore kept in `[0, 1]`.

CMY in XER is the simple complement model of RGB.

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
It does not represent CMYK, ink behavior, ICC profiles, or device-specific calibration.

---

## HSV

## `basic_hsv`

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

`basic_hsv<T>` represents hue, saturation, and value.

The components are:

- `h`: hue, represented by `cyclic<T>` in `[0, 1)`
- `s`: saturation, represented by `interval<T>` in `[0, 1]`
- `v`: value, represented by `interval<T>` in `[0, 1]`

Hue is circular, so it uses `cyclic<T>` rather than `interval<T>`.

For grayscale colors, where saturation is zero, hue is not meaningful.
The current conversion from RGB to HSV sets hue to zero in that case.

---

## XYZ

## `basic_xyz`

```cpp
template <std::floating_point T>
struct basic_xyz {
    using value_type = T;

    T x;
    T y;
    T z;
};
```

`basic_xyz<T>` represents CIE 1931 XYZ tristimulus values.

XYZ is used as the central connection point between RGB and the CIE Lab/Luv spaces.

XYZ components are raw floating-point values.
They are not represented by `interval<T>` because XYZ values are colorimetric quantities rather than normalized UI components.

The initial implementation uses D65 as the reference white for conversions.

---

## Lab

## `basic_lab`

```cpp
template <std::floating_point T>
struct basic_lab {
    using value_type = T;

    T l;
    T a;
    T b;
};
```

`basic_lab<T>` represents CIE 1976 L*a*b* values.

The member names are lowercase ASCII identifiers:

- `l`
- `a`
- `b`

The public API does not use identifiers such as `L*`, `a*`, or `b*`.

Lab components are raw floating-point values.
They are not represented by `interval<T>`.

Although `L*` is commonly in `[0, 100]`, the initial implementation does not force it into an interval wrapper.
The `a*` and `b*` components are signed and do not have a simple universal fixed range.

---

## Luv

## `basic_luv`

```cpp
template <std::floating_point T>
struct basic_luv {
    using value_type = T;

    T l;
    T u;
    T v;
};
```

`basic_luv<T>` represents CIE 1976 L*u*v* values.

The member names are lowercase ASCII identifiers:

- `l`
- `u`
- `v`

The public API does not use identifiers such as `L*`, `u*`, or `v*`.

Luv components are raw floating-point values.
They are not represented by `interval<T>`.

---

## Constructors

Each color type supports default construction and construction from component values.

### RGB

```cpp
constexpr basic_rgb();
constexpr basic_rgb(T r, T g, T b);
constexpr basic_rgb(component_type r, component_type g, component_type b) noexcept;
```

Default construction creates black.

```cpp
xer::rgb color;
// r == 0, g == 0, b == 0
```

Construction from raw component values clamps finite out-of-range values through `interval<T>`.

### CMY

```cpp
constexpr basic_cmy();
constexpr basic_cmy(T c, T m, T y);
constexpr basic_cmy(component_type c, component_type m, component_type y) noexcept;
```

Construction from raw component values clamps finite out-of-range values through `interval<T>`.

### HSV

```cpp
constexpr basic_hsv();
constexpr basic_hsv(T h, T s, T v);
constexpr basic_hsv(hue_type h, component_type s, component_type v) noexcept;
```

Hue is normalized through `cyclic<T>`.
Saturation and value are clamped through `interval<T>`.

### XYZ, Lab, and Luv

XYZ, Lab, and Luv store raw floating-point values.

```cpp
constexpr basic_xyz(T x, T y, T z) noexcept;
constexpr basic_lab(T l, T a, T b) noexcept;
constexpr basic_luv(T l, T u, T v) noexcept;
```

The initial implementation does not add special validation to these raw colorimetric values.

---

## Conversion Functions

Conversions are provided as free functions.

## RGB and CMY

```cpp
template <std::floating_point T>
constexpr auto to_cmy(basic_rgb<T> value) -> basic_cmy<T>;

template <std::floating_point T>
constexpr auto to_rgb(basic_cmy<T> value) -> basic_rgb<T>;
```

RGB and CMY conversion uses simple normalized complement conversion.

Example:

```cpp
const auto cmy = xer::to_cmy(xer::rgb(0.25f, 0.5f, 0.75f));

// cmy.c.value() == 0.75f
// cmy.m.value() == 0.5f
// cmy.y.value() == 0.25f
```

---

## RGB and HSV

```cpp
template <std::floating_point T>
constexpr auto to_hsv(basic_rgb<T> value) -> basic_hsv<T>;

template <std::floating_point T>
constexpr auto to_rgb(basic_hsv<T> value) -> basic_rgb<T>;
```

RGB and HSV conversion uses the common normalized HSV model.

- RGB components are in `[0, 1]`
- hue is normalized to `[0, 1)`
- saturation is in `[0, 1]`
- value is in `[0, 1]`

For grayscale RGB colors, `to_hsv` sets hue to zero.

Example:

```cpp
const auto hsv = xer::to_hsv(xer::rgb(0.25f, 0.5f, 0.75f));
const auto rgb = xer::to_rgb(hsv);
```

---

## RGB and XYZ

```cpp
template <std::floating_point T>
auto to_xyz(basic_rgb<T> value) -> basic_xyz<T>;

template <std::floating_point T>
auto to_rgb(basic_xyz<T> value) -> basic_rgb<T>;
```

RGB and XYZ conversion assumes sRGB with the D65 white point.

`to_xyz(rgb)` performs:

1. sRGB transfer-function decoding from nonlinear RGB to linear RGB
2. matrix conversion from linear RGB to XYZ

`to_rgb(xyz)` performs:

1. matrix conversion from XYZ to linear RGB
2. sRGB transfer-function encoding
3. clamping into RGB component intervals

The public names are `to_xyz(rgb)` and `to_rgb(xyz)`, not `to_xyz(srgb)` or `to_srgb(xyz)`.

Example:

```cpp
const auto xyz = xer::to_xyz(xer::rgb(1.0f, 1.0f, 1.0f));

// approximately D65 white:
// x == 0.95047f
// y == 1.0f
// z == 1.08883f
```

---

## XYZ and Lab

```cpp
template <std::floating_point T>
auto to_lab(basic_xyz<T> value) -> basic_lab<T>;

template <std::floating_point T>
constexpr auto to_xyz(basic_lab<T> value) -> basic_xyz<T>;
```

XYZ and Lab conversion uses CIE 1976 L*a*b* formulas with D65 as the reference white.

Example:

```cpp
const auto lab = xer::to_lab(xer::xyz(0.95047f, 1.0f, 1.08883f));

// approximately:
// l == 100
// a == 0
// b == 0
```

---

## XYZ and Luv

```cpp
template <std::floating_point T>
auto to_luv(basic_xyz<T> value) -> basic_luv<T>;

template <std::floating_point T>
auto to_xyz(basic_luv<T> value) -> basic_xyz<T>;
```

XYZ and Luv conversion uses CIE 1976 L*u*v* formulas with D65 as the reference white.

Example:

```cpp
const auto luv = xer::to_luv(xer::xyz(0.95047f, 1.0f, 1.08883f));

// approximately:
// l == 100
// u == 0
// v == 0
```

---

## Direct Conversion Policy

The initial API does not provide direct conversion functions for every possible pair of supported color systems.

For example, RGB to Lab can be written explicitly through XYZ:

```cpp
const auto xyz = xer::to_xyz(color);
const auto lab = xer::to_lab(xyz);
```

This keeps the public API small and avoids unnecessary duplication.

Direct conversion functions may be added later if they become clearly useful.

---

## Error and Exception Policy

Color conversion functions generally return the destination color value directly.

They do not return `xer::result`, because the supported conversions are deterministic arithmetic operations and do not have ordinary recoverable failure modes.

However:

- RGB, grayscale, CMY, and HSV normalized components use `interval<T>`
- hue uses `cyclic<T>`

Therefore, invalid finite-state cases such as `NaN` or infinity may be rejected according to the policies of `interval<T>` and `cyclic<T>`.

For raw colorimetric types such as XYZ, Lab, and Luv, the initial implementation stores raw floating-point values directly and does not add special validation.

---

## Supported and Unsupported Color Systems

### Supported

The initial supported color systems are:

- RGB
- Grayscale
- CMY
- HSV
- CIE 1931 XYZ
- CIE 1976 L*a*b*
- CIE 1976 L*u*v*

### Unsupported

The following systems are outside the scope of XER:

- Munsell color system
- PCCS
- Ostwald color system
- NCS
- ABC tone system

This is not merely a temporary omission.
Unless there is a major reason to reconsider, these systems should remain outside the scope of XER.

The main reason is that they are color-order, color-notation, perceptual, or tone-classification systems rather than lightweight formula-based numeric color spaces suitable for XER's core API.

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

Some of these may be useful later, but they are not part of the initial color-system facility.

---

## Relationship to Other Headers

`<xer/color.h>` is related to:

- `<xer/interval.h>`
- `<xer/cyclic.h>`
- `<xer/stdfloat.h>`

The rough boundary is:

- `<xer/interval.h>` handles bounded linear scalar values
- `<xer/cyclic.h>` handles circular scalar values such as hue
- `<xer/color.h>` composes these into color-system value types and conversions
- `<xer/stdfloat.h>` provides floating-point type aliases and literals

---

## Documentation Notes

When documenting `<xer/color.h>`, it is important to state:

- `rgb` is used as the code name, not `srgb`
- RGB/XYZ conversion assumes sRGB/D65
- RGB, grayscale, CMY, and HSV normalized components use `interval<T>`
- HSV hue uses `cyclic<T>`
- XYZ, Lab, and Luv use raw floating-point members
- alpha is not part of RGB
- the facility is not a complete color management system
- unsupported color systems are intentionally outside the scope

---

## Example

```cpp
#include <xer/color.h>
#include <xer/stdio.h>

auto main() -> int
{
    const xer::rgb color(0.25f, 0.5f, 0.75f);

    const auto cmy = xer::to_cmy(color);
    const auto hsv = xer::to_hsv(color);
    const auto xyz = xer::to_xyz(color);
    const auto lab = xer::to_lab(xyz);

    if (!xer::printf(
             u8"RGB: r=%g, g=%g, b=%g\n",
             static_cast<double>(color.r.value()),
             static_cast<double>(color.g.value()),
             static_cast<double>(color.b.value()))
             .has_value()) {
        return 1;
    }

    if (!xer::printf(
             u8"CMY: c=%g, m=%g, y=%g\n",
             static_cast<double>(cmy.c.value()),
             static_cast<double>(cmy.m.value()),
             static_cast<double>(cmy.y.value()))
             .has_value()) {
        return 1;
    }

    if (!xer::printf(
             u8"HSV: h=%g, s=%g, v=%g\n",
             static_cast<double>(hsv.h.value()),
             static_cast<double>(hsv.s.value()),
             static_cast<double>(hsv.v.value()))
             .has_value()) {
        return 1;
    }

    if (!xer::printf(
             u8"Lab: l=%g, a=%g, b=%g\n",
             static_cast<double>(lab.l),
             static_cast<double>(lab.a),
             static_cast<double>(lab.b))
             .has_value()) {
        return 1;
    }

    return 0;
}
```

This example shows the basic XER style:

- use the public header
- construct an `rgb` value directly
- convert through free functions
- use `value()` when printing interval components
- use `<xer/stdio.h>` for output in examples
- check fallible output operations explicitly

---

## See Also

- `policy_color.md`
- `policy_interval.md`
- `policy_cyclic.md`
- `header_interval.md`
- `header_cyclic.md`
