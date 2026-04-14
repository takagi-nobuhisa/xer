#include <cmath>
#include <expected>
#include <limits>
#include <type_traits>

#include <xer/arithmetic.h>
#include <xer/assert.h>
#include <xer/error.h>

namespace {

void test_min_same_type()
{
    const auto result = xer::min(3, 5);

    xer_assert(result.has_value());
    xer_assert_eq(*result, 3);
    static_assert(std::is_same_v<
                  std::remove_cvref_t<decltype(*result)>,
                  int>);
}

void test_max_same_type()
{
    const auto result = xer::max(3, 5);

    xer_assert(result.has_value());
    xer_assert_eq(*result, 5);
    static_assert(std::is_same_v<
                  std::remove_cvref_t<decltype(*result)>,
                  int>);
}

void test_min_mixed_integer_types_success()
{
    const auto result = xer::min(3, 10u);

    xer_assert(result.has_value());
    xer_assert_eq(*result, 3);
    static_assert(std::is_same_v<
                  std::remove_cvref_t<decltype(*result)>,
                  std::common_type_t<int, unsigned int>>);
}

void test_min_mixed_integer_types_out_of_range()
{
    const auto result = xer::min(-3, 10u);

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::out_of_range);
}

void test_max_mixed_integer_types()
{
    const auto result = xer::max(-3, 10u);

    xer_assert(result.has_value());
    xer_assert_eq(*result, 10);
    static_assert(std::is_same_v<
                  std::remove_cvref_t<decltype(*result)>,
                  std::common_type_t<int, unsigned int>>);
}

void test_min_mixed_integer_and_floating()
{
    const auto result = xer::min(3, 2.5);

    xer_assert(result.has_value());
    xer_assert_eq(*result, 2.5);
    static_assert(std::is_same_v<
                  std::remove_cvref_t<decltype(*result)>,
                  std::common_type_t<int, double>>);
}

void test_max_mixed_integer_and_floating()
{
    const auto result = xer::max(3, 2.5);

    xer_assert(result.has_value());
    xer_assert_eq(*result, 3.0);
    static_assert(std::is_same_v<
                  std::remove_cvref_t<decltype(*result)>,
                  std::common_type_t<int, double>>);
}

void test_min_with_equal_values()
{
    const auto result = xer::min(5, 5u);

    xer_assert(result.has_value());
    xer_assert_eq(*result, 5);
}

void test_max_with_equal_values()
{
    const auto result = xer::max(5, 5u);

    xer_assert(result.has_value());
    xer_assert_eq(*result, 5);
}

void test_clamp_value_within_range()
{
    const auto result = xer::clamp(5, 1, 10);

    xer_assert(result.has_value());
    xer_assert_eq(*result, 5);
}

void test_clamp_value_below_range()
{
    const auto result = xer::clamp(-5, 1, 10);

    xer_assert(result.has_value());
    xer_assert_eq(*result, 1);
}

void test_clamp_value_above_range()
{
    const auto result = xer::clamp(20, 1, 10);

    xer_assert(result.has_value());
    xer_assert_eq(*result, 10);
}

void test_clamp_mixed_integer_types()
{
    const auto result = xer::clamp(-5, 1u, 10u);

    xer_assert(result.has_value());
    xer_assert_eq(*result, 1);
    static_assert(std::is_same_v<
                  std::remove_cvref_t<decltype(*result)>,
                  std::common_type_t<int, unsigned int, unsigned int>>);
}

void test_clamp_mixed_integer_and_floating()
{
    const auto result = xer::clamp(3, 1.5, 2.5);

    xer_assert(result.has_value());
    xer_assert_eq(*result, 2.5);
    static_assert(std::is_same_v<
                  std::remove_cvref_t<decltype(*result)>,
                  std::common_type_t<int, double, double>>);
}

void test_clamp_equal_bounds()
{
    const auto result = xer::clamp(100, 7, 7);

    xer_assert(result.has_value());
    xer_assert_eq(*result, 7);
}

void test_clamp_invalid_bounds()
{
    const auto result = xer::clamp(5, 10, 1);

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_min_result_left_success()
{
    const xer::result<int> lhs = 3;
    const auto result = xer::min(lhs, 5u);

    xer_assert(result.has_value());
    xer_assert_eq(*result, 3);
}

void test_min_result_right_success()
{
    const xer::result<unsigned int> rhs = 5u;
    const auto result = xer::min(3, rhs);

    xer_assert(result.has_value());
    xer_assert_eq(*result, 3);
}

void test_min_result_both_success()
{
    const xer::result<int> lhs = 3;
    const xer::result<unsigned int> rhs = 5u;
    const auto result = xer::min(lhs, rhs);

    xer_assert(result.has_value());
    xer_assert_eq(*result, 3);
}

void test_min_result_both_out_of_range()
{
    const xer::result<int> lhs = -3;
    const xer::result<unsigned int> rhs = 5u;
    const auto result = xer::min(lhs, rhs);

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::out_of_range);
}

void test_max_result_left_success()
{
    const xer::result<int> lhs = 3;
    const auto result = xer::max(lhs, 5u);

    xer_assert(result.has_value());
    xer_assert_eq(*result, 5);
}

void test_max_result_right_success()
{
    const xer::result<unsigned int> rhs = 5u;
    const auto result = xer::max(3, rhs);

    xer_assert(result.has_value());
    xer_assert_eq(*result, 5);
}

void test_max_result_both_success()
{
    const xer::result<int> lhs = -3;
    const xer::result<unsigned int> rhs = 5u;
    const auto result = xer::max(lhs, rhs);

    xer_assert(result.has_value());
    xer_assert_eq(*result, 5);
}

void test_clamp_result_value_success()
{
    const xer::result<int> value = 20;
    const auto result = xer::clamp(value, 1, 10);

    xer_assert(result.has_value());
    xer_assert_eq(*result, 10);
}

void test_clamp_result_lower_bound_success()
{
    const xer::result<int> lo = 1;
    const auto result = xer::clamp(-5, lo, 10);

    xer_assert(result.has_value());
    xer_assert_eq(*result, 1);
}

void test_clamp_result_upper_bound_success()
{
    const xer::result<int> hi = 10;
    const auto result = xer::clamp(20, 1, hi);

    xer_assert(result.has_value());
    xer_assert_eq(*result, 10);
}

void test_clamp_result_all_success()
{
    const xer::result<int> value = 20;
    const xer::result<int> lo = 1;
    const xer::result<int> hi = 10;
    const auto result = xer::clamp(value, lo, hi);

    xer_assert(result.has_value());
    xer_assert_eq(*result, 10);
}

void test_min_result_error_propagation()
{
    const xer::result<int> lhs =
        std::unexpected(xer::make_error(xer::error_t::invalid_argument));
    const auto result = xer::min(lhs, 5);

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_max_result_error_propagation()
{
    const xer::result<int> rhs =
        std::unexpected(xer::make_error(xer::error_t::io_error));
    const auto result = xer::max(5, rhs);

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::io_error);
}

void test_clamp_result_error_propagation_from_value()
{
    const xer::result<int> value =
        std::unexpected(xer::make_error(xer::error_t::invalid_argument));
    const auto result = xer::clamp(value, 1, 10);

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_clamp_result_error_propagation_from_lower_bound()
{
    const xer::result<int> lo =
        std::unexpected(xer::make_error(xer::error_t::io_error));
    const auto result = xer::clamp(5, lo, 10);

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::io_error);
}

void test_clamp_result_error_propagation_from_upper_bound()
{
    const xer::result<int> hi =
        std::unexpected(xer::make_error(xer::error_t::dom));
    const auto result = xer::clamp(5, 1, hi);

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::dom);
}

void test_clamp_invalid_bounds_with_result_operands()
{
    const xer::result<int> value = 5;
    const xer::result<int> lo = 10;
    const xer::result<int> hi = 1;
    const auto result = xer::clamp(value, lo, hi);

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_min_with_nan()
{
    const auto result = xer::min(std::numeric_limits<double>::quiet_NaN(), 1.0);

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::out_of_range);
}

void test_max_with_nan()
{
    const auto result = xer::max(std::numeric_limits<double>::quiet_NaN(), 1.0);

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::out_of_range);
}

void test_clamp_with_nan_value()
{
    const auto result =
        xer::clamp(std::numeric_limits<double>::quiet_NaN(), 1.0, 10.0);

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::out_of_range);
}

} // namespace

int main()
{
    test_min_same_type();
    test_max_same_type();
    test_min_mixed_integer_types_success();
    test_min_mixed_integer_types_out_of_range();
    test_max_mixed_integer_types();
    test_min_mixed_integer_and_floating();
    test_max_mixed_integer_and_floating();
    test_min_with_equal_values();
    test_max_with_equal_values();
    test_clamp_value_within_range();
    test_clamp_value_below_range();
    test_clamp_value_above_range();
    test_clamp_mixed_integer_types();
    test_clamp_mixed_integer_and_floating();
    test_clamp_equal_bounds();
    test_clamp_invalid_bounds();
    test_min_result_left_success();
    test_min_result_right_success();
    test_min_result_both_success();
    test_min_result_both_out_of_range();
    test_max_result_left_success();
    test_max_result_right_success();
    test_max_result_both_success();
    test_clamp_result_value_success();
    test_clamp_result_lower_bound_success();
    test_clamp_result_upper_bound_success();
    test_clamp_result_all_success();
    test_min_result_error_propagation();
    test_max_result_error_propagation();
    test_clamp_result_error_propagation_from_value();
    test_clamp_result_error_propagation_from_lower_bound();
    test_clamp_result_error_propagation_from_upper_bound();
    test_clamp_invalid_bounds_with_result_operands();
    test_min_with_nan();
    test_max_with_nan();
    test_clamp_with_nan_value();

    return 0;
}
