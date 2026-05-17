// XER_EXAMPLE_BEGIN: image_curves
//
// This example draws anti-aliased curves, ellipses, and arcs on one canvas.
//
// Expected output:
// circle_top = ffffffff
// ellipse_left = ff00ff00
// arc_right = ffff8000
// thick_arc = ff0080ff
// filled_center = ffff00ff

#include <xer/image.h>
#include <xer/stdio.h>

namespace {

auto print_pixel(std::u8string_view name, xer::image::pixel value) -> bool
{
    return xer::printf(
               u8"%@ = %08x\n",
               name,
               static_cast<unsigned int>(value.argb))
        .has_value();
}

} // namespace

auto main() -> int
{
    xer::image::canvas<48, 20> img;

    img.clear();

    const auto circle = xer::image::draw_circle_aa(
        img,
        xer::image::pointf{6.0f, 6.0f},
        4.0f,
        xer::image::pixel(0xffu, 0xffu, 0xffu));
    if (!circle.has_value()) {
        return 1;
    }

    const auto ellipse = xer::image::draw_ellipse_aa(
        img,
        xer::image::pointf{18.0f, 6.0f},
        5.0f,
        3.0f,
        xer::image::pixel(0x00u, 0xffu, 0x00u));
    if (!ellipse.has_value()) {
        return 1;
    }

    const auto arc = xer::image::draw_arc_aa(
        img,
        xer::image::pointf{31.0f, 6.0f},
        4.0f,
        0.0f,
        1.57079632679f,
        xer::image::pixel(0xffu, 0x80u, 0x00u));
    if (!arc.has_value()) {
        return 1;
    }

    const auto ellipse_arc = xer::image::draw_ellipse_arc_aa(
        img,
        xer::image::pointf{42.0f, 6.0f},
        4.0f,
        3.0f,
        3.14159265359f,
        -1.57079632679f,
        3.0f,
        xer::image::pixel(0x00u, 0x80u, 0xffu));
    if (!ellipse_arc.has_value()) {
        return 1;
    }

    const auto filled = xer::image::fill_ellipse_aa(
        img,
        xer::image::pointf{24.0f, 15.0f},
        6.0f,
        3.0f,
        xer::image::pixel(0xffu, 0x00u, 0xffu));
    if (!filled.has_value()) {
        return 1;
    }

    if (!print_pixel(u8"circle_top", img.get_pixel(6, 2))) {
        return 1;
    }
    if (!print_pixel(u8"ellipse_left", img.get_pixel(13, 6))) {
        return 1;
    }
    if (!print_pixel(u8"arc_right", img.get_pixel(35, 6))) {
        return 1;
    }
    if (!print_pixel(u8"thick_arc", img.get_pixel(38, 6))) {
        return 1;
    }
    if (!print_pixel(u8"filled_center", img.get_pixel(24, 15))) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: image_curves
