// XER_EXAMPLE_BEGIN: image_circle
//
// This example draws a one-pixel circle outline and a filled circle.
//
// Expected output:
// outline_edge = ffffffff
// outline_center = ff000000
// fill_center = ff00ff00
// fill_outside = ff000000

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
    xer::image::canvas<12, 5> img;

    img.clear();

    const auto outline = xer::image::draw_circle(
        img,
        xer::image::point{2, 2},
        2,
        xer::image::pixel(0xffu, 0xffu, 0xffu));
    if (!outline.has_value()) {
        return 1;
    }

    const auto filled = xer::image::fill_circle(
        img,
        xer::image::point{9, 2},
        2,
        xer::image::pixel(0x00u, 0xffu, 0x00u));
    if (!filled.has_value()) {
        return 1;
    }

    if (!print_pixel(u8"outline_edge", img.get_pixel(2, 0))) {
        return 1;
    }
    if (!print_pixel(u8"outline_center", img.get_pixel(2, 2))) {
        return 1;
    }
    if (!print_pixel(u8"fill_center", img.get_pixel(9, 2))) {
        return 1;
    }
    if (!print_pixel(u8"fill_outside", img.get_pixel(7, 0))) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: image_circle
