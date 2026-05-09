#include <type_traits>

#include <xer/image.h>

namespace {

auto test_geometry_types() -> bool
{
    constexpr xer::image::point p{1, 2};
    constexpr xer::image::pointf pf{1.5f, 2.5f};
    constexpr xer::image::size s{3, 4};
    constexpr xer::image::sizef sf{3.5f, 4.5f};
    constexpr xer::image::rect r{1, 2, 3, 4};
    constexpr xer::image::rect r2{p, s};
    constexpr xer::image::rectf rf{1.5f, 2.5f, 3.5f, 4.5f};
    constexpr xer::image::rectf rf2{pf, sf};

    return p.x == 1 && p.y == 2 &&
           pf.x == 1.5f && pf.y == 2.5f &&
           s.width == 3 && s.height == 4 &&
           sf.width == 3.5f && sf.height == 4.5f &&
           r.x == 1 && r.y == 2 && r.width == 3 && r.height == 4 &&
           r2.x == 1 && r2.y == 2 && r2.width == 3 && r2.height == 4 &&
           rf.x == 1.5f && rf.y == 2.5f &&
           rf.width == 3.5f && rf.height == 4.5f &&
           rf2.x == 1.5f && rf2.y == 2.5f &&
           rf2.width == 3.5f && rf2.height == 4.5f;
}

auto test_pixel_construction() -> bool
{
    constexpr xer::image::pixel default_pixel;
    constexpr xer::image::pixel raw(0x12345678u);
    constexpr xer::image::pixel rgb(0x11u, 0x22u, 0x33u);
    constexpr xer::image::pixel argb(0x44u, 0x11u, 0x22u, 0x33u);

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
    auto value = xer::image::pixel{};
    value.alpha(0x12u);
    value.red(0x34u);
    value.green(0x56u);
    value.blue(0x78u);

    return value.argb == 0x12345678u;
}

auto test_argb32_policy() -> bool
{
    xer::image::argb32_policy::storage_type storage = 0;
    xer::image::argb32_policy::set(storage, xer::image::pixel(0x12u, 0x34u, 0x56u, 0x78u));
    const auto value = xer::image::argb32_policy::get(storage);

    return storage == 0x12345678u &&
           value.argb == 0x12345678u &&
           xer::image::argb32_policy::encode(value) == 0x12345678u;
}

auto test_rgba32_policy() -> bool
{
    xer::image::rgba32_policy::storage_type storage = 0;
    xer::image::rgba32_policy::set(storage, xer::image::pixel(0x12u, 0x34u, 0x56u, 0x78u));
    const auto value = xer::image::rgba32_policy::get(storage);

    return storage == 0x34567812u &&
           value.argb == 0x12345678u &&
           xer::image::rgba32_policy::encode(value) == 0x34567812u;
}

auto test_rgb24_policy() -> bool
{
    xer::image::rgb24_policy::storage_type storage{};
    xer::image::rgb24_policy::set(storage, xer::image::pixel(0x12u, 0x34u, 0x56u, 0x78u));
    const auto value = xer::image::rgb24_policy::get(storage);

    return storage[0] == 0x34u &&
           storage[1] == 0x56u &&
           storage[2] == 0x78u &&
           value.argb == 0xff345678u;
}

auto test_bgr24_policy() -> bool
{
    xer::image::bgr24_policy::storage_type storage{};
    xer::image::bgr24_policy::set(storage, xer::image::pixel(0x12u, 0x34u, 0x56u, 0x78u));
    const auto value = xer::image::bgr24_policy::get(storage);

    return storage[0] == 0x78u &&
           storage[1] == 0x56u &&
           storage[2] == 0x34u &&
           value.argb == 0xff345678u;
}

auto test_fixed_image_basic() -> bool
{
    xer::image::canvas<4, 3> img;

    if (img.width() != 4 || img.height() != 3 || img.size() != 12 || img.empty()) {
        return false;
    }

    img.set_pixel(2, 1, xer::image::pixel(0x11u, 0x22u, 0x33u));
    return img.get_pixel(2, 1).argb == 0xff112233u;
}

auto test_dynamic_canvas_basic() -> bool
{
    xer::image::dynamic_canvas<xer::image::rgba32_policy> img(3, 2);

    if (img.width() != 3 || img.height() != 2 || img.size() != 6 || img.empty()) {
        return false;
    }

    img.set_pixel(1, 1, xer::image::pixel(0x80u, 0x10u, 0x20u, 0x30u));
    return img.get_pixel(1, 1).argb == 0x80102030u;
}

auto test_set_pixel_checked_and_unchecked() -> bool
{
    xer::image::canvas<2, 2> img;
    img.clear();

    img.set_pixel(-1, 0, xer::image::pixel(0xffu, 0u, 0u));
    img.set_pixel(2, 0, xer::image::pixel(0xffu, 0u, 0u));
    if (img.get_pixel(0, 0).argb != 0xff000000u ||
        img.get_pixel(1, 0).argb != 0xff000000u) {
        return false;
    }

    img.set_pixel_unchecked(1, 1, xer::image::pixel(0u, 0xffu, 0u));
    return img.get_pixel(1, 1).argb == 0xff00ff00u;
}


auto test_set_pixel_coverage() -> bool
{
    xer::image::canvas<2, 2> img;
    img.clear();

    img.set_pixel(0, 0, xer::image::pixel(0xffu, 0u, 0u), 0.5f);
    if (img.get_pixel(0, 0).argb != 0xff800000u) {
        return false;
    }

    img.set_pixel(1, 0, xer::image::pixel(0u, 0xffu, 0u), 0.0f);
    if (img.get_pixel(1, 0).argb != 0xff000000u) {
        return false;
    }

    img.set_pixel_unchecked(1, 1, xer::image::pixel(0u, 0u, 0xffu), 2.0f);
    return img.get_pixel(1, 1).argb == 0xff0000ffu;
}

auto test_clear_and_fill() -> bool
{
    xer::image::canvas<2, 2> img;

    img.fill(xer::image::pixel(0x01u, 0x02u, 0x03u));
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
    const xer::image::canvas<3, 2> img;

    return img.contains(0, 0) &&
           img.contains(2, 1) &&
           !img.contains(-1, 0) &&
           !img.contains(0, -1) &&
           !img.contains(3, 0) &&
           !img.contains(0, 2);
}

auto test_draw_hline_clipped() -> bool
{
    xer::image::canvas<4, 2> img;
    img.clear();

    xer::image::draw_hline(img, -1, 0, 4, xer::image::pixel(0xffu, 0u, 0u));

    return img.get_pixel(0, 0).argb == 0xffff0000u &&
           img.get_pixel(1, 0).argb == 0xffff0000u &&
           img.get_pixel(2, 0).argb == 0xffff0000u &&
           img.get_pixel(3, 0).argb == 0xff000000u &&
           img.get_pixel(0, 1).argb == 0xff000000u;
}

auto test_draw_vline_clipped() -> bool
{
    xer::image::canvas<2, 4> img;
    img.clear();

    xer::image::draw_vline(img, 1, -1, 4, xer::image::pixel(0u, 0xffu, 0u));

    return img.get_pixel(1, 0).argb == 0xff00ff00u &&
           img.get_pixel(1, 1).argb == 0xff00ff00u &&
           img.get_pixel(1, 2).argb == 0xff00ff00u &&
           img.get_pixel(1, 3).argb == 0xff000000u &&
           img.get_pixel(0, 0).argb == 0xff000000u;
}

auto test_draw_line() -> bool
{
    xer::image::canvas<4, 4> img;
    img.clear();

    xer::image::draw_line(img, 0, 0, 3, 3, xer::image::pixel(0u, 0u, 0xffu));

    return img.get_pixel(0, 0).argb == 0xff0000ffu &&
           img.get_pixel(1, 1).argb == 0xff0000ffu &&
           img.get_pixel(2, 2).argb == 0xff0000ffu &&
           img.get_pixel(3, 3).argb == 0xff0000ffu &&
           img.get_pixel(3, 0).argb == 0xff000000u;
}


auto test_draw_line_aa_thin() -> bool
{
    xer::image::canvas<5, 5> img;
    img.clear();

    xer::image::draw_line_aa(img, 1.0f, 1.0f, 3.0f, 3.0f, xer::image::pixel(0u, 0u, 0xffu));

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
    xer::image::canvas<5, 4> img;
    img.clear();

    xer::image::draw_line_aa(img, 1.0f, 1.0f, 3.0f, 1.0f, 2.0f, xer::image::pixel(0xffu, 0u, 0u));

    return img.get_pixel(1, 1).argb == 0xffff0000u &&
           img.get_pixel(2, 1).argb == 0xffff0000u &&
           img.get_pixel(3, 1).argb == 0xffff0000u &&
           img.get_pixel(2, 0).argb == 0xff800000u &&
           img.get_pixel(2, 2).argb == 0xff800000u &&
           img.get_pixel(2, 3).argb == 0xff000000u;
}

auto test_draw_rect() -> bool
{
    xer::image::canvas<5, 5> img;
    img.clear();

    xer::image::draw_rect(img, 1, 1, 3, 3, xer::image::pixel(0xffu, 0xffu, 0u));

    return img.get_pixel(1, 1).argb == 0xffffff00u &&
           img.get_pixel(2, 1).argb == 0xffffff00u &&
           img.get_pixel(3, 1).argb == 0xffffff00u &&
           img.get_pixel(1, 2).argb == 0xffffff00u &&
           img.get_pixel(3, 2).argb == 0xffffff00u &&
           img.get_pixel(2, 2).argb == 0xff000000u;
}

auto test_fill_rect_clipped() -> bool
{
    xer::image::canvas<4, 4> img;
    img.clear();

    xer::image::fill_rect(img, -1, 1, 3, 2, xer::image::pixel(0xffu, 0u, 0xffu));

    return img.get_pixel(0, 1).argb == 0xffff00ffu &&
           img.get_pixel(1, 1).argb == 0xffff00ffu &&
           img.get_pixel(2, 1).argb == 0xff000000u &&
           img.get_pixel(0, 2).argb == 0xffff00ffu &&
           img.get_pixel(1, 2).argb == 0xffff00ffu &&
           img.get_pixel(0, 0).argb == 0xff000000u;
}

auto test_mosaic() -> bool
{
    xer::image::canvas<4, 3> img;

    img.set_pixel_unchecked(0, 0, xer::image::pixel(0xffu, 0u, 0u));
    img.set_pixel_unchecked(1, 0, xer::image::pixel(0u, 0xffu, 0u));
    img.set_pixel_unchecked(0, 1, xer::image::pixel(0u, 0u, 0xffu));
    img.set_pixel_unchecked(1, 1, xer::image::pixel(0xffu, 0xffu, 0xffu));

    img.set_pixel_unchecked(2, 0, xer::image::pixel(0x10u, 0x20u, 0x30u));
    img.set_pixel_unchecked(3, 0, xer::image::pixel(0x30u, 0x40u, 0x50u));
    img.set_pixel_unchecked(2, 1, xer::image::pixel(0x50u, 0x60u, 0x70u));
    img.set_pixel_unchecked(3, 1, xer::image::pixel(0x70u, 0x80u, 0x90u));

    for (std::size_t x = 0; x < 4; ++x) {
        img.set_pixel_unchecked(x, 2, xer::image::pixel(0x11u, 0x22u, 0x33u));
    }

    const auto result = xer::image::mosaic(
        img,
        xer::image::rect{0, 0, 4, 2},
        xer::image::size{2, 2});
    if (!result.has_value()) {
        return false;
    }

    for (std::size_t y = 0; y < 2; ++y) {
        for (std::size_t x = 0; x < 2; ++x) {
            if (img.get_pixel(x, y).argb != 0xff808080u) {
                return false;
            }
        }
    }
    for (std::size_t y = 0; y < 2; ++y) {
        for (std::size_t x = 2; x < 4; ++x) {
            if (img.get_pixel(x, y).argb != 0xff405060u) {
                return false;
            }
        }
    }

    return img.get_pixel(0, 2).argb == 0xff112233u;
}

auto test_mosaic_clipped_edge_block() -> bool
{
    xer::image::canvas<3, 2> img;

    img.set_pixel_unchecked(0, 0, xer::image::pixel(0x00u, 0x00u, 0x00u));
    img.set_pixel_unchecked(1, 0, xer::image::pixel(0x00u, 0x00u, 0xffu));
    img.set_pixel_unchecked(2, 0, xer::image::pixel(0xffu, 0x00u, 0x00u));
    img.set_pixel_unchecked(0, 1, xer::image::pixel(0xffu, 0x00u, 0x00u));
    img.set_pixel_unchecked(1, 1, xer::image::pixel(0x00u, 0xffu, 0x00u));
    img.set_pixel_unchecked(2, 1, xer::image::pixel(0x00u, 0x00u, 0xffu));

    const auto result = xer::image::mosaic(
        img,
        xer::image::rect{-1, 0, 4, 2},
        xer::image::size{2, 2});
    if (!result.has_value()) {
        return false;
    }

    return img.get_pixel(0, 0).argb == 0xff404040u &&
           img.get_pixel(1, 0).argb == 0xff404040u &&
           img.get_pixel(0, 1).argb == 0xff404040u &&
           img.get_pixel(1, 1).argb == 0xff404040u &&
           img.get_pixel(2, 0).argb == 0xff800080u &&
           img.get_pixel(2, 1).argb == 0xff800080u;
}

auto test_mosaic_invalid_block_size() -> bool
{
    xer::image::canvas<2, 2> img;

    const auto result = xer::image::mosaic(
        img,
        xer::image::rect{0, 0, 2, 2},
        xer::image::size{0, 2});

    return !result.has_value() &&
           result.error().code == xer::error_t::invalid_argument;
}

auto test_point_pixel_methods() -> bool
{
    xer::image::canvas<3, 3> img;
    img.clear();

    constexpr xer::image::point p{1, 2};
    img.set_pixel(p, xer::image::pixel(0x12u, 0x34u, 0x56u));

    if (!img.contains(p) || img.contains(xer::image::point{-1, 0})) {
        return false;
    }
    if (img.get_pixel(p).argb != 0xff123456u) {
        return false;
    }

    img.set_pixel(p, xer::image::pixel(0x80u, 0xffu, 0u, 0u), 0.5f);

    return img.get_pixel(p).alpha() == 0xffu &&
           img.get_pixel(p).red() > 0x12u;
}

auto test_draw_geometry_overloads() -> bool
{
    xer::image::canvas<6, 6> img;
    img.clear();

    const auto red = xer::image::pixel(0xffu, 0u, 0u);
    const auto green = xer::image::pixel(0u, 0xffu, 0u);
    const auto blue = xer::image::pixel(0u, 0u, 0xffu);
    const auto yellow = xer::image::pixel(0xffu, 0xffu, 0u);
    const auto magenta = xer::image::pixel(0xffu, 0u, 0xffu);
    const auto cyan = xer::image::pixel(0u, 0xffu, 0xffu);

    xer::image::draw_hline(img, xer::image::point{1, 0}, 3, red);
    xer::image::draw_vline(img, xer::image::point{0, 1}, 3, green);
    xer::image::draw_line(img, xer::image::point{2, 2}, xer::image::point{4, 4}, blue);
    xer::image::draw_rect(img, xer::image::point{1, 1}, xer::image::size{3, 3}, yellow);
    xer::image::fill_rect(img, xer::image::rect{4, 0, 2, 2}, magenta);
    xer::image::draw_line_aa(
        img,
        xer::image::pointf{0.0f, 5.0f},
        xer::image::pointf{2.0f, 5.0f},
        cyan);

    if (img.get_pixel(1, 0).argb != 0xffff0000u) {
        return false;
    }
    if (img.get_pixel(0, 1).argb != 0xff00ff00u) {
        return false;
    }
    if (img.get_pixel(4, 4).argb != 0xff0000ffu) {
        return false;
    }
    if (img.get_pixel(2, 1).argb != 0xffffff00u) {
        return false;
    }
    if (img.get_pixel(4, 1).argb != 0xffff00ffu) {
        return false;
    }

    return img.get_pixel(1, 5).argb == 0xff00ffffu;
}


auto test_type_properties() -> bool
{
    static_assert(std::is_default_constructible_v<xer::image::canvas<1, 1>>);
    static_assert(std::is_default_constructible_v<xer::image::dynamic_canvas<>>);

    return true;
}

} // namespace

auto main() -> int
{
    if (!test_geometry_types()) {
        return 1;
    }
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
    if (!test_dynamic_canvas_basic()) {
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
    if (!test_point_pixel_methods()) {
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
    if (!test_mosaic()) {
        return 1;
    }
    if (!test_mosaic_clipped_edge_block()) {
        return 1;
    }
    if (!test_mosaic_invalid_block_size()) {
        return 1;
    }
    if (!test_draw_geometry_overloads()) {
        return 1;
    }
    if (!test_type_properties()) {
        return 1;
    }

    return 0;
}
