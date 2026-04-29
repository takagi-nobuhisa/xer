#include <cmath>
#include <exception>
#include <limits>
#include <stdexcept>
#include <type_traits>

#include <xer/interval.h>

namespace {

auto near(float left, float right) -> bool
{
    return std::abs(left - right) < 1e-6f;
}

auto test_default_constructor() -> bool
{
    constexpr xer::interval<float> value;
    return value.value() == 0.0f;
}

auto test_custom_default_constructor() -> bool
{
    constexpr xer::interval<float, -1.0f, 1.0f> value;
    return value.value() == -1.0f;
}

auto test_construct_inside_range() -> bool
{
    constexpr xer::interval<float> value(0.5f);
    return value.value() == 0.5f;
}

auto test_construct_lower_clamp() -> bool
{
    constexpr xer::interval<float> value(-0.25f);
    return value.value() == 0.0f;
}

auto test_construct_upper_clamp() -> bool
{
    constexpr xer::interval<float> value(1.25f);
    return value.value() == 1.0f;
}

auto test_construct_nan_throws() -> bool
{
    try {
        static_cast<void>(xer::interval<float>(
            std::numeric_limits<float>::quiet_NaN()));
    } catch (const std::domain_error&) {
        return true;
    } catch (...) {
        return false;
    }

    return false;
}

auto test_construct_positive_infinity_throws() -> bool
{
    try {
        static_cast<void>(xer::interval<float>(
            std::numeric_limits<float>::infinity()));
    } catch (const std::domain_error&) {
        return true;
    } catch (...) {
        return false;
    }

    return false;
}

auto test_construct_negative_infinity_throws() -> bool
{
    try {
        static_cast<void>(xer::interval<float>(
            -std::numeric_limits<float>::infinity()));
    } catch (const std::domain_error&) {
        return true;
    } catch (...) {
        return false;
    }

    return false;
}

auto test_assign_member() -> bool
{
    xer::interval<float> value(0.5f);

    value.assign(1.25f);
    if (value.value() != 1.0f) {
        return false;
    }

    value.assign(-0.25f);
    return value.value() == 0.0f;
}

auto test_assign_operator() -> bool
{
    xer::interval<float> value;

    value = 0.75f;
    if (value.value() != 0.75f) {
        return false;
    }

    value = 2.0;
    return value.value() == 1.0f;
}

auto test_assignment_nan_throws() -> bool
{
    xer::interval<float> value;

    try {
        value = std::numeric_limits<float>::quiet_NaN();
    } catch (const std::domain_error&) {
        return true;
    } catch (...) {
        return false;
    }

    return false;
}

auto test_ratio() -> bool
{
    constexpr xer::interval<float, 10.0f, 20.0f> min_value(10.0f);
    constexpr xer::interval<float, 10.0f, 20.0f> mid_value(15.0f);
    constexpr xer::interval<float, 10.0f, 20.0f> max_value(20.0f);

    return min_value.ratio() == 0.0f &&
           mid_value.ratio() == 0.5f &&
           max_value.ratio() == 1.0f;
}

auto test_from_ratio() -> bool
{
    using gain = xer::interval<float, -1.0f, 1.0f>;

    const auto min_value = gain::from_ratio(-1.0f);
    const auto mid_value = gain::from_ratio(0.5f);
    const auto max_value = gain::from_ratio(2.0f);

    return min_value.value() == -1.0f &&
           mid_value.value() == 0.0f &&
           max_value.value() == 1.0f;
}

auto test_from_ratio_nan_throws() -> bool
{
    using value_type = xer::interval<float, 10.0f, 20.0f>;

    try {
        static_cast<void>(value_type::from_ratio(
            std::numeric_limits<float>::quiet_NaN()));
    } catch (const std::domain_error&) {
        return true;
    } catch (...) {
        return false;
    }

    return false;
}

auto test_cyclic_conversion() -> bool
{
    using level = xer::interval<float, 10.0f, 20.0f>;

    const auto min_value = xer::to_cyclic(level(10.0f));
    const auto mid_value = xer::to_cyclic(level(15.0f));
    const auto max_value = xer::to_cyclic(level(20.0f));
    const auto interval_value = xer::to_interval(xer::cyclic<float>(0.25f));
    const auto custom_value = level::from_ratio(xer::cyclic<float>(0.25f).ratio());

    return min_value.value() == 0.0f &&
           mid_value.value() == 0.5f &&
           max_value.value() == 0.0f &&
           interval_value.value() == 0.25f &&
           custom_value.value() == 12.5f;
}

auto test_comparison() -> bool
{
    const xer::interval<float> left(0.25f);
    const xer::interval<float> same(0.25f);
    const xer::interval<float> right(0.75f);

    return left == same &&
           left != right &&
           left < right &&
           left <= same &&
           right > left &&
           right >= same;
}

auto test_interval_arithmetic() -> bool
{
    const xer::interval<float> left(0.8f);
    const xer::interval<float> right(0.5f);

    const auto sum = left + right;
    const auto difference = right - left;
    const auto product = left * right;
    const auto quotient = right / xer::interval<float>(0.25f);

    return sum.value() == 1.0f &&
           difference.value() == 0.0f &&
           near(product.value(), 0.4f) &&
           quotient.value() == 1.0f;
}

auto test_interval_division_by_zero_throws() -> bool
{
    const xer::interval<float> value(0.5f);
    const xer::interval<float> zero(0.0f);

    try {
        static_cast<void>(value / zero);
    } catch (const std::domain_error&) {
        return true;
    } catch (...) {
        return false;
    }

    return false;
}

auto test_scalar_arithmetic() -> bool
{
    const xer::interval<float> value(0.5f);

    const auto sum = value + 0.25;
    const auto difference = value - 1.0;
    const auto product = value * 3.0;
    const auto quotient = value / 2.0;
    const auto left_product = 0.5 * value;

    return sum.value() == 0.75f &&
           difference.value() == 0.0f &&
           product.value() == 1.0f &&
           quotient.value() == 0.25f &&
           left_product.value() == 0.25f;
}

auto test_scalar_division_by_zero_throws() -> bool
{
    const xer::interval<float> value(0.5f);

    try {
        static_cast<void>(value / 0.0f);
    } catch (const std::domain_error&) {
        return true;
    } catch (...) {
        return false;
    }

    return false;
}

auto test_scalar_nan_throws() -> bool
{
    const xer::interval<float> value(0.5f);

    try {
        static_cast<void>(
            value + std::numeric_limits<float>::quiet_NaN());
    } catch (const std::domain_error&) {
        return true;
    } catch (...) {
        return false;
    }

    return false;
}

auto test_compound_assignment_with_interval() -> bool
{
    xer::interval<float> value(0.5f);

    value += xer::interval<float>(0.25f);
    if (value.value() != 0.75f) {
        return false;
    }

    value -= xer::interval<float>(1.0f);
    if (value.value() != 0.0f) {
        return false;
    }

    value = 0.5f;
    value *= xer::interval<float>(0.5f);
    if (value.value() != 0.25f) {
        return false;
    }

    value /= xer::interval<float>(0.5f);
    return value.value() == 0.5f;
}

auto test_compound_assignment_with_scalar() -> bool
{
    xer::interval<float> value(0.5f);

    value += 0.25;
    if (value.value() != 0.75f) {
        return false;
    }

    value -= 1.0;
    if (value.value() != 0.0f) {
        return false;
    }

    value = 0.5f;
    value *= 3.0;
    if (value.value() != 1.0f) {
        return false;
    }

    value /= 4.0;
    return value.value() == 0.25f;
}

auto test_unary_operators() -> bool
{
    const xer::interval<float> value(0.25f);

    return (+value).value() == 0.25f &&
           (-value).value() == 0.0f;
}

auto test_custom_interval_arithmetic() -> bool
{
    using gain = xer::interval<float, -1.0f, 1.0f>;

    const auto value = gain(0.25f) - 0.5f;
    const auto scaled = value * 8.0f;
    const auto negated = -gain(0.25f);

    return value.value() == -0.25f &&
           scaled.value() == -1.0f &&
           negated.value() == -0.25f;
}

auto test_value_type_and_bounds() -> bool
{
    using value_type = xer::interval<float, -1.0f, 1.0f>;

    static_assert(std::is_same_v<value_type::value_type, float>);
    static_assert(value_type::min_value == -1.0f);
    static_assert(value_type::max_value == 1.0f);

    return true;
}

} // namespace

auto main() -> int
{
    if (!test_default_constructor()) {
        return 1;
    }
    if (!test_custom_default_constructor()) {
        return 1;
    }
    if (!test_construct_inside_range()) {
        return 1;
    }
    if (!test_construct_lower_clamp()) {
        return 1;
    }
    if (!test_construct_upper_clamp()) {
        return 1;
    }
    if (!test_construct_nan_throws()) {
        return 1;
    }
    if (!test_construct_positive_infinity_throws()) {
        return 1;
    }
    if (!test_construct_negative_infinity_throws()) {
        return 1;
    }
    if (!test_assign_member()) {
        return 1;
    }
    if (!test_assign_operator()) {
        return 1;
    }
    if (!test_assignment_nan_throws()) {
        return 1;
    }
    if (!test_ratio()) {
        return 1;
    }
    if (!test_from_ratio()) {
        return 1;
    }
    if (!test_from_ratio_nan_throws()) {
        return 1;
    }
    if (!test_cyclic_conversion()) {
        return 1;
    }
    if (!test_comparison()) {
        return 1;
    }
    if (!test_interval_arithmetic()) {
        return 1;
    }
    if (!test_interval_division_by_zero_throws()) {
        return 1;
    }
    if (!test_scalar_arithmetic()) {
        return 1;
    }
    if (!test_scalar_division_by_zero_throws()) {
        return 1;
    }
    if (!test_scalar_nan_throws()) {
        return 1;
    }
    if (!test_compound_assignment_with_interval()) {
        return 1;
    }
    if (!test_compound_assignment_with_scalar()) {
        return 1;
    }
    if (!test_unary_operators()) {
        return 1;
    }
    if (!test_custom_interval_arithmetic()) {
        return 1;
    }
    if (!test_value_type_and_bounds()) {
        return 1;
    }

    return 0;
}
