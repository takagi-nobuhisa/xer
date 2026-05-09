#include <type_traits>

#include <xer/image.h>

namespace {

auto test_pixel_construction() -> bool
{
    constexpr xer::pixel default_pixel;
    constexpr xer::pixel raw(0x12345678u);
    constexpr xer::pixel rgb(0x11u, 0x22u, 0x33u);
    constexpr xer::pixel argb(0x44u, 0x11u, 0x22u, 0x33u);

    return default_pixel.argb == 0xff000000u &&
           raw.alpha() == 0x12u &&
           raw.red() == 0x34u &&
           raw.green() == 0x56u &&
           raw.blue() == 0x78u &&
           rgb.argb == 0xff112233u &&
           argb.argb == 0x44112233u;
}

auto test_pixel_setters() -> bool
{
    auto value = xer::pixel{};
    value.alpha(0x12u);
    value.red(0x34u);
    value.green(0x56u);
    value.blue(0x78u);

    return value.argb == 0x12345678u;
}

auto test_argb32_policy() -> bool
{
    xer::argb32_policy::storage_type storage = 0;
    xer::argb32_policy::set(storage, xer::pixel(0x12u, 0x34u, 0x56u, 0x78u));
    const auto value = xer::argb32_policy::get(storage);

    return storage == 0x12345678u &&
           value.argb == 0x12345678u &&
           xer::argb32_policy::encode(value) == 0x12345678u;
}

auto test_rgba32_policy() -> bool
{
    xer::rgba32_policy::storage_type storage = 0;
    xer::rgba32_policy::set(storage, xer::pixel(0x12u, 0x34u, 0x56u, 0x78u));
    const auto value = xer::rgba32_policy::get(storage);

    return storage == 0x34567812u &&
           value.argb == 0x12345678u &&
           xer::rgba32_policy::encode(value) == 0x34567812u;
}

auto test_rgb24_policy() -> bool
{
    xer::rgb24_policy::storage_type storage{};
    xer::rgb24_policy::set(storage, xer::pixel(0x12u, 0x34u, 0x56u, 0x78u));
    const auto value = xer::rgb24_policy::get(storage);

    return storage[0] == 0x34u &&
           storage[1] == 0x56u &&
           storage[2] == 0x78u &&
           value.argb == 0xff345678u;
}

auto test_bgr24_policy() -> bool
{
    xer::bgr24_policy::storage_type storage{};
    xer::bgr24_policy::set(storage, xer::pixel(0x12u, 0x34u, 0x56u, 0x78u));
    const auto value = xer::bgr24_policy::get(storage);

    return storage[0] == 0x78u &&
           storage[1] == 0x56u &&
           storage[2] == 0x34u &&
           value.argb == 0xff345678u;
}

auto test_fixed_image_basic() -> bool
{
    xer::image<4, 3> img;

    if (img.width() != 4 || img.height() != 3 || img.size() != 12 || img.empty()) {
        return false;
    }

    img.set_pixel(2, 1, xer::pixel(0x11u, 0x22u, 0x33u));
    return img.get_pixel(2, 1).argb == 0xff112233u;
}

auto test_dynamic_image_basic() -> bool
{
    xer::dynamic_image<xer::rgba32_policy> img(3, 2);

    if (img.width() != 3 || img.height() != 2 || img.size() != 6 || img.empty()) {
        return false;
    }

    img.set_pixel(1, 1, xer::pixel(0x80u, 0x10u, 0x20u, 0x30u));
    return img.get_pixel(1, 1).argb == 0x80102030u;
}

auto test_set_pixel_checked_and_unchecked() -> bool
{
    xer::image<2, 2> img;
    img.clear();

    img.set_pixel(-1, 0, xer::pixel(0xffu, 0u, 0u));
    img.set_pixel(2, 0, xer::pixel(0xffu, 0u, 0u));
    if (img.get_pixel(0, 0).argb != 0xff000000u ||
        img.get_pixel(1, 0).argb != 0xff000000u) {
        return false;
    }

    img.set_pixel_unchecked(1, 1, xer::pixel(0u, 0xffu, 0u));
    return img.get_pixel(1, 1).argb == 0xff00ff00u;
}


auto test_set_pixel_coverage() -> bool
{
    xer::image<2, 2> img;
    img.clear();

    img.set_pixel(0, 0, xer::pixel(0xffu, 0u, 0u), 0.5f);
    if (img.get_pixel(0, 0).argb != 0xff800000u) {
        return false;
    }

    img.set_pixel(1, 0, xer::pixel(0u, 0xffu, 0u), 0.0f);
    if (img.get_pixel(1, 0).argb != 0xff000000u) {
        return false;
    }

    img.set_pixel_unchecked(1, 1, xer::pixel(0u, 0u, 0xffu), 2.0f);
    return img.get_pixel(1, 1).argb == 0xff0000ffu;
}

auto test_clear_and_fill() -> bool
{
    xer::image<2, 2> img;

    img.fill(xer::pixel(0x01u, 0x02u, 0x03u));
    for (std::size_t y = 0; y < img.height(); ++y) {
        for (std::size_t x = 0; x < img.width(); ++x) {
            if (img.get_pixel(x, y).argb != 0xff010203u) {
                return false;
            }
        }
    }

    img.clear();
    for (std::size_t y = 0; y < img.height(); ++y) {
        for (std::size_t x = 0; x < img.width(); ++x) {
            if (img.get_pixel(x, y).argb != 0xff000000u) {
                return false;
            }
        }
    }

    return true;
}

auto test_contains() -> bool
{
    const xer::image<3, 2> img;

    return img.contains(0, 0) &&
           img.contains(2, 1) &&
           !img.contains(-1, 0) &&
           !img.contains(0, -1) &&
           !img.contains(3, 0) &&
           !img.contains(0, 2);
}

auto test_draw_hline_clipped() -> bool
{
    xer::image<4, 2> img;
    img.clear();

    xer::draw_hline(img, -1, 0, 4, xer::pixel(0xffu, 0u, 0u));

    return img.get_pixel(0, 0).argb == 0xffff0000u &&
           img.get_pixel(1, 0).argb == 0xffff0000u &&
           img.get_pixel(2, 0).argb == 0xffff0000u &&
           img.get_pixel(3, 0).argb == 0xff000000u &&
           img.get_pixel(0, 1).argb == 0xff000000u;
}

auto test_draw_vline_clipped() -> bool
{
    xer::image<2, 4> img;
    img.clear();

    xer::draw_vline(img, 1, -1, 4, xer::pixel(0u, 0xffu, 0u));

    return img.get_pixel(1, 0).argb == 0xff00ff00u &&
           img.get_pixel(1, 1).argb == 0xff00ff00u &&
           img.get_pixel(1, 2).argb == 0xff00ff00u &&
           img.get_pixel(1, 3).argb == 0xff000000u &&
           img.get_pixel(0, 0).argb == 0xff000000u;
}

auto test_draw_line() -> bool
{
    xer::image<4, 4> img;
    img.clear();

    xer::draw_line(img, 0, 0, 3, 3, xer::pixel(0u, 0u, 0xffu));

    return img.get_pixel(0, 0).argb == 0xff0000ffu &&
           img.get_pixel(1, 1).argb == 0xff0000ffu &&
           img.get_pixel(2, 2).argb == 0xff0000ffu &&
           img.get_pixel(3, 3).argb == 0xff0000ffu &&
           img.get_pixel(3, 0).argb == 0xff000000u;
}


auto test_draw_line_aa_thin() -> bool
{
    xer::image<5, 5> img;
    img.clear();

    xer::draw_line_aa(img, 1.0f, 1.0f, 3.0f, 3.0f, xer::pixel(0u, 0u, 0xffu));

    const auto center = img.get_pixel(2, 2);
    const auto edge = img.get_pixel(1, 2);

    return img.get_pixel(1, 1).argb == 0xff0000ffu &&
           center.argb == 0xff0000ffu &&
           img.get_pixel(3, 3).argb == 0xff0000ffu &&
           edge.blue() > 0u &&
           edge.blue() < 0xffu &&
           img.get_pixel(4, 0).argb == 0xff000000u;
}


auto test_draw_line_aa_width() -> bool
{
    xer::image<5, 4> img;
    img.clear();

    xer::draw_line_aa(img, 1.0f, 1.0f, 3.0f, 1.0f, 2.0f, xer::pixel(0xffu, 0u, 0u));

    return img.get_pixel(1, 1).argb == 0xffff0000u &&
           img.get_pixel(2, 1).argb == 0xffff0000u &&
           img.get_pixel(3, 1).argb == 0xffff0000u &&
           img.get_pixel(2, 0).argb == 0xff800000u &&
           img.get_pixel(2, 2).argb == 0xff800000u &&
           img.get_pixel(2, 3).argb == 0xff000000u;
}

auto test_draw_rect() -> bool
{
    xer::image<5, 5> img;
    img.clear();

    xer::draw_rect(img, 1, 1, 3, 3, xer::pixel(0xffu, 0xffu, 0u));

    return img.get_pixel(1, 1).argb == 0xffffff00u &&
           img.get_pixel(2, 1).argb == 0xffffff00u &&
           img.get_pixel(3, 1).argb == 0xffffff00u &&
           img.get_pixel(1, 2).argb == 0xffffff00u &&
           img.get_pixel(3, 2).argb == 0xffffff00u &&
           img.get_pixel(2, 2).argb == 0xff000000u;
}

auto test_fill_rect_clipped() -> bool
{
    xer::image<4, 4> img;
    img.clear();

    xer::fill_rect(img, -1, 1, 3, 2, xer::pixel(0xffu, 0u, 0xffu));

    return img.get_pixel(0, 1).argb == 0xffff00ffu &&
           img.get_pixel(1, 1).argb == 0xffff00ffu &&
           img.get_pixel(2, 1).argb == 0xff000000u &&
           img.get_pixel(0, 2).argb == 0xffff00ffu &&
           img.get_pixel(1, 2).argb == 0xffff00ffu &&
           img.get_pixel(0, 0).argb == 0xff000000u;
}

auto test_type_properties() -> bool
{
    static_assert(std::is_default_constructible_v<xer::image<1, 1>>);
    static_assert(std::is_default_constructible_v<xer::dynamic_image<>>);

    return true;
}

} // namespace

auto main() -> int
{
    if (!test_pixel_construction()) {
        return 1;
    }
    if (!test_pixel_setters()) {
        return 1;
    }
    if (!test_argb32_policy()) {
        return 1;
    }
    if (!test_rgba32_policy()) {
        return 1;
    }
    if (!test_rgb24_policy()) {
        return 1;
    }
    if (!test_bgr24_policy()) {
        return 1;
    }
    if (!test_fixed_image_basic()) {
        return 1;
    }
    if (!test_dynamic_image_basic()) {
        return 1;
    }
    if (!test_set_pixel_checked_and_unchecked()) {
        return 1;
    }
    if (!test_set_pixel_coverage()) {
        return 1;
    }
    if (!test_clear_and_fill()) {
        return 1;
    }
    if (!test_contains()) {
        return 1;
    }
    if (!test_draw_hline_clipped()) {
        return 1;
    }
    if (!test_draw_vline_clipped()) {
        return 1;
    }
    if (!test_draw_line()) {
        return 1;
    }
    if (!test_draw_line_aa_thin()) {
        return 1;
    }
    if (!test_draw_line_aa_width()) {
        return 1;
    }
    if (!test_draw_rect()) {
        return 1;
    }
    if (!test_fill_rect_clipped()) {
        return 1;
    }
    if (!test_type_properties()) {
        return 1;
    }

    return 0;
}
