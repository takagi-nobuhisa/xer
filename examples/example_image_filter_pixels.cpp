// XER_EXAMPLE_BEGIN: image_filter_pixels
//
// This example applies pixel-wise filters to a canvas.
//
// Expected output:
// gray0 = ff4c4c4c
// gray1 = ff969696
// gray2 = ff1d1d1d
// gray3 = ffffffff
// binary0 = ff000000
// binary1 = ff000000
// binary2 = ffffffff
// binary3 = ffffffff
// error = x=1 y=0 count=1
// err0 = ff030303
// err1 = ff140000
// err2 = ff090909

#include <xer/image.h>
#include <xer/stdio.h>

#include <cstdint>
#include <stdexcept>
#include <string_view>

namespace {

[[nodiscard]] auto luma(xer::image::pixel value) -> std::uint8_t
{
    const auto red = static_cast<unsigned int>(value.red());
    const auto green = static_cast<unsigned int>(value.green());
    const auto blue = static_cast<unsigned int>(value.blue());

    return static_cast<std::uint8_t>((red * 299u + green * 587u + blue * 114u + 500u) / 1000u);
}

[[nodiscard]] auto to_gray_pixel(xer::image::pixel value) -> xer::image::pixel
{
    const auto gray = luma(value);

    return xer::image::pixel(value.alpha(), gray, gray, gray);
}

[[nodiscard]] auto to_binary_pixel(xer::image::pixel value) -> xer::image::pixel
{
    return luma(value) >= 128u
        ? xer::image::pixel(value.alpha(), 0xffu, 0xffu, 0xffu)
        : xer::image::pixel(value.alpha(), 0x00u, 0x00u, 0x00u);
}

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
    xer::image::canvas<4, 2> img;

    img.set_pixel_unchecked(0, 0, xer::image::pixel(0xffu, 0x00u, 0x00u));
    img.set_pixel_unchecked(1, 0, xer::image::pixel(0x00u, 0xffu, 0x00u));
    img.set_pixel_unchecked(2, 0, xer::image::pixel(0x00u, 0x00u, 0xffu));
    img.set_pixel_unchecked(3, 0, xer::image::pixel(0xffu, 0xffu, 0xffu));

    img.set_pixel_unchecked(0, 1, xer::image::pixel(0x00u, 0x00u, 0x00u));
    img.set_pixel_unchecked(1, 1, xer::image::pixel(0x7fu, 0x7fu, 0x7fu));
    img.set_pixel_unchecked(2, 1, xer::image::pixel(0x80u, 0x80u, 0x80u));
    img.set_pixel_unchecked(3, 1, xer::image::pixel(0xffu, 0xffu, 0xffu));

    if (!xer::image::filter_pixels(
             img,
             xer::image::rect(xer::image::point(0, 0), xer::image::size(4, 1)),
             to_gray_pixel)
             .has_value()) {
        return 1;
    }

    if (!xer::image::filter_pixels(
             img,
             xer::image::rect(xer::image::point(0, 1), xer::image::size(4, 1)),
             to_binary_pixel)
             .has_value()) {
        return 1;
    }

    for (std::size_t x = 0; x < 4; ++x) {
        const auto name = static_cast<char8_t>(u8'0' + static_cast<char8_t>(x));
        char8_t buffer[] = {u8'g', u8'r', u8'a', u8'y', name, u8'\0'};
        if (!print_pixel(buffer, img.get_pixel(x, 0))) {
            return 1;
        }
    }

    for (std::size_t x = 0; x < 4; ++x) {
        const auto name = static_cast<char8_t>(u8'0' + static_cast<char8_t>(x));
        char8_t buffer[] = {u8'b', u8'i', u8'n', u8'a', u8'r', u8'y', name, u8'\0'};
        if (!print_pixel(buffer, img.get_pixel(x, 1))) {
            return 1;
        }
    }

    xer::image::canvas<3, 1> err_img;

    err_img.set_pixel_unchecked(0, 0, xer::image::pixel(0x0au, 0x00u, 0x00u));
    err_img.set_pixel_unchecked(1, 0, xer::image::pixel(0x14u, 0x00u, 0x00u));
    err_img.set_pixel_unchecked(2, 0, xer::image::pixel(0x1eu, 0x00u, 0x00u));

    const auto result = xer::image::filter_pixels(
        err_img,
        xer::image::rect(xer::image::point(0, 0), xer::image::size(3, 1)),
        [](xer::image::pixel value) -> xer::image::pixel {
            if (value.red() == 0x14u) {
                throw std::runtime_error("sample filter error");
            }

            return to_gray_pixel(value);
        });

    if (result.has_value()) {
        return 1;
    }

    const auto& error = result.error();
    if (!xer::printf(
             u8"error = x=%d y=%d count=%zu\n",
             error.first_error_position.x,
             error.first_error_position.y,
             error.error_count)
             .has_value()) {
        return 1;
    }

    for (std::size_t x = 0; x < 3; ++x) {
        const auto name = static_cast<char8_t>(u8'0' + static_cast<char8_t>(x));
        char8_t buffer[] = {u8'e', u8'r', u8'r', name, u8'\0'};
        if (!print_pixel(buffer, err_img.get_pixel(x, 0))) {
            return 1;
        }
    }

    return 0;
}

// XER_EXAMPLE_END: image_filter_pixels
