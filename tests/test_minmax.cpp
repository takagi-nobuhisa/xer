#include <limits>

#include <xer/arithmetic.h>
#include <xer/assert.h>
#include <xer/error.h>

namespace {

void test_max_basic()
{
    const auto result = xer::max(3, 7);
    xer_assert(result.has_value());
    xer_assert_eq(*result, 7);
}

void test_min_basic()
{
    const auto result = xer::min(3, 7);
    xer_assert(result.has_value());
    xer_assert_eq(*result, 3);
}

void test_max_equal_values()
{
    const auto result = xer::max(5, 5);
    xer_assert(result.has_value());
    xer_assert_eq(*result, 5);
}

void test_min_equal_values()
{
    const auto result = xer::min(5, 5);
    xer_assert(result.has_value());
    xer_assert_eq(*result, 5);
}

void test_max_mixed_integer_types()
{
    const auto result = xer::max(-3, 10u);
    xer_assert(result.has_value());
    xer_assert_eq(*result, 10u);
}

void test_min_mixed_integer_types_rejects_negative_result()
{
    const auto result = xer::min(-3, 10u);
    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::out_of_range);
}

void test_max_mixed_floating_types()
{
    const auto result = xer::max(3, 7.5);
    xer_assert(result.has_value());
    xer_assert_eq(*result, 7.5);
}

void test_min_mixed_floating_types()
{
    const auto result = xer::min(3, 7.5);
    xer_assert(result.has_value());
    xer_assert_eq(*result, 3.0);
}

void test_umax_basic()
{
    const auto result = xer::umax(3, 7);
    xer_assert(result.has_value());
    xer_assert_eq(*result, 7u);
}

void test_umin_basic()
{
    const auto result = xer::umin(3, 7);
    xer_assert(result.has_value());
    xer_assert_eq(*result, 3u);
}

void test_umax_mixed_integer_types()
{
    const auto result = xer::umax(-3, 10u);
    xer_assert(result.has_value());
    xer_assert_eq(*result, 10u);
}

void test_umin_rejects_negative_result()
{
    const auto result = xer::umin(-3, 10u);
    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::out_of_range);
}

void test_umax_rejects_negative_result()
{
    const auto result = xer::umax(-3, -1);
    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::out_of_range);
}

void test_max_left_result_success()
{
    const xer::result<int> lhs = 20;

    const auto result = xer::max(lhs, 10);
    xer_assert(result.has_value());
    xer_assert_eq(*result, 20);
}

void test_max_right_result_success()
{
    const xer::result<int> rhs = 20;

    const auto result = xer::max(10, rhs);
    xer_assert(result.has_value());
    xer_assert_eq(*result, 20);
}

void test_max_both_result_success()
{
    const xer::result<int> lhs = 10;
    const xer::result<unsigned int> rhs = 20u;

    const auto result = xer::max(lhs, rhs);
    xer_assert(result.has_value());
    xer_assert_eq(*result, 20u);
}

void test_min_both_result_success()
{
    const xer::result<int> lhs = 10;
    const xer::result<unsigned int> rhs = 20u;

    const auto result = xer::min(lhs, rhs);
    xer_assert(result.has_value());
    xer_assert_eq(*result, 10u);
}

void test_min_both_result_out_of_range()
{
    const xer::result<int> lhs = -10;
    const xer::result<unsigned int> rhs = 20u;

    const auto result = xer::min(lhs, rhs);
    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::out_of_range);
}

void test_umax_both_result_success()
{
    const xer::result<int> lhs = 10;
    const xer::result<unsigned int> rhs = 20u;

    const auto result = xer::umax(lhs, rhs);
    xer_assert(result.has_value());
    xer_assert_eq(*result, 20u);
}

void test_umin_both_result_success()
{
    const xer::result<int> lhs = 10;
    const xer::result<unsigned int> rhs = 20u;

    const auto result = xer::umin(lhs, rhs);
    xer_assert(result.has_value());
    xer_assert_eq(*result, 10u);
}

void test_max_propagates_left_error()
{
    const xer::result<int> lhs =
        std::unexpected(xer::make_error(xer::error_t::invalid_argument));

    const auto result = xer::max(lhs, 10);
    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_max_propagates_right_error()
{
    const xer::result<int> rhs =
        std::unexpected(xer::make_error(xer::error_t::io_error));

    const auto result = xer::max(10, rhs);
    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::io_error);
}

void test_min_propagates_left_error()
{
    const xer::result<int> lhs =
        std::unexpected(xer::make_error(xer::error_t::invalid_argument));

    const auto result = xer::min(lhs, 10);
    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_umin_propagates_right_error()
{
    const xer::result<int> rhs =
        std::unexpected(xer::make_error(xer::error_t::io_error));

    const auto result = xer::umin(10, rhs);
    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::io_error);
}

void test_max_rejects_nan()
{
    const auto result = xer::max(
        std::numeric_limits<double>::quiet_NaN(),
        1.0);
    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_min_rejects_nan()
{
    const auto result = xer::min(
        1.0,
        std::numeric_limits<double>::quiet_NaN());
    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_max_with_bool_input()
{
    const auto result = xer::max(true, 0);
    xer_assert(result.has_value());
    xer_assert_eq(*result, 1);
}

void test_min_with_bool_input()
{
    const auto result = xer::min(true, 0);
    xer_assert(result.has_value());
    xer_assert_eq(*result, 0);
}

void test_umax_with_bool_input()
{
    const auto result = xer::umax(true, 0);
    xer_assert(result.has_value());
    xer_assert_eq(*result, 1u);
}

void test_umin_with_bool_input()
{
    const auto result = xer::umin(true, 0);
    xer_assert(result.has_value());
    xer_assert_eq(*result, 0u);
}

} // namespace

int main()
{
    test_max_basic();
    test_min_basic();
    test_max_equal_values();
    test_min_equal_values();
    test_max_mixed_integer_types();
    test_min_mixed_integer_types_rejects_negative_result();
    test_max_mixed_floating_types();
    test_min_mixed_floating_types();
    test_umax_basic();
    test_umin_basic();
    test_umax_mixed_integer_types();
    test_umin_rejects_negative_result();
    test_umax_rejects_negative_result();
    test_max_left_result_success();
    test_max_right_result_success();
    test_max_both_result_success();
    test_min_both_result_success();
    test_min_both_result_out_of_range();
    test_umax_both_result_success();
    test_umin_both_result_success();
    test_max_propagates_left_error();
    test_max_propagates_right_error();
    test_min_propagates_left_error();
    test_umin_propagates_right_error();
    test_max_rejects_nan();
    test_min_rejects_nan();
    test_max_with_bool_input();
    test_min_with_bool_input();
    test_umax_with_bool_input();
    test_umin_with_bool_input();

    return 0;
}
