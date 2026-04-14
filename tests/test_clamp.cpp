#include <limits>

#include <xer/arithmetic.h>
#include <xer/assert.h>
#include <xer/error.h>

namespace {

void test_clamp_returns_value_when_in_range()
{
    const auto result = xer::clamp(5, 1, 10);
    xer_assert(result.has_value());
    xer_assert_eq(*result, 5);
}

void test_clamp_returns_low_when_value_is_below_range()
{
    const auto result = xer::clamp(0, 1, 10);
    xer_assert(result.has_value());
    xer_assert_eq(*result, 1);
}

void test_clamp_returns_high_when_value_is_above_range()
{
    const auto result = xer::clamp(20, 1, 10);
    xer_assert(result.has_value());
    xer_assert_eq(*result, 10);
}

void test_clamp_accepts_mixed_types()
{
    const auto result1 = xer::clamp(5, 1.5, 10.5);
    xer_assert(result1.has_value());
    xer_assert_eq(*result1, 5);

    const auto result2 = xer::clamp(1, 1.5, 10.5);
    xer_assert(result2.has_value());
    xer_assert_eq(*result2, 1);

    const auto result3 = xer::clamp(20, 1.5, 10.5);
    xer_assert(result3.has_value());
    xer_assert_eq(*result3, 10);
}

void test_clamp_uses_first_argument_type_for_return()
{
    const auto result = xer::clamp(static_cast<short>(20), 1, 10);
    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<short>(10));
}

void test_clamp_rejects_low_greater_than_high()
{
    const auto result = xer::clamp(5, 10, 1);
    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_clamp_propagates_result_error_from_first_argument()
{
    const xer::result<int> value =
        std::unexpected(xer::make_error(xer::error_t::io_error));

    const auto result = xer::clamp(value, 1, 10);
    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::io_error);
}

void test_clamp_accepts_successful_result_first_argument()
{
    const xer::result<int> value = 20;

    const auto result = xer::clamp(value, 1, 10);
    xer_assert(result.has_value());
    xer_assert_eq(*result, 10);
}

void test_clamp_allows_out_of_range_bound_when_not_used()
{
    const auto result =
        xer::clamp(static_cast<signed char>(0), -1000, 100);
    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<signed char>(0));
}

void test_clamp_rejects_low_out_of_range_for_first_argument_type_when_low_is_used()
{
    const auto result =
        xer::clamp(static_cast<signed char>(-120), 200, 300);
    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_clamp_rejects_high_out_of_range_for_first_argument_type_when_high_is_used()
{
    const auto result =
        xer::clamp(static_cast<signed char>(120), -300, -200);
    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_clamp_floating_value()
{
    const auto result1 = xer::clamp(5.5, 1, 10);
    xer_assert(result1.has_value());
    xer_assert_eq(*result1, 5.5);

    const auto result2 = xer::clamp(-1.0, 1, 10);
    xer_assert(result2.has_value());
    xer_assert_eq(*result2, 1.0);

    const auto result3 = xer::clamp(20.0, 1, 10);
    xer_assert(result3.has_value());
    xer_assert_eq(*result3, 10.0);
}

void test_clamp_rejects_nan_value()
{
    const auto result =
        xer::clamp(std::numeric_limits<double>::quiet_NaN(), 1.0, 10.0);
    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_clamp_rejects_nan_low()
{
    const auto result =
        xer::clamp(5.0, std::numeric_limits<double>::quiet_NaN(), 10.0);
    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_clamp_rejects_nan_high()
{
    const auto result =
        xer::clamp(5.0, 1.0, std::numeric_limits<double>::quiet_NaN());
    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_clamp_accepts_equal_bounds()
{
    const auto result = xer::clamp(5, 3, 3);
    xer_assert(result.has_value());
    xer_assert_eq(*result, 3);
}

} // namespace

int main()
{
    test_clamp_returns_value_when_in_range();
    test_clamp_returns_low_when_value_is_below_range();
    test_clamp_returns_high_when_value_is_above_range();
    test_clamp_accepts_mixed_types();
    test_clamp_uses_first_argument_type_for_return();
    test_clamp_rejects_low_greater_than_high();
    test_clamp_propagates_result_error_from_first_argument();
    test_clamp_accepts_successful_result_first_argument();
    test_clamp_allows_out_of_range_bound_when_not_used();
    test_clamp_rejects_low_out_of_range_for_first_argument_type_when_low_is_used();
    test_clamp_rejects_high_out_of_range_for_first_argument_type_when_high_is_used();
    test_clamp_floating_value();
    test_clamp_rejects_nan_value();
    test_clamp_rejects_nan_low();
    test_clamp_rejects_nan_high();
    test_clamp_accepts_equal_bounds();

    return 0;
}
