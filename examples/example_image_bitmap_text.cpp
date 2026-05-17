// XER_EXAMPLE_BEGIN: image_bitmap_text
//
// This example loads a tiny XBF bitmap font, draws text on a canvas,
// and reads back representative pixels.
//
// A real application would typically create the XBF file in advance with
// php/convert_bdf_font.php. This example writes a tiny XBF file at runtime so
// that it remains independently runnable.
//
// Expected output:
// font = half=8 full=16 height=8
// p00 = 00000000
// p30 = ffffffff
// p90 = ffffffff

#include <array>
#include <cstddef>
#include <span>
#include <string_view>

#include <xer/image.h>
#include <xer/stdio.h>

namespace {

[[nodiscard]] auto sample_xbf_bytes() -> std::array<std::byte, 76>
{
    return {
        // XBF header.
        std::byte{0x58}, std::byte{0x42}, std::byte{0x46}, std::byte{0x30}, // "XBF0"
        std::byte{0x01}, std::byte{0x00}, // version = 1
        std::byte{0x24}, std::byte{0x00}, // header size = 36
        std::byte{0x08}, std::byte{0x00}, // half-width = 8
        std::byte{0x10}, std::byte{0x00}, // full-width = 16
        std::byte{0x08}, std::byte{0x00}, // glyph height = 8
        std::byte{0x00}, std::byte{0x00}, // reserved
        std::byte{0x01}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, // range count = 1
        std::byte{0x24}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00},
        std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, // range table offset = 36
        std::byte{0x3c}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00},
        std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, // bitmap data offset = 60

        // Range: U+0041 - U+0042, half-width, bitmap offset 0.
        std::byte{0x41}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00},
        std::byte{0x42}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00},
        std::byte{0x00},
        std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00},
        std::byte{0x00}, std::byte{0x00}, std::byte{0x00},
        std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00},
        std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00},

        // U+0041 'A'.
        std::byte{0x18}, std::byte{0x24}, std::byte{0x42}, std::byte{0x7e},
        std::byte{0x42}, std::byte{0x42}, std::byte{0x42}, std::byte{0x00},

        // U+0042 'B'.
        std::byte{0x7c}, std::byte{0x42}, std::byte{0x42}, std::byte{0x7c},
        std::byte{0x42}, std::byte{0x42}, std::byte{0x7c}, std::byte{0x00},
    };
}

auto print_pixel(std::u8string_view name, xer::image::pixel value) -> bool
{
    return xer::printf(
               u8"%@ = %08x\n",
               name,
               static_cast<unsigned int>(value.argb))
        .has_value();
}

auto cleanup(const xer::path& filename) -> void
{
    if (xer::is_file(filename)) {
        static_cast<void>(xer::remove(filename));
    }
}

} // namespace

auto main() -> int
{
    const xer::path font_file(u8"example_bitmap_font.xbf");
    cleanup(font_file);

    const auto xbf = sample_xbf_bytes();
    const auto written = xer::file_put_contents(
        font_file,
        std::span<const std::byte>(xbf.data(), xbf.size()));
    if (!written.has_value()) {
        return 1;
    }

    const auto font_result = xer::image::bitmap_font_load(font_file);
    if (!font_result.has_value()) {
        cleanup(font_file);
        return 1;
    }

    const auto& font = *font_result;
    if (!xer::printf(
             u8"font = half=%d full=%d height=%d\n",
             font.half_width,
             font.full_width,
             font.glyph_height)
             .has_value()) {
        cleanup(font_file);
        return 1;
    }

    xer::image::canvas<16, 8> img;
    const auto drawn = xer::image::draw_text(
        img,
        xer::image::point{0, 0},
        u8"AB",
        font,
        xer::image::pixel(0xffu, 0xffu, 0xffu));
    if (!drawn.has_value()) {
        cleanup(font_file);
        return 1;
    }

    if (!print_pixel(u8"p00", img.get_pixel(0, 0))) {
        cleanup(font_file);
        return 1;
    }
    if (!print_pixel(u8"p30", img.get_pixel(3, 0))) {
        cleanup(font_file);
        return 1;
    }
    if (!print_pixel(u8"p90", img.get_pixel(9, 0))) {
        cleanup(font_file);
        return 1;
    }

    if (!xer::remove(font_file).has_value()) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: image_bitmap_text
