#include <expected>
#include <limits>

#include <xer/assert.h>
#include <xer/bits/abs.h>
#include <xer/error.h>

namespace {

void test_abs_int()
{
    {
        auto result = xer::abs(0);
        xer_assert(result.has_value());
        xer_assert_eq(*result, 0);
    }

    {
        auto result = xer::abs(123);
        xer_assert(result.has_value());
        xer_assert_eq(*result, 123);
    }

    {
        auto result = xer::abs(-123);
        xer_assert(result.has_value());
        xer_assert_eq(*result, 123);
    }

    {
        auto result = xer::abs(std::numeric_limits<int>::min());
        xer_assert(!result.has_value());
        xer_assert_eq(result.error().code, xer::error_t::overflow_error);
    }
}

void test_abs_long()
{
    {
        auto result = xer::abs(0L);
        xer_assert(result.has_value());
        xer_assert_eq(*result, 0L);
    }

    {
        auto result = xer::abs(123L);
        xer_assert(result.has_value());
        xer_assert_eq(*result, 123L);
    }

    {
        auto result = xer::abs(-123L);
        xer_assert(result.has_value());
        xer_assert_eq(*result, 123L);
    }

    {
        auto result = xer::abs(std::numeric_limits<long>::min());
        xer_assert(!result.has_value());
        xer_assert_eq(result.error().code, xer::error_t::overflow_error);
    }
}

void test_abs_long_long()
{
    {
        auto result = xer::abs(0LL);
        xer_assert(result.has_value());
        xer_assert_eq(*result, 0LL);
    }

    {
        auto result = xer::abs(123LL);
        xer_assert(result.has_value());
        xer_assert_eq(*result, 123LL);
    }

    {
        auto result = xer::abs(-123LL);
        xer_assert(result.has_value());
        xer_assert_eq(*result, 123LL);
    }

    {
        auto result = xer::abs(std::numeric_limits<long long>::min());
        xer_assert(!result.has_value());
        xer_assert_eq(result.error().code, xer::error_t::overflow_error);
    }
}

#if defined(__SIZEOF_INT128__)
void test_abs_int128()
{
    {
        auto result = xer::abs(static_cast<__int128>(0));
        xer_assert(result.has_value());
        xer_assert_eq(*result, static_cast<__int128>(0));
    }

    {
        auto result = xer::abs(static_cast<__int128>(123));
        xer_assert(result.has_value());
        xer_assert_eq(*result, static_cast<__int128>(123));
    }

    {
        auto result = xer::abs(static_cast<__int128>(-123));
        xer_assert(result.has_value());
        xer_assert_eq(*result, static_cast<__int128>(123));
    }

    {
        constexpr __int128 min_value = std::numeric_limits<__int128>::min();
        auto result = xer::abs(min_value);
        xer_assert(!result.has_value());
        xer_assert_eq(result.error().code, xer::error_t::overflow_error);
    }
}
#endif

void test_uabs_int()
{
    {
        auto result = xer::uabs(123);
        xer_assert(result.has_value());
        xer_assert_eq(*result, static_cast<unsigned int>(123));
    }

    {
        auto result = xer::uabs(-123);
        xer_assert(result.has_value());
        xer_assert_eq(*result, static_cast<unsigned int>(123));
    }

    {
        auto result = xer::uabs(123u);
        xer_assert(result.has_value());
        xer_assert_eq(*result, 123u);
    }
}

void test_uabs_long()
{
    {
        auto result = xer::uabs(123L);
        xer_assert(result.has_value());
        xer_assert_eq(*result, static_cast<unsigned long>(123));
    }

    {
        auto result = xer::uabs(-123L);
        xer_assert(result.has_value());
        xer_assert_eq(*result, static_cast<unsigned long>(123));
    }

    {
        auto result = xer::uabs(123UL);
        xer_assert(result.has_value());
        xer_assert_eq(*result, 123UL);
    }
}

void test_uabs_long_long()
{
    {
        auto result = xer::uabs(123LL);
        xer_assert(result.has_value());
        xer_assert_eq(*result, static_cast<unsigned long long>(123));
    }

    {
        auto result = xer::uabs(-123LL);
        xer_assert(result.has_value());
        xer_assert_eq(*result, static_cast<unsigned long long>(123));
    }

    {
        auto result = xer::uabs(123ULL);
        xer_assert(result.has_value());
        xer_assert_eq(*result, 123ULL);
    }
}

#if defined(__SIZEOF_INT128__)
void test_uabs_int128()
{
    {
        auto result = xer::uabs(static_cast<__int128>(123));
        xer_assert(result.has_value());
        xer_assert_eq(*result, static_cast<unsigned __int128>(123));
    }

    {
        auto result = xer::uabs(static_cast<__int128>(-123));
        xer_assert(result.has_value());
        xer_assert_eq(*result, static_cast<unsigned __int128>(123));
    }

    {
        auto result = xer::uabs(static_cast<unsigned __int128>(123));
        xer_assert(result.has_value());
        xer_assert_eq(*result, static_cast<unsigned __int128>(123));
    }
}
#endif

void test_abs_expected()
{
    {
        std::expected<int, xer::error<void>> value = 42;
        auto result = xer::abs(value);
        xer_assert(result.has_value());
        xer_assert_eq(*result, 42);
    }

    {
        std::expected<int, xer::error<void>> value = -42;
        auto result = xer::abs(value);
        xer_assert(result.has_value());
        xer_assert_eq(*result, 42);
    }

    {
        std::expected<int, xer::error<void>> value =
            std::unexpected(xer::make_error(xer::error_t::invalid_argument));
        auto result = xer::abs(value);
        xer_assert(!result.has_value());
        xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
    }
}

void test_uabs_expected()
{
    {
        std::expected<int, xer::error<void>> value = -42;
        auto result = xer::uabs(value);
        xer_assert(result.has_value());
        xer_assert_eq(*result, 42u);
    }

    {
        std::expected<unsigned int, xer::error<void>> value = 42u;
        auto result = xer::uabs(value);
        xer_assert(result.has_value());
        xer_assert_eq(*result, 42u);
    }

    {
        std::expected<int, xer::error<void>> value =
            std::unexpected(xer::make_error(xer::error_t::invalid_argument));
        auto result = xer::uabs(value);
        xer_assert(!result.has_value());
        xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
    }
}

} // namespace

int main()
{
    test_abs_int();
    test_abs_long();
    test_abs_long_long();
#if defined(__SIZEOF_INT128__)
    test_abs_int128();
#endif

    test_uabs_int();
    test_uabs_long();
    test_uabs_long_long();
#if defined(__SIZEOF_INT128__)
    test_uabs_int128();
#endif

    test_abs_expected();
    test_uabs_expected();

    return 0;
}
