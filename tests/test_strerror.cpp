/**
 * @file tests/test_strerror.cpp
 * @brief Tests for xer/bits/strerror.h.
 */

#include <string_view>

#include <xer/assert.h>
#include <xer/bits/strerror.h>
#include <xer/error.h>

namespace {

/**
 * @brief Tests strerror() for errno-compatible positive codes.
 */
void test_strerror_positive() {
    {
        const auto result = xer::strerror(xer::error_t::dom);
        xer_assert(result.has_value());
        xer_assert_eq(result.value(), std::u8string_view(u8"Numerical argument out of domain"));
    }

    {
        const auto result = xer::strerror(xer::error_t::range);
        xer_assert(result.has_value());
        xer_assert_eq(result.value(), std::u8string_view(u8"Numerical result out of range"));
    }

    {
        const auto result = xer::strerror(xer::error_t::inval);
        xer_assert(result.has_value());
        xer_assert_eq(result.value(), std::u8string_view(u8"Invalid argument"));
    }
}

/**
 * @brief Tests strerror() for xer-specific negative codes.
 */
void test_strerror_negative() {
    {
        const auto result = xer::strerror(xer::error_t::not_found);
        xer_assert(result.has_value());
        xer_assert_eq(result.value(), std::u8string_view(u8"not found"));
    }

    {
        const auto result = xer::strerror(xer::error_t::divide_by_zero);
        xer_assert(result.has_value());
        xer_assert_eq(result.value(), std::u8string_view(u8"divide by zero"));
    }

    {
        const auto result = xer::strerror(xer::error_t::invalid_argument);
        xer_assert(result.has_value());
        xer_assert_eq(result.value(), std::u8string_view(u8"invalid argument"));
    }

    {
        const auto result = xer::strerror(xer::error_t::io_error);
        xer_assert(result.has_value());
        xer_assert_eq(result.value(), std::u8string_view(u8"io error"));
    }

    {
        const auto result = xer::strerror(xer::error_t::encoding_error);
        xer_assert(result.has_value());
        xer_assert_eq(result.value(), std::u8string_view(u8"encoding error"));
    }
}

/**
 * @brief Tests strerror() for zero.
 */
void test_strerror_zero() {
    const auto result = xer::strerror(static_cast<xer::error_t>(0));
    xer_assert(result.has_value());
    xer_assert_eq(result.value(), std::u8string_view(u8"Undefined error: 0"));
}

/**
 * @brief Tests get_error_name() for positive codes.
 */
void test_get_error_name_positive() {
    {
        const auto result = xer::get_error_name(xer::error_t::dom);
        xer_assert(result.has_value());
        xer_assert_eq(result.value(), std::u8string_view(u8"dom"));
    }

    {
        const auto result = xer::get_error_name(xer::error_t::range);
        xer_assert(result.has_value());
        xer_assert_eq(result.value(), std::u8string_view(u8"range"));
    }

    {
        const auto result = xer::get_error_name(xer::error_t::inval);
        xer_assert(result.has_value());
        xer_assert_eq(result.value(), std::u8string_view(u8"inval"));
    }
}

/**
 * @brief Tests get_error_name() for xer-specific negative codes.
 */
void test_get_error_name_negative() {
    {
        const auto result = xer::get_error_name(xer::error_t::not_found);
        xer_assert(result.has_value());
        xer_assert_eq(result.value(), std::u8string_view(u8"not_found"));
    }

    {
        const auto result = xer::get_error_name(xer::error_t::divide_by_zero);
        xer_assert(result.has_value());
        xer_assert_eq(result.value(), std::u8string_view(u8"divide_by_zero"));
    }

    {
        const auto result = xer::get_error_name(xer::error_t::invalid_argument);
        xer_assert(result.has_value());
        xer_assert_eq(result.value(), std::u8string_view(u8"invalid_argument"));
    }

    {
        const auto result = xer::get_error_name(xer::error_t::io_error);
        xer_assert(result.has_value());
        xer_assert_eq(result.value(), std::u8string_view(u8"io_error"));
    }

    {
        const auto result = xer::get_error_name(xer::error_t::encoding_error);
        xer_assert(result.has_value());
        xer_assert_eq(result.value(), std::u8string_view(u8"encoding_error"));
    }
}

/**
 * @brief Tests get_error_name() for zero.
 */
void test_get_error_name_zero() {
    const auto result = xer::get_error_name(static_cast<xer::error_t>(0));
    xer_assert(result.has_value());
    xer_assert_eq(result.value(), std::u8string_view(u8"0"));
}

/**
 * @brief Tests get_errno_name() for positive errno-compatible codes.
 */
void test_get_errno_name_positive() {
    {
        const auto result = xer::get_errno_name(xer::error_t::dom);
        xer_assert(result.has_value());
        xer_assert_eq(result.value(), std::u8string_view(u8"EDOM"));
    }

    {
        const auto result = xer::get_errno_name(xer::error_t::range);
        xer_assert(result.has_value());
        xer_assert_eq(result.value(), std::u8string_view(u8"ERANGE"));
    }

    {
        const auto result = xer::get_errno_name(xer::error_t::inval);
        xer_assert(result.has_value());
        xer_assert_eq(result.value(), std::u8string_view(u8"EINVAL"));
    }
}

/**
 * @brief Tests get_errno_name() failure for xer-specific negative codes.
 */
void test_get_errno_name_negative() {
    {
        const auto result = xer::get_errno_name(xer::error_t::not_found);
        xer_assert(!result.has_value());
        xer_assert_eq(result.error().code, xer::error_t::not_found);
    }

    {
        const auto result = xer::get_errno_name(xer::error_t::divide_by_zero);
        xer_assert(!result.has_value());
        xer_assert_eq(result.error().code, xer::error_t::not_found);
    }

    {
        const auto result = xer::get_errno_name(xer::error_t::invalid_argument);
        xer_assert(!result.has_value());
        xer_assert_eq(result.error().code, xer::error_t::not_found);
    }

    {
        const auto result = xer::get_errno_name(xer::error_t::io_error);
        xer_assert(!result.has_value());
        xer_assert_eq(result.error().code, xer::error_t::not_found);
    }

    {
        const auto result = xer::get_errno_name(xer::error_t::encoding_error);
        xer_assert(!result.has_value());
        xer_assert_eq(result.error().code, xer::error_t::not_found);
    }
}

/**
 * @brief Tests get_errno_name() for zero.
 */
void test_get_errno_name_zero() {
    const auto result = xer::get_errno_name(static_cast<xer::error_t>(0));
    xer_assert(result.has_value());
    xer_assert_eq(result.value(), std::u8string_view(u8"0"));
}

} // namespace

/**
 * @brief Entry point of the test program.
 *
 * @return 0 on success.
 */
int main() {
    test_strerror_positive();
    test_strerror_negative();
    test_strerror_zero();

    test_get_error_name_positive();
    test_get_error_name_negative();
    test_get_error_name_zero();

    test_get_errno_name_positive();
    test_get_errno_name_negative();
    test_get_errno_name_zero();

    return 0;
}
