/**
 * @file xer/bits/itostr.h
 * @brief Integer-to-string conversion functions.
 */

#pragma once

#ifndef XER_BITS_ITOSTR_H_INCLUDED_
#define XER_BITS_ITOSTR_H_INCLUDED_

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <expected>
#include <limits>
#include <string>
#include <type_traits>

#include <xer/bits/common.h>
#include <xer/bits/strto_common.h>
#include <xer/error.h>

namespace xer {

namespace detail {

template<typename T>
concept itostr_integer =
    std::integral<std::remove_cv_t<T>> &&
    (!std::same_as<std::remove_cv_t<T>, bool>);

template<strto_character CharT>
[[nodiscard]] constexpr auto ascii_digit_char(unsigned digit) noexcept -> std::remove_cv_t<CharT>
{
    return ascii_char<CharT>(static_cast<char>(digit < 10 ? '0' + digit : 'a' + (digit - 10)));
}

template<itostr_integer Integer, strto_character CharT>
[[nodiscard]] constexpr auto itostr_make_string(Integer value, int radix) -> result<std::basic_string<CharT>>
{
    using integer_type = std::remove_cv_t<Integer>;
    using unsigned_type = std::make_unsigned_t<integer_type>;

    if (radix < 2 || radix > 36) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    const bool negative = std::is_signed_v<integer_type> && value < 0;
    unsigned_type magnitude = 0;

    if (negative) {
        magnitude = unsigned_type{0} - static_cast<unsigned_type>(value);
    } else {
        magnitude = static_cast<unsigned_type>(value);
    }

    std::array<CharT, std::numeric_limits<unsigned_type>::digits + 2> buffer{};
    std::size_t length = 0;
    const unsigned_type base = static_cast<unsigned_type>(radix);

    do {
        const unsigned digit = static_cast<unsigned>(magnitude % base);
        buffer[length] = ascii_digit_char<CharT>(digit);
        ++length;
        magnitude /= base;
    } while (magnitude != 0);

    if (negative) {
        buffer[length] = ascii_char<CharT>('-');
        ++length;
    }

    std::basic_string<CharT> result;
    result.reserve(length);

    while (length != 0) {
        --length;
        result.push_back(buffer[length]);
    }

    return result;
}

} // namespace detail

template<detail::itostr_integer Integer, detail::strto_character CharT>
[[nodiscard]] constexpr auto itostr(
    Integer value,
    std::basic_string<CharT>& str,
    int radix = 10) -> result<std::basic_string<CharT>*>
{
    auto converted = detail::itostr_make_string<Integer, CharT>(value, radix);
    if (!converted) {
        return std::unexpected(converted.error());
    }

    str = std::move(*converted);
    return &str;
}

template<detail::itostr_integer Integer, detail::strto_character CharT, std::size_t N>
[[nodiscard]] constexpr auto itostr(
    Integer value,
    CharT (&s)[N],
    int radix = 10) -> result<CharT*>
{
    auto converted = detail::itostr_make_string<Integer, CharT>(value, radix);
    if (!converted) {
        return std::unexpected(converted.error());
    }

    if (converted->size() + 1 > N) {
        return std::unexpected(make_error(error_t::length_error));
    }

    std::copy(converted->begin(), converted->end(), s);
    s[converted->size()] = CharT{};
    return s;
}

template<detail::itostr_integer Integer, detail::strto_character CharT>
[[nodiscard]] constexpr auto itoa(
    Integer value,
    std::basic_string<CharT>& str,
    int radix = 10) -> result<std::basic_string<CharT>*>
{
    return itostr(value, str, radix);
}

template<detail::itostr_integer Integer, detail::strto_character CharT, std::size_t N>
[[nodiscard]] constexpr auto itoa(
    Integer value,
    CharT (&s)[N],
    int radix = 10) -> result<CharT*>
{
    return itostr(value, s, radix);
}

} // namespace xer

#endif /* XER_BITS_ITOSTR_H_INCLUDED_ */
