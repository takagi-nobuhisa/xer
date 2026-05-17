// XER_EXAMPLE_BEGIN: image_flood_fill
//
// This example draws a closed outline and fills only its interior.
//
// Expected output:
// outside = ff000000
// wall = ffffffff
// inside = ff00ff00

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
    xer::image::canvas<7, 5> img;

    img.clear();

    const auto wall = xer::image::pixel(0xffu, 0xffu, 0xffu);
    const auto fill = xer::image::pixel(0x00u, 0xffu, 0x00u);

    xer::image::draw_hline(img, xer::image::point{1, 1}, 5, wall);
    xer::image::draw_hline(img, xer::image::point{1, 3}, 5, wall);
    xer::image::draw_vline(img, xer::image::point{1, 1}, 3, wall);
    xer::image::draw_vline(img, xer::image::point{5, 1}, 3, wall);

    const auto filled = xer::image::flood_fill(
        img,
        xer::image::point{3, 2},
        fill);
    if (!filled.has_value()) {
        return 1;
    }

    if (!print_pixel(u8"outside", img.get_pixel(0, 0))) {
        return 1;
    }
    if (!print_pixel(u8"wall", img.get_pixel(1, 2))) {
        return 1;
    }
    if (!print_pixel(u8"inside", img.get_pixel(3, 2))) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: image_flood_fill
