#include <cmath>
#include <optional>

#include <xer/assert.h>
#include <xer/error.h>
#include <xer/math.h>

namespace {


void test_vec2_members_and_index_access()
{
    xer::vec<double> v{1.0, 2.0};

    xer_assert(v.x == 1.0);
    xer_assert(v.y == 2.0);
    xer_assert(v[0] == 1.0);
    xer_assert(v[1] == 2.0);

    v[0] = 3.0;
    v[1] = 4.0;

    xer_assert(v.x == 3.0);
    xer_assert(v.y == 4.0);
}

void test_vec3_and_vec4_index_access()
{
    xer::vec<int, 3> v3{1, 2, 3};
    xer::vec<int, 4> v4{4, 5, 6, 7};

    xer_assert(v3[0] == 1);
    xer_assert(v3[1] == 2);
    xer_assert(v3[2] == 3);
    xer_assert(v4[0] == 4);
    xer_assert(v4[1] == 5);
    xer_assert(v4[2] == 6);
    xer_assert(v4[3] == 7);
}

void test_vec_at_success()
{
    xer::vec<double> v{1.0, 2.0};
    auto x = v.at(0);
    auto y = v.at(1);

    xer_assert(x.has_value());
    xer_assert(y.has_value());

    x->get() = 5.0;
    y->get() = 6.0;

    xer_assert(v.x == 5.0);
    xer_assert(v.y == 6.0);
}

void test_vec_at_out_of_range()
{
    xer::vec<double> v{1.0, 2.0};
    const auto value = v.at(2);

    xer_assert_not(value.has_value());
    xer_assert(value.error().code == xer::error_t::out_of_range);
}

void test_to_polar()
{
    const auto p = xer::to_polar(xer::vec<double>{3.0, 4.0});

    xer_assert(xer::detail::equation_near(p.r, 5.0));
    xer_assert(xer::detail::equation_near(p.theta, std::atan2(4.0, 3.0)));
}

void test_to_cartesian()
{
    const auto v = xer::to_cartesian(xer::polar<double>{5.0, std::atan2(4.0, 3.0)});

    xer_assert(xer::detail::equation_near(v.x, 3.0));
    xer_assert(xer::detail::equation_near(v.y, 4.0));
}

void test_heron_regular_triangle()
{
    const auto area = xer::heron(3.0, 4.0, 5.0);

    xer_assert(area.has_value());
    xer_assert(xer::detail::equation_near(*area, 6.0));
}

void test_heron_degenerate_triangle()
{
    const auto area = xer::heron(1.0, 2.0, 3.0);

    xer_assert(area.has_value());
    xer_assert(xer::detail::equation_near(*area, 0.0));
}

void test_heron_invalid_negative_side()
{
    const auto area = xer::heron(-1.0, 2.0, 3.0);

    xer_assert_not(area.has_value());
    xer_assert(area.error().code == xer::error_t::invalid_argument);
}

void test_heron_invalid_triangle()
{
    const auto area = xer::heron(1.0, 2.0, 4.0);

    xer_assert_not(area.has_value());
    xer_assert(area.error().code == xer::error_t::invalid_argument);
}

void test_quadratic_two_real_roots()
{
    const auto roots = xer::quadratic(1.0, -5.0, 6.0);

    xer_assert(roots.has_value());
    xer_assert(roots->at(0).has_value());
    xer_assert(roots->at(1).has_value());
    xer_assert(xer::detail::equation_near(*roots->at(0), 2.0));
    xer_assert(xer::detail::equation_near(*roots->at(1), 3.0));
}

void test_quadratic_one_real_root()
{
    const auto roots = xer::quadratic(1.0, -2.0, 1.0);

    xer_assert(roots.has_value());
    xer_assert(roots->at(0).has_value());
    xer_assert_not(roots->at(1).has_value());
    xer_assert(xer::detail::equation_near(*roots->at(0), 1.0));
}

void test_quadratic_no_real_root()
{
    const auto roots = xer::quadratic(1.0, 0.0, 1.0);

    xer_assert(roots.has_value());
    xer_assert_not(roots->at(0).has_value());
    xer_assert_not(roots->at(1).has_value());
}

void test_quadratic_invalid_argument()
{
    const auto roots = xer::quadratic(0.0, 2.0, 1.0);

    xer_assert_not(roots.has_value());
    xer_assert(roots.error().code == xer::error_t::invalid_argument);
}

void test_cubic_three_real_roots()
{
    const auto roots = xer::cubic(1.0, -6.0, 11.0, -6.0);

    xer_assert(roots.has_value());
    xer_assert(roots->at(0).has_value());
    xer_assert(roots->at(1).has_value());
    xer_assert(roots->at(2).has_value());
    xer_assert(xer::detail::equation_near(*roots->at(0), 1.0));
    xer_assert(xer::detail::equation_near(*roots->at(1), 2.0));
    xer_assert(xer::detail::equation_near(*roots->at(2), 3.0));
}

void test_cubic_one_real_root()
{
    const auto roots = xer::cubic(1.0, 0.0, 0.0, -8.0);

    xer_assert(roots.has_value());
    xer_assert(roots->at(0).has_value());
    xer_assert_not(roots->at(1).has_value());
    xer_assert_not(roots->at(2).has_value());
    xer_assert(xer::detail::equation_near(*roots->at(0), 2.0));
}

void test_cubic_multiple_real_roots()
{
    const auto roots = xer::cubic(1.0, -3.0, 3.0, -1.0);

    xer_assert(roots.has_value());
    xer_assert(roots->at(0).has_value());
    xer_assert_not(roots->at(1).has_value());
    xer_assert_not(roots->at(2).has_value());
    xer_assert(xer::detail::equation_near(*roots->at(0), 1.0));
}

void test_cubic_invalid_argument()
{
    const auto roots = xer::cubic(0.0, 1.0, 2.0, 3.0);

    xer_assert_not(roots.has_value());
    xer_assert(roots.error().code == xer::error_t::invalid_argument);
}

} // namespace

auto main() -> int
{
    test_vec2_members_and_index_access();
    test_vec3_and_vec4_index_access();
    test_vec_at_success();
    test_vec_at_out_of_range();
    test_to_polar();
    test_to_cartesian();
    test_heron_regular_triangle();
    test_heron_degenerate_triangle();
    test_heron_invalid_negative_side();
    test_heron_invalid_triangle();
    test_quadratic_two_real_roots();
    test_quadratic_one_real_root();
    test_quadratic_no_real_root();
    test_quadratic_invalid_argument();
    test_cubic_three_real_roots();
    test_cubic_one_real_root();
    test_cubic_multiple_real_roots();
    test_cubic_invalid_argument();
    return 0;
}
