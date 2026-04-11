/**
 * @file tests/bits/test_basic_arithmetic.cpp
 * @brief Runtime tests for xer/bits/basic_arithmetic.h.
 */

#include <cstdint>
#include <expected>
#include <limits>

#include <xer/assert.h>
#include <xer/bits/basic_arithmetic.h>
#include <xer/error.h>

namespace {

/**
 * @brief Tests integer addition.
 */
void test_add()
{
    const auto r1 = xer::add(1, 2u);
    xer_assert(r1.has_value());
    xer_assert_eq(*r1, static_cast<std::int64_t>(3));

    const auto r2 = xer::add(std::uint64_t{10}, -3);
    xer_assert(r2.has_value());
    xer_assert_eq(*r2, static_cast<std::int64_t>(7));

    const short a = 12;
    const unsigned short b = 34;
    const auto r3 = xer::add(a, b);
    xer_assert(r3.has_value());
    xer_assert_eq(*r3, static_cast<std::int64_t>(46));

    const auto r4 = xer::add(std::numeric_limits<std::int64_t>::max(), 0);
    xer_assert(r4.has_value());
    xer_assert_eq(*r4, std::numeric_limits<std::int64_t>::max());

    const auto r5 = xer::add(std::numeric_limits<std::int64_t>::max(), 1);
    xer_assert_not(r5.has_value());

    const auto r6 = xer::add(std::numeric_limits<std::uint64_t>::max(), 0);
    xer_assert_not(r6.has_value());
}

/**
 * @brief Tests unsigned integer addition.
 */
void test_uadd()
{
    const auto r1 = xer::uadd(1, 2u);
    xer_assert(r1.has_value());
    xer_assert_eq(*r1, static_cast<std::uint64_t>(3));

    const auto r2 = xer::uadd(-1, 1u);
    xer_assert(r2.has_value());
    xer_assert_eq(*r2, static_cast<std::uint64_t>(0));

    const auto r3 = xer::uadd(-2, 1u);
    xer_assert_not(r3.has_value());

    const auto r4 = xer::uadd(std::numeric_limits<std::uint64_t>::max(), 0u);
    xer_assert(r4.has_value());
    xer_assert_eq(*r4, std::numeric_limits<std::uint64_t>::max());

    const auto r5 = xer::uadd(std::numeric_limits<std::uint64_t>::max(), 1u);
    xer_assert_not(r5.has_value());
}

/**
 * @brief Tests integer subtraction.
 */
void test_sub()
{
    const auto r1 = xer::sub(10, 3u);
    xer_assert(r1.has_value());
    xer_assert_eq(*r1, static_cast<std::int64_t>(7));

    const auto r2 = xer::sub(3, 10u);
    xer_assert(r2.has_value());
    xer_assert_eq(*r2, static_cast<std::int64_t>(-7));

    const signed char a = 100;
    const unsigned char b = 1;
    const auto r3 = xer::sub(a, b);
    xer_assert(r3.has_value());
    xer_assert_eq(*r3, static_cast<std::int64_t>(99));

    const auto r4 = xer::sub(std::numeric_limits<std::int64_t>::min(), 0);
    xer_assert(r4.has_value());
    xer_assert_eq(*r4, std::numeric_limits<std::int64_t>::min());

    const auto r5 = xer::sub(std::numeric_limits<std::int64_t>::min(), 1);
    xer_assert_not(r5.has_value());
}

/**
 * @brief Tests unsigned integer subtraction.
 */
void test_usub()
{
    const auto r1 = xer::usub(10, 3u);
    xer_assert(r1.has_value());
    xer_assert_eq(*r1, static_cast<std::uint64_t>(7));

    const auto r2 = xer::usub(3, 10u);
    xer_assert_not(r2.has_value());

    const auto r3 = xer::usub(0u, 0);
    xer_assert(r3.has_value());
    xer_assert_eq(*r3, static_cast<std::uint64_t>(0));

    const auto r4 = xer::usub(-1, -1);
    xer_assert(r4.has_value());
    xer_assert_eq(*r4, static_cast<std::uint64_t>(0));
}

/**
 * @brief Tests integer multiplication.
 */
void test_mul()
{
    const auto r1 = xer::mul(3, 4u);
    xer_assert(r1.has_value());
    xer_assert_eq(*r1, static_cast<std::int64_t>(12));

    const auto r2 = xer::mul(-3, 4u);
    xer_assert(r2.has_value());
    xer_assert_eq(*r2, static_cast<std::int64_t>(-12));

    const auto r3 = xer::mul(std::numeric_limits<std::int64_t>::min(), -1);
    xer_assert_not(r3.has_value());

    const auto r4 = xer::mul(0, std::numeric_limits<std::uint64_t>::max());
    xer_assert(r4.has_value());
    xer_assert_eq(*r4, static_cast<std::int64_t>(0));

    const auto r5 = xer::mul(std::numeric_limits<std::int64_t>::max(), 2);
    xer_assert_not(r5.has_value());
}

/**
 * @brief Tests unsigned integer multiplication.
 */
void test_umul()
{
    const auto r1 = xer::umul(3, 4);
    xer_assert(r1.has_value());
    xer_assert_eq(*r1, static_cast<std::uint64_t>(12));

    const auto r2 = xer::umul(-3, 4);
    xer_assert_not(r2.has_value());

    const auto r3 = xer::umul(0, -3);
    xer_assert(r3.has_value());
    xer_assert_eq(*r3, static_cast<std::uint64_t>(0));

    const auto r4 = xer::umul(std::numeric_limits<std::uint64_t>::max(), 2u);
    xer_assert_not(r4.has_value());
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

    const auto r1 = xer::add(ok1, ok2);
    xer_assert(r1.has_value());
    xer_assert_eq(*r1, static_cast<std::int64_t>(13));

    const auto r2 = xer::sub(ok1, ng);
    xer_assert_not(r2.has_value());

    const auto r3 = xer::mul(ng, ok2);
    xer_assert_not(r3.has_value());

    const auto r4 = xer::uadd(ok1, ok2);
    xer_assert(r4.has_value());
    xer_assert_eq(*r4, static_cast<std::uint64_t>(13));

    const auto r5 = xer::umul(ok1, ng);
    xer_assert_not(r5.has_value());
}

#if defined(__SIZEOF_INT128__)
/**
 * @brief Tests 128-bit integer operands.
 */
void test_int128_operands()
{
    const __int128 one = 1;
    const __int128 big = one << 62;
    const unsigned __int128 ubig = static_cast<unsigned __int128>(big);

    const auto r1 = xer::add(big, 1);
    xer_assert(r1.has_value());
    xer_assert_eq(*r1, static_cast<std::int64_t>((one << 62) + 1));

    const auto r2 = xer::sub(ubig, 1);
    xer_assert(r2.has_value());
    xer_assert_eq(*r2, static_cast<std::int64_t>((one << 62) - 1));

    const auto r3 = xer::add(
        static_cast<unsigned __int128>(std::numeric_limits<std::uint64_t>::max()) + 1,
        0);
    xer_assert_not(r3.has_value());

    const auto r4 = xer::umul(big, 3);
    xer_assert(r4.has_value());
    xer_assert_eq(*r4, static_cast<std::uint64_t>(3) << 62);

    const auto r5 = xer::umul((one << 63), 3);
    xer_assert_not(r5.has_value());
}
#endif

} // namespace

/**
 * @brief Program entry point.
 *
 * @return Exit status.
 */
int main()
{
    test_add();
    test_uadd();
    test_sub();
    test_usub();
    test_mul();
    test_umul();
    test_expected_propagation();

#if defined(__SIZEOF_INT128__)
    test_int128_operands();
#endif

    return 0;
}
