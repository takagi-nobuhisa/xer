/**
 * @file xer/bits/printf_format.h
 * @brief Internal printf-style formatting helpers.
 */

#pragma once

#ifndef XER_BITS_PRINTF_FORMAT_H_INCLUDED_
#define XER_BITS_PRINTF_FORMAT_H_INCLUDED_

#include <algorithm>
#include <array>
#include <charconv>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <expected>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include <xer/bits/common.h>
#include <xer/bits/utf8_char_encode.h>
#include <xer/error.h>

namespace xer::detail {

/**
 * @brief Represents one parsed printf conversion.
 */
struct printf_conversion_spec {
    bool left_justify = false;
    bool force_sign = false;
    bool space_sign = false;
    bool alternative = false;
    bool zero_pad = false;

    std::optional<int> width;
    std::optional<int> precision;

    enum class length_t {
        none,
        hh,
        h,
        l,
        ll,
        j,
        z,
        t,
        L
    };

    length_t length = length_t::none;
    char32_t conversion = U'\0';

    bool positional = false;
    std::size_t position = 0;
    bool width_from_arg = false;
    bool width_positional = false;
    std::size_t width_position = 0;
    bool precision_from_arg = false;
    bool precision_positional = false;
    std::size_t precision_position = 0;
};

/**
 * @brief One token of a parsed format string.
 */
struct printf_token {
    bool is_conversion = false;
    std::u8string literal;
    printf_conversion_spec spec;
};

/**
 * @brief Runtime printf argument kind.
 */
enum class printf_arg_kind {
    signed_integer,
    unsigned_integer,
    floating,
    character,
    c_string,
    utf8_string,
    pointer
};

/**
 * @brief Runtime printf argument holder.
 */
struct printf_argument {
    printf_arg_kind kind;
    std::variant<std::intmax_t,
                 std::uintmax_t,
                 long double,
                 char32_t,
                 const char*,
                 std::u8string_view,
                 const void*>
        value;
};

template<typename T>
concept printf_signed_integer =
    std::integral<std::remove_cvref_t<T>> &&
    std::is_signed_v<std::remove_cvref_t<T>> &&
    !std::same_as<std::remove_cvref_t<T>, bool> &&
    !std::same_as<std::remove_cvref_t<T>, char> &&
    !std::same_as<std::remove_cvref_t<T>, wchar_t> &&
    !std::same_as<std::remove_cvref_t<T>, char8_t> &&
    !std::same_as<std::remove_cvref_t<T>, char16_t> &&
    !std::same_as<std::remove_cvref_t<T>, char32_t>;

template<typename T>
concept printf_unsigned_integer =
    std::integral<std::remove_cvref_t<T>> &&
    std::is_unsigned_v<std::remove_cvref_t<T>> &&
    !std::same_as<std::remove_cvref_t<T>, bool> &&
    !std::same_as<std::remove_cvref_t<T>, char> &&
    !std::same_as<std::remove_cvref_t<T>, wchar_t> &&
    !std::same_as<std::remove_cvref_t<T>, char8_t> &&
    !std::same_as<std::remove_cvref_t<T>, char16_t> &&
    !std::same_as<std::remove_cvref_t<T>, char32_t>;

template<typename T>
concept printf_floating =
    std::floating_point<std::remove_cvref_t<T>>;

template<typename T>
concept printf_pointer =
    std::is_pointer_v<std::remove_cvref_t<T>> &&
    !std::same_as<std::remove_cvref_t<T>, const char*> &&
    !std::same_as<std::remove_cvref_t<T>, char*> &&
    !std::same_as<std::remove_cvref_t<T>, const char8_t*> &&
    !std::same_as<std::remove_cvref_t<T>, char8_t*>;

/**
 * @brief Converts one value to a runtime printf argument.
 *
 * @param value Source value.
 * @return Converted argument.
 */
template<printf_signed_integer T>
[[nodiscard]] constexpr printf_argument make_printf_argument(T value) noexcept
{
    return printf_argument{
        .kind = printf_arg_kind::signed_integer,
        .value = static_cast<std::intmax_t>(value),
    };
}

template<printf_unsigned_integer T>
[[nodiscard]] constexpr printf_argument make_printf_argument(T value) noexcept
{
    return printf_argument{
        .kind = printf_arg_kind::unsigned_integer,
        .value = static_cast<std::uintmax_t>(value),
    };
}

template<printf_floating T>
[[nodiscard]] constexpr printf_argument make_printf_argument(T value) noexcept
{
    return printf_argument{
        .kind = printf_arg_kind::floating,
        .value = static_cast<long double>(value),
    };
}

[[nodiscard]] constexpr printf_argument make_printf_argument(char value) noexcept
{
    return printf_argument{
        .kind = printf_arg_kind::character,
        .value = static_cast<char32_t>(static_cast<unsigned char>(value)),
    };
}

[[nodiscard]] constexpr printf_argument make_printf_argument(char8_t value) noexcept
{
    return printf_argument{
        .kind = printf_arg_kind::character,
        .value = static_cast<char32_t>(value),
    };
}

[[nodiscard]] constexpr printf_argument make_printf_argument(char32_t value) noexcept
{
    return printf_argument{
        .kind = printf_arg_kind::character,
        .value = value,
    };
}

[[nodiscard]] inline printf_argument make_printf_argument(const char* value) noexcept
{
    return printf_argument{
        .kind = printf_arg_kind::c_string,
        .value = value,
    };
}

[[nodiscard]] inline printf_argument make_printf_argument(char* value) noexcept
{
    return make_printf_argument(static_cast<const char*>(value));
}

[[nodiscard]] inline printf_argument make_printf_argument(std::u8string_view value) noexcept
{
    return printf_argument{
        .kind = printf_arg_kind::utf8_string,
        .value = value,
    };
}

[[nodiscard]] inline printf_argument make_printf_argument(const char8_t* value) noexcept
{
    if (value == nullptr) {
        return printf_argument{
            .kind = printf_arg_kind::pointer,
            .value = static_cast<const void*>(nullptr),
        };
    }

    return printf_argument{
        .kind = printf_arg_kind::utf8_string,
        .value = std::u8string_view(value),
    };
}

[[nodiscard]] inline printf_argument make_printf_argument(char8_t* value) noexcept
{
    return make_printf_argument(static_cast<const char8_t*>(value));
}

template<printf_pointer T>
[[nodiscard]] constexpr printf_argument make_printf_argument(T value) noexcept
{
    return printf_argument{
        .kind = printf_arg_kind::pointer,
        .value = static_cast<const void*>(value),
    };
}

[[nodiscard]] constexpr printf_argument make_printf_argument(bool value) noexcept
{
    return printf_argument{
        .kind = printf_arg_kind::utf8_string,
        .value = value ? std::u8string_view(u8"true")
                       : std::u8string_view(u8"false"),
    };
}

[[nodiscard]] constexpr printf_argument make_printf_argument(std::nullptr_t) noexcept
{
    return printf_argument{
        .kind = printf_arg_kind::utf8_string,
        .value = std::u8string_view(u8"null"),
    };
}

/**
 * @brief Appends repeated ASCII bytes.
 *
 * @param out Output string.
 * @param ch Byte value.
 * @param count Repeat count.
 */
inline auto append_repeated_ascii(std::u8string& out, char8_t ch, std::size_t count) -> void {
    out.append(count, ch);
}

/**
 * @brief Converts an unsigned integer to ASCII digits.
 *
 * @param value Source value.
 * @param base Numeric base.
 * @param upper Whether hex digits should be uppercase.
 * @return ASCII digit string.
 */
[[nodiscard]] inline auto unsigned_to_ascii(
    std::uintmax_t value,
    unsigned base,
    bool upper = false) -> std::u8string {
    if (base < 2 || base > 16) {
        return {};
    }

    std::array<char8_t, 64> buffer{};
    std::size_t index = buffer.size();

    do {
        const unsigned digit = static_cast<unsigned>(value % base);
        value /= base;

        if (digit < 10) {
            buffer[--index] = static_cast<char8_t>(u8'0' + digit);
        } else if (upper) {
            buffer[--index] = static_cast<char8_t>(u8'A' + (digit - 10));
        } else {
            buffer[--index] = static_cast<char8_t>(u8'a' + (digit - 10));
        }
    } while (value != 0);

    return std::u8string(buffer.begin() + static_cast<std::ptrdiff_t>(index), buffer.end());
}

/**
 * @brief Parses a non-negative decimal integer.
 *
 * @param format Format string.
 * @param index Current parsing position.
 * @return Parsed value on success.
 */
[[nodiscard]] inline auto parse_decimal(
    std::u8string_view format,
    std::size_t& index) noexcept -> std::optional<int> {
    if (index >= format.size() || format[index] < u8'0' || format[index] > u8'9') {
        return std::nullopt;
    }

    int value = 0;
    while (index < format.size() && format[index] >= u8'0' && format[index] <= u8'9') {
        const int digit = static_cast<int>(format[index] - u8'0');

        if (value > (std::numeric_limits<int>::max() - digit) / 10) {
            return std::nullopt;
        }

        value = value * 10 + digit;
        ++index;
    }

    return value;
}

/**
 * @brief Parses one printf conversion specification.
 *
 * @param format Format string.
 * @param index Current parsing position. Must point just after '%'.
 * @return Parsed conversion token on success.
 */
[[nodiscard]] inline auto parse_printf_conversion(
    std::u8string_view format,
    std::size_t& index) noexcept -> result<printf_conversion_spec> {
    printf_conversion_spec spec{};

    const std::size_t original_index = index;
    {
        std::size_t temp = index;
        const auto positional = parse_decimal(format, temp);
        if (positional.has_value() && temp < format.size() && format[temp] == u8'$') {
            spec.positional = true;
            spec.position = static_cast<std::size_t>(*positional);
            index = temp + 1;
        }
    }

    while (index < format.size()) {
        const char8_t ch = format[index];
        bool consumed = true;

        switch (ch) {
        case u8'-':
            spec.left_justify = true;
            break;
        case u8'+':
            spec.force_sign = true;
            break;
        case u8' ':
            spec.space_sign = true;
            break;
        case u8'#':
            spec.alternative = true;
            break;
        case u8'0':
            spec.zero_pad = true;
            break;
        default:
            consumed = false;
            break;
        }

        if (!consumed) {
            break;
        }

        ++index;
    }

    if (index < format.size() && format[index] == u8'*') {
        ++index;
        spec.width_from_arg = true;

        std::size_t temp = index;
        const auto positional = parse_decimal(format, temp);
        if (positional.has_value() && temp < format.size() && format[temp] == u8'$') {
            spec.width_positional = true;
            spec.width_position = static_cast<std::size_t>(*positional);
            index = temp + 1;
        }
    } else {
        spec.width = parse_decimal(format, index);
    }

    if (index < format.size() && format[index] == u8'.') {
        ++index;

        if (index < format.size() && format[index] == u8'*') {
            ++index;
            spec.precision_from_arg = true;

            std::size_t temp = index;
            const auto positional = parse_decimal(format, temp);
            if (positional.has_value() && temp < format.size() && format[temp] == u8'$') {
                spec.precision_positional = true;
                spec.precision_position = static_cast<std::size_t>(*positional);
                index = temp + 1;
            }
        } else {
            spec.precision = parse_decimal(format, index).value_or(0);
        }
    }

    if (index < format.size()) {
        if (format[index] == u8'h') {
            ++index;
            if (index < format.size() && format[index] == u8'h') {
                ++index;
                spec.length = printf_conversion_spec::length_t::hh;
            } else {
                spec.length = printf_conversion_spec::length_t::h;
            }
        } else if (format[index] == u8'l') {
            ++index;
            if (index < format.size() && format[index] == u8'l') {
                ++index;
                spec.length = printf_conversion_spec::length_t::ll;
            } else {
                spec.length = printf_conversion_spec::length_t::l;
            }
        } else if (format[index] == u8'j') {
            ++index;
            spec.length = printf_conversion_spec::length_t::j;
        } else if (format[index] == u8'z') {
            ++index;
            spec.length = printf_conversion_spec::length_t::z;
        } else if (format[index] == u8't') {
            ++index;
            spec.length = printf_conversion_spec::length_t::t;
        } else if (format[index] == u8'L') {
            ++index;
            spec.length = printf_conversion_spec::length_t::L;
        }
    }

    if (index >= format.size()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    spec.conversion = static_cast<char32_t>(format[index]);
    ++index;

    if (spec.positional && spec.position == 0) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (spec.width_positional && spec.width_position == 0) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (spec.precision_positional && spec.precision_position == 0) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (index == original_index) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return spec;
}

/**
 * @brief Parses a full printf format string into tokens.
 *
 * @param format Format string.
 * @return Parsed token sequence on success.
 */
[[nodiscard]] inline auto parse_printf_tokens(
    std::u8string_view format) noexcept -> result<std::vector<printf_token>> {
    std::vector<printf_token> tokens;
    std::u8string literal;

    std::size_t index = 0;
    while (index < format.size()) {
        if (format[index] != u8'%') {
            literal.push_back(format[index]);
            ++index;
            continue;
        }

        if (index + 1 < format.size() && format[index + 1] == u8'%') {
            literal.push_back(u8'%');
            index += 2;
            continue;
        }

        if (!literal.empty()) {
            tokens.push_back(printf_token{
                .is_conversion = false,
                .literal = std::move(literal),
                .spec = {},
            });
            literal.clear();
        }

        ++index;
        const auto spec = parse_printf_conversion(format, index);
        if (!spec.has_value()) {
            return std::unexpected(spec.error());
        }

        tokens.push_back(printf_token{
            .is_conversion = true,
            .literal = {},
            .spec = *spec,
        });
    }

    if (!literal.empty()) {
        tokens.push_back(printf_token{
            .is_conversion = false,
            .literal = std::move(literal),
            .spec = {},
        });
    }

    return tokens;
}

/**
 * @brief Returns one positional or sequential argument.
 *
 * @param args Runtime arguments.
 * @param spec Conversion spec.
 * @param next_index Sequential argument cursor.
 * @return Selected argument on success.
 */
[[nodiscard]] inline auto select_printf_argument(
    const std::vector<printf_argument>& args,
    const printf_conversion_spec& spec,
    std::size_t& next_index) noexcept -> result<const printf_argument*> {
    std::size_t index = 0;

    if (spec.positional) {
        index = spec.position - 1;
    } else {
        index = next_index;
        ++next_index;
    }

    if (index >= args.size()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return &args[index];
}

/**
 * @brief Returns width or precision argument as int.
 *
 * @param args Runtime arguments.
 * @param positional Whether the lookup is positional.
 * @param position Positional index when applicable.
 * @param next_index Sequential argument cursor.
 * @return Extracted integer value on success.
 */
[[nodiscard]] inline auto select_printf_int_argument(
    const std::vector<printf_argument>& args,
    bool positional,
    std::size_t position,
    std::size_t& next_index) noexcept -> result<int> {
    std::size_t index = 0;

    if (positional) {
        index = position - 1;
    } else {
        index = next_index;
        ++next_index;
    }

    if (index >= args.size()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    const printf_argument& arg = args[index];
    if (arg.kind == printf_arg_kind::signed_integer) {
        return static_cast<int>(std::get<std::intmax_t>(arg.value));
    }

    if (arg.kind == printf_arg_kind::unsigned_integer) {
        const auto value = std::get<std::uintmax_t>(arg.value);
        if (value > static_cast<std::uintmax_t>(std::numeric_limits<int>::max())) {
            return std::unexpected(make_error(error_t::out_of_range));
        }
        return static_cast<int>(value);
    }

    return std::unexpected(make_error(error_t::invalid_argument));
}

/**
 * @brief Applies width padding to a formatted field.
 *
 * @param field Field string.
 * @param spec Conversion spec.
 * @param prefix_size Prefix length that should precede zero padding.
 * @return Padded field.
 */
[[nodiscard]] inline auto apply_printf_width(
    std::u8string field,
    const printf_conversion_spec& spec,
    std::size_t prefix_size = 0) -> std::u8string {
    if (!spec.width.has_value()) {
        return field;
    }

    const std::size_t width = static_cast<std::size_t>(std::max(0, *spec.width));
    if (field.size() >= width) {
        return field;
    }

    const std::size_t pad_count = width - field.size();

    if (spec.left_justify) {
        append_repeated_ascii(field, u8' ', pad_count);
        return field;
    }

    if (spec.zero_pad && !spec.precision.has_value()) {
        std::u8string result;
        result.reserve(width);

        const std::size_t actual_prefix_size = std::min(prefix_size, field.size());
        result.append(field, 0, actual_prefix_size);
        append_repeated_ascii(result, u8'0', pad_count);
        result.append(field, actual_prefix_size, std::u8string::npos);
        return result;
    }

    std::u8string result;
    result.reserve(width);
    append_repeated_ascii(result, u8' ', pad_count);
    result += field;
    return result;
}

/**
 * @brief Formats a signed integer.
 *
 * @param value Source value.
 * @param spec Conversion spec.
 * @return Formatted field.
 */
[[nodiscard]] inline auto format_printf_signed(
    std::intmax_t value,
    const printf_conversion_spec& spec) -> std::u8string {
    bool negative = value < 0;
    std::uintmax_t magnitude = 0;

    if (negative) {
        magnitude = static_cast<std::uintmax_t>(-(value + 1)) + 1;
    } else {
        magnitude = static_cast<std::uintmax_t>(value);
    }

    std::u8string digits = unsigned_to_ascii(magnitude, 10, false);

    if (spec.precision.has_value()) {
        if (*spec.precision == 0 && magnitude == 0) {
            digits.clear();
        } else if (digits.size() < static_cast<std::size_t>(*spec.precision)) {
            std::u8string padded;
            padded.reserve(static_cast<std::size_t>(*spec.precision));
            append_repeated_ascii(
                padded,
                u8'0',
                static_cast<std::size_t>(*spec.precision) - digits.size());
            padded += digits;
            digits = std::move(padded);
        }
    }

    std::u8string prefix;
    if (negative) {
        prefix.push_back(u8'-');
    } else if (spec.force_sign) {
        prefix.push_back(u8'+');
    } else if (spec.space_sign) {
        prefix.push_back(u8' ');
    }

    std::u8string field = prefix + digits;
    return apply_printf_width(std::move(field), spec, prefix.size());
}

/**
 * @brief Formats an unsigned integer.
 *
 * @param value Source value.
 * @param spec Conversion spec.
 * @return Formatted field.
 */
[[nodiscard]] inline auto format_printf_unsigned(
    std::uintmax_t value,
    const printf_conversion_spec& spec) -> std::u8string {
    unsigned base = 10;
    bool upper = false;
    std::u8string alt_prefix;

    switch (spec.conversion) {
    case U'o':
        base = 8;
        if (spec.alternative && (value != 0 || spec.precision == 0)) {
            alt_prefix = u8"0";
        }
        break;
    case U'x':
        base = 16;
        if (spec.alternative && value != 0) {
            alt_prefix = u8"0x";
        }
        break;
    case U'X':
        base = 16;
        upper = true;
        if (spec.alternative && value != 0) {
            alt_prefix = u8"0X";
        }
        break;
    default:
        base = 10;
        break;
    }

    std::u8string digits = unsigned_to_ascii(value, base, upper);

    if (spec.precision.has_value()) {
        if (*spec.precision == 0 && value == 0) {
            digits.clear();
        } else if (digits.size() < static_cast<std::size_t>(*spec.precision)) {
            std::u8string padded;
            padded.reserve(static_cast<std::size_t>(*spec.precision));
            append_repeated_ascii(
                padded,
                u8'0',
                static_cast<std::size_t>(*spec.precision) - digits.size());
            padded += digits;
            digits = std::move(padded);
        }
    }

    std::u8string field = alt_prefix + digits;
    return apply_printf_width(std::move(field), spec, alt_prefix.size());
}

/**
 * @brief Formats one character conversion.
 *
 * @param value Source character.
 * @param spec Conversion spec.
 * @return Formatted field on success.
 */
[[nodiscard]] inline auto format_printf_character(
    char32_t value,
    const printf_conversion_spec& spec) -> result<std::u8string> {
    std::u8string field;
    const auto appended = append_utf8_char(field, value);
    if (!appended.has_value()) {
        return std::unexpected(appended.error());
    }

    return apply_printf_width(std::move(field), spec);
}

/**
 * @brief Formats one UTF-8 string conversion.
 *
 * @param value Source string.
 * @param spec Conversion spec.
 * @return Formatted field.
 */
[[nodiscard]] inline auto format_printf_utf8_string(
    std::u8string_view value,
    const printf_conversion_spec& spec) -> std::u8string {
    std::u8string field(value);

    if (spec.precision.has_value() &&
        static_cast<std::size_t>(*spec.precision) < field.size()) {
        field.resize(static_cast<std::size_t>(*spec.precision));
    }

    return apply_printf_width(std::move(field), spec);
}

/**
 * @brief Formats one narrow string conversion as UTF-8 bytes.
 *
 * @param value Source C string.
 * @param spec Conversion spec.
 * @return Formatted field.
 */
[[nodiscard]] inline auto format_printf_c_string(
    const char* value,
    const printf_conversion_spec& spec) -> std::u8string {
    if (value == nullptr) {
        return format_printf_utf8_string(u8"(null)", spec);
    }

    const std::string_view sv(value);
    std::u8string converted;
    converted.reserve(sv.size());

    for (unsigned char ch : sv) {
        converted.push_back(static_cast<char8_t>(ch));
    }

    return format_printf_utf8_string(converted, spec);
}

/**
 * @brief Formats one pointer conversion.
 *
 * @param value Source pointer.
 * @param spec Conversion spec.
 * @return Formatted field.
 */
[[nodiscard]] inline auto format_printf_pointer(
    const void* value,
    const printf_conversion_spec& spec) -> std::u8string {
    const auto raw = reinterpret_cast<std::uintptr_t>(value);

    printf_conversion_spec hex_spec = spec;
    hex_spec.conversion = U'x';
    hex_spec.alternative = true;

    return format_printf_unsigned(static_cast<std::uintmax_t>(raw), hex_spec);
}

/**
 * @brief Formats one floating conversion.
 *
 * This implementation is intentionally simple and uses snprintf-compatible
 * formatting through an intermediate narrow format string.
 *
 * @param value Source value.
 * @param spec Conversion spec.
 * @return Formatted field on success.
 */
[[nodiscard]] inline auto format_printf_floating(
    long double value,
    const printf_conversion_spec& spec) -> result<std::u8string> {
    std::string format = "%";

    if (spec.left_justify) {
        format += '-';
    }
    if (spec.force_sign) {
        format += '+';
    }
    if (spec.space_sign) {
        format += ' ';
    }
    if (spec.alternative) {
        format += '#';
    }
    if (spec.zero_pad) {
        format += '0';
    }
    if (spec.width.has_value()) {
        format += std::to_string(*spec.width);
    }
    if (spec.precision.has_value()) {
        format += '.';
        format += std::to_string(*spec.precision);
    }
    if (spec.length == printf_conversion_spec::length_t::L) {
        format += 'L';
    }

    format.push_back(static_cast<char>(spec.conversion));

    std::array<char, 512> buffer{};
    int written = 0;

    switch (spec.conversion) {
    case U'e':
    case U'E':
    case U'f':
    case U'F':
    case U'g':
    case U'G':
    case U'a':
    case U'A':
        written = std::snprintf(buffer.data(), buffer.size(), format.c_str(), value);
        break;
    default:
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (written < 0) {
        return std::unexpected(make_error(error_t::runtime_error));
    }

    return std::u8string(
        reinterpret_cast<const char8_t*>(buffer.data()),
        reinterpret_cast<const char8_t*>(buffer.data() + written));
}

/**
 * @brief Formats one conversion against one runtime argument.
 *
 * @param arg Runtime argument.
 * @param spec Conversion spec.
 * @return Formatted field on success.
 */
[[nodiscard]] inline auto format_printf_argument(
    const printf_argument& arg,
    const printf_conversion_spec& spec) -> result<std::u8string> {
    switch (spec.conversion) {
    case U'@':
        switch (arg.kind) {
        case printf_arg_kind::signed_integer:
            return format_printf_signed(std::get<std::intmax_t>(arg.value), spec);

        case printf_arg_kind::unsigned_integer:
            return format_printf_unsigned(std::get<std::uintmax_t>(arg.value), spec);

        case printf_arg_kind::floating:
            return format_printf_floating(std::get<long double>(arg.value), spec);

        case printf_arg_kind::character:
            return format_printf_character(std::get<char32_t>(arg.value), spec);

        case printf_arg_kind::c_string:
            return format_printf_c_string(std::get<const char*>(arg.value), spec);

        case printf_arg_kind::utf8_string:
            return format_printf_utf8_string(std::get<std::u8string_view>(arg.value), spec);

        case printf_arg_kind::pointer:
            return format_printf_pointer(std::get<const void*>(arg.value), spec);
        }

        return std::unexpected(make_error(error_t::invalid_argument));

    case U'd':
    case U'i':
        if (arg.kind != printf_arg_kind::signed_integer) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }
        return format_printf_signed(std::get<std::intmax_t>(arg.value), spec);

    case U'u':
    case U'o':
    case U'x':
    case U'X':
        if (arg.kind == printf_arg_kind::unsigned_integer) {
            return format_printf_unsigned(std::get<std::uintmax_t>(arg.value), spec);
        }
        if (arg.kind == printf_arg_kind::signed_integer) {
            return format_printf_unsigned(
                static_cast<std::uintmax_t>(std::get<std::intmax_t>(arg.value)),
                spec);
        }
        return std::unexpected(make_error(error_t::invalid_argument));

    case U'c':
        if (arg.kind != printf_arg_kind::character) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }
        return format_printf_character(std::get<char32_t>(arg.value), spec);

    case U's':
        if (arg.kind == printf_arg_kind::utf8_string) {
            return format_printf_utf8_string(std::get<std::u8string_view>(arg.value), spec);
        }
        if (arg.kind == printf_arg_kind::c_string) {
            return format_printf_c_string(std::get<const char*>(arg.value), spec);
        }
        return std::unexpected(make_error(error_t::invalid_argument));

    case U'p':
        if (arg.kind != printf_arg_kind::pointer) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }
        return format_printf_pointer(std::get<const void*>(arg.value), spec);

    case U'e':
    case U'E':
    case U'f':
    case U'F':
    case U'g':
    case U'G':
    case U'a':
    case U'A':
        if (arg.kind != printf_arg_kind::floating) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }
        return format_printf_floating(std::get<long double>(arg.value), spec);

    default:
        return std::unexpected(make_error(error_t::invalid_argument));
    }
}

/**
 * @brief Formats a parsed printf token sequence.
 *
 * @param tokens Parsed tokens.
 * @param args Runtime arguments.
 * @return Formatted UTF-8 string on success.
 */
[[nodiscard]] inline auto format_printf_tokens(
    const std::vector<printf_token>& tokens,
    const std::vector<printf_argument>& args) -> result<std::u8string> {
    std::u8string output;
    std::size_t next_arg_index = 0;
    bool saw_positional = false;
    bool saw_sequential = false;

    for (const auto& token : tokens) {
        if (!token.is_conversion) {
            output += token.literal;
            continue;
        }

        auto spec = token.spec;

        if (spec.positional || spec.width_positional || spec.precision_positional) {
            saw_positional = true;
        }
        if (!spec.positional ||
            (spec.width_from_arg && !spec.width_positional) ||
            (spec.precision_from_arg && !spec.precision_positional)) {
            saw_sequential = true;
        }

        if (saw_positional && saw_sequential) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        if (spec.width_from_arg) {
            const auto width = select_printf_int_argument(
                args,
                spec.width_positional,
                spec.width_position,
                next_arg_index);
            if (!width.has_value()) {
                return std::unexpected(width.error());
            }

            if (*width < 0) {
                spec.left_justify = true;
                spec.width = -*width;
            } else {
                spec.width = *width;
            }
        }

        if (spec.precision_from_arg) {
            const auto precision = select_printf_int_argument(
                args,
                spec.precision_positional,
                spec.precision_position,
                next_arg_index);
            if (!precision.has_value()) {
                return std::unexpected(precision.error());
            }

            if (*precision >= 0) {
                spec.precision = *precision;
            } else {
                spec.precision.reset();
            }
        }

        const auto arg = select_printf_argument(args, spec, next_arg_index);
        if (!arg.has_value()) {
            return std::unexpected(arg.error());
        }

        const auto field = format_printf_argument(**arg, spec);
        if (!field.has_value()) {
            return std::unexpected(field.error());
        }

        output += *field;
    }

    return output;
}

/**
 * @brief Formats a printf-style UTF-8 string with runtime arguments.
 *
 * @param format Format string.
 * @param args Runtime arguments.
 * @return Formatted UTF-8 string on success.
 */
[[nodiscard]] inline auto vformat_printf(
    std::u8string_view format,
    const std::vector<printf_argument>& args) -> result<std::u8string> {
    const auto tokens = parse_printf_tokens(format);
    if (!tokens.has_value()) {
        return std::unexpected(tokens.error());
    }

    return format_printf_tokens(*tokens, args);
}

template<typename... Args>
[[nodiscard]] inline auto format_printf(
    std::u8string_view format,
    Args&&... args) -> result<std::u8string> {
    std::vector<printf_argument> runtime_args;
    runtime_args.reserve(sizeof...(Args));
    (runtime_args.push_back(make_printf_argument(std::forward<Args>(args))), ...);
    return vformat_printf(format, runtime_args);
}

} // namespace xer::detail

#endif /* XER_BITS_PRINTF_FORMAT_H_INCLUDED_ */
