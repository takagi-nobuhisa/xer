/**
 * @file tests/bits/test_floating_arithmetic.cpp
 * @brief Runtime tests for xer/bits/floating_arithmetic.h.
 */

#include <cmath>
#include <expected>
#include <limits>

#include <xer/assert.h>
#include <xer/bits/floating_arithmetic.h>
#include <xer/error.h>

namespace {

/**
 * @brief Compares two long double values with a tolerance.
 *
 * @param lhs Left operand.
 * @param rhs Right operand.
 * @param epsilon Allowed absolute error.
 * @return `true` if the values are close enough.
 */
[[nodiscard]] bool almost_equal(
    long double lhs,
    long double rhs,
    long double epsilon = 1.0e-12L) noexcept
{
    const long double diff = lhs - rhs;
    return diff < 0 ? -diff <= epsilon : diff <= epsilon;
}

/**
 * @brief Tests floating-point addition.
 */
void test_add()
{
    const auto r1 = xer::add(1.5, 2.25);
    xer_assert(r1.has_value());
    xer_assert(almost_equal(*r1, 3.75L));

    const auto r2 = xer::add(1.5f, 2);
    xer_assert(r2.has_value());
    xer_assert(almost_equal(*r2, 3.5L));

    const auto r3 = xer::add(-0.5, 0.25f);
    xer_assert(r3.has_value());
    xer_assert(almost_equal(*r3, -0.25L));
}

/**
 * @brief Tests floating-point subtraction.
 */
void test_sub()
{
    const auto r1 = xer::sub(5.5, 2.25);
    xer_assert(r1.has_value());
    xer_assert(almost_equal(*r1, 3.25L));

    const auto r2 = xer::sub(2, 3.5);
    xer_assert(r2.has_value());
    xer_assert(almost_equal(*r2, -1.5L));

    const auto r3 = xer::sub(-1.25f, -0.25);
    xer_assert(r3.has_value());
    xer_assert(almost_equal(*r3, -1.0L));
}

/**
 * @brief Tests floating-point multiplication.
 */
void test_mul()
{
    const auto r1 = xer::mul(1.5, 2.0);
    xer_assert(r1.has_value());
    xer_assert(almost_equal(*r1, 3.0L));

    const auto r2 = xer::mul(-2.0, 0.5f);
    xer_assert(r2.has_value());
    xer_assert(almost_equal(*r2, -1.0L));

    const auto r3 = xer::mul(3, 0.25);
    xer_assert(r3.has_value());
    xer_assert(almost_equal(*r3, 0.75L));
}

/**
 * @brief Tests floating-point division without remainder.
 */
void test_div_without_remainder()
{
    const auto r1 = xer::div(7.5, 2.0);
    xer_assert(r1.has_value());
    xer_assert(almost_equal(*r1, 3.75L));

    const auto r2 = xer::div(3, 2.0);
    xer_assert(r2.has_value());
    xer_assert(almost_equal(*r2, 1.5L));

    const auto r3 = xer::div(-9.0f, 2);
    xer_assert(r3.has_value());
    xer_assert(almost_equal(*r3, -4.5L));
}

/**
 * @brief Tests floating-point division with remainder.
 *
 * Quotient should be trunc(lhs / rhs), and remainder should be
 * lhs - rhs * quotient.
 */
void test_div_with_remainder()
{
    long double rem1 = 0;
    const auto r1 = xer::div(7.5, 2.0, &rem1);
    xer_assert(r1.has_value());
    xer_assert(almost_equal(*r1, 3.0L));
    xer_assert(almost_equal(rem1, 1.5L));

    long double rem2 = 0;
    const auto r2 = xer::div(-7.5, 2.0, &rem2);
    xer_assert(r2.has_value());
    xer_assert(almost_equal(*r2, -3.0L));
    xer_assert(almost_equal(rem2, -1.5L));

    long double rem3 = 0;
    const auto r3 = xer::div(7.5, -2.0, &rem3);
    xer_assert(r3.has_value());
    xer_assert(almost_equal(*r3, -3.0L));
    xer_assert(almost_equal(rem3, 1.5L));

    long double rem4 = 0;
    const auto r4 = xer::div(-7.5, -2.0, &rem4);
    xer_assert(r4.has_value());
    xer_assert(almost_equal(*r4, 3.0L));
    xer_assert(almost_equal(rem4, -1.5L));

    const auto r5 = xer::div(7.5, 2.0, nullptr);
    xer_assert(r5.has_value());
    xer_assert(almost_equal(*r5, 3.0L));
}

/**
 * @brief Tests floating-point modulo.
 */
void test_mod()
{
    const auto r1 = xer::mod(7.5, 2.0);
    xer_assert(r1.has_value());
    xer_assert(almost_equal(*r1, 1.5L));

    const auto r2 = xer::mod(-7.5, 2.0);
    xer_assert(r2.has_value());
    xer_assert(almost_equal(*r2, -1.5L));

    const auto r3 = xer::mod(7.5, -2.0);
    xer_assert(r3.has_value());
    xer_assert(almost_equal(*r3, 1.5L));

    const auto r4 = xer::mod(-7.5, -2.0);
    xer_assert(r4.has_value());
    xer_assert(almost_equal(*r4, -1.5L));
}

/**
 * @brief Tests division by zero and modulo by zero.
 */
void test_zero_division()
{
    const auto r1 = xer::div(1.0, 0.0);
    xer_assert_not(r1.has_value());

    long double rem = 0;
    const auto r2 = xer::div(1.0, 0.0, &rem);
    xer_assert_not(r2.has_value());

    const auto r3 = xer::mod(1.0, 0.0);
    xer_assert_not(r3.has_value());
}

/**
 * @brief Tests NaN handling.
 */
void test_nan()
{
    const double nan = std::numeric_limits<double>::quiet_NaN();

    const auto r1 = xer::add(nan, 1.0);
    xer_assert_not(r1.has_value());

    const auto r2 = xer::sub(1.0, nan);
    xer_assert_not(r2.has_value());

    const auto r3 = xer::mul(nan, nan);
    xer_assert_not(r3.has_value());

    const auto r4 = xer::div(nan, 1.0);
    xer_assert_not(r4.has_value());

    const auto r5 = xer::mod(1.0, nan);
    xer_assert_not(r5.has_value());
}

/**
 * @brief Tests infinity handling.
 */
void test_infinity()
{
    const double inf = std::numeric_limits<double>::infinity();

    const auto r1 = xer::add(inf, 1.0);
    xer_assert_not(r1.has_value());

    const auto r2 = xer::sub(1.0, inf);
    xer_assert_not(r2.has_value());

    const auto r3 = xer::mul(inf, 2.0);
    xer_assert_not(r3.has_value());

    const auto r4 = xer::div(inf, 2.0);
    xer_assert_not(r4.has_value());

    const auto r5 = xer::mod(2.0, inf);
    xer_assert_not(r5.has_value());
}

/**
 * @brief Tests propagation from expected operands.
 */
void test_expected_propagation()
{
    const std::expected<double, xer::error<void>> ok1 = 7.5;
    const std::expected<float, xer::error<void>> ok2 = 2.0f;
    const std::expected<double, xer::error<void>> ng =
        std::unexpected(xer::make_error(xer::error_t::dom));

    const auto r1 = xer::add(ok1, ok2);
    xer_assert(r1.has_value());
    xer_assert(almost_equal(*r1, 9.5L));

    const auto r2 = xer::sub(ok1, ng);
    xer_assert_not(r2.has_value());

    const auto r3 = xer::mul(ng, ok2);
    xer_assert_not(r3.has_value());

    long double rem = 0;
    const auto r4 = xer::div(ok1, ok2, &rem);
    xer_assert(r4.has_value());
    xer_assert(almost_equal(*r4, 3.0L));
    xer_assert(almost_equal(rem, 1.5L));

    const auto r5 = xer::mod(ok1, ok2);
    xer_assert(r5.has_value());
    xer_assert(almost_equal(*r5, 1.5L));
}

/**
 * @brief Tests mixed integer and floating-point operands.
 */
void test_mixed_operands()
{
    const auto r1 = xer::add(2, 0.5);
    xer_assert(r1.has_value());
    xer_assert(almost_equal(*r1, 2.5L));

    const auto r2 = xer::sub(5u, 1.25f);
    xer_assert(r2.has_value());
    xer_assert(almost_equal(*r2, 3.75L));

    const auto r3 = xer::mul(-4, 0.25);
    xer_assert(r3.has_value());
    xer_assert(almost_equal(*r3, -1.0L));

    const auto r4 = xer::div(7, 2.0);
    xer_assert(r4.has_value());
    xer_assert(almost_equal(*r4, 3.5L));

    const auto r5 = xer::mod(7, 2.5);
    xer_assert(r5.has_value());
    xer_assert(almost_equal(*r5, 2.0L));
}

} // namespace

/**
 * @brief Program entry point.
 *
 * @return Exit status.
 */
int main()
{
    test_add();
    test_sub();
    test_mul();
    test_div_without_remainder();
    test_div_with_remainder();
    test_mod();
    test_zero_division();
    test_nan();
    test_infinity();
    test_expected_propagation();
    test_mixed_operands();

    return 0;
}
