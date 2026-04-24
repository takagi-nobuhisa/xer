/**
 * @file xer/bits/stdint.h
 * @brief Integer type aliases, integer literal parsers, and integer width helpers.
 */

#pragma once

#ifndef XER_BITS_STDINT_H_INCLUDED_
#define XER_BITS_STDINT_H_INCLUDED_

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include <xer/bits/common.h>
#include <xer/bits/numeric_limits.h>

namespace xer {

using std::int8_t;
using std::int16_t;
using std::int32_t;
using std::int64_t;

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;

using std::int_least8_t;
using std::int_least16_t;
using std::int_least32_t;
using std::int_least64_t;

using std::uint_least8_t;
using std::uint_least16_t;
using std::uint_least32_t;
using std::uint_least64_t;

using std::int_fast8_t;
using std::int_fast16_t;
using std::int_fast32_t;
using std::int_fast64_t;

using std::uint_fast8_t;
using std::uint_fast16_t;
using std::uint_fast32_t;
using std::uint_fast64_t;

using std::intptr_t;
using std::uintptr_t;

#if defined(__SIZEOF_INT128__)
using int128_t = __int128;
using uint128_t = unsigned __int128;
using intmax_t = int128_t;
using uintmax_t = uint128_t;
#else
using std::intmax_t;
using std::uintmax_t;
#endif


template<typename T>
inline constexpr int bit_width_of =
    std::numeric_limits<T>::digits + (std::numeric_limits<T>::is_signed ? 1 : 0);

namespace detail {

/**
 * @brief Represents the detected integer literal base.
 */
enum class integer_literal_base : int {
    binary = 2,
    octal = 8,
    decimal = 10,
    hexadecimal = 16,
};

/**
 * @brief Holds intermediate parsing state for an integer literal.
 */
struct parsed_integer_literal {
    uintmax_t value;
    integer_literal_base base;
    bool has_digits;
};

/**
 * @brief Reports whether the character is a digit separator.
 * @param ch Character to test.
 * @return `true` if the character is `'`.
 */
[[nodiscard]] consteval bool is_digit_separator(char ch)
{
    return ch == '\'';
}

/**
 * @brief Converts an integer literal digit character to its numeric value.
 * @param ch Character to convert.
 * @return The digit value, or `-1` if the character is invalid.
 */
[[nodiscard]] consteval int digit_value_of(char ch)
{
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    }

    if (ch >= 'a' && ch <= 'f') {
        return 10 + (ch - 'a');
    }

    if (ch >= 'A' && ch <= 'F') {
        return 10 + (ch - 'A');
    }

    return -1;
}

/**
 * @brief Appends a digit to the accumulated integer literal value.
 * @param value Current accumulated value.
 * @param base Literal base.
 * @param digit Digit value.
 * @return Updated accumulated value.
 */
[[nodiscard]] consteval uintmax_t append_digit(
    uintmax_t value,
    integer_literal_base base,
    unsigned digit)
{
    const uintmax_t base_value = static_cast<uintmax_t>(static_cast<int>(base));

    if (value > (max_of<uintmax_t> - digit) / base_value) {
        throw "integer literal is too large";
    }

    return value * base_value + digit;
}

/**
 * @brief Parses the digits of an integer literal.
 * @tparam Chars Literal character sequence.
 * @return Parsed literal information.
 */
template<char... Chars>
[[nodiscard]] consteval parsed_integer_literal parse_integer_literal()
{
    constexpr char chars[] = {Chars..., '\0'};
    constexpr std::size_t count = sizeof...(Chars);

    std::size_t index = 0;
    integer_literal_base base = integer_literal_base::decimal;

    if (count >= 2 && chars[0] == '0') {
        if (chars[1] == 'x' || chars[1] == 'X') {
            base = integer_literal_base::hexadecimal;
            index = 2;
        } else if (chars[1] == 'b' || chars[1] == 'B') {
            base = integer_literal_base::binary;
            index = 2;
        } else {
            base = integer_literal_base::octal;
            index = 1;
        }
    }

    uintmax_t value = 0;
    bool has_digits = false;

    for (; index < count; ++index) {
        const char ch = chars[index];

        if (is_digit_separator(ch)) {
            continue;
        }

        const int digit = digit_value_of(ch);
        if (digit < 0 || digit >= static_cast<int>(base)) {
            throw "invalid digit in integer literal";
        }

        value = append_digit(value, base, static_cast<unsigned>(digit));
        has_digits = true;
    }

    if (!has_digits) {
        throw "integer literal has no digits";
    }

    return parsed_integer_literal{value, base, has_digits};
}

/**
 * @brief Casts the parsed integer literal to an unsigned target type.
 * @tparam T Unsigned target type.
 * @tparam Chars Literal character sequence.
 * @return Parsed value converted to `T`.
 */
template<typename T, char... Chars>
[[nodiscard]] consteval T parse_unsigned_integer_literal()
{
    static_assert(std::is_integral_v<T>);
    static_assert(std::is_unsigned_v<T>);

    constexpr parsed_integer_literal parsed = parse_integer_literal<Chars...>();

    if (parsed.value > static_cast<uintmax_t>(max_of<T>)) {
        throw "integer literal is out of range";
    }

    return static_cast<T>(parsed.value);
}

/**
 * @brief Casts the parsed integer literal to a signed target type.
 * @tparam T Signed target type.
 * @tparam Chars Literal character sequence.
 * @return Parsed value converted to `T`.
 */
template<typename T, char... Chars>
[[nodiscard]] consteval T parse_signed_integer_literal()
{
    static_assert(std::is_integral_v<T>);
    static_assert(std::is_signed_v<T>);

    constexpr parsed_integer_literal parsed = parse_integer_literal<Chars...>();
    constexpr auto max_value = static_cast<uintmax_t>(max_of<T>);

    if (parsed.value > max_value) {
        throw "integer literal is out of range";
    }

    return static_cast<T>(parsed.value);
}

} // namespace detail

namespace literals::integer_literals {

template<char... Chars>
[[nodiscard]] consteval int8_t operator""_i8()
{
    return detail::parse_signed_integer_literal<int8_t, Chars...>();
}

template<char... Chars>
[[nodiscard]] consteval int16_t operator""_i16()
{
    return detail::parse_signed_integer_literal<int16_t, Chars...>();
}

template<char... Chars>
[[nodiscard]] consteval int32_t operator""_i32()
{
    return detail::parse_signed_integer_literal<int32_t, Chars...>();
}

template<char... Chars>
[[nodiscard]] consteval int64_t operator""_i64()
{
    return detail::parse_signed_integer_literal<int64_t, Chars...>();
}

template<char... Chars>
[[nodiscard]] consteval uint8_t operator""_u8()
{
    return detail::parse_unsigned_integer_literal<uint8_t, Chars...>();
}

template<char... Chars>
[[nodiscard]] consteval uint16_t operator""_u16()
{
    return detail::parse_unsigned_integer_literal<uint16_t, Chars...>();
}

template<char... Chars>
[[nodiscard]] consteval uint32_t operator""_u32()
{
    return detail::parse_unsigned_integer_literal<uint32_t, Chars...>();
}

template<char... Chars>
[[nodiscard]] consteval uint64_t operator""_u64()
{
    return detail::parse_unsigned_integer_literal<uint64_t, Chars...>();
}


template<char... Chars>
[[nodiscard]] consteval int_least8_t operator""_il8()
{
    return detail::parse_signed_integer_literal<int_least8_t, Chars...>();
}

template<char... Chars>
[[nodiscard]] consteval int_least16_t operator""_il16()
{
    return detail::parse_signed_integer_literal<int_least16_t, Chars...>();
}

template<char... Chars>
[[nodiscard]] consteval int_least32_t operator""_il32()
{
    return detail::parse_signed_integer_literal<int_least32_t, Chars...>();
}

template<char... Chars>
[[nodiscard]] consteval int_least64_t operator""_il64()
{
    return detail::parse_signed_integer_literal<int_least64_t, Chars...>();
}

template<char... Chars>
[[nodiscard]] consteval uint_least8_t operator""_ul8()
{
    return detail::parse_unsigned_integer_literal<uint_least8_t, Chars...>();
}

template<char... Chars>
[[nodiscard]] consteval uint_least16_t operator""_ul16()
{
    return detail::parse_unsigned_integer_literal<uint_least16_t, Chars...>();
}

template<char... Chars>
[[nodiscard]] consteval uint_least32_t operator""_ul32()
{
    return detail::parse_unsigned_integer_literal<uint_least32_t, Chars...>();
}

template<char... Chars>
[[nodiscard]] consteval uint_least64_t operator""_ul64()
{
    return detail::parse_unsigned_integer_literal<uint_least64_t, Chars...>();
}

#if defined(__SIZEOF_INT128__)
template<char... Chars>
[[nodiscard]] consteval int128_t operator""_i128()
{
    return detail::parse_signed_integer_literal<int128_t, Chars...>();
}

template<char... Chars>
[[nodiscard]] consteval uint128_t operator""_u128()
{
    return detail::parse_unsigned_integer_literal<uint128_t, Chars...>();
}
#endif

} // namespace literals::integer_literals

} // namespace xer

#endif /* XER_BITS_STDINT_H_INCLUDED_ */
