#include <limits>

#include <xer/arithmetic.h>
#include <xer/assert.h>
#include <xer/error.h>

namespace {

void test_is_close_raw_values()
{
    xer_assert(xer::is_close(0.1 + 0.2, 0.3, 1e-12));
    xer_assert(xer::is_close(-1.0, -1.05, 0.05));
    xer_assert_not(xer::is_close(0.1 + 0.2, 0.31, 1e-12));
    xer_assert_not(xer::is_close(1.0, 1.0, -0.1));
    xer_assert_not(xer::is_close(
        std::numeric_limits<double>::quiet_NaN(),
        0.0,
        1e-12));
    xer_assert_not(xer::is_close(
        std::numeric_limits<double>::infinity(),
        0.0,
        1e-12));
}

void test_is_close_zero_tolerance()
{
    xer_assert(xer::is_close(1.0, 1.0, 0.0));
    xer_assert_not(xer::is_close(1.0, 1.0 + 1e-12, 0.0));
}

void test_is_close_result_values()
{
    const xer::result<double> lhs = 0.1 + 0.2;
    const xer::result<double> rhs = 0.3;

    const auto success = xer::is_close(lhs, rhs, 1e-12);
    xer_assert(success.has_value());
    xer_assert(*success);

    const xer::result<double> failure = std::unexpected(
        xer::make_error(xer::error_t::invalid_argument));

    const auto failed = xer::is_close(failure, 0.0, 1e-12);
    xer_assert_not(failed.has_value());
}

} // namespace

auto main() -> int
{
    test_is_close_raw_values();
    test_is_close_zero_tolerance();
    test_is_close_result_values();
    return 0;
}
