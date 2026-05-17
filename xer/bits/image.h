/**
 * @file xer/bits/image.h
 * @brief Image namespace and framebuffer implementation.
 */

#pragma once

#ifndef XER_BITS_IMAGE_H_INCLUDED_
#define XER_BITS_IMAGE_H_INCLUDED_

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <limits>
#include <string_view>
#include <type_traits>
#include <vector>

#include <xer/bits/advanced_encoding.h>
#include <xer/bits/error.h>
#include <xer/bits/file_contents.h>
#include <xer/parse.h>
#include <xer/path.h>

namespace xer::image {

/**
 * @brief Integer pixel coordinate.
 */
struct point {
    int x = 0;
    int y = 0;

    constexpr point() noexcept = default;

    constexpr point(int x_, int y_) noexcept
        : x(x_),
          y(y_)
    {
    }

    [[nodiscard]] friend constexpr auto operator==(
        point lhs,
        point rhs) noexcept -> bool = default;
};

/**
 * @brief Floating-point pixel coordinate.
 */
struct pointf {
    float x = 0.0f;
    float y = 0.0f;

    constexpr pointf() noexcept = default;

    constexpr pointf(float x_, float y_) noexcept
        : x(x_),
          y(y_)
    {
    }

    [[nodiscard]] friend constexpr auto operator==(
        pointf lhs,
        pointf rhs) noexcept -> bool = default;
};

/**
 * @brief Integer pixel extent.
 */
struct size {
    int width = 0;
    int height = 0;

    constexpr size() noexcept = default;

    constexpr size(int width_, int height_) noexcept
        : width(width_),
          height(height_)
    {
    }

    [[nodiscard]] friend constexpr auto operator==(
        size lhs,
        size rhs) noexcept -> bool = default;
};

/**
 * @brief Floating-point pixel extent.
 */
struct sizef {
    float width = 0.0f;
    float height = 0.0f;

    constexpr sizef() noexcept = default;

    constexpr sizef(float width_, float height_) noexcept
        : width(width_),
          height(height_)
    {
    }

    [[nodiscard]] friend constexpr auto operator==(
        sizef lhs,
        sizef rhs) noexcept -> bool = default;
};

/**
 * @brief Integer pixel rectangle.
 */
struct rect {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;

    constexpr rect() noexcept = default;

    constexpr rect(int x_, int y_, int width_, int height_) noexcept
        : x(x_),
          y(y_),
          width(width_),
          height(height_)
    {
    }

    constexpr rect(const point& origin, const size& extent) noexcept
        : x(origin.x),
          y(origin.y),
          width(extent.width),
          height(extent.height)
    {
    }

    [[nodiscard]] friend constexpr auto operator==(
        rect lhs,
        rect rhs) noexcept -> bool = default;
};

/**
 * @brief Floating-point pixel rectangle.
 */
struct rectf {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;

    constexpr rectf() noexcept = default;

    constexpr rectf(float x_, float y_, float width_, float height_) noexcept
        : x(x_),
          y(y_),
          width(width_),
          height(height_)
    {
    }

    constexpr rectf(const pointf& origin, const sizef& extent) noexcept
        : x(origin.x),
          y(origin.y),
          width(extent.width),
          height(extent.height)
    {
    }

    [[nodiscard]] friend constexpr auto operator==(
        rectf lhs,
        rectf rhs) noexcept -> bool = default;
};

/**
 * @brief Error detail returned by filter_pixels when one or more pixels fail.
 *
 * `first_error_position` is the first pixel where the user-supplied filter
 * threw an exception, expressed in canvas coordinates. `error_count` is the
 * total number of pixels whose filter call failed.
 */
struct filter_pixels_error_detail {
    point first_error_position{};
    std::size_t error_count = 0;
};

/**
 * @brief Width kind of glyph cells in a bitmap font range.
 */
enum class bitmap_glyph_width : std::uint8_t {
    half,
    full,
};

/**
 * @brief Continuous Unicode code point range stored in a bitmap font.
 */
struct bitmap_font_range {
    char32_t first_code_point{};
    char32_t last_code_point{};
    bitmap_glyph_width glyph_width = bitmap_glyph_width::half;
    std::uint64_t bitmap_offset = 0;
};

/**
 * @brief Runtime representation of a loaded monospaced bitmap font.
 */
struct bitmap_font {
    int half_width = 0;
    int full_width = 0;
    int glyph_height = 0;
    std::vector<bitmap_font_range> ranges{};
    std::vector<std::uint8_t> bitmap{};
};

/**
 * @brief Per-call spacing options for bitmap text drawing.
 *
 * `letter_spacing` is added after each drawn glyph cell. `line_spacing` is
 * added to the bitmap font glyph height when a line break is processed.
 * Negative spacing values are permitted and may cause overlapping glyph cells.
 */
struct text_draw_options {
    int letter_spacing = 0;
    int line_spacing = 0;
};

/**
 * @brief Logical ARGB pixel value.
 *
 * `pixel` is a logical color value. It is not necessarily identical to the
 * physical framebuffer storage element used by `canvas`.
 */
struct pixel {
    /**
     * @brief Logical ARGB value in the form `0xAARRGGBB`.
     */
    std::uint32_t argb = 0xff000000u;

    /**
     * @brief Constructs opaque black.
     */
    constexpr pixel() noexcept = default;

    /**
     * @brief Constructs from a raw logical ARGB value.
     *
     * @param value Logical ARGB value in the form `0xAARRGGBB`.
     */
    constexpr explicit pixel(std::uint32_t value) noexcept : argb(value) {}

    /**
     * @brief Constructs an opaque RGB pixel.
     *
     * Alpha is set to `0xff`.
     *
     * @param red Red component.
     * @param green Green component.
     * @param blue Blue component.
     */
    constexpr pixel(
        std::uint8_t red,
        std::uint8_t green,
        std::uint8_t blue) noexcept
        : pixel(0xffu, red, green, blue)
    {
    }

    /**
     * @brief Constructs an ARGB pixel.
     *
     * @param alpha Alpha component.
     * @param red Red component.
     * @param green Green component.
     * @param blue Blue component.
     */
    constexpr pixel(
        std::uint8_t alpha,
        std::uint8_t red,
        std::uint8_t green,
        std::uint8_t blue) noexcept
        : argb(
              (static_cast<std::uint32_t>(alpha) << 24) |
              (static_cast<std::uint32_t>(red) << 16) |
              (static_cast<std::uint32_t>(green) << 8) |
              static_cast<std::uint32_t>(blue))
    {
    }

    /**
     * @brief Returns the alpha component.
     */
    [[nodiscard]] constexpr auto alpha() const noexcept -> std::uint8_t
    {
        return static_cast<std::uint8_t>(argb >> 24);
    }

    /**
     * @brief Returns the red component.
     */
    [[nodiscard]] constexpr auto red() const noexcept -> std::uint8_t
    {
        return static_cast<std::uint8_t>(argb >> 16);
    }

    /**
     * @brief Returns the green component.
     */
    [[nodiscard]] constexpr auto green() const noexcept -> std::uint8_t
    {
        return static_cast<std::uint8_t>(argb >> 8);
    }

    /**
     * @brief Returns the blue component.
     */
    [[nodiscard]] constexpr auto blue() const noexcept -> std::uint8_t
    {
        return static_cast<std::uint8_t>(argb);
    }

    /**
     * @brief Sets the alpha component.
     */
    constexpr auto alpha(std::uint8_t value) noexcept -> void
    {
        argb = (argb & 0x00ffffffu) |
               (static_cast<std::uint32_t>(value) << 24);
    }

    /**
     * @brief Sets the red component.
     */
    constexpr auto red(std::uint8_t value) noexcept -> void
    {
        argb = (argb & 0xff00ffffu) |
               (static_cast<std::uint32_t>(value) << 16);
    }

    /**
     * @brief Sets the green component.
     */
    constexpr auto green(std::uint8_t value) noexcept -> void
    {
        argb = (argb & 0xffff00ffu) |
               (static_cast<std::uint32_t>(value) << 8);
    }

    /**
     * @brief Sets the blue component.
     */
    constexpr auto blue(std::uint8_t value) noexcept -> void
    {
        argb = (argb & 0xffffff00u) | static_cast<std::uint32_t>(value);
    }

    /**
     * @brief Compares two logical pixels.
     */
    [[nodiscard]] friend constexpr auto operator==(
        pixel lhs,
        pixel rhs) noexcept -> bool = default;
};

/**
 * @brief ARGB32 physical framebuffer policy.
 *
 * Storage values use the same logical layout as `pixel`: `0xAARRGGBB`.
 */
struct argb32_policy {
    using storage_type = std::uint32_t;

    [[nodiscard]] static constexpr auto get(
        const storage_type& value) noexcept -> pixel
    {
        return pixel(value);
    }

    [[nodiscard]] static constexpr auto encode(pixel value) noexcept
        -> storage_type
    {
        return value.argb;
    }

    static constexpr auto set(storage_type& dst, pixel value) noexcept -> void
    {
        dst = encode(value);
    }
};

/**
 * @brief RGBA32 physical framebuffer policy.
 *
 * Storage values are interpreted as `0xRRGGBBAA`.
 */
struct rgba32_policy {
    using storage_type = std::uint32_t;

    [[nodiscard]] static constexpr auto get(
        const storage_type& value) noexcept -> pixel
    {
        return pixel(
            static_cast<std::uint8_t>(value),
            static_cast<std::uint8_t>(value >> 24),
            static_cast<std::uint8_t>(value >> 16),
            static_cast<std::uint8_t>(value >> 8));
    }

    [[nodiscard]] static constexpr auto encode(pixel value) noexcept
        -> storage_type
    {
        return (static_cast<std::uint32_t>(value.red()) << 24) |
               (static_cast<std::uint32_t>(value.green()) << 16) |
               (static_cast<std::uint32_t>(value.blue()) << 8) |
               static_cast<std::uint32_t>(value.alpha());
    }

    static constexpr auto set(storage_type& dst, pixel value) noexcept -> void
    {
        dst = encode(value);
    }
};

/**
 * @brief RGB24 physical framebuffer policy.
 */
struct rgb24_policy {
    using storage_type = std::array<std::uint8_t, 3>;

    [[nodiscard]] static constexpr auto get(
        const storage_type& value) noexcept -> pixel
    {
        return pixel(value[0], value[1], value[2]);
    }

    [[nodiscard]] static constexpr auto encode(pixel value) noexcept
        -> storage_type
    {
        return {value.red(), value.green(), value.blue()};
    }

    static constexpr auto set(storage_type& dst, pixel value) noexcept -> void
    {
        dst = encode(value);
    }
};

/**
 * @brief BGR24 physical framebuffer policy.
 */
struct bgr24_policy {
    using storage_type = std::array<std::uint8_t, 3>;

    [[nodiscard]] static constexpr auto get(
        const storage_type& value) noexcept -> pixel
    {
        return pixel(value[2], value[1], value[0]);
    }

    [[nodiscard]] static constexpr auto encode(pixel value) noexcept
        -> storage_type
    {
        return {value.blue(), value.green(), value.red()};
    }

    static constexpr auto set(storage_type& dst, pixel value) noexcept -> void
    {
        dst = encode(value);
    }
};

namespace detail {

inline constexpr std::size_t xbf_header_size = 36;
inline constexpr std::size_t xbf_range_entry_size = 24;
inline constexpr std::uint16_t xbf_format_version = 1;
inline constexpr std::uint32_t xbf_unicode_max = 0x10ffffu;
inline constexpr std::uint32_t xbf_surrogate_first = 0xd800u;
inline constexpr std::uint32_t xbf_surrogate_last = 0xdfffu;

[[nodiscard]] inline auto bitmap_font_parse_error(
    xer::parse_error_reason reason,
    std::size_t offset) noexcept -> xer::error<xer::parse_error_detail>
{
    return xer::make_error<xer::parse_error_detail>(
        xer::error_t::invalid_argument,
        xer::parse_error_detail{
            .offset = offset,
            .line = 0,
            .column = 0,
            .reason = reason,
        });
}

[[nodiscard]] constexpr auto xbf_byte(
    const std::vector<std::byte>& bytes,
    std::size_t offset) noexcept -> std::uint8_t
{
    return std::to_integer<std::uint8_t>(bytes[offset]);
}

[[nodiscard]] constexpr auto xbf_read_u16_le(
    const std::vector<std::byte>& bytes,
    std::size_t offset) noexcept -> std::uint16_t
{
    return static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(xbf_byte(bytes, offset)) |
        (static_cast<std::uint16_t>(xbf_byte(bytes, offset + 1)) << 8));
}

[[nodiscard]] constexpr auto xbf_read_u32_le(
    const std::vector<std::byte>& bytes,
    std::size_t offset) noexcept -> std::uint32_t
{
    return static_cast<std::uint32_t>(
        static_cast<std::uint32_t>(xbf_byte(bytes, offset)) |
        (static_cast<std::uint32_t>(xbf_byte(bytes, offset + 1)) << 8) |
        (static_cast<std::uint32_t>(xbf_byte(bytes, offset + 2)) << 16) |
        (static_cast<std::uint32_t>(xbf_byte(bytes, offset + 3)) << 24));
}

[[nodiscard]] constexpr auto xbf_read_u64_le(
    const std::vector<std::byte>& bytes,
    std::size_t offset) noexcept -> std::uint64_t
{
    return static_cast<std::uint64_t>(
        static_cast<std::uint64_t>(xbf_byte(bytes, offset)) |
        (static_cast<std::uint64_t>(xbf_byte(bytes, offset + 1)) << 8) |
        (static_cast<std::uint64_t>(xbf_byte(bytes, offset + 2)) << 16) |
        (static_cast<std::uint64_t>(xbf_byte(bytes, offset + 3)) << 24) |
        (static_cast<std::uint64_t>(xbf_byte(bytes, offset + 4)) << 32) |
        (static_cast<std::uint64_t>(xbf_byte(bytes, offset + 5)) << 40) |
        (static_cast<std::uint64_t>(xbf_byte(bytes, offset + 6)) << 48) |
        (static_cast<std::uint64_t>(xbf_byte(bytes, offset + 7)) << 56));
}

[[nodiscard]] constexpr auto xbf_range_intersects_surrogates(
    std::uint32_t first_code_point,
    std::uint32_t last_code_point) noexcept -> bool
{
    return first_code_point <= xbf_surrogate_last &&
           last_code_point >= xbf_surrogate_first;
}

[[nodiscard]] constexpr auto xbf_size_to_offset(
    std::uint64_t value) noexcept -> std::size_t
{
    if (value > static_cast<std::uint64_t>(
                    std::numeric_limits<std::size_t>::max())) {
        return std::numeric_limits<std::size_t>::max();
    }

    return static_cast<std::size_t>(value);
}

[[nodiscard]] inline auto bitmap_font_decode_utf8_char(
    std::u8string_view text,
    std::size_t& offset) noexcept -> xer::result<char32_t>
{
    if (offset >= text.size()) {
        return std::unexpected(xer::make_error(xer::error_t::not_found));
    }

    const std::uint8_t b1 = static_cast<std::uint8_t>(text[offset]);

    if (b1 <= 0x7fu) {
        ++offset;
        return static_cast<char32_t>(b1);
    }

    if (b1 >= 0xc2u && b1 <= 0xdfu) {
        if (offset + 1 >= text.size()) {
            return std::unexpected(xer::make_error(xer::error_t::encoding_error));
        }

        const std::uint8_t b2 = static_cast<std::uint8_t>(text[offset + 1]);
        const std::uint32_t packed =
            static_cast<std::uint32_t>(b1) |
            (static_cast<std::uint32_t>(b2) << 8);

        const char32_t ch = xer::advanced::packed_utf8_to_utf32(packed);
        if (ch == xer::advanced::detail::invalid_utf32) {
            return std::unexpected(xer::make_error(xer::error_t::encoding_error));
        }

        offset += 2;
        return ch;
    }

    if (b1 >= 0xe0u && b1 <= 0xefu) {
        if (offset + 2 >= text.size()) {
            return std::unexpected(xer::make_error(xer::error_t::encoding_error));
        }

        const std::uint8_t b2 = static_cast<std::uint8_t>(text[offset + 1]);
        const std::uint8_t b3 = static_cast<std::uint8_t>(text[offset + 2]);
        const std::uint32_t packed =
            static_cast<std::uint32_t>(b1) |
            (static_cast<std::uint32_t>(b2) << 8) |
            (static_cast<std::uint32_t>(b3) << 16);

        const char32_t ch = xer::advanced::packed_utf8_to_utf32(packed);
        if (ch == xer::advanced::detail::invalid_utf32) {
            return std::unexpected(xer::make_error(xer::error_t::encoding_error));
        }

        offset += 3;
        return ch;
    }

    if (b1 >= 0xf0u && b1 <= 0xf4u) {
        if (offset + 3 >= text.size()) {
            return std::unexpected(xer::make_error(xer::error_t::encoding_error));
        }

        const std::uint8_t b2 = static_cast<std::uint8_t>(text[offset + 1]);
        const std::uint8_t b3 = static_cast<std::uint8_t>(text[offset + 2]);
        const std::uint8_t b4 = static_cast<std::uint8_t>(text[offset + 3]);
        const std::uint32_t packed =
            static_cast<std::uint32_t>(b1) |
            (static_cast<std::uint32_t>(b2) << 8) |
            (static_cast<std::uint32_t>(b3) << 16) |
            (static_cast<std::uint32_t>(b4) << 24);

        const char32_t ch = xer::advanced::packed_utf8_to_utf32(packed);
        if (ch == xer::advanced::detail::invalid_utf32) {
            return std::unexpected(xer::make_error(xer::error_t::encoding_error));
        }

        offset += 4;
        return ch;
    }

    return std::unexpected(xer::make_error(xer::error_t::encoding_error));
}

[[nodiscard]] constexpr auto bitmap_font_saturating_add(
    long long value,
    long long delta) noexcept -> long long
{
    if (delta > 0 && value > std::numeric_limits<long long>::max() - delta) {
        return std::numeric_limits<long long>::max();
    }
    if (delta < 0 && value < std::numeric_limits<long long>::min() - delta) {
        return std::numeric_limits<long long>::min();
    }
    return value + delta;
}

[[nodiscard]] inline auto bitmap_font_find_range(
    const bitmap_font& font,
    char32_t code_point) noexcept -> const bitmap_font_range*
{
    const auto it = std::lower_bound(
        font.ranges.begin(),
        font.ranges.end(),
        code_point,
        [](const bitmap_font_range& range, char32_t value) noexcept {
            return range.last_code_point < value;
        });

    if (it == font.ranges.end() || code_point < it->first_code_point) {
        return nullptr;
    }

    return &*it;
}

[[nodiscard]] constexpr auto bitmap_font_width_for_range(
    const bitmap_font& font,
    const bitmap_font_range& range) noexcept -> int
{
    if (range.glyph_width == bitmap_glyph_width::half) {
        return font.half_width;
    }
    if (range.glyph_width == bitmap_glyph_width::full) {
        return font.full_width;
    }
    return 0;
}

struct image_access;

template<std::size_t Width, std::size_t Height, class Policy>
class image_storage {
public:
    using storage_type = typename Policy::storage_type;

    static_assert(Width > 0);
    static_assert(Height > 0);

    constexpr image_storage() noexcept = default;

    [[nodiscard]] static constexpr auto width() noexcept -> std::size_t
    {
        return Width;
    }

    [[nodiscard]] static constexpr auto height() noexcept -> std::size_t
    {
        return Height;
    }

    [[nodiscard]] static constexpr auto size() noexcept -> std::size_t
    {
        return Width * Height;
    }

    [[nodiscard]] constexpr auto data() noexcept -> storage_type*
    {
        return buffer_.data();
    }

    [[nodiscard]] constexpr auto data() const noexcept -> const storage_type*
    {
        return buffer_.data();
    }

private:
    std::array<storage_type, Width * Height> buffer_{};
};

template<class Policy>
class image_storage<0, 0, Policy> {
public:
    using storage_type = typename Policy::storage_type;

    image_storage() = default;

    image_storage(std::size_t width, std::size_t height)
        : width_(width), height_(height), buffer_(width * height)
    {
    }

    [[nodiscard]] auto width() const noexcept -> std::size_t
    {
        return width_;
    }

    [[nodiscard]] auto height() const noexcept -> std::size_t
    {
        return height_;
    }

    [[nodiscard]] auto size() const noexcept -> std::size_t
    {
        return buffer_.size();
    }

    [[nodiscard]] auto data() noexcept -> storage_type*
    {
        return buffer_.data();
    }

    [[nodiscard]] auto data() const noexcept -> const storage_type*
    {
        return buffer_.data();
    }

private:
    std::size_t width_ = 0;
    std::size_t height_ = 0;
    std::vector<storage_type> buffer_{};
};

template<std::size_t Width, std::size_t Height>
inline constexpr bool valid_image_extent =
    (Width == 0 && Height == 0) || (Width > 0 && Height > 0);


[[nodiscard]] constexpr auto clamp_coverage(float coverage) noexcept -> float
{
    if (!(coverage > 0.0f)) {
        return 0.0f;
    }
    if (coverage >= 1.0f) {
        return 1.0f;
    }
    return coverage;
}

[[nodiscard]] constexpr auto round_to_u8(float value) noexcept -> std::uint8_t
{
    if (!(value > 0.0f)) {
        return 0u;
    }
    if (value >= 255.0f) {
        return 255u;
    }
    return static_cast<std::uint8_t>(value + 0.5f);
}

[[nodiscard]] constexpr auto blend_pixel(
    pixel dst,
    pixel src,
    float coverage) noexcept -> pixel
{
    const auto src_alpha =
        (static_cast<float>(src.alpha()) / 255.0f) * clamp_coverage(coverage);
    if (!(src_alpha > 0.0f)) {
        return dst;
    }
    if (src_alpha >= 1.0f) {
        return pixel(0xffu, src.red(), src.green(), src.blue());
    }

    const auto dst_alpha = static_cast<float>(dst.alpha()) / 255.0f;
    const auto inv_src_alpha = 1.0f - src_alpha;
    const auto out_alpha = src_alpha + dst_alpha * inv_src_alpha;
    if (!(out_alpha > 0.0f)) {
        return pixel(0u, 0u, 0u, 0u);
    }

    const auto blend_component = [=](
        std::uint8_t src_component,
        std::uint8_t dst_component) noexcept -> std::uint8_t {
        const auto src_value = static_cast<float>(src_component) * src_alpha;
        const auto dst_value =
            static_cast<float>(dst_component) * dst_alpha * inv_src_alpha;
        return round_to_u8((src_value + dst_value) / out_alpha);
    };

    return pixel(
        round_to_u8(out_alpha * 255.0f),
        blend_component(src.red(), dst.red()),
        blend_component(src.green(), dst.green()),
        blend_component(src.blue(), dst.blue()));
}

} // namespace detail

/**
 * @brief Loads an XBF bitmap font file.
 *
 * This function reads the compact little-endian XBF binary format used by the
 * bitmap-font facility. File I/O failures preserve the underlying file error
 * and use an empty @ref parse_error_detail. Malformed XBF input returns
 * `error_t::invalid_argument` with a populated @ref parse_error_detail whose
 * `offset` is a byte offset in the binary input.
 *
 * @param filename XBF file path to read.
 * @return Loaded bitmap font on success.
 */
[[nodiscard]] inline auto bitmap_font_load(const xer::path& filename)
    -> xer::result<bitmap_font, xer::parse_error_detail>
{
    const auto contents = xer::file_get_contents(filename);
    if (!contents.has_value()) {
        return std::unexpected(xer::make_error<xer::parse_error_detail>(
            contents.error().code,
            xer::parse_error_detail{}));
    }

    const auto& bytes = *contents;
    if (bytes.size() < detail::xbf_header_size) {
        return std::unexpected(detail::bitmap_font_parse_error(
            xer::parse_error_reason::truncated_input,
            bytes.size()));
    }

    if (detail::xbf_byte(bytes, 0) != static_cast<std::uint8_t>('X') ||
        detail::xbf_byte(bytes, 1) != static_cast<std::uint8_t>('B') ||
        detail::xbf_byte(bytes, 2) != static_cast<std::uint8_t>('F') ||
        detail::xbf_byte(bytes, 3) != static_cast<std::uint8_t>('0')) {
        return std::unexpected(detail::bitmap_font_parse_error(
            xer::parse_error_reason::invalid_magic,
            0));
    }

    const auto format_version = detail::xbf_read_u16_le(bytes, 4);
    if (format_version != detail::xbf_format_version) {
        return std::unexpected(detail::bitmap_font_parse_error(
            xer::parse_error_reason::unsupported_version,
            4));
    }

    const auto header_size = detail::xbf_read_u16_le(bytes, 6);
    if (header_size < detail::xbf_header_size) {
        return std::unexpected(detail::bitmap_font_parse_error(
            xer::parse_error_reason::invalid_header,
            6));
    }
    if (static_cast<std::size_t>(header_size) > bytes.size()) {
        return std::unexpected(detail::bitmap_font_parse_error(
            xer::parse_error_reason::truncated_input,
            bytes.size()));
    }

    const auto half_width = detail::xbf_read_u16_le(bytes, 8);
    if (half_width == 0) {
        return std::unexpected(detail::bitmap_font_parse_error(
            xer::parse_error_reason::invalid_header,
            8));
    }

    const auto full_width = detail::xbf_read_u16_le(bytes, 10);
    if (full_width == 0) {
        return std::unexpected(detail::bitmap_font_parse_error(
            xer::parse_error_reason::invalid_header,
            10));
    }

    const auto glyph_height = detail::xbf_read_u16_le(bytes, 12);
    if (glyph_height == 0) {
        return std::unexpected(detail::bitmap_font_parse_error(
            xer::parse_error_reason::invalid_header,
            12));
    }

    if (detail::xbf_read_u16_le(bytes, 14) != 0) {
        return std::unexpected(detail::bitmap_font_parse_error(
            xer::parse_error_reason::invalid_header,
            14));
    }

    const auto range_count = detail::xbf_read_u32_le(bytes, 16);
    const auto range_table_offset = detail::xbf_read_u64_le(bytes, 20);
    const auto bitmap_data_offset = detail::xbf_read_u64_le(bytes, 28);

    if (range_table_offset < static_cast<std::uint64_t>(header_size) ||
        range_table_offset > static_cast<std::uint64_t>(bytes.size())) {
        return std::unexpected(detail::bitmap_font_parse_error(
            xer::parse_error_reason::invalid_offset,
            20));
    }

    const auto range_table_size =
        static_cast<std::uint64_t>(range_count) *
        static_cast<std::uint64_t>(detail::xbf_range_entry_size);
    if (range_table_offset >
        std::numeric_limits<std::uint64_t>::max() - range_table_size) {
        return std::unexpected(detail::bitmap_font_parse_error(
            xer::parse_error_reason::truncated_input,
            detail::xbf_size_to_offset(range_table_offset)));
    }

    const auto range_table_end = range_table_offset + range_table_size;
    if (range_table_end > static_cast<std::uint64_t>(bytes.size())) {
        return std::unexpected(detail::bitmap_font_parse_error(
            xer::parse_error_reason::truncated_input,
            detail::xbf_size_to_offset(range_table_offset)));
    }

    if (bitmap_data_offset < range_table_end ||
        bitmap_data_offset > static_cast<std::uint64_t>(bytes.size())) {
        return std::unexpected(detail::bitmap_font_parse_error(
            xer::parse_error_reason::invalid_offset,
            28));
    }

    bitmap_font font{};
    font.half_width = static_cast<int>(half_width);
    font.full_width = static_cast<int>(full_width);
    font.glyph_height = static_cast<int>(glyph_height);
    font.ranges.reserve(static_cast<std::size_t>(range_count));

    const auto bitmap_data_size =
        static_cast<std::uint64_t>(bytes.size()) - bitmap_data_offset;

    bool have_previous_range = false;
    std::uint32_t previous_last_code_point = 0;

    for (std::uint32_t index = 0; index < range_count; ++index) {
        const auto entry_offset =
            range_table_offset +
            static_cast<std::uint64_t>(index) *
                static_cast<std::uint64_t>(detail::xbf_range_entry_size);
        const auto entry = static_cast<std::size_t>(entry_offset);

        const auto first_code_point = detail::xbf_read_u32_le(bytes, entry);
        const auto last_code_point = detail::xbf_read_u32_le(bytes, entry + 4);
        const auto width_kind = detail::xbf_byte(bytes, entry + 8);

        if (first_code_point > last_code_point ||
            last_code_point > detail::xbf_unicode_max ||
            detail::xbf_range_intersects_surrogates(
                first_code_point,
                last_code_point)) {
            return std::unexpected(detail::bitmap_font_parse_error(
                xer::parse_error_reason::invalid_range,
                entry));
        }

        if (width_kind != static_cast<std::uint8_t>(bitmap_glyph_width::half) &&
            width_kind != static_cast<std::uint8_t>(bitmap_glyph_width::full)) {
            return std::unexpected(detail::bitmap_font_parse_error(
                xer::parse_error_reason::invalid_range,
                entry + 8));
        }

        for (std::size_t reserved_index = 9; reserved_index < 16; ++reserved_index) {
            if (detail::xbf_byte(bytes, entry + reserved_index) != 0) {
                return std::unexpected(detail::bitmap_font_parse_error(
                    xer::parse_error_reason::invalid_range,
                    entry + reserved_index));
            }
        }

        if (have_previous_range &&
            first_code_point <= previous_last_code_point) {
            return std::unexpected(detail::bitmap_font_parse_error(
                xer::parse_error_reason::invalid_range,
                entry));
        }

        const auto bitmap_offset = detail::xbf_read_u64_le(bytes, entry + 16);
        if (bitmap_offset > bitmap_data_size) {
            return std::unexpected(detail::bitmap_font_parse_error(
                xer::parse_error_reason::invalid_offset,
                entry + 16));
        }

        const auto glyph_width =
            width_kind == static_cast<std::uint8_t>(bitmap_glyph_width::half)
                ? static_cast<std::uint64_t>(half_width)
                : static_cast<std::uint64_t>(full_width);
        const auto bytes_per_row = (glyph_width + 7u) / 8u;
        const auto bytes_per_glyph =
            bytes_per_row * static_cast<std::uint64_t>(glyph_height);
        const auto glyph_count =
            static_cast<std::uint64_t>(last_code_point) -
            static_cast<std::uint64_t>(first_code_point) + 1u;

        if (bytes_per_glyph != 0 &&
            glyph_count >
                std::numeric_limits<std::uint64_t>::max() / bytes_per_glyph) {
            return std::unexpected(detail::bitmap_font_parse_error(
                xer::parse_error_reason::truncated_input,
                detail::xbf_size_to_offset(bitmap_data_offset + bitmap_offset)));
        }

        const auto required_bitmap_size = glyph_count * bytes_per_glyph;
        if (required_bitmap_size > bitmap_data_size - bitmap_offset) {
            return std::unexpected(detail::bitmap_font_parse_error(
                xer::parse_error_reason::truncated_input,
                detail::xbf_size_to_offset(bitmap_data_offset + bitmap_offset)));
        }

        font.ranges.push_back(bitmap_font_range{
            .first_code_point = static_cast<char32_t>(first_code_point),
            .last_code_point = static_cast<char32_t>(last_code_point),
            .glyph_width = static_cast<bitmap_glyph_width>(width_kind),
            .bitmap_offset = bitmap_offset,
        });

        have_previous_range = true;
        previous_last_code_point = last_code_point;
    }

    const auto bitmap_start = static_cast<std::size_t>(bitmap_data_offset);
    font.bitmap.reserve(bytes.size() - bitmap_start);
    for (std::size_t index = bitmap_start; index < bytes.size(); ++index) {
        font.bitmap.push_back(detail::xbf_byte(bytes, index));
    }

    return font;
}

/**
 * @brief Fixed-size or dynamic-size drawing canvas.
 *
 * `canvas<Width, Height, Policy>` stores physical framebuffer elements whose
 * format is controlled by `Policy`. Public pixel operations use the logical
 * `pixel` type.
 *
 * `canvas<0, 0, Policy>` is the dynamic-size specialization.
 */
template<
    std::size_t Width,
    std::size_t Height,
    class Policy = argb32_policy>
class canvas {
    static_assert(
        detail::valid_image_extent<Width, Height>,
        "canvas dimensions must both be fixed or both be zero");

public:
    using policy_type = Policy;
    using storage_type = typename policy_type::storage_type;
    using pixel_type = pixel;

    /**
     * @brief Constructs a fixed-size canvas, or an empty dynamic-size canvas.
     */
    canvas() = default;

    /**
     * @brief Constructs a dynamic-size canvas.
     */
    canvas(std::size_t width, std::size_t height)
        requires(Width == 0 && Height == 0)
        : storage_(width, height)
    {
    }

    /**
     * @brief Returns the canvas width in pixels.
     */
    [[nodiscard]] auto width() const noexcept -> std::size_t
    {
        return storage_.width();
    }

    /**
     * @brief Returns the canvas height in pixels.
     */
    [[nodiscard]] auto height() const noexcept -> std::size_t
    {
        return storage_.height();
    }

    /**
     * @brief Returns the number of logical pixels.
     */
    [[nodiscard]] auto size() const noexcept -> std::size_t
    {
        return storage_.size();
    }

    /**
     * @brief Returns whether this canvas has no drawable area.
     */
    [[nodiscard]] auto empty() const noexcept -> bool
    {
        return width() == 0 || height() == 0;
    }

    /**
     * @brief Returns whether signed coordinates are inside the canvas.
     */
    [[nodiscard]] auto contains(int x, int y) const noexcept -> bool
    {
        return x >= 0 && y >= 0 &&
               static_cast<std::size_t>(x) < width() &&
               static_cast<std::size_t>(y) < height();
    }

    /**
     * @brief Returns whether a point is inside the canvas.
     */
    [[nodiscard]] auto contains(const point& p) const noexcept -> bool
    {
        return contains(p.x, p.y);
    }

    /**
     * @brief Returns the logical pixel at the given coordinates.
     *
     * The coordinates must be inside the image.
     */
    [[nodiscard]] auto get_pixel(std::size_t x, std::size_t y) const noexcept
        -> pixel
    {
        return policy_type::get(storage_.data()[offset(x, y)]);
    }

    /**
     * @brief Returns the logical pixel at the given point.
     *
     * The point must be inside the image.
     */
    [[nodiscard]] auto get_pixel(const point& p) const noexcept -> pixel
    {
        return get_pixel(
            static_cast<std::size_t>(p.x),
            static_cast<std::size_t>(p.y));
    }

    /**
     * @brief Sets a logical pixel when the coordinates are inside the image.
     *
     * If the coordinates are outside the image boundary, this function does
     * nothing. Use `set_pixel_unchecked` only when the caller has already
     * guaranteed that the coordinates are inside the framebuffer.
     */
    auto set_pixel(int x, int y, pixel value) noexcept -> void
    {
        if (!contains(x, y)) {
            return;
        }
        set_pixel_unchecked(
            static_cast<std::size_t>(x),
            static_cast<std::size_t>(y),
            value);
    }

    /**
     * @brief Sets a logical pixel when the point is inside the image.
     */
    auto set_pixel(const point& p, pixel value) noexcept -> void
    {
        set_pixel(p.x, p.y, value);
    }

    /**
     * @brief Blends a logical pixel when the coordinates are inside the image.
     *
     * `coverage` is clamped to the range `[0.0f, 1.0f]`. A coverage value of
     * `0.0f` leaves the destination unchanged. A coverage value of `1.0f`
     * applies the source pixel alpha normally.
     */
    auto set_pixel(int x, int y, pixel value, float coverage) noexcept -> void
    {
        if (!contains(x, y)) {
            return;
        }
        set_pixel_unchecked(
            static_cast<std::size_t>(x),
            static_cast<std::size_t>(y),
            value,
            coverage);
    }

    /**
     * @brief Blends a logical pixel when the point is inside the image.
     */
    auto set_pixel(const point& p, pixel value, float coverage) noexcept -> void
    {
        set_pixel(p.x, p.y, value, coverage);
    }

    /**
     * @brief Sets a logical pixel without checking the image boundary.
     *
     * The caller must guarantee `x < width()` and `y < height()`. This
     * function is intended for code that performs clipping or bounds checking
     * outside the inner drawing loop.
     */
    auto set_pixel_unchecked(
        std::size_t x,
        std::size_t y,
        pixel value) noexcept -> void
    {
        storage_.data()[offset(x, y)] = policy_type::encode(value);
    }

    /**
     * @brief Blends a logical pixel without checking the image boundary.
     *
     * The caller must guarantee `x < width()` and `y < height()`.
     */
    auto set_pixel_unchecked(
        std::size_t x,
        std::size_t y,
        pixel value,
        float coverage) noexcept -> void
    {
        if (!(coverage > 0.0f)) {
            return;
        }
        if (coverage >= 1.0f && value.alpha() == 0xffu) {
            set_pixel_unchecked(x, y, value);
            return;
        }
        const auto current = get_pixel(x, y);
        const auto blended = detail::blend_pixel(current, value, coverage);
        storage_.data()[offset(x, y)] = policy_type::encode(blended);
    }

    /**
     * @brief Fills the image with a logical pixel value.
     */
    auto fill(pixel value) noexcept -> void
    {
        const auto encoded = policy_type::encode(value);
        auto* first = storage_.data();
        auto* const last = first + size();
        std::fill(first, last, encoded);
    }

    /**
     * @brief Clears the canvas to opaque black.
     */
    auto clear() noexcept -> void
    {
        fill(pixel{});
    }

private:
    friend struct detail::image_access;
    [[nodiscard]] auto offset(std::size_t x, std::size_t y) const noexcept
        -> std::size_t
    {
        return y * width() + x;
    }

    detail::image_storage<Width, Height, Policy> storage_{};
};

/**
 * @brief Dynamic-size canvas alias.
 */
template<class Policy = argb32_policy>
using dynamic_canvas = canvas<0, 0, Policy>;

namespace detail {

struct image_access {
    template<std::size_t Width, std::size_t Height, class Policy>
    [[nodiscard]] static auto data(canvas<Width, Height, Policy>& img) noexcept
        -> typename canvas<Width, Height, Policy>::storage_type*
    {
        return img.storage_.data();
    }

    template<std::size_t Width, std::size_t Height, class Policy>
    [[nodiscard]] static auto data(
        const canvas<Width, Height, Policy>& img) noexcept
        -> const typename canvas<Width, Height, Policy>::storage_type*
    {
        return img.storage_.data();
    }

    template<std::size_t Width, std::size_t Height, class Policy>
    [[nodiscard]] static auto ptr(
        canvas<Width, Height, Policy>& img,
        std::size_t x,
        std::size_t y) noexcept
        -> typename canvas<Width, Height, Policy>::storage_type*
    {
        return data(img) + y * img.width() + x;
    }
};

} // namespace detail

/**
 * @brief Draws a clipped horizontal line.
 *
 * The requested line is clipped to the image boundary. After clipping, this
 * function writes directly to framebuffer storage instead of calling
 * `set_pixel` for every pixel.
 */
template<std::size_t Width, std::size_t Height, class Policy>
auto draw_hline(
    canvas<Width, Height, Policy>& img,
    int x,
    int y,
    int length,
    pixel color) noexcept -> void
{
    if (length == 0 || y < 0 || static_cast<std::size_t>(y) >= img.height()) {
        return;
    }

    auto x0 = static_cast<long long>(x);
    auto x1 = x0 + static_cast<long long>(length);
    if (x1 < x0) {
        std::swap(x0, x1);
    }

    if (x1 <= 0 || x0 >= static_cast<long long>(img.width())) {
        return;
    }

    x0 = std::max(x0, 0LL);
    x1 = std::min(x1, static_cast<long long>(img.width()));

    const auto encoded = Policy::encode(color);
    auto* first = detail::image_access::ptr(
        img,
        static_cast<std::size_t>(x0),
        static_cast<std::size_t>(y));
    auto* const last = first + (x1 - x0);
    for (auto* p = first; p != last; ++p) {
        *p = encoded;
    }
}

/**
 * @brief Draws a clipped horizontal line from a point.
 */
template<std::size_t Width, std::size_t Height, class Policy>
auto draw_hline(
    canvas<Width, Height, Policy>& img,
    const point& p,
    int length,
    pixel color) noexcept -> void
{
    draw_hline(img, p.x, p.y, length, color);
}

/**
 * @brief Draws a clipped vertical line.
 *
 * The requested line is clipped to the image boundary. After clipping, this
 * function writes directly to framebuffer storage instead of calling
 * `set_pixel` for every pixel.
 */
template<std::size_t Width, std::size_t Height, class Policy>
auto draw_vline(
    canvas<Width, Height, Policy>& img,
    int x,
    int y,
    int length,
    pixel color) noexcept -> void
{
    if (length == 0 || x < 0 || static_cast<std::size_t>(x) >= img.width()) {
        return;
    }

    auto y0 = static_cast<long long>(y);
    auto y1 = y0 + static_cast<long long>(length);
    if (y1 < y0) {
        std::swap(y0, y1);
    }

    if (y1 <= 0 || y0 >= static_cast<long long>(img.height())) {
        return;
    }

    y0 = std::max(y0, 0LL);
    y1 = std::min(y1, static_cast<long long>(img.height()));

    const auto encoded = Policy::encode(color);
    auto* p = detail::image_access::ptr(
        img,
        static_cast<std::size_t>(x),
        static_cast<std::size_t>(y0));
    const auto stride = img.width();
    for (auto yy = y0; yy < y1; ++yy) {
        *p = encoded;
        p += stride;
    }
}

/**
 * @brief Draws a clipped vertical line from a point.
 */
template<std::size_t Width, std::size_t Height, class Policy>
auto draw_vline(
    canvas<Width, Height, Policy>& img,
    const point& p,
    int length,
    pixel color) noexcept -> void
{
    draw_vline(img, p.x, p.y, length, color);
}

/**
 * @brief Draws a clipped Bresenham line.
 */
template<std::size_t Width, std::size_t Height, class Policy>
auto draw_line(
    canvas<Width, Height, Policy>& img,
    int x0,
    int y0,
    int x1,
    int y1,
    pixel color) noexcept -> void
{
    const int dx = x0 < x1 ? x1 - x0 : x0 - x1;
    const int sx = x0 < x1 ? 1 : -1;
    const int dy = y0 < y1 ? -(y1 - y0) : -(y0 - y1);
    const int sy = y0 < y1 ? 1 : -1;
    int error = dx + dy;

    for (;;) {
        if (img.contains(x0, y0)) {
            img.set_pixel_unchecked(
                static_cast<std::size_t>(x0),
                static_cast<std::size_t>(y0),
                color);
        }

        if (x0 == x1 && y0 == y1) {
            break;
        }

        const int twice_error = error * 2;
        if (twice_error >= dy) {
            error += dy;
            x0 += sx;
        }
        if (twice_error <= dx) {
            error += dx;
            y0 += sy;
        }
    }
}

/**
 * @brief Draws a clipped Bresenham line between two points.
 */
template<std::size_t Width, std::size_t Height, class Policy>
auto draw_line(
    canvas<Width, Height, Policy>& img,
    const point& p0,
    const point& p1,
    pixel color) noexcept -> void
{
    draw_line(img, p0.x, p0.y, p1.x, p1.y, color);
}

/**
 * @brief Draws a clipped antialiased line.
 *
 * The coordinates are expressed in pixel-center coordinates. For example,
 * `(0.0f, 0.0f)` is the center of the top-left pixel. The rendered stroke is
 * a capsule around the center line; the endpoints use round caps.
 */
template<std::size_t Width, std::size_t Height, class Policy>
auto draw_line_aa(
    canvas<Width, Height, Policy>& img,
    float x0,
    float y0,
    float x1,
    float y1,
    float width,
    pixel color) noexcept -> void
{
    if (img.empty() || !(width > 0.0f)) {
        return;
    }
    if (!std::isfinite(x0) || !std::isfinite(y0) ||
        !std::isfinite(x1) || !std::isfinite(y1) ||
        !std::isfinite(width)) {
        return;
    }

    const auto dx = x1 - x0;
    const auto dy = y1 - y0;
    const auto length_sq = dx * dx + dy * dy;
    const auto radius = width * 0.5f;
    const auto aa_extent = radius + 0.5f;

    auto min_x = static_cast<int>(std::floor(std::min(x0, x1) - aa_extent));
    auto max_x = static_cast<int>(std::ceil(std::max(x0, x1) + aa_extent));
    auto min_y = static_cast<int>(std::floor(std::min(y0, y1) - aa_extent));
    auto max_y = static_cast<int>(std::ceil(std::max(y0, y1) + aa_extent));

    min_x = std::max(min_x, 0);
    min_y = std::max(min_y, 0);
    max_x = std::min(max_x, static_cast<int>(img.width()) - 1);
    max_y = std::min(max_y, static_cast<int>(img.height()) - 1);

    if (min_x > max_x || min_y > max_y) {
        return;
    }

    for (auto y = min_y; y <= max_y; ++y) {
        for (auto x = min_x; x <= max_x; ++x) {
            const auto px = static_cast<float>(x);
            const auto py = static_cast<float>(y);
            float nearest_x = x0;
            float nearest_y = y0;

            if (length_sq > 0.0f) {
                auto t = ((px - x0) * dx + (py - y0) * dy) / length_sq;
                t = std::clamp(t, 0.0f, 1.0f);
                nearest_x = x0 + dx * t;
                nearest_y = y0 + dy * t;
            }

            const auto ddx = px - nearest_x;
            const auto ddy = py - nearest_y;
            const auto distance = std::sqrt(ddx * ddx + ddy * ddy);
            const auto coverage = radius + 0.5f - distance;
            if (coverage > 0.0f) {
                img.set_pixel_unchecked(
                    static_cast<std::size_t>(x),
                    static_cast<std::size_t>(y),
                    color,
                    coverage);
            }
        }
    }
}

/**
 * @brief Draws a clipped antialiased line with a width of one pixel.
 */
template<std::size_t Width, std::size_t Height, class Policy>
auto draw_line_aa(
    canvas<Width, Height, Policy>& img,
    float x0,
    float y0,
    float x1,
    float y1,
    pixel color) noexcept -> void
{
    draw_line_aa(img, x0, y0, x1, y1, 1.0f, color);
}

/**
 * @brief Draws a clipped antialiased line between two points.
 */
template<std::size_t Width, std::size_t Height, class Policy>
auto draw_line_aa(
    canvas<Width, Height, Policy>& img,
    const pointf& p0,
    const pointf& p1,
    float width,
    pixel color) noexcept -> void
{
    draw_line_aa(img, p0.x, p0.y, p1.x, p1.y, width, color);
}

/**
 * @brief Draws a clipped antialiased line between two points with a width of one pixel.
 */
template<std::size_t Width, std::size_t Height, class Policy>
auto draw_line_aa(
    canvas<Width, Height, Policy>& img,
    const pointf& p0,
    const pointf& p1,
    pixel color) noexcept -> void
{
    draw_line_aa(img, p0, p1, 1.0f, color);
}

/**
 * @brief Draws a clipped rectangle outline.
 */
template<std::size_t Width, std::size_t Height, class Policy>
auto draw_rect(
    canvas<Width, Height, Policy>& img,
    int x,
    int y,
    int width,
    int height,
    pixel color) noexcept -> void
{
    if (width <= 0 || height <= 0) {
        return;
    }

    draw_hline(img, x, y, width, color);
    if (height > 1) {
        draw_hline(img, x, y + height - 1, width, color);
    }
    if (height > 2) {
        draw_vline(img, x, y + 1, height - 2, color);
        if (width > 1) {
            draw_vline(img, x + width - 1, y + 1, height - 2, color);
        }
    }
}

/**
 * @brief Draws a clipped rectangle outline from an origin and extent.
 */
template<std::size_t Width, std::size_t Height, class Policy>
auto draw_rect(
    canvas<Width, Height, Policy>& img,
    const point& origin,
    const size& extent,
    pixel color) noexcept -> void
{
    draw_rect(img, origin.x, origin.y, extent.width, extent.height, color);
}

/**
 * @brief Draws a clipped rectangle outline from a rectangle.
 */
template<std::size_t Width, std::size_t Height, class Policy>
auto draw_rect(
    canvas<Width, Height, Policy>& img,
    const rect& area,
    pixel color) noexcept -> void
{
    draw_rect(img, area.x, area.y, area.width, area.height, color);
}

/**
 * @brief Fills a clipped rectangle.
 */
template<std::size_t Width, std::size_t Height, class Policy>
auto fill_rect(
    canvas<Width, Height, Policy>& img,
    int x,
    int y,
    int width,
    int height,
    pixel color) noexcept -> void
{
    if (width <= 0 || height <= 0) {
        return;
    }

    auto x0 = static_cast<long long>(x);
    auto x1 = x0 + static_cast<long long>(width);
    auto y0 = static_cast<long long>(y);
    auto y1 = y0 + static_cast<long long>(height);

    if (x1 <= 0 || y1 <= 0 ||
        x0 >= static_cast<long long>(img.width()) ||
        y0 >= static_cast<long long>(img.height())) {
        return;
    }

    x0 = std::max(x0, 0LL);
    y0 = std::max(y0, 0LL);
    x1 = std::min(x1, static_cast<long long>(img.width()));
    y1 = std::min(y1, static_cast<long long>(img.height()));

    const auto encoded = Policy::encode(color);
    const auto count = x1 - x0;
    for (auto yy = y0; yy < y1; ++yy) {
        auto* first = detail::image_access::ptr(
            img,
            static_cast<std::size_t>(x0),
            static_cast<std::size_t>(yy));
        auto* const last = first + count;
        for (auto* p = first; p != last; ++p) {
            *p = encoded;
        }
    }
}

/**
 * @brief Fills a clipped rectangle from an origin and extent.
 */
template<std::size_t Width, std::size_t Height, class Policy>
auto fill_rect(
    canvas<Width, Height, Policy>& img,
    const point& origin,
    const size& extent,
    pixel color) noexcept -> void
{
    fill_rect(img, origin.x, origin.y, extent.width, extent.height, color);
}

/**
 * @brief Fills a clipped rectangle from a rectangle.
 */
template<std::size_t Width, std::size_t Height, class Policy>
auto fill_rect(
    canvas<Width, Height, Policy>& img,
    const rect& area,
    pixel color) noexcept -> void
{
    fill_rect(img, area.x, area.y, area.width, area.height, color);
}

/**
 * @brief Draws UTF-8 text using a loaded bitmap font.
 *
 * Glyph cells are positioned from the specified top-left origin. LF, CR, and
 * CRLF line breaks reset the pen to the original x coordinate and advance the
 * pen by `font.glyph_height + options.line_spacing`. Glyphs that are not
 * present in the font are skipped without advancing the pen. This avoids
 * guessing a cell width for unrecorded code points.
 *
 * Bitmap pixels outside the canvas are clipped. The function reports
 * `error_t::encoding_error` for invalid UTF-8 input and
 * `error_t::invalid_argument` when the supplied bitmap font structure is not
 * usable for a glyph that must be drawn.
 */
template<std::size_t Width, std::size_t Height, class Policy>
[[nodiscard]] auto draw_text(
    canvas<Width, Height, Policy>& img,
    int x,
    int y,
    std::u8string_view text,
    const bitmap_font& font,
    pixel color,
    const text_draw_options& options = {}) noexcept -> xer::result<void>
{
    if (font.half_width <= 0 || font.full_width <= 0 || font.glyph_height <= 0) {
        return std::unexpected(xer::make_error(xer::error_t::invalid_argument));
    }
    if (img.empty() || text.empty()) {
        return {};
    }

    const auto origin_x = static_cast<long long>(x);
    auto pen_x = origin_x;
    auto pen_y = static_cast<long long>(y);
    const auto line_advance =
        static_cast<long long>(font.glyph_height) +
        static_cast<long long>(options.line_spacing);

    std::size_t text_offset = 0;
    while (text_offset < text.size()) {
        const auto decoded = detail::bitmap_font_decode_utf8_char(text, text_offset);
        if (!decoded.has_value()) {
            return std::unexpected(decoded.error());
        }

        const char32_t code_point = *decoded;
        if (code_point == U'\r') {
            if (text_offset < text.size() && text[text_offset] == u8'\n') {
                ++text_offset;
            }
            pen_x = origin_x;
            pen_y = detail::bitmap_font_saturating_add(pen_y, line_advance);
            continue;
        }
        if (code_point == U'\n') {
            pen_x = origin_x;
            pen_y = detail::bitmap_font_saturating_add(pen_y, line_advance);
            continue;
        }

        const auto* const range = detail::bitmap_font_find_range(font, code_point);
        if (range == nullptr) {
            continue;
        }

        const int glyph_width = detail::bitmap_font_width_for_range(font, *range);
        if (glyph_width <= 0) {
            return std::unexpected(xer::make_error(xer::error_t::invalid_argument));
        }

        const auto bytes_per_row =
            (static_cast<std::uint64_t>(glyph_width) + UINT64_C(7)) / UINT64_C(8);
        const auto bytes_per_glyph =
            bytes_per_row * static_cast<std::uint64_t>(font.glyph_height);
        const auto glyph_index =
            static_cast<std::uint64_t>(code_point - range->first_code_point);

        if (bytes_per_glyph != 0 &&
            glyph_index > std::numeric_limits<std::uint64_t>::max() / bytes_per_glyph) {
            return std::unexpected(xer::make_error(xer::error_t::invalid_argument));
        }

        const auto glyph_offset_in_range = glyph_index * bytes_per_glyph;
        if (range->bitmap_offset >
            std::numeric_limits<std::uint64_t>::max() - glyph_offset_in_range) {
            return std::unexpected(xer::make_error(xer::error_t::invalid_argument));
        }

        const auto glyph_offset = range->bitmap_offset + glyph_offset_in_range;
        const auto bitmap_size = static_cast<std::uint64_t>(font.bitmap.size());
        if (glyph_offset > bitmap_size || bytes_per_glyph > bitmap_size - glyph_offset) {
            return std::unexpected(xer::make_error(xer::error_t::invalid_argument));
        }

        const auto glyph_begin = static_cast<std::size_t>(glyph_offset);
        const auto canvas_width = static_cast<long long>(img.width());
        const auto canvas_height = static_cast<long long>(img.height());

        for (int row = 0; row < font.glyph_height; ++row) {
            const auto py = pen_y + static_cast<long long>(row);
            if (py < 0 || py >= canvas_height) {
                continue;
            }

            const auto row_offset =
                glyph_begin + static_cast<std::size_t>(row) *
                    static_cast<std::size_t>(bytes_per_row);
            for (int column = 0; column < glyph_width; ++column) {
                const auto px = pen_x + static_cast<long long>(column);
                if (px < 0 || px >= canvas_width) {
                    continue;
                }

                const auto byte_index =
                    row_offset + static_cast<std::size_t>(column / 8);
                const auto bit_mask = static_cast<std::uint8_t>(
                    1u << (7 - (column % 8)));
                if ((font.bitmap[byte_index] & bit_mask) == 0u) {
                    continue;
                }

                img.set_pixel_unchecked(
                    static_cast<std::size_t>(px),
                    static_cast<std::size_t>(py),
                    color);
            }
        }

        const auto glyph_advance =
            static_cast<long long>(glyph_width) +
            static_cast<long long>(options.letter_spacing);
        pen_x = detail::bitmap_font_saturating_add(pen_x, glyph_advance);
    }

    return {};
}

/**
 * @brief Draws UTF-8 text using a loaded bitmap font from a point origin.
 */
template<std::size_t Width, std::size_t Height, class Policy>
[[nodiscard]] auto draw_text(
    canvas<Width, Height, Policy>& img,
    const point& origin,
    std::u8string_view text,
    const bitmap_font& font,
    pixel color,
    const text_draw_options& options = {}) noexcept -> xer::result<void>
{
    return draw_text(img, origin.x, origin.y, text, font, color, options);
}

/**
 * @brief Flood-fills a four-connected region from a starting coordinate.
 *
 * The original color is taken from the starting pixel. Every four-connected
 * pixel whose logical ARGB value exactly matches that original color is
 * replaced with `color`. Alpha participates in the equality comparison.
 *
 * If the starting coordinate is outside the canvas, or if the starting pixel
 * already has the replacement color, this function does nothing and returns
 * success.
 */
template<std::size_t Width, std::size_t Height, class Policy>
[[nodiscard]] auto flood_fill(
    canvas<Width, Height, Policy>& img,
    int x,
    int y,
    pixel color) -> xer::result<void>
{
    if (!img.contains(x, y)) {
        return {};
    }

    const auto start_x = static_cast<std::size_t>(x);
    const auto start_y = static_cast<std::size_t>(y);
    const pixel original = img.get_pixel(start_x, start_y);
    if (original == color) {
        return {};
    }

    std::vector<point> pending{};
    pending.emplace_back(x, y);

    while (!pending.empty()) {
        const point current = pending.back();
        pending.pop_back();

        if (!img.contains(current)) {
            continue;
        }

        const auto current_x = static_cast<std::size_t>(current.x);
        const auto current_y = static_cast<std::size_t>(current.y);
        if (img.get_pixel(current_x, current_y) != original) {
            continue;
        }

        img.set_pixel_unchecked(current_x, current_y, color);

        if (current.x > 0) {
            pending.emplace_back(current.x - 1, current.y);
        }
        if (static_cast<std::size_t>(current.x) + 1 < img.width() &&
            current.x < std::numeric_limits<int>::max()) {
            pending.emplace_back(current.x + 1, current.y);
        }
        if (current.y > 0) {
            pending.emplace_back(current.x, current.y - 1);
        }
        if (static_cast<std::size_t>(current.y) + 1 < img.height() &&
            current.y < std::numeric_limits<int>::max()) {
            pending.emplace_back(current.x, current.y + 1);
        }
    }

    return {};
}

/**
 * @brief Flood-fills a four-connected region from a starting point.
 */
template<std::size_t Width, std::size_t Height, class Policy>
[[nodiscard]] auto flood_fill(
    canvas<Width, Height, Policy>& img,
    const point& origin,
    pixel color) -> xer::result<void>
{
    return flood_fill(img, origin.x, origin.y, color);
}

/**
 * @brief Applies a mosaic effect to a clipped rectangular area.
 *
 * The target area is clipped to the canvas boundary. Each block is replaced
 * with the average logical ARGB color of the pixels in that block. Blocks at
 * the right and bottom edges use their actual clipped size.
 *
 * @return `error_t::invalid_argument` if either block dimension is not
 * positive. Otherwise returns success, including empty or fully clipped areas.
 */
template<std::size_t Width, std::size_t Height, class Policy>
[[nodiscard]] auto mosaic(
    canvas<Width, Height, Policy>& img,
    const rect& area,
    const size& block_size) noexcept -> xer::result<void>
{
    if (block_size.width <= 0 || block_size.height <= 0) {
        return std::unexpected(xer::make_error(xer::error_t::invalid_argument));
    }
    if (img.empty() || area.width <= 0 || area.height <= 0) {
        return {};
    }

    auto x0 = static_cast<long long>(area.x);
    auto y0 = static_cast<long long>(area.y);
    auto x1 = x0 + static_cast<long long>(area.width);
    auto y1 = y0 + static_cast<long long>(area.height);

    if (x1 <= 0 || y1 <= 0 ||
        x0 >= static_cast<long long>(img.width()) ||
        y0 >= static_cast<long long>(img.height())) {
        return {};
    }

    x0 = std::max(x0, 0LL);
    y0 = std::max(y0, 0LL);
    x1 = std::min(x1, static_cast<long long>(img.width()));
    y1 = std::min(y1, static_cast<long long>(img.height()));

    const auto step_x = static_cast<long long>(block_size.width);
    const auto step_y = static_cast<long long>(block_size.height);

    for (auto by = y0; by < y1; by += step_y) {
        const auto ey = std::min(by + step_y, y1);
        for (auto bx = x0; bx < x1; bx += step_x) {
            const auto ex = std::min(bx + step_x, x1);

            std::uint64_t alpha = 0;
            std::uint64_t red = 0;
            std::uint64_t green = 0;
            std::uint64_t blue = 0;
            std::uint64_t count = 0;

            for (auto yy = by; yy < ey; ++yy) {
                for (auto xx = bx; xx < ex; ++xx) {
                    const auto value = img.get_pixel(
                        static_cast<std::size_t>(xx),
                        static_cast<std::size_t>(yy));
                    alpha += value.alpha();
                    red += value.red();
                    green += value.green();
                    blue += value.blue();
                    ++count;
                }
            }

            const auto half = count / 2;
            const auto averaged = pixel(
                static_cast<std::uint8_t>((alpha + half) / count),
                static_cast<std::uint8_t>((red + half) / count),
                static_cast<std::uint8_t>((green + half) / count),
                static_cast<std::uint8_t>((blue + half) / count));
            const auto encoded = Policy::encode(averaged);

            for (auto yy = by; yy < ey; ++yy) {
                auto* first = detail::image_access::ptr(
                    img,
                    static_cast<std::size_t>(bx),
                    static_cast<std::size_t>(yy));
                auto* const last = first + (ex - bx);
                for (auto* p = first; p != last; ++p) {
                    *p = encoded;
                }
            }
        }
    }

    return {};
}

/**
 * @brief Applies a box blur to a clipped rectangular area.
 *
 * The target area is clipped to the canvas boundary. `box_size` specifies the
 * averaging kernel size in pixels. Source samples are taken from the original
 * pixels in the clipped target area, so pixels outside the requested area do
 * not affect the blur result. Kernel portions outside the clipped area are
 * ignored.
 *
 * Even kernel dimensions are supported. In that case, the extra sample is
 * placed on the left or top side of the current pixel.
 *
 * @return `error_t::invalid_argument` if either box dimension is not positive.
 * Otherwise returns success, including empty or fully clipped areas.
 */
template<std::size_t Width, std::size_t Height, class Policy>
[[nodiscard]] auto box_blur(
    canvas<Width, Height, Policy>& img,
    const rect& area,
    const size& box_size) -> xer::result<void>
{
    if (box_size.width <= 0 || box_size.height <= 0) {
        return std::unexpected(xer::make_error(xer::error_t::invalid_argument));
    }
    if (img.empty() || area.width <= 0 || area.height <= 0) {
        return {};
    }

    auto x0 = static_cast<long long>(area.x);
    auto y0 = static_cast<long long>(area.y);
    auto x1 = x0 + static_cast<long long>(area.width);
    auto y1 = y0 + static_cast<long long>(area.height);

    if (x1 <= 0 || y1 <= 0 ||
        x0 >= static_cast<long long>(img.width()) ||
        y0 >= static_cast<long long>(img.height())) {
        return {};
    }

    x0 = std::max(x0, 0LL);
    y0 = std::max(y0, 0LL);
    x1 = std::min(x1, static_cast<long long>(img.width()));
    y1 = std::min(y1, static_cast<long long>(img.height()));

    const auto clipped_width = static_cast<std::size_t>(x1 - x0);
    const auto clipped_height = static_cast<std::size_t>(y1 - y0);
    std::vector<pixel> source(clipped_width * clipped_height);

    for (std::size_t yy = 0; yy < clipped_height; ++yy) {
        for (std::size_t xx = 0; xx < clipped_width; ++xx) {
            source[yy * clipped_width + xx] = img.get_pixel(
                static_cast<std::size_t>(x0) + xx,
                static_cast<std::size_t>(y0) + yy);
        }
    }

    const auto left = static_cast<long long>(box_size.width / 2);
    const auto right = static_cast<long long>(box_size.width - 1) - left;
    const auto top = static_cast<long long>(box_size.height / 2);
    const auto bottom = static_cast<long long>(box_size.height - 1) - top;
    const auto max_source_x = static_cast<long long>(clipped_width) - 1;
    const auto max_source_y = static_cast<long long>(clipped_height) - 1;

    for (std::size_t yy = 0; yy < clipped_height; ++yy) {
        for (std::size_t xx = 0; xx < clipped_width; ++xx) {
            const auto center_x = static_cast<long long>(xx);
            const auto center_y = static_cast<long long>(yy);
            const auto sx0 = std::max(center_x - left, 0LL);
            const auto sx1 = std::min(center_x + right, max_source_x);
            const auto sy0 = std::max(center_y - top, 0LL);
            const auto sy1 = std::min(center_y + bottom, max_source_y);

            std::uint64_t alpha = 0;
            std::uint64_t red = 0;
            std::uint64_t green = 0;
            std::uint64_t blue = 0;
            std::uint64_t count = 0;

            for (auto sy = sy0; sy <= sy1; ++sy) {
                for (auto sx = sx0; sx <= sx1; ++sx) {
                    const auto value = source[
                        static_cast<std::size_t>(sy) * clipped_width +
                        static_cast<std::size_t>(sx)];
                    alpha += value.alpha();
                    red += value.red();
                    green += value.green();
                    blue += value.blue();
                    ++count;
                }
            }

            const auto half = count / 2;
            img.set_pixel_unchecked(
                static_cast<std::size_t>(x0) + xx,
                static_cast<std::size_t>(y0) + yy,
                pixel(
                    static_cast<std::uint8_t>((alpha + half) / count),
                    static_cast<std::uint8_t>((red + half) / count),
                    static_cast<std::uint8_t>((green + half) / count),
                    static_cast<std::uint8_t>((blue + half) / count)));
        }
    }

    return {};
}


/**
 * @brief Applies a per-pixel filter to a clipped rectangular area.
 *
 * The target area is clipped to the canvas boundary. For each pixel in the
 * clipped area, `filter` receives the current logical pixel value and must
 * return the replacement logical pixel value.
 *
 * If `filter` throws an exception for a pixel, that pixel is left unchanged and
 * processing continues with the next pixel. If any pixel fails, the function
 * returns `error_t::user_error` with `filter_pixels_error_detail`. The detail
 * records the first failing pixel position and the total number of failures.
 *
 * @return Success when all pixels are filtered successfully, or
 * `error_t::user_error` with detail when one or more filter calls fail.
 */
template<std::size_t Width, std::size_t Height, class Policy, class F>
[[nodiscard]] auto filter_pixels(
    canvas<Width, Height, Policy>& img,
    const rect& area,
    F&& filter) -> xer::result<void, filter_pixels_error_detail>
{
    if (img.empty() || area.width <= 0 || area.height <= 0) {
        return {};
    }

    auto x0 = static_cast<long long>(area.x);
    auto y0 = static_cast<long long>(area.y);
    auto x1 = x0 + static_cast<long long>(area.width);
    auto y1 = y0 + static_cast<long long>(area.height);

    if (x1 <= 0 || y1 <= 0 ||
        x0 >= static_cast<long long>(img.width()) ||
        y0 >= static_cast<long long>(img.height())) {
        return {};
    }

    x0 = std::max(x0, 0LL);
    y0 = std::max(y0, 0LL);
    x1 = std::min(x1, static_cast<long long>(img.width()));
    y1 = std::min(y1, static_cast<long long>(img.height()));

    filter_pixels_error_detail error_detail{};
    bool has_error = false;

    for (auto yy = y0; yy < y1; ++yy) {
        for (auto xx = x0; xx < x1; ++xx) {
            const auto x = static_cast<std::size_t>(xx);
            const auto y = static_cast<std::size_t>(yy);
            const auto old_value = img.get_pixel(x, y);

            try {
                const pixel new_value = filter(old_value);
                img.set_pixel_unchecked(x, y, new_value);
            } catch (...) {
                if (!has_error) {
                    error_detail.first_error_position = point(
                        static_cast<int>(xx),
                        static_cast<int>(yy));
                    has_error = true;
                }
                ++error_detail.error_count;
            }
        }
    }

    if (has_error) {
        return std::unexpected(xer::make_error<filter_pixels_error_detail>(
            xer::error_t::user_error,
            error_detail));
    }

    return {};
}


} // namespace xer::image

#endif /* XER_BITS_IMAGE_H_INCLUDED_ */
