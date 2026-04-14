/**
 * @file tests/test_compare.cpp
 * @brief Runtime tests for xer/bits/compare.h.
 */

#include <cmath>
#include <cstdint>
#include <expected>
#include <limits>

#include <xer/assert.h>
#include <xer/bits/compare.h>
#include <xer/error.h>

namespace {

/**
 * @brief Tests equality and inequality.
 */
void test_eq_ne()
{
    xer_assert(xer::eq(1, 1u));
    xer_assert_not(xer::eq(1, 2u));

    xer_assert(xer::ne(1, 2u));
    xer_assert_not(xer::ne(1, 1u));

    xer_assert(xer::eq(-1, -1));
    xer_assert_not(xer::eq(-1, 1u));

    xer_assert(xer::eq(0, 0u));
    xer_assert_not(xer::ne(0, 0u));
}

/**
 * @brief Tests ordering comparisons for signed and unsigned integers.
 */
void test_ordering()
{
    xer_assert(xer::lt(-1, 0u));
    xer_assert(xer::lt(0, 1u));
    xer_assert(xer::lt(-10, 3u));

    xer_assert(xer::le(-1, -1));
    xer_assert(xer::le(-1, 0u));
    xer_assert(xer::le(3u, 3));

    xer_assert(xer::gt(1u, -1));
    xer_assert(xer::gt(10u, 3));
    xer_assert(xer::gt(0, -1));

    xer_assert(xer::ge(-1, -1));
    xer_assert(xer::ge(0u, 0));
    xer_assert(xer::ge(10u, 3));
}

/**
 * @brief Tests forwarding from narrow integer types.
 */
void test_narrow_integer_forwarding()
{
    const signed char sc = -1;
    const unsigned char uc = 1;
    const short ss = -10;
    const unsigned short us = 10;

    xer_assert(xer::lt(sc, uc));
    xer_assert(xer::gt(us, ss));
    xer_assert(xer::eq(static_cast<unsigned char>(0), static_cast<short>(0)));
    xer_assert(xer::le(
        static_cast<signed char>(-5),
        static_cast<unsigned short>(0)));
    xer_assert(xer::ge(
        static_cast<unsigned char>(5),
        static_cast<short>(5)));
}

/**
 * @brief Tests boundary values.
 */
void test_boundaries()
{
    xer_assert(xer::lt(std::numeric_limits<std::int64_t>::min(), 0u));
    xer_assert(xer::gt(std::numeric_limits<std::uint64_t>::max(), 0));
    xer_assert(xer::lt(
        std::numeric_limits<std::int64_t>::min(),
        std::numeric_limits<std::int64_t>::max()));
    xer_assert(xer::gt(
        std::numeric_limits<std::uint64_t>::max(),
        std::numeric_limits<std::int64_t>::max()));
    xer_assert(xer::eq(
        std::numeric_limits<std::uint64_t>::max(),
        std::numeric_limits<std::uint64_t>::max()));
}

/**
 * @brief Tests mixed comparisons between integers and floating-point numbers.
 */
void test_mixed_integer_and_floating()
{
    xer_assert(xer::eq(1, 1.0));
    xer_assert(xer::eq(1.0, 1u));
    xer_assert_not(xer::eq(1, 1.5));

    xer_assert(xer::ne(1, 1.5));
    xer_assert_not(xer::ne(2.0, 2u));

    xer_assert(xer::lt(-1, 0.0));
    xer_assert(xer::lt(1, 1.5));
    xer_assert(xer::lt(1.5, 2));
    xer_assert_not(xer::lt(2.0, 2));

    xer_assert(xer::le(2, 2.0));
    xer_assert(xer::le(2, 2.5));
    xer_assert_not(xer::le(3.0, 2));

    xer_assert(xer::gt(2, 1.5));
    xer_assert(xer::gt(2.5, 2));
    xer_assert_not(xer::gt(2.0, 2));

    xer_assert(xer::ge(2, 2.0));
    xer_assert(xer::ge(3.0, 2));
    xer_assert_not(xer::ge(1.5, 2));
}

/**
 * @brief Tests floating-point comparisons.
 */
void test_floating_point_comparisons()
{
    xer_assert(xer::eq(1.5, 1.5));
    xer_assert_not(xer::eq(1.5, 2.5));

    xer_assert(xer::ne(1.5, 2.5));
    xer_assert_not(xer::ne(1.5, 1.5));

    xer_assert(xer::lt(1.5, 2.5));
    xer_assert_not(xer::lt(2.5, 1.5));

    xer_assert(xer::le(1.5, 1.5));
    xer_assert(xer::le(1.5, 2.5));
    xer_assert_not(xer::le(2.5, 1.5));

    xer_assert(xer::gt(2.5, 1.5));
    xer_assert_not(xer::gt(1.5, 2.5));

    xer_assert(xer::ge(1.5, 1.5));
    xer_assert(xer::ge(2.5, 1.5));
    xer_assert_not(xer::ge(1.5, 2.5));
}

/**
 * @brief Tests NaN handling in comparisons.
 */
void test_nan_handling()
{
    const double nan = std::numeric_limits<double>::quiet_NaN();

    xer_assert_not(xer::eq(nan, 0.0));
    xer_assert_not(xer::eq(0.0, nan));
    xer_assert_not(xer::eq(nan, nan));

    xer_assert(xer::ne(nan, 0.0));
    xer_assert(xer::ne(0.0, nan));
    xer_assert(xer::ne(nan, nan));

    xer_assert_not(xer::lt(nan, 0.0));
    xer_assert_not(xer::lt(0.0, nan));
    xer_assert_not(xer::lt(nan, nan));

    xer_assert_not(xer::le(nan, 0.0));
    xer_assert_not(xer::le(0.0, nan));
    xer_assert_not(xer::le(nan, nan));

    xer_assert_not(xer::gt(nan, 0.0));
    xer_assert_not(xer::gt(0.0, nan));
    xer_assert_not(xer::gt(nan, nan));

    xer_assert_not(xer::ge(nan, 0.0));
    xer_assert_not(xer::ge(0.0, nan));
    xer_assert_not(xer::ge(nan, nan));
}

#if defined(__SIZEOF_INT128__)
/**
 * @brief Tests 128-bit integer operands.
 */
void test_int128_operands()
{
    const __int128 one = 1;
    const __int128 big = one << 100;
    const unsigned __int128 ubig = static_cast<unsigned __int128>(big);

    xer_assert(xer::eq(big, big));
    xer_assert(xer::lt(-big, big));
    xer_assert(xer::gt(ubig, 0));
    xer_assert(xer::lt(-1, ubig));
    xer_assert(xer::gt(ubig, static_cast<unsigned long long>(0)));
    xer_assert(xer::ge(ubig, big));
}
#endif

/**
 * @brief Tests propagation from expected operands.
 */
void test_expected_propagation()
{
    const std::expected<int, xer::error<void>> ok1 = 10;
    const std::expected<unsigned int, xer::error<void>> ok2 = 10u;
    const std::expected<int, xer::error<void>> ok3 = -1;
    const std::expected<double, xer::error<void>> ok4 = 10.0;
    const std::expected<double, xer::error<void>> ok5 = 10.5;
    const std::expected<int, xer::error<void>> ng =
        std::unexpected(xer::make_error(xer::error_t::dom));

    {
        const auto result = xer::eq(ok1, ok2);
        xer_assert(result.has_value());
        xer_assert(*result);
    }

    {
        const auto result = xer::ne(ok1, ok3);
        xer_assert(result.has_value());
        xer_assert(*result);
    }

    {
        const auto result = xer::lt(ok3, ok2);
        xer_assert(result.has_value());
        xer_assert(*result);
    }

    {
        const auto result = xer::le(ok1, ok2);
        xer_assert(result.has_value());
        xer_assert(*result);
    }

    {
        const auto result = xer::gt(ok1, ok3);
        xer_assert(result.has_value());
        xer_assert(*result);
    }

    {
        const auto result = xer::ge(ok1, ok2);
        xer_assert(result.has_value());
        xer_assert(*result);
    }

    {
        const auto result = xer::eq(ok1, ok4);
        xer_assert(result.has_value());
        xer_assert(*result);
    }

    {
        const auto result = xer::lt(ok1, ok5);
        xer_assert(result.has_value());
        xer_assert(*result);
    }

    {
        const auto result = xer::gt(ok5, ok1);
        xer_assert(result.has_value());
        xer_assert(*result);
    }

    {
        const auto result = xer::eq(ng, ok2);
        xer_assert_not(result.has_value());
        xer_assert_eq(result.error().code, xer::error_t::dom);
    }

    {
        const auto result = xer::lt(ok1, ng);
        xer_assert_not(result.has_value());
        xer_assert_eq(result.error().code, xer::error_t::dom);
    }

    {
        const auto result = xer::ge(ng, ng);
        xer_assert_not(result.has_value());
        xer_assert_eq(result.error().code, xer::error_t::dom);
    }
}

} // namespace

/**
 * @brief Program entry point.
 *
 * @return Exit status.
 */
int main()
{
    test_eq_ne();
    test_ordering();
    test_narrow_integer_forwarding();
    test_boundaries();
    test_mixed_integer_and_floating();
    test_floating_point_comparisons();
    test_nan_handling();
#if defined(__SIZEOF_INT128__)
    test_int128_operands();
#endif
    test_expected_propagation();

    return 0;
}
