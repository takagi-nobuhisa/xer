/**
 * @file tests/test_bitmap_font.cpp
 * @brief Tests for XBF bitmap-font loading.
 */

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

#include <xer/assert.h>
#include <xer/error.h>
#include <xer/image.h>
#include <xer/path.h>
#include <xer/stdio.h>

namespace {

using byte_buffer = std::vector<std::byte>;

constexpr std::uint16_t xbf_version = 1;
constexpr std::uint16_t xbf_header_size = 36;
constexpr std::uint16_t half_width = 8;
constexpr std::uint16_t full_width = 16;
constexpr std::uint16_t glyph_height = 8;
constexpr std::uint32_t range_count = 2;
constexpr std::uint64_t range_table_offset = xbf_header_size;
constexpr std::uint64_t bitmap_data_offset = xbf_header_size + 24u * range_count;

void cleanup(const xer::path& filename)
{
    if (xer::is_file(filename)) {
        static_cast<void>(xer::remove(filename));
    }
}

void append_u8(byte_buffer& bytes, std::uint8_t value)
{
    bytes.push_back(static_cast<std::byte>(value));
}

void append_u16_le(byte_buffer& bytes, std::uint16_t value)
{
    append_u8(bytes, static_cast<std::uint8_t>(value & 0xffu));
    append_u8(bytes, static_cast<std::uint8_t>((value >> 8u) & 0xffu));
}

void append_u32_le(byte_buffer& bytes, std::uint32_t value)
{
    append_u8(bytes, static_cast<std::uint8_t>(value & UINT32_C(0xff)));
    append_u8(bytes, static_cast<std::uint8_t>((value >> 8u) & UINT32_C(0xff)));
    append_u8(bytes, static_cast<std::uint8_t>((value >> 16u) & UINT32_C(0xff)));
    append_u8(bytes, static_cast<std::uint8_t>((value >> 24u) & UINT32_C(0xff)));
}

void append_u64_le(byte_buffer& bytes, std::uint64_t value)
{
    append_u8(bytes, static_cast<std::uint8_t>(value & UINT64_C(0xff)));
    append_u8(bytes, static_cast<std::uint8_t>((value >> 8u) & UINT64_C(0xff)));
    append_u8(bytes, static_cast<std::uint8_t>((value >> 16u) & UINT64_C(0xff)));
    append_u8(bytes, static_cast<std::uint8_t>((value >> 24u) & UINT64_C(0xff)));
    append_u8(bytes, static_cast<std::uint8_t>((value >> 32u) & UINT64_C(0xff)));
    append_u8(bytes, static_cast<std::uint8_t>((value >> 40u) & UINT64_C(0xff)));
    append_u8(bytes, static_cast<std::uint8_t>((value >> 48u) & UINT64_C(0xff)));
    append_u8(bytes, static_cast<std::uint8_t>((value >> 56u) & UINT64_C(0xff)));
}

void append_bitmap_bytes(byte_buffer& bytes)
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

auto make_valid_xbf() -> byte_buffer
{
    byte_buffer bytes;
    bytes.reserve(164);

    append_u8(bytes, static_cast<std::uint8_t>('X'));
    append_u8(bytes, static_cast<std::uint8_t>('B'));
    append_u8(bytes, static_cast<std::uint8_t>('F'));
    append_u8(bytes, static_cast<std::uint8_t>('0'));
    append_u16_le(bytes, xbf_version);
    append_u16_le(bytes, xbf_header_size);
    append_u16_le(bytes, half_width);
    append_u16_le(bytes, full_width);
    append_u16_le(bytes, glyph_height);
    append_u16_le(bytes, 0);
    append_u32_le(bytes, range_count);
    append_u64_le(bytes, range_table_offset);
    append_u64_le(bytes, bitmap_data_offset);

    // U+003F - U+0042, half-width, bitmap offset 0.
    append_u32_le(bytes, UINT32_C(0x003f));
    append_u32_le(bytes, UINT32_C(0x0042));
    append_u8(bytes, static_cast<std::uint8_t>(xer::image::bitmap_glyph_width::half));
    for (int index = 0; index < 7; ++index) {
        append_u8(bytes, 0);
    }
    append_u64_le(bytes, 0);

    // U+3042 - U+3044, full-width, bitmap offset 32.
    append_u32_le(bytes, UINT32_C(0x3042));
    append_u32_le(bytes, UINT32_C(0x3044));
    append_u8(bytes, static_cast<std::uint8_t>(xer::image::bitmap_glyph_width::full));
    for (int index = 0; index < 7; ++index) {
        append_u8(bytes, 0);
    }
    append_u64_le(bytes, 32);

    append_bitmap_bytes(bytes);

    xer_assert_eq(bytes.size(), static_cast<std::size_t>(164));
    return bytes;
}

void write_xbf(const xer::path& filename, const byte_buffer& bytes)
{
    cleanup(filename);

    const auto written = xer::file_put_contents(
        filename,
        std::span<const std::byte>(bytes.data(), bytes.size()));
    xer_assert(written.has_value());
}

void expect_parse_error(
    const xer::path& filename,
    const byte_buffer& bytes,
    xer::parse_error_reason reason,
    std::size_t offset)
{
    write_xbf(filename, bytes);

    const auto loaded = xer::image::bitmap_font_load(filename);
    xer_assert_not(loaded.has_value());
    xer_assert_eq(loaded.error().code, xer::error_t::invalid_argument);
    xer_assert_eq(loaded.error().reason, reason);
    xer_assert_eq(loaded.error().offset, offset);
    xer_assert_eq(loaded.error().line, static_cast<std::size_t>(0));
    xer_assert_eq(loaded.error().column, static_cast<std::size_t>(0));

    cleanup(filename);
}

void test_bitmap_font_load_valid_xbf()
{
    const xer::path filename(u8"test_bitmap_font_valid.xbf");
    write_xbf(filename, make_valid_xbf());

    const auto loaded = xer::image::bitmap_font_load(filename);
    xer_assert(loaded.has_value());

    xer_assert_eq(loaded->half_width, 8);
    xer_assert_eq(loaded->full_width, 16);
    xer_assert_eq(loaded->glyph_height, 8);

    xer_assert_eq(loaded->ranges.size(), static_cast<std::size_t>(2));

    const auto& half_range = loaded->ranges[0];
    xer_assert_eq(half_range.first_code_point, U'\u003f');
    xer_assert_eq(half_range.last_code_point, U'\u0042');
    xer_assert_eq(half_range.glyph_width, xer::image::bitmap_glyph_width::half);
    xer_assert_eq(half_range.bitmap_offset, UINT64_C(0));

    const auto& full_range = loaded->ranges[1];
    xer_assert_eq(full_range.first_code_point, U'\u3042');
    xer_assert_eq(full_range.last_code_point, U'\u3044');
    xer_assert_eq(full_range.glyph_width, xer::image::bitmap_glyph_width::full);
    xer_assert_eq(full_range.bitmap_offset, UINT64_C(32));

    xer_assert_eq(loaded->bitmap.size(), static_cast<std::size_t>(80));
    xer_assert_eq(loaded->bitmap[0], static_cast<std::uint8_t>(0x3cu));
    xer_assert_eq(loaded->bitmap[31], static_cast<std::uint8_t>(0x00u));
    xer_assert_eq(loaded->bitmap[32], static_cast<std::uint8_t>(0x07u));
    xer_assert_eq(loaded->bitmap[79], static_cast<std::uint8_t>(0x00u));

    cleanup(filename);
}

void test_bitmap_font_load_missing_file_has_no_parse_detail()
{
    const xer::path filename(u8"test_bitmap_font_missing.xbf");
    cleanup(filename);

    const auto loaded = xer::image::bitmap_font_load(filename);
    xer_assert_not(loaded.has_value());
    xer_assert_eq(loaded.error().reason, xer::parse_error_reason::none);
    xer_assert_eq(loaded.error().offset, static_cast<std::size_t>(0));
    xer_assert_eq(loaded.error().line, static_cast<std::size_t>(0));
    xer_assert_eq(loaded.error().column, static_cast<std::size_t>(0));
}

void test_bitmap_font_load_rejects_invalid_magic()
{
    auto bytes = make_valid_xbf();
    bytes[0] = std::byte{0x5au};

    expect_parse_error(
        xer::path(u8"test_bitmap_font_invalid_magic.xbf"),
        bytes,
        xer::parse_error_reason::invalid_magic,
        0);
}

void test_bitmap_font_load_rejects_unsupported_version()
{
    auto bytes = make_valid_xbf();
    bytes[4] = std::byte{0x02u};
    bytes[5] = std::byte{0x00u};

    expect_parse_error(
        xer::path(u8"test_bitmap_font_unsupported_version.xbf"),
        bytes,
        xer::parse_error_reason::unsupported_version,
        4);
}

void test_bitmap_font_load_rejects_invalid_range_width_kind()
{
    auto bytes = make_valid_xbf();
    bytes[44] = std::byte{0xffu};

    expect_parse_error(
        xer::path(u8"test_bitmap_font_invalid_range.xbf"),
        bytes,
        xer::parse_error_reason::invalid_range,
        44);
}

void test_bitmap_font_load_rejects_invalid_bitmap_data_offset()
{
    auto bytes = make_valid_xbf();
    bytes[28] = std::byte{0x00u};

    expect_parse_error(
        xer::path(u8"test_bitmap_font_invalid_offset.xbf"),
        bytes,
        xer::parse_error_reason::invalid_offset,
        28);
}

void test_bitmap_font_load_rejects_truncated_bitmap_data()
{
    auto bytes = make_valid_xbf();
    bytes.pop_back();

    expect_parse_error(
        xer::path(u8"test_bitmap_font_truncated_bitmap.xbf"),
        bytes,
        xer::parse_error_reason::truncated_input,
        116);
}

} // namespace

auto main() -> int
{
    test_bitmap_font_load_valid_xbf();
    test_bitmap_font_load_missing_file_has_no_parse_detail();
    test_bitmap_font_load_rejects_invalid_magic();
    test_bitmap_font_load_rejects_unsupported_version();
    test_bitmap_font_load_rejects_invalid_range_width_kind();
    test_bitmap_font_load_rejects_invalid_bitmap_data_offset();
    test_bitmap_font_load_rejects_truncated_bitmap_data();

    return 0;
}
