#include <cmath>
#include <type_traits>

#include <xer/assert.h>
#include <xer/cyclic.h>

namespace {

void test_default_constructor()
{
    constexpr xer::cyclic<float> value;

    xer_assert_eq(value.value(), 0.0f);
}

void test_normalize_positive_wrap()
{
    const xer::cyclic<float> value(1.25f);

    xer_assert_eq(value.value(), 0.25f);
}

void test_normalize_negative_wrap()
{
    const xer::cyclic<float> value(-0.25f);

    xer_assert_eq(value.value(), 0.75f);
}

void test_cw_and_ccw()
{
    const xer::cyclic<float> from(0.1f);
    const xer::cyclic<float> to(0.3f);

    xer_assert(std::abs(from.ccw(to) - 0.2f) < 1e-6f);
    xer_assert(std::abs(from.cw(to) - 0.8f) < 1e-6f);
}

void test_diff_positive()
{
    const xer::cyclic<float> from(0.9f);
    const xer::cyclic<float> to(0.1f);

    xer_assert(std::abs(from.diff(to) - 0.2f) < 1e-6f);
}

void test_diff_negative()
{
    const xer::cyclic<float> from(0.1f);
    const xer::cyclic<float> to(0.9f);

    xer_assert(std::abs(from.diff(to) + 0.2f) < 1e-6f);
}

void test_diff_half_turn_uses_negative_side()
{
    const xer::cyclic<float> from(0.0f);
    const xer::cyclic<float> to(0.5f);

    xer_assert(std::abs(from.diff(to) + 0.5f) < 1e-6f);
}

void test_eq_around_zero_boundary()
{
    const xer::cyclic<double> left(0.0);
    const xer::cyclic<double> right(1.0 - xer::cyclic<double>::default_epsilon / 2.0);

    xer_assert(left.eq(right));
    xer_assert_not(left.ne(right));
}

void test_eq_with_explicit_epsilon()
{
    const xer::cyclic<float> left(0.1f);
    const xer::cyclic<float> right(0.1005f);

    xer_assert_not(left.eq(right));
    xer_assert(left.eq(right, 0.001f));
    xer_assert_not(left.ne(right, 0.001f));
}

void test_addition_and_subtraction()
{
    const xer::cyclic<float> left(0.8f);
    const xer::cyclic<float> right(0.5f);

    const auto sum = left + right;
    const auto diff = left - right;

    xer_assert(std::abs(sum.value() - 0.3f) < 1e-6f);
    xer_assert(std::abs(diff.value() - 0.3f) < 1e-6f);
}

void test_compound_assignment()
{
    xer::cyclic<float> value(0.2f);

    value += xer::cyclic<float>(0.9f);
    xer_assert(std::abs(value.value() - 0.1f) < 1e-6f);

    value -= xer::cyclic<float>(0.3f);
    xer_assert(std::abs(value.value() - 0.8f) < 1e-6f);
}

void test_unary_minus()
{
    const xer::cyclic<float> value(0.25f);
    const auto negated = -value;

    xer_assert(std::abs(negated.value() - 0.75f) < 1e-6f);
}

void test_degree_conversion()
{
    const auto value = xer::from_degree<float>(450.0f);

    xer_assert(std::abs(value.value() - 0.25f) < 1e-6f);
    xer_assert(std::abs(xer::to_degree(value) - 90.0f) < 1e-4f);
}

void test_radian_conversion()
{
    const auto quarter_turn = xer::from_radian<double>(xer::pi_v<double> / 2.0);

    xer_assert(std::abs(quarter_turn.value() - 0.25) < 1e-12);
    xer_assert(std::abs(xer::to_radian(quarter_turn) - (xer::pi_v<double> / 2.0)) < 1e-12);
    xer_assert(std::abs(xer::𝜋<double> - xer::pi_v<double>) < 1e-18);
}

void test_value_type_alias()
{
    static_assert(std::is_same_v<xer::cyclic<float>::value_type, float>);
}

} // namespace

int main()
{
    test_default_constructor();
    test_normalize_positive_wrap();
    test_normalize_negative_wrap();
    test_cw_and_ccw();
    test_diff_positive();
    test_diff_negative();
    test_diff_half_turn_uses_negative_side();
    test_eq_around_zero_boundary();
    test_eq_with_explicit_epsilon();
    test_addition_and_subtraction();
    test_compound_assignment();
    test_unary_minus();
    test_degree_conversion();
    test_radian_conversion();
    test_value_type_alias();

    return 0;
}
