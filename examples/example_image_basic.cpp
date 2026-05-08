// XER_EXAMPLE_BEGIN: image_basic
//
// This example creates a fixed-size framebuffer image, draws clipped shapes,
// and reads logical pixels back from the framebuffer.
//
// Expected output:
// p00 = ffff0000
// p11 = ff000000
// p22 = ff00ff00
// p33 = ff0000ff

#include <xer/image.h>
#include <xer/stdio.h>

namespace {

auto print_pixel(std::u8string_view name, xer::pixel value) -> bool
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
    xer::image<4, 4> img;

    img.clear();

    // Drawing functions clip their requested area to the framebuffer boundary.
    xer::draw_hline(img, -2, 0, 4, xer::pixel(0xffu, 0x00u, 0x00u));
    xer::draw_vline(img, 2, -1, 4, xer::pixel(0x00u, 0xffu, 0x00u));
    xer::fill_rect(img, 3, 2, 4, 4, xer::pixel(0x00u, 0x00u, 0xffu));

    if (!print_pixel(u8"p00", img.get_pixel(0, 0))) {
        return 1;
    }
    if (!print_pixel(u8"p11", img.get_pixel(1, 1))) {
        return 1;
    }
    if (!print_pixel(u8"p22", img.get_pixel(2, 2))) {
        return 1;
    }
    if (!print_pixel(u8"p33", img.get_pixel(3, 3))) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: image_basic
