// XER_EXAMPLE_BEGIN: color_basic
//
// This example shows basic color-system conversion.
//
// Expected output:
// RGB: r=0.25, g=0.5, b=0.75
// CMY: c=0.75, m=0.5, y=0.25
// HSV: h=0.583333, s=0.666667, v=0.75
// Luma gray: y=0.4649
// Luminance gray: y=0.510946
// RGB from gray: r=0.4649, g=0.4649, b=0.4649
// RGB from HSV: r=0.25, g=0.5, b=0.75
// XYZ: x=0.191803, y=0.201624, z=0.523052
// Lab: l=52.018, a=0.0952846, b=-39.3692
// Luv: l=52.018, u=-28.3934, v=-60.8235

#include <xer/color.h>
#include <xer/stdio.h>

auto main() -> int
{
    const xer::rgb color(0.25f, 0.5f, 0.75f);

    if (!xer::printf(
             u8"RGB: r=%g, g=%g, b=%g\n",
             static_cast<double>(color.r.value()),
             static_cast<double>(color.g.value()),
             static_cast<double>(color.b.value()))
             .has_value()) {
        return 1;
    }

    const auto cmy = xer::to_cmy(color);

    if (!xer::printf(
             u8"CMY: c=%g, m=%g, y=%g\n",
             static_cast<double>(cmy.c.value()),
             static_cast<double>(cmy.m.value()),
             static_cast<double>(cmy.y.value()))
             .has_value()) {
        return 1;
    }

    const auto hsv = xer::to_hsv(color);

    if (!xer::printf(
             u8"HSV: h=%g, s=%g, v=%g\n",
             static_cast<double>(hsv.h.value()),
             static_cast<double>(hsv.s.value()),
             static_cast<double>(hsv.v.value()))
             .has_value()) {
        return 1;
    }

    const auto luma_gray = xer::to_gray(color);
    const auto luminance_gray = xer::to_luminance_gray(color);
    const auto rgb_from_gray = xer::to_rgb(luma_gray);

    if (!xer::printf(
             u8"Luma gray: y=%g\n",
             static_cast<double>(luma_gray.y.value()))
             .has_value()) {
        return 1;
    }

    if (!xer::printf(
             u8"Luminance gray: y=%g\n",
             static_cast<double>(luminance_gray.y.value()))
             .has_value()) {
        return 1;
    }

    if (!xer::printf(
             u8"RGB from gray: r=%g, g=%g, b=%g\n",
             static_cast<double>(rgb_from_gray.r.value()),
             static_cast<double>(rgb_from_gray.g.value()),
             static_cast<double>(rgb_from_gray.b.value()))
             .has_value()) {
        return 1;
    }

    const auto rgb_from_hsv = xer::to_rgb(hsv);

    if (!xer::printf(
             u8"RGB from HSV: r=%g, g=%g, b=%g\n",
             static_cast<double>(rgb_from_hsv.r.value()),
             static_cast<double>(rgb_from_hsv.g.value()),
             static_cast<double>(rgb_from_hsv.b.value()))
             .has_value()) {
        return 1;
    }

    const auto xyz = xer::to_xyz(color);

    if (!xer::printf(
             u8"XYZ: x=%g, y=%g, z=%g\n",
             static_cast<double>(xyz.x),
             static_cast<double>(xyz.y),
             static_cast<double>(xyz.z))
             .has_value()) {
        return 1;
    }

    const auto lab = xer::to_lab(xyz);

    if (!xer::printf(
             u8"Lab: l=%g, a=%g, b=%g\n",
             static_cast<double>(lab.l),
             static_cast<double>(lab.a),
             static_cast<double>(lab.b))
             .has_value()) {
        return 1;
    }

    const auto luv = xer::to_luv(xyz);

    if (!xer::printf(
             u8"Luv: l=%g, u=%g, v=%g\n",
             static_cast<double>(luv.l),
             static_cast<double>(luv.u),
             static_cast<double>(luv.v))
             .has_value()) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: color_basic
