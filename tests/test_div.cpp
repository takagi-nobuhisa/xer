/**
 * @file tests/bits/test_div.cpp
 * @brief Runtime tests for xer/bits/div.h.
 */

#include <cstdint>
#include <expected>
#include <limits>

#include <xer/assert.h>
#include <xer/bits/div.h>
#include <xer/error.h>

namespace {

/**
 * @brief Tests basic signed division.
 */
void test_div_basic()
{
    {
        const auto result = xer::div(10, 3);
        xer_assert(result.has_value());
        xer_assert_eq(result->quot, 3);
        xer_assert_eq(result->rem, 1);
    }

    {
        const auto result = xer::div(-10, 3);
        xer_assert(result.has_value());
        xer_assert_eq(result->quot, -3);
        xer_assert_eq(result->rem, -1);
    }

    {
        const auto result = xer::div(10, -3);
        xer_assert(result.has_value());
        xer_assert_eq(result->quot, -3);
        xer_assert_eq(result->rem, 1);
    }

    {
        const auto result = xer::div(-10, -3);
        xer_assert(result.has_value());
        xer_assert_eq(result->quot, 3);
        xer_assert_eq(result->rem, -1);
    }

    {
        const auto result = xer::div(0, 3);
        xer_assert(result.has_value());
        xer_assert_eq(result->quot, 0);
        xer_assert_eq(result->rem, 0);
    }
}

/**
 * @brief Tests division by zero.
 */
void test_divide_by_zero()
{
    {
        const auto result = xer::div(1, 0);
        xer_assert_not(result.has_value());
        xer_assert_eq(result.error().code, xer::error_t::divide_by_zero);
    }

    {
        const auto result = xer::udiv(1, 0);
        xer_assert_not(result.has_value());
        xer_assert_eq(result.error().code, xer::error_t::divide_by_zero);
    }
}

/**
 * @brief Tests udiv with same-sign operands.
 */
void test_udiv_same_sign()
{
    {
        const auto result = xer::udiv(10, 3);
        xer_assert(result.has_value());
        xer_assert_eq(result->quot, static_cast<unsigned int>(3));
        xer_assert_eq(result->rem, static_cast<unsigned int>(1));
    }

    {
        const auto result = xer::udiv(-10, -3);
        xer_assert(result.has_value());
        xer_assert_eq(result->quot, static_cast<unsigned int>(3));
        xer_assert_eq(result->rem, static_cast<unsigned int>(1));
    }

    {
        const auto result = xer::udiv(10u, 3u);
        xer_assert(result.has_value());
        xer_assert_eq(result->quot, static_cast<unsigned int>(3));
        xer_assert_eq(result->rem, static_cast<unsigned int>(1));
    }

    {
        const auto result = xer::udiv(0, -3);
        xer_assert_not(result.has_value());
        xer_assert_eq(result.error().code, xer::error_t::dom);
    }

    {
        const auto result = xer::udiv(0, 3);
        xer_assert(result.has_value());
        xer_assert_eq(result->quot, static_cast<unsigned int>(0));
        xer_assert_eq(result->rem, static_cast<unsigned int>(0));
    }
}

/**
 * @brief Tests udiv with sign mismatch.
 */
void test_udiv_sign_mismatch()
{
    {
        const auto result = xer::udiv(-10, 3);
        xer_assert_not(result.has_value());
        xer_assert_eq(result.error().code, xer::error_t::dom);
    }

    {
        const auto result = xer::udiv(10, -3);
        xer_assert_not(result.has_value());
        xer_assert_eq(result.error().code, xer::error_t::dom);
    }

    {
        const auto result = xer::udiv(-1, 1u);
        xer_assert_not(result.has_value());
        xer_assert_eq(result.error().code, xer::error_t::dom);
    }
}

/**
 * @brief Tests forwarding from narrow integer types.
 */
void test_narrow_integer_forwarding()
{
    {
        const signed char lhs = 10;
        const unsigned char rhs = 3;
        const auto result = xer::div(lhs, rhs);
        xer_assert(result.has_value());
        xer_assert_eq(result->quot, 3);
        xer_assert_eq(result->rem, 1);
    }

    {
        const short lhs = -10;
        const short rhs = 3;
        const auto result = xer::div(lhs, rhs);
        xer_assert(result.has_value());
        xer_assert_eq(result->quot, -3);
        xer_assert_eq(result->rem, -1);
    }

    {
        const unsigned short lhs = 10;
        const unsigned short rhs = 3;
        const auto result = xer::udiv(lhs, rhs);
        xer_assert(result.has_value());
        xer_assert_eq(result->quot, static_cast<unsigned int>(3));
        xer_assert_eq(result->rem, static_cast<unsigned int>(1));
    }
}

/**
 * @brief Tests propagation from expected operands.
 */
void test_expected_propagation()
{
    const std::expected<int, xer::error<void>> ok1 = 10;
    const std::expected<unsigned int, xer::error<void>> ok2 = 3u;
    const std::expected<int, xer::error<void>> ng =
        std::unexpected(xer::make_error(xer::error_t::dom));

    {
        const auto result = xer::div(ok1, ok2);
        xer_assert(result.has_value());
        xer_assert_eq(result->quot, 3);
        xer_assert_eq(result->rem, 1);
    }

    {
        const auto result = xer::div(ok1, ng);
        xer_assert_not(result.has_value());
        xer_assert_eq(result.error().code, xer::error_t::dom);
    }

    {
        const auto result = xer::udiv(ng, ok2);
        xer_assert_not(result.has_value());
        xer_assert_eq(result.error().code, xer::error_t::dom);
    }
}

#if defined(__SIZEOF_INT128__)
/**
 * @brief Tests 128-bit integer operands.
 */
void test_int128_operands()
{
    const __int128 one = 1;
    const __int128 big = (one << 62) + 5;
    const unsigned __int128 ubig = static_cast<unsigned __int128>(big);

    {
        const auto result = xer::div(big, 4);
        xer_assert(result.has_value());
        xer_assert_eq(
            result->quot,
            static_cast<xer::int128_t>((one << 60) + 1));
        xer_assert_eq(result->rem, static_cast<xer::int128_t>(1));
    }

    {
        const auto result = xer::div(-big, 4);
        xer_assert(result.has_value());
        xer_assert_eq(
            result->quot,
            -static_cast<xer::int128_t>((one << 60) + 1));
        xer_assert_eq(result->rem, static_cast<xer::int128_t>(-1));
    }

    {
        const auto result = xer::udiv(ubig, 4u);
        xer_assert(result.has_value());
        xer_assert_eq(
            result->quot,
            static_cast<xer::uint128_t>((one << 60) + 1));
        xer_assert_eq(result->rem, static_cast<xer::uint128_t>(1));
    }

    {
        const auto result = xer::udiv(-big, -4);
        xer_assert(result.has_value());
        xer_assert_eq(
            result->quot,
            static_cast<xer::uint128_t>((one << 60) + 1));
        xer_assert_eq(result->rem, static_cast<xer::uint128_t>(1));
    }
}
#endif

/**
 * @brief Tests type aliases.
 */
void test_type_aliases()
{
    {
        xer::div_t value{.rem = 1, .quot = 3};
        xer_assert_eq(value.rem, 1);
        xer_assert_eq(value.quot, 3);
    }

    {
        xer::i32div_t value{.rem = INT32_C(-1), .quot = INT32_C(3)};
        xer_assert_eq(value.rem, INT32_C(-1));
        xer_assert_eq(value.quot, INT32_C(3));
    }

    {
        xer::u64div_t value{.rem = UINT64_C(1), .quot = UINT64_C(3)};
        xer_assert_eq(value.rem, UINT64_C(1));
        xer_assert_eq(value.quot, UINT64_C(3));
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
    test_div_basic();
    test_divide_by_zero();
    test_udiv_same_sign();
    test_udiv_sign_mismatch();
    test_narrow_integer_forwarding();
    test_expected_propagation();
#if defined(__SIZEOF_INT128__)
    test_int128_operands();
#endif
    test_type_aliases();

    return 0;
}
