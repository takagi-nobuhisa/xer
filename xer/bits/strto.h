/**
 * @file xer/bits/strto.h
 * @brief Integer parsing functions based on UTF-8 character sequences.
 */

#pragma once

#ifndef XER_BITS_STRTO_H_INCLUDED_
#define XER_BITS_STRTO_H_INCLUDED_

#include <concepts>
#include <expected>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>

#include <xer/bits/common.h>
#include <xer/bits/strto_common.h>
#include <xer/error.h>

namespace xer {

namespace detail {

template<typename T>
concept strto_integer =
    std::integral<std::remove_cv_t<T>> &&
    (!std::same_as<std::remove_cv_t<T>, bool>) &&
    (sizeof(std::remove_cv_t<T>) >= sizeof(int));

struct parsed_integer_prefix {
    std::size_t digit_offset;
    int base;
};

[[nodiscard]] constexpr auto parse_integer_prefix(
    std::u8string_view str,
    std::size_t offset,
    int base) -> parsed_integer_prefix
{
    if (base != 0 && (base < 2 || base > 36)) {
        return parsed_integer_prefix{offset, -1};
    }

    std::size_t digit_offset = offset;
    int actual_base = base;

    if (digit_offset < str.size() && str[digit_offset] == u8'0') {
        if (digit_offset + 1 < str.size()) {
            const char8_t next = str[digit_offset + 1];

            if ((actual_base == 0 || actual_base == 16) &&
                (next == u8'x' || next == u8'X')) {
                actual_base = 16;
                digit_offset += 2;
            } else if ((actual_base == 0 || actual_base == 2) &&
                       (next == u8'b' || next == u8'B')) {
                actual_base = 2;
                digit_offset += 2;
            } else if (actual_base == 0) {
                actual_base = 8;
                digit_offset += 1;
            }
        } else if (actual_base == 0) {
            actual_base = 8;
            digit_offset += 1;
        }
    } else if (actual_base == 0) {
        actual_base = 10;
    }

    return parsed_integer_prefix{digit_offset, actual_base};
}

template<typename T>
struct strto_result {
    T value;
    std::size_t end_offset;
};

template<strto_integer T>
[[nodiscard]] constexpr auto parse_integer_view(
    std::u8string_view str,
    int base) -> result<strto_result<T>>
{
    using value_type = std::remove_cv_t<T>;

    std::size_t offset = 0;
    while (offset < str.size() && is_ascii_space(str[offset])) {
        ++offset;
    }

    const std::size_t sign_offset = offset;
    bool negative = false;

    if (offset < str.size()) {
        if (str[offset] == u8'+') {
            ++offset;
        } else if (str[offset] == u8'-') {
            negative = true;
            ++offset;
        }
    }

    const parsed_integer_prefix prefix = parse_integer_prefix(str, offset, base);
    if (prefix.base < 0) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    offset = prefix.digit_offset;
    const std::size_t digit_begin = offset;
    (void)digit_begin;

    using unsigned_type = std::make_unsigned_t<value_type>;
    unsigned_type limit = 0;

    if constexpr (std::is_unsigned_v<value_type>) {
        limit = std::numeric_limits<value_type>::max();
    } else if (negative) {
        limit = static_cast<unsigned_type>(std::numeric_limits<value_type>::max()) +
                static_cast<unsigned_type>(1);
    } else {
        limit = static_cast<unsigned_type>(std::numeric_limits<value_type>::max());
    }

    unsigned_type value = 0;
    bool has_digits = false;
    bool overflow = false;

    for (; offset < str.size(); ++offset) {
        const int digit = ascii_digit_value(str[offset]);
        if (digit < 0 || digit >= prefix.base) {
            break;
        }

        has_digits = true;
        const unsigned_type base_value = static_cast<unsigned_type>(prefix.base);
        const unsigned_type digit_value = static_cast<unsigned_type>(digit);

        if (value > (limit - digit_value) / base_value) {
            overflow = true;
            value = limit;
        } else {
            value = value * base_value + digit_value;
        }
    }

    if (!has_digits) {
        return strto_result<T>{static_cast<value_type>(0), sign_offset};
    }

    if (overflow) {
        return std::unexpected(make_error(error_t::range));
    }

    if constexpr (std::is_unsigned_v<value_type>) {
        if (negative) {
            return strto_result<T>{static_cast<value_type>(-value), offset};
        }

        return strto_result<T>{static_cast<value_type>(value), offset};
    } else {
        if (negative) {
            if (value == limit) {
                return strto_result<T>{std::numeric_limits<value_type>::lowest(), offset};
            }

            return strto_result<T>{static_cast<value_type>(-static_cast<value_type>(value)), offset};
        }

        return strto_result<T>{static_cast<value_type>(value), offset};
    }
}

template<strto_integer T>
[[nodiscard]] constexpr auto strto_from_view(
    std::u8string_view str,
    std::size_t* end_offset,
    int base) -> result<T>
{
    const auto parsed = parse_integer_view<T>(str, base);
    if (!parsed) {
        if (end_offset != nullptr) {
            *end_offset = 0;
        }
        return std::unexpected(parsed.error());
    }

    if (end_offset != nullptr) {
        *end_offset = parsed->end_offset;
    }

    return parsed->value;
}

template<strto_integer T>
[[nodiscard]] constexpr auto ato_from_view(std::u8string_view str) -> result<T>
{
    return strto_from_view<T>(str, nullptr, 10);
}

} // namespace detail

template<detail::strto_integer T>
[[nodiscard]] constexpr auto ato(std::u8string_view str) -> result<T>
{
    return detail::ato_from_view<T>(str);
}

template<detail::strto_integer T>
[[nodiscard]] constexpr auto ato(const std::u8string& str) -> result<T>
{
    return detail::ato_from_view<T>(std::u8string_view(str));
}

template<detail::strto_integer T>
[[nodiscard]] constexpr auto ato(const char8_t* str) -> result<T>
{
    return detail::ato_from_view<T>(std::u8string_view(str));
}

template<detail::strto_integer T>
[[nodiscard]] constexpr auto ato(char8_t* str) -> result<T>
{
    return detail::ato_from_view<T>(std::u8string_view(str));
}

template<detail::strto_integer T>
[[nodiscard]] constexpr auto strto(
    std::u8string_view str,
    std::u8string_view::const_iterator* endit = nullptr,
    int base = 0) -> result<T>
{
    std::size_t end_offset = 0;
    const auto result = detail::strto_from_view<T>(str, &end_offset, base);

    if (endit != nullptr) {
        *endit = str.begin() + static_cast<std::ptrdiff_t>(end_offset);
    }

    return result;
}

template<detail::strto_integer T>
[[nodiscard]] constexpr auto strto(
    const std::u8string& str,
    std::u8string::const_iterator* endit = nullptr,
    int base = 0) -> result<T>
{
    std::size_t end_offset = 0;
    const auto result = detail::strto_from_view<T>(std::u8string_view(str), &end_offset, base);

    if (endit != nullptr) {
        *endit = str.begin() + static_cast<std::ptrdiff_t>(end_offset);
    }

    return result;
}

template<detail::strto_integer T>
[[nodiscard]] constexpr auto strto(
    std::u8string& str,
    std::u8string::iterator* endit = nullptr,
    int base = 0) -> result<T>
{
    std::size_t end_offset = 0;
    const auto result = detail::strto_from_view<T>(std::u8string_view(str), &end_offset, base);

    if (endit != nullptr) {
        *endit = str.begin() + static_cast<std::ptrdiff_t>(end_offset);
    }

    return result;
}

template<detail::strto_integer T>
[[nodiscard]] constexpr auto strto(
    const char8_t* str,
    const char8_t** endptr = nullptr,
    int base = 0) -> result<T>
{
    const std::u8string_view view(str);
    std::size_t end_offset = 0;
    const auto result = detail::strto_from_view<T>(view, &end_offset, base);

    if (endptr != nullptr) {
        *endptr = str + static_cast<std::ptrdiff_t>(end_offset);
    }

    return result;
}

template<detail::strto_integer T>
[[nodiscard]] constexpr auto strto(
    char8_t* str,
    char8_t** endptr = nullptr,
    int base = 0) -> result<T>
{
    const std::u8string_view view(str);
    std::size_t end_offset = 0;
    const auto result = detail::strto_from_view<T>(view, &end_offset, base);

    if (endptr != nullptr) {
        *endptr = str + static_cast<std::ptrdiff_t>(end_offset);
    }

    return result;
}

[[nodiscard]] constexpr auto atoi(std::u8string_view str) -> result<int>
{
    return ato<int>(str);
}

[[nodiscard]] constexpr auto atoi(const std::u8string& str) -> result<int>
{
    return ato<int>(str);
}

[[nodiscard]] constexpr auto atoi(const char8_t* str) -> result<int>
{
    return ato<int>(str);
}

[[nodiscard]] constexpr auto atoi(char8_t* str) -> result<int>
{
    return ato<int>(str);
}

[[nodiscard]] constexpr auto atol(std::u8string_view str) -> result<long>
{
    return ato<long>(str);
}

[[nodiscard]] constexpr auto atol(const std::u8string& str) -> result<long>
{
    return ato<long>(str);
}

[[nodiscard]] constexpr auto atol(const char8_t* str) -> result<long>
{
    return ato<long>(str);
}

[[nodiscard]] constexpr auto atol(char8_t* str) -> result<long>
{
    return ato<long>(str);
}

[[nodiscard]] constexpr auto atoll(std::u8string_view str) -> result<long long>
{
    return ato<long long>(str);
}

[[nodiscard]] constexpr auto atoll(const std::u8string& str) -> result<long long>
{
    return ato<long long>(str);
}

[[nodiscard]] constexpr auto atoll(const char8_t* str) -> result<long long>
{
    return ato<long long>(str);
}

[[nodiscard]] constexpr auto atoll(char8_t* str) -> result<long long>
{
    return ato<long long>(str);
}

[[nodiscard]] constexpr auto strtol(
    std::u8string_view str,
    std::u8string_view::const_iterator* endit = nullptr,
    int base = 0) -> result<long>
{
    return strto<long>(str, endit, base);
}

[[nodiscard]] constexpr auto strtol(
    const std::u8string& str,
    std::u8string::const_iterator* endit = nullptr,
    int base = 0) -> result<long>
{
    return strto<long>(str, endit, base);
}

[[nodiscard]] constexpr auto strtol(
    std::u8string& str,
    std::u8string::iterator* endit = nullptr,
    int base = 0) -> result<long>
{
    return strto<long>(str, endit, base);
}

[[nodiscard]] constexpr auto strtol(
    const char8_t* str,
    const char8_t** endptr = nullptr,
    int base = 0) -> result<long>
{
    return strto<long>(str, endptr, base);
}

[[nodiscard]] constexpr auto strtol(
    char8_t* str,
    char8_t** endptr = nullptr,
    int base = 0) -> result<long>
{
    return strto<long>(str, endptr, base);
}

[[nodiscard]] constexpr auto strtoll(
    std::u8string_view str,
    std::u8string_view::const_iterator* endit = nullptr,
    int base = 0) -> result<long>
{
    return strto<long long>(str, endit, base);
}

[[nodiscard]] constexpr auto strtoll(
    const std::u8string& str,
    std::u8string::const_iterator* endit = nullptr,
    int base = 0) -> result<long>
{
    return strto<long long>(str, endit, base);
}

[[nodiscard]] constexpr auto strtoll(
    std::u8string& str,
    std::u8string::iterator* endit = nullptr,
    int base = 0) -> result<long>
{
    return strto<long long>(str, endit, base);
}

[[nodiscard]] constexpr auto strtoll(
    const char8_t* str,
    const char8_t** endptr = nullptr,
    int base = 0) -> result<long>
{
    return strto<long long>(str, endptr, base);
}

[[nodiscard]] constexpr auto strtoll(
    char8_t* str,
    char8_t** endptr = nullptr,
    int base = 0) -> result<long>
{
    return strto<long long>(str, endptr, base);
}

[[nodiscard]] constexpr auto strtoul(
    std::u8string_view str,
    std::u8string_view::const_iterator* endit = nullptr,
    int base = 0) -> result<long>
{
    return strto<unsigned long>(str, endit, base);
}

[[nodiscard]] constexpr auto strtoul(
    const std::u8string& str,
    std::u8string::const_iterator* endit = nullptr,
    int base = 0) -> result<long>
{
    return strto<unsigned long>(str, endit, base);
}

[[nodiscard]] constexpr auto strtoul(
    std::u8string& str,
    std::u8string::iterator* endit = nullptr,
    int base = 0) -> result<long>
{
    return strto<unsigned long>(str, endit, base);
}

[[nodiscard]] constexpr auto strtoul(
    const char8_t* str,
    const char8_t** endptr = nullptr,
    int base = 0) -> result<long>
{
    return strto<unsigned long>(str, endptr, base);
}

[[nodiscard]] constexpr auto strtoul(
    char8_t* str,
    char8_t** endptr = nullptr,
    int base = 0) -> result<long>
{
    return strto<unsigned long>(str, endptr, base);
}

[[nodiscard]] constexpr auto strtoull(
    std::u8string_view str,
    std::u8string_view::const_iterator* endit = nullptr,
    int base = 0) -> result<long>
{
    return strto<unsigned long long>(str, endit, base);
}

[[nodiscard]] constexpr auto strtoull(
    const std::u8string& str,
    std::u8string::const_iterator* endit = nullptr,
    int base = 0) -> result<long>
{
    return strto<unsigned long long>(str, endit, base);
}

[[nodiscard]] constexpr auto strtoull(
    std::u8string& str,
    std::u8string::iterator* endit = nullptr,
    int base = 0) -> result<long>
{
    return strto<unsigned long long>(str, endit, base);
}

[[nodiscard]] constexpr auto strtoull(
    const char8_t* str,
    const char8_t** endptr = nullptr,
    int base = 0) -> result<long>
{
    return strto<unsigned long long>(str, endptr, base);
}

[[nodiscard]] constexpr auto strtoull(
    char8_t* str,
    char8_t** endptr = nullptr,
    int base = 0) -> result<long>
{
    return strto<unsigned long long>(str, endptr, base);
}

} // namespace xer

#endif /* XER_BITS_STRTO_H_INCLUDED_ */
