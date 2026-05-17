/**
 * @file tests/test_draw_text.cpp
 * @brief Tests for bitmap-font text drawing.
 */

#include <cstdint>
#include <string>
#include <vector>

#include <xer/assert.h>
#include <xer/error.h>
#include <xer/image.h>

namespace {

using xer::image::bitmap_font;
using xer::image::bitmap_font_range;
using xer::image::bitmap_glyph_width;
using xer::image::canvas;
using xer::image::pixel;
using xer::image::point;
using xer::image::text_draw_options;

auto append_u8(std::vector<std::uint8_t>& bytes, std::uint8_t value) -> void
{
    bytes.push_back(value);
}

auto append_bitmap_bytes(std::vector<std::uint8_t>& bytes) -> void
{
    // U+003F '?'
    append_u8(bytes, 0x3cu);
    append_u8(bytes, 0x42u);
    append_u8(bytes, 0x02u);
    append_u8(bytes, 0x0cu);
    append_u8(bytes, 0x10u);
    append_u8(bytes, 0x00u);
    append_u8(bytes, 0x10u);
    append_u8(bytes, 0x00u);

    // U+0040 '@'
    append_u8(bytes, 0x3cu);
    append_u8(bytes, 0x42u);
    append_u8(bytes, 0x5au);
    append_u8(bytes, 0x56u);
    append_u8(bytes, 0x5cu);
    append_u8(bytes, 0x40u);
    append_u8(bytes, 0x3cu);
    append_u8(bytes, 0x00u);

    // U+0041 'A'
    append_u8(bytes, 0x18u);
    append_u8(bytes, 0x24u);
    append_u8(bytes, 0x42u);
    append_u8(bytes, 0x7eu);
    append_u8(bytes, 0x42u);
    append_u8(bytes, 0x42u);
    append_u8(bytes, 0x42u);
    append_u8(bytes, 0x00u);

    // U+0042 'B'
    append_u8(bytes, 0x7cu);
    append_u8(bytes, 0x42u);
    append_u8(bytes, 0x42u);
    append_u8(bytes, 0x7cu);
    append_u8(bytes, 0x42u);
    append_u8(bytes, 0x42u);
    append_u8(bytes, 0x7cu);
    append_u8(bytes, 0x00u);

    // U+3042 'あ'
    append_u8(bytes, 0x07u);
    append_u8(bytes, 0xc0u);
    append_u8(bytes, 0x01u);
    append_u8(bytes, 0x00u);
    append_u8(bytes, 0x3fu);
    append_u8(bytes, 0xf0u);
    append_u8(bytes, 0x11u);
    append_u8(bytes, 0x10u);
    append_u8(bytes, 0x21u);
    append_u8(bytes, 0x20u);
    append_u8(bytes, 0x41u);
    append_u8(bytes, 0x40u);
    append_u8(bytes, 0x3eu);
    append_u8(bytes, 0x80u);
    append_u8(bytes, 0x00u);
    append_u8(bytes, 0x00u);

    // U+3043 'ぃ'
    append_u8(bytes, 0x00u);
    append_u8(bytes, 0x00u);
    append_u8(bytes, 0x03u);
    append_u8(bytes, 0xc0u);
    append_u8(bytes, 0x04u);
    append_u8(bytes, 0x20u);
    append_u8(bytes, 0x00u);
    append_u8(bytes, 0x20u);
    append_u8(bytes, 0x00u);
    append_u8(bytes, 0x40u);
    append_u8(bytes, 0x00u);
    append_u8(bytes, 0x80u);
    append_u8(bytes, 0x01u);
    append_u8(bytes, 0x00u);
    append_u8(bytes, 0x00u);
    append_u8(bytes, 0x00u);

    // U+3044 'い'
    append_u8(bytes, 0x04u);
    append_u8(bytes, 0x10u);
    append_u8(bytes, 0x04u);
    append_u8(bytes, 0x10u);
    append_u8(bytes, 0x04u);
    append_u8(bytes, 0x10u);
    append_u8(bytes, 0x04u);
    append_u8(bytes, 0x10u);
    append_u8(bytes, 0x04u);
    append_u8(bytes, 0x20u);
    append_u8(bytes, 0x02u);
    append_u8(bytes, 0x40u);
    append_u8(bytes, 0x01u);
    append_u8(bytes, 0x80u);
    append_u8(bytes, 0x00u);
    append_u8(bytes, 0x00u);
}

[[nodiscard]] auto make_font() -> bitmap_font
{
    bitmap_font font;
    font.half_width = 8;
    font.full_width = 16;
    font.glyph_height = 8;

    font.ranges.push_back(bitmap_font_range{
        U'\u003f',
        U'\u0042',
        bitmap_glyph_width::half,
        0,
    });

    font.ranges.push_back(bitmap_font_range{
        U'\u3042',
        U'\u3044',
        bitmap_glyph_width::full,
        32,
    });

    append_bitmap_bytes(font.bitmap);
    xer_assert_eq(font.bitmap.size(), static_cast<std::size_t>(80));

    return font;
}

[[nodiscard]] auto white() -> pixel
{
    return pixel(255, 255, 255);
}

template<std::size_t Width, std::size_t Height, class Policy>
auto assert_is_white(
    const canvas<Width, Height, Policy>& img,
    std::size_t x,
    std::size_t y) -> void
{
    xer_assert_eq(img.get_pixel(x, y).argb, 0xffffffffu);
}

template<std::size_t Width, std::size_t Height, class Policy>
auto assert_is_transparent_black(
    const canvas<Width, Height, Policy>& img,
    std::size_t x,
    std::size_t y) -> void
{
    xer_assert_eq(img.get_pixel(x, y).argb, 0x00000000u);
}

auto test_draw_text_draws_half_full_and_line_feed() -> void
{
    canvas<64, 32> img;
    const auto font = make_font();

    const auto result = xer::image::draw_text(
        img,
        point{0, 0},
        u8"Aあ\nB",
        font,
        white());

    xer_assert(result.has_value());

    // A, row 0: 00011000
    assert_is_white(img, 3, 0);
    assert_is_white(img, 4, 0);
    assert_is_transparent_black(img, 2, 0);

    // あ, row 0: 0000011111000000, placed after the 8-pixel A cell.
    assert_is_white(img, 13, 0);
    assert_is_white(img, 17, 0);
    assert_is_transparent_black(img, 12, 0);

    // B starts on the next line.
    assert_is_white(img, 1, 8);
    assert_is_white(img, 5, 8);
    assert_is_transparent_black(img, 0, 8);
}

auto test_draw_text_applies_letter_and_line_spacing() -> void
{
    canvas<64, 32> img;
    const auto font = make_font();
    const text_draw_options options{
        .letter_spacing = 2,
        .line_spacing = 3,
    };

    const auto result = xer::image::draw_text(
        img,
        0,
        0,
        u8"AB\nA",
        font,
        white(),
        options);

    xer_assert(result.has_value());

    // A starts at x = 0.
    assert_is_white(img, 3, 0);

    // B starts at x = 8 + 2.
    assert_is_white(img, 11, 0);
    assert_is_white(img, 15, 0);

    // The second A starts at y = 8 + 3.
    assert_is_white(img, 3, 11);
    assert_is_transparent_black(img, 3, 8);
}

auto test_draw_text_handles_cr_lf_and_crlf() -> void
{
    canvas<32, 40> img;
    const auto font = make_font();

    const auto result = xer::image::draw_text(
        img,
        0,
        0,
        u8"A\r\nB\rA\nB",
        font,
        white());

    xer_assert(result.has_value());

    assert_is_white(img, 3, 0);
    assert_is_white(img, 1, 8);
    assert_is_white(img, 3, 16);
    assert_is_white(img, 1, 24);
}

auto test_draw_text_skips_missing_glyph_without_advance() -> void
{
    canvas<32, 16> img;
    const auto font = make_font();

    const auto result = xer::image::draw_text(
        img,
        0,
        0,
        u8"ACB",
        font,
        white());

    xer_assert(result.has_value());

    // A at x = 0.
    assert_is_white(img, 3, 0);

    // C is missing and does not advance; B therefore starts at x = 8.
    assert_is_white(img, 9, 0);
    assert_is_white(img, 13, 0);
    assert_is_transparent_black(img, 17, 0);
}

auto test_draw_text_clips_negative_origin() -> void
{
    canvas<16, 16> img;
    const auto font = make_font();

    const auto result = xer::image::draw_text(
        img,
        -3,
        -1,
        u8"A",
        font,
        white());

    xer_assert(result.has_value());

    // A row 1 is 00100100. With origin (-3, -1), only the right set bit appears at (2, 0).
    assert_is_white(img, 2, 0);
    assert_is_transparent_black(img, 1, 0);
}

auto test_draw_text_rejects_invalid_utf8() -> void
{
    canvas<16, 16> img;
    const auto font = make_font();

    std::u8string text;
    text.push_back(static_cast<char8_t>(0xe3u));

    const auto result = xer::image::draw_text(
        img,
        0,
        0,
        text,
        font,
        white());

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::encoding_error);
}

auto test_draw_text_rejects_invalid_font_metrics() -> void
{
    canvas<16, 16> img;
    auto font = make_font();
    font.half_width = 0;

    const auto result = xer::image::draw_text(
        img,
        0,
        0,
        u8"A",
        font,
        white());

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

auto test_draw_text_rejects_invalid_bitmap_span() -> void
{
    canvas<16, 16> img;
    auto font = make_font();
    font.bitmap.pop_back();

    const auto result = xer::image::draw_text(
        img,
        0,
        0,
        u8"い",
        font,
        white());

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

} // namespace

auto main() -> int
{
    test_draw_text_draws_half_full_and_line_feed();
    test_draw_text_applies_letter_and_line_spacing();
    test_draw_text_handles_cr_lf_and_crlf();
    test_draw_text_skips_missing_glyph_without_advance();
    test_draw_text_clips_negative_origin();
    test_draw_text_rejects_invalid_utf8();
    test_draw_text_rejects_invalid_font_metrics();
    test_draw_text_rejects_invalid_bitmap_span();

    return 0;
}
