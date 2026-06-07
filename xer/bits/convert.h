/**
 * @file xer/bits/convert.h
 * @brief Internal generic value conversion function implementations.
 */

#pragma once

#ifndef XER_BITS_CONVERT_H_INCLUDED_
#define XER_BITS_CONVERT_H_INCLUDED_

#include <concepts>
#include <cstddef>
#include <expected>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include <xer/arithmetic.h>
#include <xer/bits/common.h>
#include <xer/bits/printf.h>
#include <xer/bits/printf_format.h>
#include <xer/bits/scanf.h>
#include <xer/bits/strto.h>
#include <xer/bits/strto_common.h>
#include <xer/bits/strto_floating.h>
#include <xer/bits/unicode_common.h>
#include <xer/error.h>
#include <xer/path.h>

namespace xer::detail {

template<typename T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

template<typename T>
concept to_character =
    std::same_as<remove_cvref_t<T>, char> ||
    std::same_as<remove_cvref_t<T>, char8_t> ||
    std::same_as<remove_cvref_t<T>, wchar_t> ||
    std::same_as<remove_cvref_t<T>, char16_t> ||
    std::same_as<remove_cvref_t<T>, char32_t>;

template<typename T>
concept to_integer_character =
    std::same_as<remove_cvref_t<T>, signed char> ||
    std::same_as<remove_cvref_t<T>, unsigned char>;

template<typename T>
concept to_supported_text_character =
    std::same_as<remove_cvref_t<T>, char8_t> ||
    std::same_as<remove_cvref_t<T>, wchar_t> ||
    std::same_as<remove_cvref_t<T>, char16_t> ||
    std::same_as<remove_cvref_t<T>, char32_t>;

template<typename T>
struct to_basic_string_character {
    using type = void;
};

template<typename CharT, typename Traits, typename Allocator>
struct to_basic_string_character<std::basic_string<CharT, Traits, Allocator>> {
    using type = CharT;
};

template<typename T>
struct to_basic_string_view_character {
    using type = void;
};

template<typename CharT, typename Traits>
struct to_basic_string_view_character<std::basic_string_view<CharT, Traits>> {
    using type = CharT;
};

template<typename T>
using to_basic_string_character_t =
    typename to_basic_string_character<remove_cvref_t<T>>::type;

template<typename T>
using to_basic_string_view_character_t =
    typename to_basic_string_view_character<remove_cvref_t<T>>::type;

template<typename T>
concept to_owning_supported_text_string =
    to_supported_text_character<to_basic_string_character_t<T>>;

template<typename T>
concept to_supported_text_string_view =
    to_supported_text_character<to_basic_string_view_character_t<T>>;

template<typename T>
concept to_supported_text_pointer =
    std::is_pointer_v<remove_cvref_t<T>> &&
    to_supported_text_character<std::remove_pointer_t<remove_cvref_t<T>>>;

template<typename T>
concept to_supported_text_array =
    std::is_array_v<std::remove_reference_t<T>> &&
    to_supported_text_character<std::remove_extent_t<std::remove_reference_t<T>>>;

template<typename T>
concept to_supported_text_source =
    to_owning_supported_text_string<T> ||
    to_supported_text_string_view<T> ||
    to_supported_text_pointer<T> ||
    to_supported_text_array<T>;

template<typename T>
concept to_narrow_text_source =
    std::same_as<remove_cvref_t<T>, std::string> ||
    std::same_as<remove_cvref_t<T>, std::string_view> ||
    (std::is_pointer_v<remove_cvref_t<T>> &&
     std::same_as<std::remove_cv_t<std::remove_pointer_t<remove_cvref_t<T>>>, char>) ||
    (std::is_array_v<std::remove_reference_t<T>> &&
     std::same_as<std::remove_cv_t<std::remove_extent_t<std::remove_reference_t<T>>>, char>);

template<typename T>
concept to_text_output =
    std::same_as<remove_cvref_t<T>, std::u8string> ||
    std::same_as<remove_cvref_t<T>, std::u16string> ||
    std::same_as<remove_cvref_t<T>, std::u32string> ||
    std::same_as<remove_cvref_t<T>, std::wstring>;

template<typename T>
concept to_numeric_value =
    std::is_arithmetic_v<remove_cvref_t<T>> &&
    !to_character<T>;

template<typename T>
concept to_non_bool_numeric_value =
    to_numeric_value<T> && !std::same_as<remove_cvref_t<T>, bool>;

template<typename T>
concept to_path = std::same_as<remove_cvref_t<T>, xer::path>;

template<to_supported_text_source T>
struct to_text_source_character;

template<typename CharT, typename Traits, typename Allocator>
struct to_text_source_character<std::basic_string<CharT, Traits, Allocator>> {
    using type = CharT;
};

template<typename CharT, typename Traits>
struct to_text_source_character<std::basic_string_view<CharT, Traits>> {
    using type = CharT;
};

template<typename T>
    requires(to_supported_text_pointer<T>)
struct to_text_source_character<T> {
    using type = std::remove_cv_t<std::remove_pointer_t<remove_cvref_t<T>>>;
};

template<typename T>
    requires(to_supported_text_array<T>)
struct to_text_source_character<T> {
    using type = std::remove_cv_t<std::remove_extent_t<std::remove_reference_t<T>>>;
};

template<to_supported_text_source T>
using to_text_source_character_t = typename to_text_source_character<remove_cvref_t<T>>::type;

template<to_supported_text_array T>
[[nodiscard]] constexpr auto to_array_view(T&& value)
    -> std::basic_string_view<std::remove_cv_t<std::remove_extent_t<std::remove_reference_t<T>>>>
{
    using raw_array_type = std::remove_reference_t<T>;
    using char_type = std::remove_cv_t<std::remove_extent_t<raw_array_type>>;
    constexpr std::size_t extent = std::extent_v<raw_array_type>;

    std::size_t size = extent;
    if constexpr (extent > 0) {
        if (value[extent - 1] == char_type{}) {
            --size;
        }
    }

    return std::basic_string_view<char_type>(value, size);
}

template<to_supported_text_source T>
[[nodiscard]] constexpr auto to_text_view(T&& value)
    -> result<std::basic_string_view<to_text_source_character_t<T>>>
{
    using source_type = remove_cvref_t<T>;
    using char_type = to_text_source_character_t<T>;

    if constexpr (to_supported_text_array<T>) {
        return to_array_view(std::forward<T>(value));
    } else if constexpr (to_supported_text_pointer<T>) {
        if (value == nullptr) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        return std::basic_string_view<char_type>(value);
    } else if constexpr (to_supported_text_string_view<T>) {
        return std::basic_string_view<char_type>(value.data(), value.size());
    } else {
        static_assert(to_owning_supported_text_string<source_type>);
        return std::basic_string_view<char_type>(value.data(), value.size());
    }
}

template<typename CharT>
[[nodiscard]] inline auto text_view_to_u8string(
    std::basic_string_view<CharT> value) -> result<std::u8string>
{
    if constexpr (std::same_as<CharT, char8_t>) {
        return std::u8string(value);
    } else if constexpr (std::same_as<CharT, char16_t>) {
        return utf16_to_printf_utf8(value);
    } else if constexpr (std::same_as<CharT, char32_t>) {
        return utf32_to_printf_utf8(value);
    } else if constexpr (std::same_as<CharT, wchar_t>) {
        return wide_to_printf_utf8(value);
    } else {
        return std::unexpected(make_error(error_t::invalid_argument));
    }
}

template<to_supported_text_source T>
[[nodiscard]] inline auto text_source_to_u8string(T&& value) -> result<std::u8string>
{
    auto view = to_text_view(std::forward<T>(value));
    if (!view.has_value()) {
        return std::unexpected(view.error());
    }

    return text_view_to_u8string(*view);
}

[[nodiscard]] inline auto u8string_to_u16string(
    std::u8string_view value) -> result<std::u16string>
{
    return scan_utf8_to_u16string(value);
}

[[nodiscard]] inline auto u8string_to_u32string(
    std::u8string_view value) -> result<std::u32string>
{
    return scan_utf8_to_u32string(value);
}

[[nodiscard]] inline auto u8string_to_wstring(
    std::u8string_view value) -> result<std::wstring>
{
    return scan_utf8_to_wstring(value);
}

template<typename To>
[[nodiscard]] inline auto u8string_to_text_output(
    std::u8string_view value) -> result<remove_cvref_t<To>>
    requires(to_text_output<To>)
{
    using to_type = remove_cvref_t<To>;

    if constexpr (std::same_as<to_type, std::u8string>) {
        return std::u8string(value);
    } else if constexpr (std::same_as<to_type, std::u16string>) {
        return u8string_to_u16string(value);
    } else if constexpr (std::same_as<to_type, std::u32string>) {
        return u8string_to_u32string(value);
    } else {
        static_assert(std::same_as<to_type, std::wstring>);
        return u8string_to_wstring(value);
    }
}

template<typename To, typename From>
[[nodiscard]] inline auto format_to_text_output(const From& value)
    -> result<remove_cvref_t<To>>
    requires(to_text_output<To>)
{
    std::u8string formatted;
    auto formatted_result = xer::sprintf(formatted, u8"%@", value);
    if (!formatted_result.has_value()) {
        return std::unexpected(formatted_result.error());
    }

    return u8string_to_text_output<To>(formatted);
}

template<typename CharT>
[[nodiscard]] constexpr auto text_has_possible_number(
    std::basic_string_view<CharT> value,
    bool floating) noexcept -> bool
{
    std::size_t offset = 0;
    while (offset < value.size() && is_ascii_space(value[offset])) {
        ++offset;
    }

    if (offset >= value.size()) {
        return false;
    }

    if (value[offset] == ascii_char<CharT>('+') ||
        value[offset] == ascii_char<CharT>('-')) {
        ++offset;
    }

    if (offset >= value.size()) {
        return false;
    }

    const CharT ch = value[offset];
    if (ascii_digit_value(ch) >= 0) {
        return true;
    }

    if (floating) {
        return ch == ascii_char<CharT>('.') ||
               ascii_iequal(ch, 'i') ||
               ascii_iequal(ch, 'n');
    }

    return false;
}

template<typename To, typename CharT>
[[nodiscard]] inline auto parse_text_view_to_number(
    std::basic_string_view<CharT> value) -> result<remove_cvref_t<To>>
    requires(to_numeric_value<To>)
{
    using to_type = remove_cvref_t<To>;

    if constexpr (std::same_as<to_type, bool>) {
        return std::unexpected(make_error(error_t::invalid_argument));
    } else if constexpr (std::floating_point<to_type>) {
        if (!text_has_possible_number(value, true)) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        typename std::basic_string_view<CharT>::const_iterator endit{};
        auto parsed = xer::strto<to_type>(value, &endit);
        if (!parsed.has_value()) {
            return std::unexpected(parsed.error());
        }

        if (endit != value.end()) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        return *parsed;
    } else if constexpr (std::integral<to_type>) {
        if (!text_has_possible_number(value, false)) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        typename std::basic_string_view<CharT>::const_iterator endit{};

        if constexpr (std::is_signed_v<to_type>) {
            auto parsed = xer::strto<long long>(value, &endit, 0);
            if (!parsed.has_value()) {
                return std::unexpected(parsed.error());
            }

            if (endit != value.end()) {
                return std::unexpected(make_error(error_t::invalid_argument));
            }

            if (!xer::in_range<to_type>(*parsed)) {
                return std::unexpected(make_error(error_t::range));
            }

            return static_cast<to_type>(*parsed);
        } else {
            auto parsed = xer::strto<unsigned long long>(value, &endit, 0);
            if (!parsed.has_value()) {
                return std::unexpected(parsed.error());
            }

            if (endit != value.end()) {
                return std::unexpected(make_error(error_t::invalid_argument));
            }

            if (!xer::in_range<to_type>(*parsed)) {
                return std::unexpected(make_error(error_t::range));
            }

            return static_cast<to_type>(*parsed);
        }
    } else {
        return std::unexpected(make_error(error_t::invalid_argument));
    }
}

template<typename To, to_supported_text_source From>
[[nodiscard]] inline auto parse_text_source_to_number(From&& value)
    -> result<remove_cvref_t<To>>
    requires(to_numeric_value<To>)
{
    auto view = to_text_view(std::forward<From>(value));
    if (!view.has_value()) {
        return std::unexpected(view.error());
    }

    return parse_text_view_to_number<To>(*view);
}

[[nodiscard]] inline auto append_code_point_to_u8string(
    std::u8string& out,
    char32_t value) -> result<void>
{
    return append_utf8_char(out, value);
}

[[nodiscard]] inline auto append_code_point_to_u16string(
    std::u16string& out,
    char32_t value) -> result<void>
{
    if (!is_unicode_scalar_value(value)) {
        return std::unexpected(make_error(error_t::encoding_error));
    }

    if (value <= unicode_bmp_max_code_point) {
        out.push_back(static_cast<char16_t>(value));
        return {};
    }

    const char32_t value20 = value - unicode_supplementary_first;
    out.push_back(static_cast<char16_t>(unicode_high_surrogate_first + ((value20 >> 10) & 0x3ffu)));
    out.push_back(static_cast<char16_t>(unicode_low_surrogate_first + (value20 & 0x3ffu)));
    return {};
}

[[nodiscard]] inline auto append_code_point_to_u32string(
    std::u32string& out,
    char32_t value) -> result<void>
{
    if (!is_unicode_scalar_value(value)) {
        return std::unexpected(make_error(error_t::encoding_error));
    }

    out.push_back(value);
    return {};
}

[[nodiscard]] inline auto append_code_point_to_wstring(
    std::wstring& out,
    char32_t value) -> result<void>
{
    if constexpr (sizeof(wchar_t) == sizeof(char16_t)) {
        std::u16string temp;
        auto appended = append_code_point_to_u16string(temp, value);
        if (!appended.has_value()) {
            return std::unexpected(appended.error());
        }

        out.reserve(out.size() + temp.size());
        for (char16_t ch : temp) {
            out.push_back(static_cast<wchar_t>(ch));
        }

        return {};
    } else if constexpr (sizeof(wchar_t) == sizeof(char32_t)) {
        if (!is_unicode_scalar_value(value)) {
            return std::unexpected(make_error(error_t::encoding_error));
        }

        out.push_back(static_cast<wchar_t>(value));
        return {};
    } else {
        return std::unexpected(make_error(error_t::encoding_error));
    }
}

template<typename To>
[[nodiscard]] inline auto code_point_to_text_output(char32_t value)
    -> result<remove_cvref_t<To>>
    requires(to_text_output<To>)
{
    using to_type = remove_cvref_t<To>;
    to_type out;

    if constexpr (std::same_as<to_type, std::u8string>) {
        auto appended = append_code_point_to_u8string(out, value);
        if (!appended.has_value()) {
            return std::unexpected(appended.error());
        }
    } else if constexpr (std::same_as<to_type, std::u16string>) {
        auto appended = append_code_point_to_u16string(out, value);
        if (!appended.has_value()) {
            return std::unexpected(appended.error());
        }
    } else if constexpr (std::same_as<to_type, std::u32string>) {
        auto appended = append_code_point_to_u32string(out, value);
        if (!appended.has_value()) {
            return std::unexpected(appended.error());
        }
    } else {
        static_assert(std::same_as<to_type, std::wstring>);
        auto appended = append_code_point_to_wstring(out, value);
        if (!appended.has_value()) {
            return std::unexpected(appended.error());
        }
    }

    return out;
}

template<typename T>
[[nodiscard]] constexpr auto character_to_code_point(T value) noexcept -> char32_t
    requires(to_character<T>)
{
    using value_type = remove_cvref_t<T>;

    if constexpr (std::same_as<value_type, char>) {
        return static_cast<unsigned char>(value);
    } else {
        return static_cast<char32_t>(value);
    }
}

[[nodiscard]] constexpr auto code_point_to_char(char32_t value) -> result<char>
{
    if (value > 0x7fu) {
        return std::unexpected(make_error(error_t::range));
    }

    return static_cast<char>(value);
}

[[nodiscard]] constexpr auto code_point_to_char8(char32_t value) -> result<char8_t>
{
    if (value > 0x7fu) {
        return std::unexpected(make_error(error_t::range));
    }

    return static_cast<char8_t>(value);
}

[[nodiscard]] constexpr auto code_point_to_char16(char32_t value) -> result<char16_t>
{
    if (!is_unicode_bmp_scalar_value(value)) {
        return std::unexpected(make_error(error_t::range));
    }

    return static_cast<char16_t>(value);
}

[[nodiscard]] constexpr auto code_point_to_wchar(char32_t value) -> result<wchar_t>
{
    if constexpr (sizeof(wchar_t) == sizeof(char16_t)) {
        auto converted = code_point_to_char16(value);
        if (!converted.has_value()) {
            return std::unexpected(converted.error());
        }

        return static_cast<wchar_t>(*converted);
    } else if constexpr (sizeof(wchar_t) == sizeof(char32_t)) {
        if (!is_unicode_scalar_value(value)) {
            return std::unexpected(make_error(error_t::range));
        }

        return static_cast<wchar_t>(value);
    } else {
        return std::unexpected(make_error(error_t::range));
    }
}

template<typename To>
[[nodiscard]] constexpr auto code_point_to_character(char32_t value)
    -> result<remove_cvref_t<To>>
    requires(to_character<To>)
{
    using to_type = remove_cvref_t<To>;

    if constexpr (std::same_as<to_type, char>) {
        return code_point_to_char(value);
    } else if constexpr (std::same_as<to_type, char8_t>) {
        return code_point_to_char8(value);
    } else if constexpr (std::same_as<to_type, char16_t>) {
        return code_point_to_char16(value);
    } else if constexpr (std::same_as<to_type, wchar_t>) {
        return code_point_to_wchar(value);
    } else {
        static_assert(std::same_as<to_type, char32_t>);
        if (!is_unicode_scalar_value(value)) {
            return std::unexpected(make_error(error_t::range));
        }

        return value;
    }
}

[[nodiscard]] inline auto decode_single_utf8_code_point(
    std::u8string_view value) -> result<char32_t>
{
    std::size_t index = 0;
    auto decoded = decode_one_utf8(value, index);
    if (!decoded.has_value()) {
        return std::unexpected(decoded.error());
    }

    if (index != value.size()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return *decoded;
}

[[nodiscard]] constexpr auto decode_single_utf16_code_point(
    std::u16string_view value) -> result<char32_t>
{
    if (value.empty()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (value.size() == 1) {
        const char16_t ch = value[0];
        if (is_unicode_surrogate(ch)) {
            return std::unexpected(make_error(error_t::encoding_error));
        }

        return static_cast<char32_t>(ch);
    }

    if (value.size() == 2 && is_unicode_high_surrogate(value[0]) &&
        is_unicode_low_surrogate(value[1])) {
        return combine_unicode_surrogates(value[0], value[1]);
    }

    return std::unexpected(make_error(error_t::invalid_argument));
}

[[nodiscard]] constexpr auto decode_single_utf32_code_point(
    std::u32string_view value) -> result<char32_t>
{
    if (value.size() != 1) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (!is_unicode_scalar_value(value[0])) {
        return std::unexpected(make_error(error_t::encoding_error));
    }

    return value[0];
}

[[nodiscard]] inline auto decode_single_wide_code_point(
    std::wstring_view value) -> result<char32_t>
{
    if constexpr (sizeof(wchar_t) == sizeof(char16_t)) {
        const auto first = reinterpret_cast<const char16_t*>(value.data());
        return decode_single_utf16_code_point(std::u16string_view(first, value.size()));
    } else if constexpr (sizeof(wchar_t) == sizeof(char32_t)) {
        if (value.size() != 1) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        const auto ch = static_cast<char32_t>(value[0]);
        if (!is_unicode_scalar_value(ch)) {
            return std::unexpected(make_error(error_t::encoding_error));
        }

        return ch;
    } else {
        return std::unexpected(make_error(error_t::encoding_error));
    }
}

template<typename CharT>
[[nodiscard]] inline auto decode_single_text_code_point(
    std::basic_string_view<CharT> value) -> result<char32_t>
{
    if constexpr (std::same_as<CharT, char8_t>) {
        return decode_single_utf8_code_point(value);
    } else if constexpr (std::same_as<CharT, char16_t>) {
        return decode_single_utf16_code_point(value);
    } else if constexpr (std::same_as<CharT, char32_t>) {
        return decode_single_utf32_code_point(value);
    } else if constexpr (std::same_as<CharT, wchar_t>) {
        return decode_single_wide_code_point(value);
    } else {
        return std::unexpected(make_error(error_t::invalid_argument));
    }
}

template<typename To, typename From>
[[nodiscard]] constexpr auto arithmetic_to_arithmetic(From value)
    -> result<remove_cvref_t<To>>
    requires(to_numeric_value<To> && to_numeric_value<From>)
{
    using to_type = remove_cvref_t<To>;

    if constexpr (std::same_as<to_type, bool>) {
        return std::unexpected(make_error(error_t::invalid_argument));
    } else {
        if (!xer::in_range<to_type>(value)) {
            return std::unexpected(make_error(error_t::range));
        }

        return static_cast<to_type>(value);
    }
}

template<typename From>
[[nodiscard]] constexpr auto pointer_conversion_removes_cv() noexcept -> bool
{
    return false;
}

} // namespace xer::detail

namespace xer {

/**
 * @brief Converts a value to another type using xer's safe conversion rules.
 *
 * `to<T>` is not a C++ cast expression. It may parse, format, validate, and
 * fail. Ambiguous narrow `char` strings are intentionally not interpreted as
 * text by this function.
 *
 * @tparam To Destination type.
 * @tparam From Source type.
 * @param from Source value.
 * @return Converted value on success.
 */
template<typename To, typename From>
[[nodiscard]] inline auto to(From&& from) -> result<To>
{
    using to_type = detail::remove_cvref_t<To>;
    using from_type = detail::remove_cvref_t<From>;

    if constexpr (std::same_as<To, from_type> || std::same_as<to_type, from_type>) {
        return static_cast<To>(std::forward<From>(from));
    } else if constexpr (detail::to_narrow_text_source<From> ||
                         detail::to_narrow_text_source<To>) {
        return std::unexpected(make_error(error_t::invalid_argument));
    } else if constexpr (detail::to_character<To> && detail::to_supported_text_source<From>) {
        auto view = detail::to_text_view(std::forward<From>(from));
        if (!view.has_value()) {
            return std::unexpected(view.error());
        }

        auto code_point = detail::decode_single_text_code_point(*view);
        if (!code_point.has_value()) {
            return std::unexpected(code_point.error());
        }

        auto converted = detail::code_point_to_character<To>(*code_point);
        if (!converted.has_value()) {
            return std::unexpected(converted.error());
        }

        return static_cast<To>(*converted);
    } else if constexpr (detail::to_supported_text_source<From> &&
                         detail::to_numeric_value<To>) {
        auto converted = detail::parse_text_source_to_number<To>(std::forward<From>(from));
        if (!converted.has_value()) {
            return std::unexpected(converted.error());
        }

        return static_cast<To>(*converted);
    } else if constexpr (detail::to_supported_text_source<From> &&
                         detail::to_text_output<To>) {
        auto converted = detail::text_source_to_u8string(std::forward<From>(from));
        if (!converted.has_value()) {
            return std::unexpected(converted.error());
        }

        auto output = detail::u8string_to_text_output<To>(*converted);
        if (!output.has_value()) {
            return std::unexpected(output.error());
        }

        return static_cast<To>(std::move(*output));
    } else if constexpr (detail::to_supported_text_source<From> && detail::to_path<To>) {
        auto converted = detail::text_source_to_u8string(std::forward<From>(from));
        if (!converted.has_value()) {
            return std::unexpected(converted.error());
        }

        return To(std::u8string_view(*converted));
    } else if constexpr (detail::to_character<From> && detail::to_character<To>) {
        const char32_t code_point = detail::character_to_code_point(from);
        auto converted = detail::code_point_to_character<To>(code_point);
        if (!converted.has_value()) {
            return std::unexpected(converted.error());
        }

        return static_cast<To>(*converted);
    } else if constexpr (detail::to_character<From> && detail::to_text_output<To>) {
        const char32_t code_point = detail::character_to_code_point(from);
        auto converted = detail::code_point_to_text_output<To>(code_point);
        if (!converted.has_value()) {
            return std::unexpected(converted.error());
        }

        return static_cast<To>(std::move(*converted));
    } else if constexpr ((detail::to_character<From> && detail::to_numeric_value<To>) ||
                         (detail::to_numeric_value<From> && detail::to_character<To>)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    } else if constexpr (detail::to_numeric_value<To> && detail::to_numeric_value<From>) {
        auto converted = detail::arithmetic_to_arithmetic<To>(from);
        if (!converted.has_value()) {
            return std::unexpected(converted.error());
        }

        return static_cast<To>(*converted);
    } else if constexpr (detail::to_text_output<To> && detail::to_path<From>) {
        auto converted = detail::u8string_to_text_output<To>(from.str());
        if (!converted.has_value()) {
            return std::unexpected(converted.error());
        }

        return static_cast<To>(std::move(*converted));
    } else if constexpr (detail::to_text_output<To>) {
        auto converted = detail::format_to_text_output<To>(from);
        if (!converted.has_value()) {
            return std::unexpected(converted.error());
        }

        return static_cast<To>(std::move(*converted));
    } else if constexpr (std::is_pointer_v<to_type> || std::is_pointer_v<from_type>) {
        if constexpr (std::is_convertible_v<From, To>) {
            return static_cast<To>(from);
        } else {
            return std::unexpected(make_error(error_t::invalid_argument));
        }
    } else if constexpr (std::is_convertible_v<From, To>) {
        return static_cast<To>(std::forward<From>(from));
    } else {
        return std::unexpected(make_error(error_t::invalid_argument));
    }
}

} // namespace xer

#endif /* XER_BITS_CONVERT_H_INCLUDED_ */
