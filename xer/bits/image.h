/**
 * @file xer/bits/image.h
 * @brief Image and framebuffer implementation.
 */

#pragma once

#ifndef XER_BITS_IMAGE_H_INCLUDED_
#define XER_BITS_IMAGE_H_INCLUDED_

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <vector>

namespace xer {

/**
 * @brief Logical ARGB pixel value.
 *
 * `pixel` is a logical color value. It is not necessarily identical to the
 * physical framebuffer storage element used by `image`.
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

} // namespace detail

/**
 * @brief Fixed-size or dynamic-size framebuffer image.
 *
 * `image<Width, Height, Policy>` stores physical framebuffer elements whose
 * format is controlled by `Policy`. Public pixel operations use the logical
 * `pixel` type.
 *
 * `image<0, 0, Policy>` is the dynamic-size specialization.
 */
template<
    std::size_t Width,
    std::size_t Height,
    class Policy = argb32_policy>
class image {
    static_assert(
        detail::valid_image_extent<Width, Height>,
        "image dimensions must both be fixed or both be zero");

public:
    using policy_type = Policy;
    using storage_type = typename policy_type::storage_type;
    using pixel_type = pixel;

    /**
     * @brief Constructs a fixed-size image, or an empty dynamic-size image.
     */
    image() = default;

    /**
     * @brief Constructs a dynamic-size image.
     */
    image(std::size_t width, std::size_t height)
        requires(Width == 0 && Height == 0)
        : storage_(width, height)
    {
    }

    /**
     * @brief Returns the image width in pixels.
     */
    [[nodiscard]] auto width() const noexcept -> std::size_t
    {
        return storage_.width();
    }

    /**
     * @brief Returns the image height in pixels.
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
     * @brief Returns whether this image has no drawable area.
     */
    [[nodiscard]] auto empty() const noexcept -> bool
    {
        return width() == 0 || height() == 0;
    }

    /**
     * @brief Returns whether signed coordinates are inside the image.
     */
    [[nodiscard]] auto contains(int x, int y) const noexcept -> bool
    {
        return x >= 0 && y >= 0 &&
               static_cast<std::size_t>(x) < width() &&
               static_cast<std::size_t>(y) < height();
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
     * @brief Clears the image to opaque black.
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
 * @brief Dynamic-size image alias.
 */
template<class Policy = argb32_policy>
using dynamic_image = image<0, 0, Policy>;

namespace detail {

struct image_access {
    template<std::size_t Width, std::size_t Height, class Policy>
    [[nodiscard]] static auto data(image<Width, Height, Policy>& img) noexcept
        -> typename image<Width, Height, Policy>::storage_type*
    {
        return img.storage_.data();
    }

    template<std::size_t Width, std::size_t Height, class Policy>
    [[nodiscard]] static auto data(
        const image<Width, Height, Policy>& img) noexcept
        -> const typename image<Width, Height, Policy>::storage_type*
    {
        return img.storage_.data();
    }

    template<std::size_t Width, std::size_t Height, class Policy>
    [[nodiscard]] static auto ptr(
        image<Width, Height, Policy>& img,
        std::size_t x,
        std::size_t y) noexcept
        -> typename image<Width, Height, Policy>::storage_type*
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
    image<Width, Height, Policy>& img,
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
 * @brief Draws a clipped vertical line.
 *
 * The requested line is clipped to the image boundary. After clipping, this
 * function writes directly to framebuffer storage instead of calling
 * `set_pixel` for every pixel.
 */
template<std::size_t Width, std::size_t Height, class Policy>
auto draw_vline(
    image<Width, Height, Policy>& img,
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
 * @brief Draws a clipped Bresenham line.
 */
template<std::size_t Width, std::size_t Height, class Policy>
auto draw_line(
    image<Width, Height, Policy>& img,
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
 * @brief Draws a clipped rectangle outline.
 */
template<std::size_t Width, std::size_t Height, class Policy>
auto draw_rect(
    image<Width, Height, Policy>& img,
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
 * @brief Fills a clipped rectangle.
 */
template<std::size_t Width, std::size_t Height, class Policy>
auto fill_rect(
    image<Width, Height, Policy>& img,
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

} // namespace xer

#endif /* XER_BITS_IMAGE_H_INCLUDED_ */
