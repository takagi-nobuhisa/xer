// XER_EXAMPLE_BEGIN: image_effects
//
// This example applies mosaic and box_blur to small canvases.
//
// Expected output:
// mosaic = 050 050 228 228
// blur = 025 050 100 150 175

#include <cstddef>

#include <xer/image.h>
#include <xer/stdio.h>

namespace {

template <std::size_t Width, std::size_t Height>
auto print_red_row(
    std::u8string_view name,
    const xer::image::canvas<Width, Height>& img) -> bool
{
    if (!xer::printf(u8"%@ =", name).has_value()) {
        return false;
    }

    for (std::size_t x = 0; x < img.width(); ++x) {
        const auto value = img.get_pixel(x, 0);
        if (!xer::printf(u8" %03u", static_cast<unsigned int>(value.red()))
                 .has_value()) {
            return false;
        }
    }

    return xer::printf(u8"\n").has_value();
}

} // namespace

auto main() -> int
{
    xer::image::canvas<4, 1> mosaic_img;
    mosaic_img.set_pixel(0, 0, xer::image::pixel(0u, 0u, 0u));
    mosaic_img.set_pixel(1, 0, xer::image::pixel(100u, 0u, 0u));
    mosaic_img.set_pixel(2, 0, xer::image::pixel(200u, 0u, 0u));
    mosaic_img.set_pixel(3, 0, xer::image::pixel(255u, 0u, 0u));

    const auto mosaic_result = xer::image::mosaic(
        mosaic_img,
        xer::image::rect(xer::image::point(0, 0), xer::image::size(4, 1)),
        xer::image::size(2, 1));
    if (!mosaic_result.has_value()) {
        return 1;
    }

    if (!print_red_row(u8"mosaic", mosaic_img)) {
        return 1;
    }

    xer::image::canvas<5, 1> blur_img;
    blur_img.set_pixel(0, 0, xer::image::pixel(0u, 0u, 0u));
    blur_img.set_pixel(1, 0, xer::image::pixel(50u, 0u, 0u));
    blur_img.set_pixel(2, 0, xer::image::pixel(100u, 0u, 0u));
    blur_img.set_pixel(3, 0, xer::image::pixel(150u, 0u, 0u));
    blur_img.set_pixel(4, 0, xer::image::pixel(200u, 0u, 0u));

    const auto blur_result = xer::image::box_blur(
        blur_img,
        xer::image::rect(xer::image::point(0, 0), xer::image::size(5, 1)),
        xer::image::size(3, 1));
    if (!blur_result.has_value()) {
        return 1;
    }

    return print_red_row(u8"blur", blur_img) ? 0 : 1;
}

// XER_EXAMPLE_END: image_effects
