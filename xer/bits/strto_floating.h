/**
 * @file xer/bits/strto_floating.h
 * @brief Floating-point parsing functions based on ASCII-compatible character sequences.
 */

#pragma once

#ifndef XER_BITS_STRTO_FLOATING_H_INCLUDED_
#define XER_BITS_STRTO_FLOATING_H_INCLUDED_

#include <cmath>
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
concept strto_floating_point = std::floating_point<std::remove_cv_t<T>>;

template<strto_character CharT>
[[nodiscard]] constexpr auto starts_with_ascii_icase(
    std::basic_string_view<CharT> str,
    std::size_t offset,
    std::string_view keyword) noexcept -> bool
{
    if (offset + keyword.size() > str.size()) {
        return false;
    }

    for (std::size_t i = 0; i < keyword.size(); ++i) {
        if (!ascii_iequal(str[offset + i], keyword[i])) {
            return false;
        }
    }

    return true;
}

struct floating_special_result {
    long double value;
    std::size_t end_offset;
};

template<strto_character CharT>
[[nodiscard]] inline auto parse_special_floating(
    std::basic_string_view<CharT> str,
    std::size_t offset,
    bool negative) -> result<floating_special_result>
{
    if (starts_with_ascii_icase(str, offset, "infinity")) {
        const long double value = negative
                                      ? -std::numeric_limits<long double>::infinity()
                                      : std::numeric_limits<long double>::infinity();
        return floating_special_result{value, offset + 8};
    }

    if (starts_with_ascii_icase(str, offset, "inf")) {
        const long double value = negative
                                      ? -std::numeric_limits<long double>::infinity()
                                      : std::numeric_limits<long double>::infinity();
        return floating_special_result{value, offset + 3};
    }

    if (starts_with_ascii_icase(str, offset, "nan")) {
        long double value = std::numeric_limits<long double>::quiet_NaN();
        if (negative) {
            value = -value;
        }

        std::size_t end_offset = offset + 3;

        if (end_offset < str.size() && str[end_offset] == ascii_char<CharT>('(')) {
            std::size_t pos = end_offset + 1;
            while (pos < str.size() && str[pos] != ascii_char<CharT>(')')) {
                ++pos;
            }

            if (pos < str.size() && str[pos] == ascii_char<CharT>(')')) {
                end_offset = pos + 1;
            }
        }

        return floating_special_result{value, end_offset};
    }

    return std::unexpected(make_error(error_t::invalid_argument));
}

template<typename T>
struct floating_parse_result {
    T value;
    std::size_t end_offset;
};

[[nodiscard]] inline auto scale_by_power_of_two(
    long double value,
    long long exponent) -> long double
{
    if (value == 0.0L) {
        return 0.0L;
    }

    long double result = value;

    if (exponent > 0) {
        while (exponent > static_cast<long long>((std::numeric_limits<int>::max)())) {
            result = std::ldexp(result, (std::numeric_limits<int>::max)());
            exponent -= static_cast<long long>((std::numeric_limits<int>::max)());
        }

        result = std::ldexp(result, static_cast<int>(exponent));
    } else if (exponent < 0) {
        while (exponent < static_cast<long long>(std::numeric_limits<int>::min())) {
            result = std::ldexp(result, std::numeric_limits<int>::min());
            exponent -= static_cast<long long>(std::numeric_limits<int>::min());
        }

        result = std::ldexp(result, static_cast<int>(exponent));
    }

    return result;
}

[[nodiscard]] inline auto scale_by_power_of_ten(
    long double value,
    long long exponent) -> long double
{
    if (value == 0.0L) {
        return 0.0L;
    }

    long double result = value;
    long double factor = 10.0L;
    long long remaining = exponent;

    if (remaining < 0) {
        factor = 0.1L;
        remaining = -remaining;
    }

    while (remaining > 0) {
        if ((remaining & 1LL) != 0) {
            result *= factor;

            if (std::isinf(result) || result == 0.0L) {
                return result;
            }
        }

        remaining >>= 1;
        if (remaining == 0) {
            break;
        }

        factor *= factor;
        if (std::isinf(factor) || factor == 0.0L) {
            return (exponent < 0) ? 0.0L : std::numeric_limits<long double>::infinity();
        }
    }

    return result;
}

[[nodiscard]] inline auto apply_floating_exponent(
    long double significand,
    bool hexadecimal,
    long long exponent) -> long double
{
    if (hexadecimal) {
        return scale_by_power_of_two(significand, exponent);
    }

    return scale_by_power_of_ten(significand, exponent);
}

template<strto_floating_point T, strto_character CharT>
[[nodiscard]] inline auto parse_floating_view(
    std::basic_string_view<CharT> str) -> result<floating_parse_result<T>>
{
    std::size_t offset = 0;
    while (offset < str.size() && is_ascii_space(str[offset])) {
        ++offset;
    }

    const std::size_t start_offset = offset;

    bool negative = false;
    if (offset < str.size()) {
        if (str[offset] == ascii_char<CharT>('+')) {
            ++offset;
        } else if (str[offset] == ascii_char<CharT>('-')) {
            negative = true;
            ++offset;
        }
    }

    if (offset < str.size()) {
        const auto special = parse_special_floating(str, offset, negative);
        if (special) {
            const long double special_value = special->value;
            const long double abs_value = std::fabs(special_value);

            if (std::isinf(abs_value) &&
                !std::numeric_limits<std::remove_cv_t<T>>::has_infinity) {
                return std::unexpected(make_error(error_t::range));
            }

            return floating_parse_result<T>{
                static_cast<std::remove_cv_t<T>>(special_value),
                special->end_offset};
        }
    }

    bool hexadecimal = false;

    if (offset + 1 < str.size() &&
        str[offset] == ascii_char<CharT>('0') &&
        (str[offset + 1] == ascii_char<CharT>('x') ||
         str[offset + 1] == ascii_char<CharT>('X'))) {
        hexadecimal = true;
        offset += 2;
    }

    const int digit_base = hexadecimal ? 16 : 10;
    const long double digit_base_ld = static_cast<long double>(digit_base);
    const long double accumulation_limit =
        (std::numeric_limits<long double>::max)() / digit_base_ld;

    long double significand = 0.0L;
    long long exponent_adjust = 0;
    bool has_digits = false;
    bool any_nonzero_digit = false;

    while (offset < str.size()) {
        const int digit = ascii_digit_value(str[offset]);
        if (digit < 0 || digit >= digit_base) {
            break;
        }

        has_digits = true;
        if (digit != 0) {
            any_nonzero_digit = true;
        }

        if (significand <= accumulation_limit) {
            significand = significand * digit_base_ld + static_cast<long double>(digit);
        } else {
            ++exponent_adjust;
        }

        ++offset;
    }

    if (offset < str.size() && str[offset] == ascii_char<CharT>('.')) {
        ++offset;

        while (offset < str.size()) {
            const int digit = ascii_digit_value(str[offset]);
            if (digit < 0 || digit >= digit_base) {
                break;
            }

            has_digits = true;
            if (digit != 0) {
                any_nonzero_digit = true;
            }

            if (significand <= accumulation_limit) {
                significand = significand * digit_base_ld + static_cast<long double>(digit);
                --exponent_adjust;
            } else {
                --exponent_adjust;
            }

            ++offset;
        }
    }

    if (!has_digits) {
        return floating_parse_result<T>{static_cast<T>(0), start_offset};
    }

    long long explicit_exponent = 0;
    if (offset < str.size()) {
        const CharT exponent_marker = str[offset];
        const bool is_exponent_marker =
            (!hexadecimal &&
             (exponent_marker == ascii_char<CharT>('e') ||
              exponent_marker == ascii_char<CharT>('E'))) ||
            (hexadecimal &&
             (exponent_marker == ascii_char<CharT>('p') ||
              exponent_marker == ascii_char<CharT>('P')));

        if (is_exponent_marker) {
            const std::size_t exponent_begin = offset;
            std::size_t pos = offset + 1;
            bool exponent_negative = false;

            if (pos < str.size()) {
                if (str[pos] == ascii_char<CharT>('+')) {
                    ++pos;
                } else if (str[pos] == ascii_char<CharT>('-')) {
                    exponent_negative = true;
                    ++pos;
                }
            }

            bool has_exponent_digits = false;
            long long exponent_value = 0;
            constexpr long long exponent_limit =
                (std::numeric_limits<long long>::max)() / 10;

            while (pos < str.size() &&
                   str[pos] >= ascii_char<CharT>('0') &&
                   str[pos] <= ascii_char<CharT>('9')) {
                has_exponent_digits = true;
                const int digit = static_cast<int>(str[pos] - ascii_char<CharT>('0'));

                if (exponent_value > exponent_limit) {
                    exponent_value = (std::numeric_limits<long long>::max)();
                } else {
                    exponent_value = exponent_value * 10 + static_cast<long long>(digit);
                    if (exponent_value < 0) {
                        exponent_value = (std::numeric_limits<long long>::max)();
                    }
                }

                ++pos;
            }

            if (has_exponent_digits) {
                explicit_exponent = exponent_negative ? -exponent_value : exponent_value;
                offset = pos;
            } else {
                offset = exponent_begin;
            }
        }
    }

    long long total_exponent = explicit_exponent;

    if (hexadecimal) {
        if (exponent_adjust > (std::numeric_limits<long long>::max)() / 4) {
            total_exponent = (std::numeric_limits<long long>::max)();
        } else if (exponent_adjust < std::numeric_limits<long long>::min() / 4) {
            total_exponent = std::numeric_limits<long long>::min();
        } else {
            const long long binary_adjust = exponent_adjust * 4;

            if (binary_adjust > 0 &&
                total_exponent > (std::numeric_limits<long long>::max)() - binary_adjust) {
                total_exponent = (std::numeric_limits<long long>::max)();
            } else if (binary_adjust < 0 &&
                       total_exponent < std::numeric_limits<long long>::min() - binary_adjust) {
                total_exponent = std::numeric_limits<long long>::min();
            } else {
                total_exponent += binary_adjust;
            }
        }
    } else {
        if (exponent_adjust > 0 &&
            total_exponent > (std::numeric_limits<long long>::max)() - exponent_adjust) {
            total_exponent = (std::numeric_limits<long long>::max)();
        } else if (exponent_adjust < 0 &&
                   total_exponent < std::numeric_limits<long long>::min() - exponent_adjust) {
            total_exponent = std::numeric_limits<long long>::min();
        } else {
            total_exponent += exponent_adjust;
        }
    }

    long double value = apply_floating_exponent(significand, hexadecimal, total_exponent);
    if (negative) {
        value = -value;
    }

    const long double abs_value = std::fabs(value);
    if (std::isinf(abs_value)) {
        return std::unexpected(make_error(error_t::range));
    }

    using value_type = std::remove_cv_t<T>;

    if (abs_value != 0.0L &&
        abs_value > static_cast<long double>((std::numeric_limits<value_type>::max)())) {
        return std::unexpected(make_error(error_t::range));
    }

    const value_type converted = static_cast<value_type>(value);
    const long double abs_converted = std::fabs(static_cast<long double>(converted));

    if (any_nonzero_digit && converted == static_cast<value_type>(0)) {
        return std::unexpected(make_error(error_t::range));
    }

    if (abs_value != 0.0L &&
        abs_converted != 0.0L &&
        abs_converted < static_cast<long double>(std::numeric_limits<value_type>::min())) {
        return std::unexpected(make_error(error_t::range));
    }

    if (std::isinf(abs_converted) &&
        !std::numeric_limits<value_type>::has_infinity) {
        return std::unexpected(make_error(error_t::range));
    }

    return floating_parse_result<T>{converted, offset};
}

template<strto_floating_point T, strto_character CharT>
[[nodiscard]] inline auto strto_from_view(
    std::basic_string_view<CharT> str,
    std::size_t* end_offset) -> result<T>
{
    const auto parsed = parse_floating_view<T>(str);
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

} // namespace detail

template<detail::strto_floating_point T, detail::strto_character CharT>
[[nodiscard]] inline auto strto(
    std::basic_string_view<CharT> str,
    typename std::basic_string_view<CharT>::const_iterator* endit = nullptr) -> result<T>
{
    std::size_t end_offset = 0;
    const auto result = detail::strto_from_view<T>(str, &end_offset);

    if (endit != nullptr) {
        *endit = str.begin() + static_cast<std::ptrdiff_t>(end_offset);
    }

    return result;
}

template<detail::strto_floating_point T, detail::strto_character CharT>
[[nodiscard]] inline auto strto(
    const std::basic_string<CharT>& str,
    typename std::basic_string<CharT>::const_iterator* endit = nullptr) -> result<T>
{
    std::size_t end_offset = 0;
    const auto result = detail::strto_from_view<T>(std::basic_string_view<CharT>(str), &end_offset);

    if (endit != nullptr) {
        *endit = str.begin() + static_cast<std::ptrdiff_t>(end_offset);
    }

    return result;
}

template<detail::strto_floating_point T, detail::strto_character CharT>
[[nodiscard]] inline auto strto(
    std::basic_string<CharT>& str,
    typename std::basic_string<CharT>::iterator* endit = nullptr) -> result<T>
{
    std::size_t end_offset = 0;
    const auto result = detail::strto_from_view<T>(std::basic_string_view<CharT>(str), &end_offset);

    if (endit != nullptr) {
        *endit = str.begin() + static_cast<std::ptrdiff_t>(end_offset);
    }

    return result;
}

template<detail::strto_floating_point T, detail::strto_character CharT>
[[nodiscard]] inline auto strto(
    const CharT* str,
    const CharT** endptr = nullptr) -> result<T>
{
    const std::basic_string_view<CharT> view(str);
    std::size_t end_offset = 0;
    const auto result = detail::strto_from_view<T>(view, &end_offset);

    if (endptr != nullptr) {
        *endptr = str + static_cast<std::ptrdiff_t>(end_offset);
    }

    return result;
}

template<detail::strto_floating_point T, detail::strto_character CharT>
[[nodiscard]] inline auto strto(
    CharT* str,
    CharT** endptr = nullptr) -> result<T>
{
    const std::basic_string_view<CharT> view(str);
    std::size_t end_offset = 0;
    const auto result = detail::strto_from_view<T>(view, &end_offset);

    if (endptr != nullptr) {
        *endptr = str + static_cast<std::ptrdiff_t>(end_offset);
    }

    return result;
}

template<detail::strto_floating_point T, detail::strto_character CharT>
[[nodiscard]] inline auto strto(
    const CharT* str,
    std::nullptr_t) -> result<T>
{
    return strto<T>(str, static_cast<const CharT**>(nullptr));
}

template<detail::strto_floating_point T, detail::strto_character CharT>
[[nodiscard]] inline auto strto(
    CharT* str,
    std::nullptr_t) -> result<T>
{
    return strto<T>(str, static_cast<CharT**>(nullptr));
}

#define XER_DEFINE_STRTO_FLOATING_WRAPPER(name, type) \
    template<detail::strto_character CharT> \
    [[nodiscard]] inline auto name( \
        std::basic_string_view<CharT> str, \
        typename std::basic_string_view<CharT>::const_iterator* endit = nullptr) -> result<type> \
    { \
        return strto<type>(str, endit); \
    } \
    template<detail::strto_character CharT> \
    [[nodiscard]] inline auto name( \
        const std::basic_string<CharT>& str, \
        typename std::basic_string<CharT>::const_iterator* endit = nullptr) -> result<type> \
    { \
        return strto<type>(str, endit); \
    } \
    template<detail::strto_character CharT> \
    [[nodiscard]] inline auto name( \
        std::basic_string<CharT>& str, \
        typename std::basic_string<CharT>::iterator* endit = nullptr) -> result<type> \
    { \
        return strto<type>(str, endit); \
    } \
    template<detail::strto_character CharT> \
    [[nodiscard]] inline auto name( \
        const CharT* str, \
        const CharT** endptr = nullptr) -> result<type> \
    { \
        return strto<type>(str, endptr); \
    } \
    template<detail::strto_character CharT> \
    [[nodiscard]] inline auto name( \
        CharT* str, \
        CharT** endptr = nullptr) -> result<type> \
    { \
        return strto<type>(str, endptr); \
    } \
    template<detail::strto_character CharT> \
    [[nodiscard]] inline auto name( \
        const CharT* str, \
        std::nullptr_t) -> result<type> \
    { \
        return strto<type>(str, nullptr); \
    } \
    template<detail::strto_character CharT> \
    [[nodiscard]] inline auto name( \
        CharT* str, \
        std::nullptr_t) -> result<type> \
    { \
        return strto<type>(str, nullptr); \
    }

XER_DEFINE_STRTO_FLOATING_WRAPPER(strtof, float)
XER_DEFINE_STRTO_FLOATING_WRAPPER(strtod, double)
XER_DEFINE_STRTO_FLOATING_WRAPPER(strtold, long double)
XER_DEFINE_STRTO_FLOATING_WRAPPER(strtof32, float)
XER_DEFINE_STRTO_FLOATING_WRAPPER(strtof64, double)

#undef XER_DEFINE_STRTO_FLOATING_WRAPPER

} // namespace xer

#endif /* XER_BITS_STRTO_FLOATING_H_INCLUDED_ */
