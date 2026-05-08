// XER_EXAMPLE_BEGIN: tk_photo_basic
//
// This example creates a Tk photo image, writes RGBA pixel data through
// XER's Tk photo block helpers, and shows it in a small Tk window.
//
// Expected output:
// Opening Tk photo window...
// Tk photo window closed.

// XER_TEST_FEATURES: tcltk

#include <array>
#include <cstddef>
#include <expected>

#include <xer/error.h>
#include <xer/stdio.h>
#include <xer/tk.h>

auto main() -> int
{
    const auto result = xer::tk::main([](xer::tk::interpreter& interp) -> xer::result<void> {
        if (!xer::puts(u8"Opening Tk photo window...").has_value()) {
            return std::unexpected(xer::make_error(xer::error_t::io_error));
        }

        const auto setup = xer::tk::eval(
            interp,
            u8"wm title . {XER Tk Photo Example}\n"
            u8"image create photo xer_example_photo -width 64 -height 64\n"
            u8"label .image -image xer_example_photo -padx 24 -pady 16\n"
            u8"pack .image\n"
            u8"after 500 {destroy .}\n");
        if (!setup.has_value()) {
            return std::unexpected(xer::make_error(xer::error_t::runtime_error));
        }

        const auto photo = xer::tk::find_photo(interp, u8"xer_example_photo");
        if (!photo.has_value()) {
            return std::unexpected(xer::make_error(xer::error_t::runtime_error));
        }

        std::array<unsigned char, 16> pixels = {
            255, 0, 0, 255,
            0, 255, 0, 255,
            0, 0, 255, 255,
            255, 255, 255, 255,
        };

        xer::tk::photo_image_block block{};
        block.pixelPtr = pixels.data();
        block.width = 2;
        block.height = 2;
        block.pitch = 8;
        block.pixelSize = 4;
        block.offset[0] = 0;
        block.offset[1] = 1;
        block.offset[2] = 2;
        block.offset[3] = 3;

        const auto put = xer::tk::photo_put_zoomed_block(
            interp,
            *photo,
            &block,
            0,
            0,
            2,
            2,
            32,
            32,
            1,
            1,
            xer::tk::photo_composite_set);
        if (!put.has_value()) {
            return std::unexpected(xer::make_error(xer::error_t::runtime_error));
        }

        return {};
    });

    if (!result.has_value()) {
        return 1;
    }

    if (!xer::puts(u8"Tk photo window closed.").has_value()) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: tk_photo_basic
