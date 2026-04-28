#include <cmath>
#include <type_traits>

#include <xer/assert.h>
#include <xer/color.h>

namespace {

template<class T>
auto near(T left, T right, T epsilon) -> bool
{
    return std::abs(left - right) <= epsilon;
}

void test_rgb_default_constructor()
{
    constexpr xer::rgb value;

    xer_assert_eq(value.r.value(), 0.0f);
    xer_assert_eq(value.g.value(), 0.0f);
    xer_assert_eq(value.b.value(), 0.0f);
}

void test_rgb_constructor_clamps_components()
{
    const xer::rgb value(1.25f, 0.5f, -0.25f);

    xer_assert_eq(value.r.value(), 1.0f);
    xer_assert_eq(value.g.value(), 0.5f);
    xer_assert_eq(value.b.value(), 0.0f);
}

void test_cmy_constructor_clamps_components()
{
    const xer::cmy value(1.25f, 0.5f, -0.25f);

    xer_assert_eq(value.c.value(), 1.0f);
    xer_assert_eq(value.m.value(), 0.5f);
    xer_assert_eq(value.y.value(), 0.0f);
}

void test_hsv_constructor_normalizes_hue_and_clamps_components()
{
    const xer::hsv value(1.25f, 1.5f, -0.25f);

    xer_assert(near(value.h.value(), 0.25f, 1e-6f));
    xer_assert_eq(value.s.value(), 1.0f);
    xer_assert_eq(value.v.value(), 0.0f);
}

void test_rgb_to_cmy()
{
    const auto value = xer::to_cmy(xer::rgb(0.25f, 0.5f, 0.75f));

    xer_assert(near(value.c.value(), 0.75f, 1e-6f));
    xer_assert(near(value.m.value(), 0.5f, 1e-6f));
    xer_assert(near(value.y.value(), 0.25f, 1e-6f));
}

void test_cmy_to_rgb()
{
    const auto value = xer::to_rgb(xer::cmy(0.25f, 0.5f, 0.75f));

    xer_assert(near(value.r.value(), 0.75f, 1e-6f));
    xer_assert(near(value.g.value(), 0.5f, 1e-6f));
    xer_assert(near(value.b.value(), 0.25f, 1e-6f));
}

void test_rgb_to_hsv_red()
{
    const auto value = xer::to_hsv(xer::rgb(1.0f, 0.0f, 0.0f));

    xer_assert(near(value.h.value(), 0.0f, 1e-6f));
    xer_assert(near(value.s.value(), 1.0f, 1e-6f));
    xer_assert(near(value.v.value(), 1.0f, 1e-6f));
}

void test_rgb_to_hsv_green()
{
    const auto value = xer::to_hsv(xer::rgb(0.0f, 1.0f, 0.0f));

    xer_assert(near(value.h.value(), 1.0f / 3.0f, 1e-6f));
    xer_assert(near(value.s.value(), 1.0f, 1e-6f));
    xer_assert(near(value.v.value(), 1.0f, 1e-6f));
}

void test_rgb_to_hsv_blue()
{
    const auto value = xer::to_hsv(xer::rgb(0.0f, 0.0f, 1.0f));

    xer_assert(near(value.h.value(), 2.0f / 3.0f, 1e-6f));
    xer_assert(near(value.s.value(), 1.0f, 1e-6f));
    xer_assert(near(value.v.value(), 1.0f, 1e-6f));
}

void test_rgb_to_hsv_gray_sets_hue_to_zero()
{
    const auto value = xer::to_hsv(xer::rgb(0.5f, 0.5f, 0.5f));

    xer_assert(near(value.h.value(), 0.0f, 1e-6f));
    xer_assert(near(value.s.value(), 0.0f, 1e-6f));
    xer_assert(near(value.v.value(), 0.5f, 1e-6f));
}

void test_hsv_to_rgb_red()
{
    const auto value = xer::to_rgb(xer::hsv(0.0f, 1.0f, 1.0f));

    xer_assert(near(value.r.value(), 1.0f, 1e-6f));
    xer_assert(near(value.g.value(), 0.0f, 1e-6f));
    xer_assert(near(value.b.value(), 0.0f, 1e-6f));
}

void test_hsv_to_rgb_green()
{
    const auto value = xer::to_rgb(xer::hsv(1.0f / 3.0f, 1.0f, 1.0f));

    xer_assert(near(value.r.value(), 0.0f, 1e-6f));
    xer_assert(near(value.g.value(), 1.0f, 1e-6f));
    xer_assert(near(value.b.value(), 0.0f, 1e-6f));
}

void test_hsv_to_rgb_blue()
{
    const auto value = xer::to_rgb(xer::hsv(2.0f / 3.0f, 1.0f, 1.0f));

    xer_assert(near(value.r.value(), 0.0f, 1e-6f));
    xer_assert(near(value.g.value(), 0.0f, 1e-6f));
    xer_assert(near(value.b.value(), 1.0f, 1e-6f));
}

void test_rgb_xyz_white_point()
{
    const auto value = xer::to_xyz(xer::rgb(1.0f, 1.0f, 1.0f));

    xer_assert(near(value.x, 0.95047f, 1e-4f));
    xer_assert(near(value.y, 1.0f, 1e-4f));
    xer_assert(near(value.z, 1.08883f, 1e-4f));
}

void test_xyz_rgb_round_trip()
{
    const xer::rgb source(0.25f, 0.5f, 0.75f);
    const auto value = xer::to_rgb(xer::to_xyz(source));

    xer_assert(near(value.r.value(), source.r.value(), 1e-5f));
    xer_assert(near(value.g.value(), source.g.value(), 1e-5f));
    xer_assert(near(value.b.value(), source.b.value(), 1e-5f));
}

void test_xyz_lab_white()
{
    const auto value = xer::to_lab(xer::xyz(0.95047f, 1.0f, 1.08883f));

    xer_assert(near(value.l, 100.0f, 1e-3f));
    xer_assert(near(value.a, 0.0f, 1e-3f));
    xer_assert(near(value.b, 0.0f, 1e-3f));
}

void test_lab_xyz_round_trip()
{
    const xer::xyz source(0.25f, 0.4f, 0.2f);
    const auto value = xer::to_xyz(xer::to_lab(source));

    xer_assert(near(value.x, source.x, 1e-5f));
    xer_assert(near(value.y, source.y, 1e-5f));
    xer_assert(near(value.z, source.z, 1e-5f));
}

void test_xyz_luv_white()
{
    const auto value = xer::to_luv(xer::xyz(0.95047f, 1.0f, 1.08883f));

    xer_assert(near(value.l, 100.0f, 1e-3f));
    xer_assert(near(value.u, 0.0f, 1e-3f));
    xer_assert(near(value.v, 0.0f, 1e-3f));
}

void test_luv_xyz_round_trip()
{
    const xer::xyz source(0.25f, 0.4f, 0.2f);
    const auto value = xer::to_xyz(xer::to_luv(source));

    xer_assert(near(value.x, source.x, 1e-5f));
    xer_assert(near(value.y, source.y, 1e-5f));
    xer_assert(near(value.z, source.z, 1e-5f));
}

void test_type_aliases()
{
    static_assert(std::is_same_v<xer::rgb, xer::basic_rgb<float>>);
    static_assert(std::is_same_v<xer::cmy, xer::basic_cmy<float>>);
    static_assert(std::is_same_v<xer::hsv, xer::basic_hsv<float>>);
    static_assert(std::is_same_v<xer::xyz, xer::basic_xyz<float>>);
    static_assert(std::is_same_v<xer::lab, xer::basic_lab<float>>);
    static_assert(std::is_same_v<xer::luv, xer::basic_luv<float>>);

    static_assert(std::is_same_v<xer::basic_rgb<double>::value_type, double>);
    static_assert(std::is_same_v<
                  xer::basic_rgb<double>::component_type,
                  xer::interval<double>>);
    static_assert(std::is_same_v<
                  xer::basic_hsv<double>::hue_type,
                  xer::cyclic<double>>);
}

} // namespace

auto main() -> int
{
    test_rgb_default_constructor();
    test_rgb_constructor_clamps_components();
    test_cmy_constructor_clamps_components();
    test_hsv_constructor_normalizes_hue_and_clamps_components();
    test_rgb_to_cmy();
    test_cmy_to_rgb();
    test_rgb_to_hsv_red();
    test_rgb_to_hsv_green();
    test_rgb_to_hsv_blue();
    test_rgb_to_hsv_gray_sets_hue_to_zero();
    test_hsv_to_rgb_red();
    test_hsv_to_rgb_green();
    test_hsv_to_rgb_blue();
    test_rgb_xyz_white_point();
    test_xyz_rgb_round_trip();
    test_xyz_lab_white();
    test_lab_xyz_round_trip();
    test_xyz_luv_white();
    test_luv_xyz_round_trip();
    test_type_aliases();

    return 0;
}
