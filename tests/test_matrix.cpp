#include <xer/assert.h>
#include <xer/error.h>
#include <xer/matrix.h>

#include <cmath>
#include <concepts>
#include <type_traits>

namespace {

template<std::floating_point T>
auto assert_near(T actual, T expected, T epsilon = static_cast<T>(1e-10)) -> void
{
    xer_assert(std::fabs(actual - expected) <= epsilon);
}

auto test_matrix_aliases() -> void
{
    static_assert(std::same_as<xer::matrix3<double>, xer::matrix<double, 3, 3>>);
    static_assert(std::same_as<xer::matrix4<double>, xer::matrix<double, 4, 4>>);
    static_assert(std::same_as<xer::vector3<double>, xer::matrix<double, 3, 1>>);
    static_assert(std::same_as<xer::vector4<double>, xer::matrix<double, 4, 1>>);
}

auto test_default_matrix_is_zero() -> void
{
    const xer::matrix<double, 2, 3> value;

    xer_assert_eq(value(0, 0), 0.0);
    xer_assert_eq(value(0, 1), 0.0);
    xer_assert_eq(value(0, 2), 0.0);
    xer_assert_eq(value(1, 0), 0.0);
    xer_assert_eq(value(1, 1), 0.0);
    xer_assert_eq(value(1, 2), 0.0);
}

auto test_matrix_construction_and_access() -> void
{
    xer::matrix<double, 2, 3> value{
        1.0, 2.0, 3.0,
        4.0, 5.0, 6.0};

    xer_assert_eq(value(0, 0), 1.0);
    xer_assert_eq(value(0, 2), 3.0);
    xer_assert_eq(value(1, 0), 4.0);
    xer_assert_eq(value(1, 2), 6.0);

    value(1, 2) = 60.0;
    xer_assert_eq(value(1, 2), 60.0);
}

auto test_identity_matrix() -> void
{
    const auto identity = xer::identity_matrix<double, 3>();
    const xer::vector3<double> point{2.0, 3.0, 1.0};
    const auto transformed = identity * point;

    xer_assert_eq(transformed(0, 0), 2.0);
    xer_assert_eq(transformed(1, 0), 3.0);
    xer_assert_eq(transformed(2, 0), 1.0);
}

auto test_identity3_and_identity4() -> void
{
    const auto identity3 = xer::identity3<double>();
    const auto identity4 = xer::identity4<double>();

    xer_assert_eq(identity3(0, 0), 1.0);
    xer_assert_eq(identity3(1, 1), 1.0);
    xer_assert_eq(identity3(2, 2), 1.0);

    xer_assert_eq(identity4(0, 0), 1.0);
    xer_assert_eq(identity4(1, 1), 1.0);
    xer_assert_eq(identity4(2, 2), 1.0);
    xer_assert_eq(identity4(3, 3), 1.0);
}

auto test_general_matrix_multiplication() -> void
{
    const xer::matrix<double, 2, 3> left{
        1.0, 2.0, 3.0,
        4.0, 5.0, 6.0};
    const xer::matrix<double, 3, 2> right{
        7.0, 8.0,
        9.0, 10.0,
        11.0, 12.0};

    const auto result = left * right;

    static_assert(std::same_as<decltype(result), const xer::matrix<double, 2, 2>>);
    xer_assert_eq(result(0, 0), 58.0);
    xer_assert_eq(result(0, 1), 64.0);
    xer_assert_eq(result(1, 0), 139.0);
    xer_assert_eq(result(1, 1), 154.0);
}

auto test_2d_translate_scale_and_composition() -> void
{
    const xer::vector3<double> point{2.0, 3.0, 1.0};

    const auto translated = xer::translate2(5.0, 10.0) * point;
    xer_assert_eq(translated(0, 0), 7.0);
    xer_assert_eq(translated(1, 0), 13.0);
    xer_assert_eq(translated(2, 0), 1.0);

    const auto scaled = xer::scale2(2.0, 3.0) * point;
    xer_assert_eq(scaled(0, 0), 4.0);
    xer_assert_eq(scaled(1, 0), 9.0);
    xer_assert_eq(scaled(2, 0), 1.0);

    const auto composed = xer::translate2(5.0, 10.0) * xer::scale2(2.0, 3.0);
    const auto result = composed * point;

    xer_assert_eq(result(0, 0), 9.0);
    xer_assert_eq(result(1, 0), 19.0);
    xer_assert_eq(result(2, 0), 1.0);
}

auto test_2d_rotation() -> void
{
    const xer::vector3<double> point{1.0, 0.0, 1.0};
    const auto rotated = xer::rotate2(xer::pi_v<double> / 2.0) * point;

    assert_near(rotated(0, 0), 0.0);
    assert_near(rotated(1, 0), 1.0);
    assert_near(rotated(2, 0), 1.0);
}

auto test_inverse_3x3() -> void
{
    const xer::vector3<double> point{4.0, 5.0, 1.0};
    const auto transform =
        xer::translate2(10.0, 20.0) *
        xer::rotate2(xer::pi_v<double> / 2.0) *
        xer::scale2(2.0, 3.0);

    const auto transformed = transform * point;
    const auto inverse = xer::inverse(transform);

    xer_assert(inverse.has_value());

    const auto restored = *inverse * transformed;

    assert_near(restored(0, 0), point(0, 0));
    assert_near(restored(1, 0), point(1, 0));
    assert_near(restored(2, 0), point(2, 0));
}

auto test_inverse_3x3_singular() -> void
{
    const auto singular = xer::scale2(1.0, 0.0);
    const auto inverse = xer::inverse(singular);

    xer_assert_not(inverse.has_value());
    xer_assert_eq(inverse.error().code, xer::error_t::divide_by_zero);
}

auto test_3d_translate_scale_and_rotation() -> void
{
    const xer::vector4<double> point{1.0, 2.0, 3.0, 1.0};

    const auto translated = xer::translate3(10.0, 20.0, 30.0) * point;
    xer_assert_eq(translated(0, 0), 11.0);
    xer_assert_eq(translated(1, 0), 22.0);
    xer_assert_eq(translated(2, 0), 33.0);
    xer_assert_eq(translated(3, 0), 1.0);

    const auto scaled = xer::scale3(2.0, 3.0, 4.0) * point;
    xer_assert_eq(scaled(0, 0), 2.0);
    xer_assert_eq(scaled(1, 0), 6.0);
    xer_assert_eq(scaled(2, 0), 12.0);
    xer_assert_eq(scaled(3, 0), 1.0);

    const xer::vector4<double> x_axis{1.0, 0.0, 0.0, 1.0};
    const auto rotated = xer::rotate_z(xer::pi_v<double> / 2.0) * x_axis;

    assert_near(rotated(0, 0), 0.0);
    assert_near(rotated(1, 0), 1.0);
    assert_near(rotated(2, 0), 0.0);
    assert_near(rotated(3, 0), 1.0);
}

auto test_inverse_4x4() -> void
{
    const xer::vector4<double> point{4.0, 5.0, 6.0, 1.0};
    const auto transform =
        xer::translate3(10.0, 20.0, 30.0) *
        xer::rotate_z(xer::pi_v<double> / 2.0) *
        xer::scale3(2.0, 3.0, 4.0);

    const auto transformed = transform * point;
    const auto inverse = xer::inverse(transform);

    xer_assert(inverse.has_value());

    const auto restored = *inverse * transformed;

    assert_near(restored(0, 0), point(0, 0));
    assert_near(restored(1, 0), point(1, 0));
    assert_near(restored(2, 0), point(2, 0));
    assert_near(restored(3, 0), point(3, 0));
}

auto test_inverse_4x4_singular() -> void
{
    const auto singular = xer::scale3(1.0, 0.0, 1.0);
    const auto inverse = xer::inverse(singular);

    xer_assert_not(inverse.has_value());
    xer_assert_eq(inverse.error().code, xer::error_t::divide_by_zero);
}

} // namespace

auto main() -> int
{
    test_matrix_aliases();
    test_default_matrix_is_zero();
    test_matrix_construction_and_access();
    test_identity_matrix();
    test_identity3_and_identity4();
    test_general_matrix_multiplication();
    test_2d_translate_scale_and_composition();
    test_2d_rotation();
    test_inverse_3x3();
    test_inverse_3x3_singular();
    test_3d_translate_scale_and_rotation();
    test_inverse_4x4();
    test_inverse_4x4_singular();

    return 0;
}
