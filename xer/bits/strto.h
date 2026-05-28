/**
 * @file xer/bits/strto.h
 * @brief Integer parsing functions based on ASCII-compatible character sequences.
 */

#pragma once

#ifndef XER_BITS_STRTO_H_INCLUDED_
#define XER_BITS_STRTO_H_INCLUDED_

#include <concepts>
#include <cstddef>
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

template<strto_character CharT>
[[nodiscard]] constexpr auto parse_integer_prefix(
    std::basic_string_view<CharT> str,
    std::size_t offset,
    int base) -> parsed_integer_prefix
{
    if (base != 0 && (base < 2 || base > 36)) {
        return parsed_integer_prefix{offset, -1};
    }

    std::size_t digit_offset = offset;
    int actual_base = base;

    if (digit_offset < str.size() && str[digit_offset] == ascii_char<CharT>('0')) {
        if (digit_offset + 1 < str.size()) {
            const CharT next = str[digit_offset + 1];

            if ((actual_base == 0 || actual_base == 16) &&
                (next == ascii_char<CharT>('x') || next == ascii_char<CharT>('X'))) {
                actual_base = 16;
                digit_offset += 2;
            } else if ((actual_base == 0 || actual_base == 2) &&
                       (next == ascii_char<CharT>('b') || next == ascii_char<CharT>('B'))) {
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

template<strto_integer T, strto_character CharT>
[[nodiscard]] constexpr auto parse_integer_view(
    std::basic_string_view<CharT> str,
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
        if (str[offset] == ascii_char<CharT>('+')) {
            ++offset;
        } else if (str[offset] == ascii_char<CharT>('-')) {
            negative = true;
            ++offset;
        }
    }

    const parsed_integer_prefix prefix = parse_integer_prefix(str, offset, base);
    if (prefix.base < 0) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    offset = prefix.digit_offset;

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

template<strto_integer T, strto_character CharT>
[[nodiscard]] constexpr auto strto_from_view(
    std::basic_string_view<CharT> str,
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

template<strto_integer T, strto_character CharT>
[[nodiscard]] constexpr auto ato_from_view(std::basic_string_view<CharT> str) -> result<T>
{
    return strto_from_view<T>(str, nullptr, 10);
}

} // namespace detail

template<detail::strto_integer T, detail::strto_character CharT>
[[nodiscard]] constexpr auto ato(std::basic_string_view<CharT> str) -> result<T>
{
    return detail::ato_from_view<T>(str);
}

template<detail::strto_integer T, detail::strto_character CharT>
[[nodiscard]] constexpr auto ato(const std::basic_string<CharT>& str) -> result<T>
{
    return detail::ato_from_view<T>(std::basic_string_view<CharT>(str));
}

template<detail::strto_integer T, detail::strto_character CharT>
[[nodiscard]] constexpr auto ato(const CharT* str) -> result<T>
{
    return detail::ato_from_view<T>(std::basic_string_view<CharT>(str));
}

template<detail::strto_integer T, detail::strto_character CharT>
[[nodiscard]] constexpr auto ato(CharT* str) -> result<T>
{
    return detail::ato_from_view<T>(std::basic_string_view<CharT>(str));
}

template<detail::strto_integer T, detail::strto_character CharT>
[[nodiscard]] constexpr auto strto(
    std::basic_string_view<CharT> str,
    typename std::basic_string_view<CharT>::const_iterator* endit = nullptr,
    int base = 0) -> result<T>
{
    std::size_t end_offset = 0;
    const auto result = detail::strto_from_view<T>(str, &end_offset, base);

    if (endit != nullptr) {
        *endit = str.begin() + static_cast<std::ptrdiff_t>(end_offset);
    }

    return result;
}

template<detail::strto_integer T, detail::strto_character CharT>
[[nodiscard]] constexpr auto strto(
    const std::basic_string<CharT>& str,
    typename std::basic_string<CharT>::const_iterator* endit = nullptr,
    int base = 0) -> result<T>
{
    std::size_t end_offset = 0;
    const auto result = detail::strto_from_view<T>(std::basic_string_view<CharT>(str), &end_offset, base);

    if (endit != nullptr) {
        *endit = str.begin() + static_cast<std::ptrdiff_t>(end_offset);
    }

    return result;
}

template<detail::strto_integer T, detail::strto_character CharT>
[[nodiscard]] constexpr auto strto(
    std::basic_string<CharT>& str,
    typename std::basic_string<CharT>::iterator* endit = nullptr,
    int base = 0) -> result<T>
{
    std::size_t end_offset = 0;
    const auto result = detail::strto_from_view<T>(std::basic_string_view<CharT>(str), &end_offset, base);

    if (endit != nullptr) {
        *endit = str.begin() + static_cast<std::ptrdiff_t>(end_offset);
    }

    return result;
}

template<detail::strto_integer T, detail::strto_character CharT>
[[nodiscard]] constexpr auto strto(
    const CharT* str,
    const CharT** endptr = nullptr,
    int base = 0) -> result<T>
{
    const std::basic_string_view<CharT> view(str);
    std::size_t end_offset = 0;
    const auto result = detail::strto_from_view<T>(view, &end_offset, base);

    if (endptr != nullptr) {
        *endptr = str + static_cast<std::ptrdiff_t>(end_offset);
    }

    return result;
}

template<detail::strto_integer T, detail::strto_character CharT>
[[nodiscard]] constexpr auto strto(
    CharT* str,
    CharT** endptr = nullptr,
    int base = 0) -> result<T>
{
    const std::basic_string_view<CharT> view(str);
    std::size_t end_offset = 0;
    const auto result = detail::strto_from_view<T>(view, &end_offset, base);

    if (endptr != nullptr) {
        *endptr = str + static_cast<std::ptrdiff_t>(end_offset);
    }

    return result;
}

template<detail::strto_integer T, detail::strto_character CharT>
[[nodiscard]] constexpr auto strto(
    const CharT* str,
    std::nullptr_t,
    int base = 0) -> result<T>
{
    return strto<T>(str, static_cast<const CharT**>(nullptr), base);
}

template<detail::strto_integer T, detail::strto_character CharT>
[[nodiscard]] constexpr auto strto(
    CharT* str,
    std::nullptr_t,
    int base = 0) -> result<T>
{
    return strto<T>(str, static_cast<CharT**>(nullptr), base);
}

template<detail::strto_character CharT>
[[nodiscard]] constexpr auto atoi(std::basic_string_view<CharT> str) -> result<int>
{
    return ato<int>(str);
}

template<detail::strto_character CharT>
[[nodiscard]] constexpr auto atoi(const std::basic_string<CharT>& str) -> result<int>
{
    return ato<int>(str);
}

template<detail::strto_character CharT>
[[nodiscard]] constexpr auto atoi(const CharT* str) -> result<int>
{
    return ato<int>(str);
}

template<detail::strto_character CharT>
[[nodiscard]] constexpr auto atoi(CharT* str) -> result<int>
{
    return ato<int>(str);
}

template<detail::strto_character CharT>
[[nodiscard]] constexpr auto atol(std::basic_string_view<CharT> str) -> result<long>
{
    return ato<long>(str);
}

template<detail::strto_character CharT>
[[nodiscard]] constexpr auto atol(const std::basic_string<CharT>& str) -> result<long>
{
    return ato<long>(str);
}

template<detail::strto_character CharT>
[[nodiscard]] constexpr auto atol(const CharT* str) -> result<long>
{
    return ato<long>(str);
}

template<detail::strto_character CharT>
[[nodiscard]] constexpr auto atol(CharT* str) -> result<long>
{
    return ato<long>(str);
}

template<detail::strto_character CharT>
[[nodiscard]] constexpr auto atoll(std::basic_string_view<CharT> str) -> result<long long>
{
    return ato<long long>(str);
}

template<detail::strto_character CharT>
[[nodiscard]] constexpr auto atoll(const std::basic_string<CharT>& str) -> result<long long>
{
    return ato<long long>(str);
}

template<detail::strto_character CharT>
[[nodiscard]] constexpr auto atoll(const CharT* str) -> result<long long>
{
    return ato<long long>(str);
}

template<detail::strto_character CharT>
[[nodiscard]] constexpr auto atoll(CharT* str) -> result<long long>
{
    return ato<long long>(str);
}

#define XER_DEFINE_STRTO_INTEGER_WRAPPER(name, type) \
    template<detail::strto_character CharT> \
    [[nodiscard]] constexpr auto name( \
        std::basic_string_view<CharT> str, \
        typename std::basic_string_view<CharT>::const_iterator* endit = nullptr, \
        int base = 0) -> result<type> \
    { \
        return strto<type>(str, endit, base); \
    } \
    template<detail::strto_character CharT> \
    [[nodiscard]] constexpr auto name( \
        const std::basic_string<CharT>& str, \
        typename std::basic_string<CharT>::const_iterator* endit = nullptr, \
        int base = 0) -> result<type> \
    { \
        return strto<type>(str, endit, base); \
    } \
    template<detail::strto_character CharT> \
    [[nodiscard]] constexpr auto name( \
        std::basic_string<CharT>& str, \
        typename std::basic_string<CharT>::iterator* endit = nullptr, \
        int base = 0) -> result<type> \
    { \
        return strto<type>(str, endit, base); \
    } \
    template<detail::strto_character CharT> \
    [[nodiscard]] constexpr auto name( \
        const CharT* str, \
        const CharT** endptr = nullptr, \
        int base = 0) -> result<type> \
    { \
        return strto<type>(str, endptr, base); \
    } \
    template<detail::strto_character CharT> \
    [[nodiscard]] constexpr auto name( \
        CharT* str, \
        CharT** endptr = nullptr, \
        int base = 0) -> result<type> \
    { \
        return strto<type>(str, endptr, base); \
    } \
    template<detail::strto_character CharT> \
    [[nodiscard]] constexpr auto name( \
        const CharT* str, \
        std::nullptr_t, \
        int base = 0) -> result<type> \
    { \
        return strto<type>(str, nullptr, base); \
    } \
    template<detail::strto_character CharT> \
    [[nodiscard]] constexpr auto name( \
        CharT* str, \
        std::nullptr_t, \
        int base = 0) -> result<type> \
    { \
        return strto<type>(str, nullptr, base); \
    }

XER_DEFINE_STRTO_INTEGER_WRAPPER(strtol, long)
XER_DEFINE_STRTO_INTEGER_WRAPPER(strtoll, long long)
XER_DEFINE_STRTO_INTEGER_WRAPPER(strtoul, unsigned long)
XER_DEFINE_STRTO_INTEGER_WRAPPER(strtoull, unsigned long long)

#undef XER_DEFINE_STRTO_INTEGER_WRAPPER

} // namespace xer

#endif /* XER_BITS_STRTO_H_INCLUDED_ */
