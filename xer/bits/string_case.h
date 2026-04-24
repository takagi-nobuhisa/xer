/**
 * @file xer/bits/string_case.h
 * @brief Internal string character transformation implementations.
 */

#pragma once

#ifndef XER_BITS_STRING_CASE_H_INCLUDED_
#define XER_BITS_STRING_CASE_H_INCLUDED_

#include <cstddef>
#include <cstdint>
#include <expected>
#include <limits>
#include <string>
#include <string_view>
#include <xer/bits/kana_width.h>
#include <type_traits>
#include <vector>

#include <xer/bits/advanced_encoding.h>
#include <xer/bits/common.h>
#include <xer/bits/string_character.h>
#include <xer/bits/string_read.h>
#include <xer/bits/toctrans.h>
#include <xer/error.h>

namespace xer::detail {

/**
 * @brief Returns the effective length of an array string for transformation.
 *
 * If the final code unit is NUL, it is excluded. Otherwise, the whole array is
 * used. Embedded NUL code units are treated as ordinary code units.
 *
 * @tparam CharT Character type.
 * @tparam N Array size.
 * @param source Source array.
 * @return Effective code-unit length.
 */
template<typename CharT, std::size_t N>
    requires supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] constexpr auto transformation_array_string_length(
    const CharT (&source)[N]) noexcept -> std::size_t
{
    if constexpr (N == 0) {
        return 0;
    } else {
        using bare_char_t = std::remove_cv_t<CharT>;

        if (source[N - 1] == static_cast<bare_char_t>(0)) {
            return N - 1;
        }

        return N;
    }
}

/**
 * @brief Creates a string view for transformation from an array string.
 *
 * @tparam CharT Character type.
 * @tparam N Array size.
 * @param source Source array.
 * @return String view over the effective array contents.
 */
template<typename CharT, std::size_t N>
    requires supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] constexpr auto transformation_array_string_view(
    const CharT (&source)[N]) noexcept
    -> std::basic_string_view<std::remove_cv_t<CharT>>
{
    using bare_char_t = std::remove_cv_t<CharT>;
    return std::basic_string_view<bare_char_t>(
        source,
        transformation_array_string_length(source));
}

/**
 * @brief Appends a transformed byte-sized code point to a string.
 *
 * @tparam CharT Destination character type.
 * @param output Destination string.
 * @param value Code point to append.
 * @return Success or error.
 */
template<typename CharT>
    requires(std::same_as<CharT, char> || std::same_as<CharT, unsigned char>)
[[nodiscard]] inline auto append_byte_character(
    std::basic_string<CharT>& output,
    const char32_t value) -> result<void>
{
    if (value > static_cast<char32_t>(std::numeric_limits<unsigned char>::max())) {
        return std::unexpected(make_error(error_t::encoding_error));
    }

    output.push_back(static_cast<CharT>(static_cast<unsigned char>(value)));
    return {};
}

/**
 * @brief Appends a UTF-8 encoded code point to a string.
 *
 * @param output Destination UTF-8 string.
 * @param value Code point to append.
 * @return Success or error.
 */
[[nodiscard]] inline auto append_utf8_character(
    std::u8string& output,
    const char32_t value) -> result<void>
{
    const std::uint32_t packed = xer::advanced::utf32_to_packed_utf8(value);
    if (packed == xer::advanced::detail::invalid_packed_utf8) {
        return std::unexpected(make_error(error_t::encoding_error));
    }

    output.push_back(static_cast<char8_t>(packed & 0xFFu));

    if ((packed >> 8) != 0) {
        output.push_back(static_cast<char8_t>((packed >> 8) & 0xFFu));
    }

    if ((packed >> 16) != 0) {
        output.push_back(static_cast<char8_t>((packed >> 16) & 0xFFu));
    }

    if ((packed >> 24) != 0) {
        output.push_back(static_cast<char8_t>((packed >> 24) & 0xFFu));
    }

    return {};
}

/**
 * @brief Appends a UTF-16 encoded code point to a string.
 *
 * @param output Destination UTF-16 string.
 * @param value Code point to append.
 * @return Success or error.
 */
[[nodiscard]] inline auto append_utf16_character(
    std::u16string& output,
    const char32_t value) -> result<void>
{
    const std::uint32_t packed = xer::advanced::utf32_to_packed_utf16(value);
    if (packed == xer::advanced::detail::invalid_packed_utf16) {
        return std::unexpected(make_error(error_t::encoding_error));
    }

    output.push_back(static_cast<char16_t>(packed & 0xFFFFu));

    if ((packed >> 16) != 0) {
        output.push_back(static_cast<char16_t>((packed >> 16) & 0xFFFFu));
    }

    return {};
}

/**
 * @brief Appends a UTF-32 code point to a string.
 *
 * @param output Destination UTF-32 string.
 * @param value Code point to append.
 * @return Success or error.
 */
[[nodiscard]] inline auto append_utf32_character(
    std::u32string& output,
    const char32_t value) -> result<void>
{
    if (!is_valid_code_point(value)) {
        return std::unexpected(make_error(error_t::encoding_error));
    }

    output.push_back(value);
    return {};
}

/**
 * @brief Appends a code point encoded for the destination string character type.
 *
 * @tparam CharT Destination character type.
 * @param output Destination string.
 * @param value Code point to append.
 * @return Success or error.
 */
template<supported_string_character CharT>
[[nodiscard]] inline auto append_transformed_code_point(
    std::basic_string<CharT>& output,
    const char32_t value) -> result<void>
{
    if constexpr (std::same_as<CharT, char> ||
                  std::same_as<CharT, unsigned char>) {
        return append_byte_character(output, value);
    } else if constexpr (std::same_as<CharT, char8_t>) {
        return append_utf8_character(output, value);
    } else if constexpr (std::same_as<CharT, char16_t>) {
        return append_utf16_character(output, value);
    } else {
        return append_utf32_character(output, value);
    }
}


/**
 * @brief Decodes a string into Unicode code points for string-level transforms.
 *
 * @tparam CharT Source character type.
 * @param source Source string.
 * @return Decoded code points on success.
 */
template<supported_string_character CharT>
[[nodiscard]] inline auto decode_string_code_points(
    const std::basic_string_view<CharT> source) -> result<std::vector<char32_t>>
{
    std::vector<char32_t> output;
    output.reserve(source.size());

    if constexpr (std::same_as<CharT, char> ||
                  std::same_as<CharT, unsigned char>) {
        using unsigned_char_type = std::make_unsigned_t<CharT>;

        for (const CharT unit : source) {
            output.push_back(static_cast<char32_t>(
                static_cast<unsigned_char_type>(unit)));
        }
    } else if constexpr (std::same_as<CharT, char8_t>) {
        for (std::size_t index = 0; index < source.size();) {
            const auto decoded = decode_utf8_at(source, index);
            if (!decoded.has_value()) {
                return std::unexpected(decoded.error());
            }

            output.push_back(decoded->value);
            index += decoded->size;
        }
    } else if constexpr (std::same_as<CharT, char16_t>) {
        for (std::size_t index = 0; index < source.size();) {
            const auto decoded = decode_utf16_at(source, index);
            if (!decoded.has_value()) {
                return std::unexpected(decoded.error());
            }

            output.push_back(decoded->value);
            index += decoded->size;
        }
    } else {
        for (const char32_t value : source) {
            if (!is_valid_code_point(value)) {
                return std::unexpected(make_error(error_t::encoding_error));
            }

            output.push_back(value);
        }
    }

    return output;
}

/**
 * @brief Performs string-level fullwidth/halfwidth conversion.
 *
 * Unlike toctrans(), this function may combine two halfwidth code points into
 * one fullwidth code point or decompose one fullwidth code point into two
 * halfwidth code points.
 *
 * @tparam CharT String character type.
 * @param source Source string.
 * @param id Width transformation identifier.
 * @return Converted string on success.
 */
template<supported_string_character CharT>
[[nodiscard]] inline auto strtoctrans_width(
    const std::basic_string_view<CharT> source,
    const ctrans_id id) -> result<std::basic_string<CharT>>
{
    const auto decoded = decode_string_code_points(source);
    if (!decoded.has_value()) {
        return std::unexpected(decoded.error());
    }

    std::basic_string<CharT> output;
    output.reserve(source.size());

    for (std::size_t index = 0; index < decoded->size(); ++index) {
        const char32_t value = (*decoded)[index];

        if (id == ctrans_id::fullwidth || id == ctrans_id::fullwidth_kana ||
            id == ctrans_id::fullwidth_graph || id == ctrans_id::fullwidth_print) {
            if (index + 1 < decoded->size() &&
                (is_halfwidth_voiced_sound_mark((*decoded)[index + 1]) ||
                 is_halfwidth_semivoiced_sound_mark((*decoded)[index + 1]))) {
                const char32_t composed = compose_halfwidth_kana(
                    value,
                    (*decoded)[index + 1]);
                if (composed != U'\0') {
                    const auto appended = append_transformed_code_point(
                        output,
                        composed);
                    if (!appended.has_value()) {
                        return std::unexpected(appended.error());
                    }

                    ++index;
                    continue;
                }
            }

            const auto converted = xer::toctrans(value, id);
            if (!converted.has_value()) {
                return std::unexpected(converted.error());
            }

            const auto appended = append_transformed_code_point(
                output,
                *converted);
            if (!appended.has_value()) {
                return std::unexpected(appended.error());
            }

            continue;
        }

        if (id == ctrans_id::halfwidth || id == ctrans_id::halfwidth_kana ||
            id == ctrans_id::halfwidth_graph || id == ctrans_id::halfwidth_print) {
            const halfwidth_kana_decomposition decomposed =
                fullwidth_kana_to_halfwidth(value);

            if (decomposed.mark != U'\0' || decomposed.base != value ||
                id == ctrans_id::halfwidth_kana) {
                const auto appended_base = append_transformed_code_point(
                    output,
                    decomposed.base);
                if (!appended_base.has_value()) {
                    return std::unexpected(appended_base.error());
                }

                if (decomposed.mark != U'\0') {
                    const auto appended_mark = append_transformed_code_point(
                        output,
                        decomposed.mark);
                    if (!appended_mark.has_value()) {
                        return std::unexpected(appended_mark.error());
                    }
                }

                continue;
            }

            const auto converted = xer::toctrans(value, id);
            if (!converted.has_value()) {
                return std::unexpected(converted.error());
            }

            const auto appended = append_transformed_code_point(output, *converted);
            if (!appended.has_value()) {
                return std::unexpected(appended.error());
            }

            continue;
        }

        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return output;
}
} // namespace xer::detail

namespace xer {

/**
 * @brief Transforms each character in a string according to a transformation ID.
 *
 * For char and unsigned char strings, each code unit is treated as one byte-sized
 * code point. For UTF-8, UTF-16, and UTF-32 strings, the input is decoded by
 * Unicode code point and encoded back to the same string character type.
 *
 * @tparam CharT String character type.
 * @param source Source string.
 * @param id Transformation identifier.
 * @return Transformed string on success.
 */
template<detail::supported_string_character CharT>
[[nodiscard]] inline auto strtoctrans(
    const std::basic_string_view<CharT> source,
    const ctrans_id id) -> result<std::basic_string<CharT>>
{
    std::basic_string<CharT> output;
    output.reserve(source.size());

    if (id == ctrans_id::fullwidth || id == ctrans_id::halfwidth ||
        id == ctrans_id::fullwidth_kana || id == ctrans_id::halfwidth_kana ||
        id == ctrans_id::fullwidth_graph || id == ctrans_id::halfwidth_graph ||
        id == ctrans_id::fullwidth_print || id == ctrans_id::halfwidth_print) {
        return detail::strtoctrans_width(source, id);
    }

    if constexpr (std::same_as<CharT, char> ||
                  std::same_as<CharT, unsigned char>) {
        using unsigned_char_type = std::make_unsigned_t<CharT>;

        for (const CharT unit : source) {
            const char32_t value = static_cast<char32_t>(
                static_cast<unsigned_char_type>(unit));
            const auto converted = xer::toctrans(value, id);
            if (!converted.has_value()) {
                return std::unexpected(converted.error());
            }

            const auto appended = detail::append_transformed_code_point(
                output,
                *converted);
            if (!appended.has_value()) {
                return std::unexpected(appended.error());
            }
        }
    } else if constexpr (std::same_as<CharT, char8_t>) {
        for (std::size_t index = 0; index < source.size();) {
            const auto decoded = detail::decode_utf8_at(source, index);
            if (!decoded.has_value()) {
                return std::unexpected(decoded.error());
            }

            const auto converted = xer::toctrans(decoded->value, id);
            if (!converted.has_value()) {
                return std::unexpected(converted.error());
            }

            const auto appended = detail::append_transformed_code_point(
                output,
                *converted);
            if (!appended.has_value()) {
                return std::unexpected(appended.error());
            }

            index += decoded->size;
        }
    } else if constexpr (std::same_as<CharT, char16_t>) {
        for (std::size_t index = 0; index < source.size();) {
            const auto decoded = detail::decode_utf16_at(source, index);
            if (!decoded.has_value()) {
                return std::unexpected(decoded.error());
            }

            const auto converted = xer::toctrans(decoded->value, id);
            if (!converted.has_value()) {
                return std::unexpected(converted.error());
            }

            const auto appended = detail::append_transformed_code_point(
                output,
                *converted);
            if (!appended.has_value()) {
                return std::unexpected(appended.error());
            }

            index += decoded->size;
        }
    } else {
        for (const char32_t value : source) {
            if (!detail::is_valid_code_point(value)) {
                return std::unexpected(make_error(error_t::encoding_error));
            }

            const auto converted = xer::toctrans(value, id);
            if (!converted.has_value()) {
                return std::unexpected(converted.error());
            }

            const auto appended = detail::append_transformed_code_point(
                output,
                *converted);
            if (!appended.has_value()) {
                return std::unexpected(appended.error());
            }
        }
    }

    return output;
}

/**
 * @brief Transforms each character in a pointer-sized string.
 *
 * The supplied size is used directly. A NUL code unit is treated as ordinary
 * input when it is included in the specified range.
 *
 * @tparam CharT String character type.
 * @param source Source pointer.
 * @param size Source size in code units.
 * @param id Transformation identifier.
 * @return Transformed string on success.
 */
template<typename CharT>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] inline auto strtoctrans(
    const CharT* source,
    const std::size_t size,
    const ctrans_id id) -> result<std::basic_string<std::remove_cv_t<CharT>>>
{
    if (source == nullptr && size != 0) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    using bare_char_t = std::remove_cv_t<CharT>;
    return xer::strtoctrans(
        std::basic_string_view<bare_char_t>(source, size),
        id);
}

/**
 * @brief Transforms each character in an array string.
 *
 * A trailing NUL code unit is excluded when present. Embedded NUL code units are
 * treated as ordinary input.
 *
 * @tparam CharT String character type.
 * @tparam N Source array size.
 * @param source Source array.
 * @param id Transformation identifier.
 * @return Transformed string on success.
 */
template<typename CharT, std::size_t N>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] inline auto strtoctrans(
    const CharT (&source)[N],
    const ctrans_id id) -> result<std::basic_string<std::remove_cv_t<CharT>>>
{
    return xer::strtoctrans(detail::transformation_array_string_view(source), id);
}

/**
 * @brief Converts a string to lowercase using ASCII lowercase conversion.
 *
 * @tparam CharT String character type.
 * @param source Source string.
 * @return Lowercase string on success.
 */
template<detail::supported_string_character CharT>
[[nodiscard]] inline auto strtolower(
    const std::basic_string_view<CharT> source) -> result<std::basic_string<CharT>>
{
    return xer::strtoctrans(source, ctrans_id::lower);
}

/**
 * @brief Converts a pointer-sized string to lowercase.
 *
 * @tparam CharT String character type.
 * @param source Source pointer.
 * @param size Source size in code units.
 * @return Lowercase string on success.
 */
template<typename CharT>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] inline auto strtolower(
    const CharT* source,
    const std::size_t size) -> result<std::basic_string<std::remove_cv_t<CharT>>>
{
    return xer::strtoctrans(source, size, ctrans_id::lower);
}

/**
 * @brief Converts an array string to lowercase.
 *
 * @tparam CharT String character type.
 * @tparam N Source array size.
 * @param source Source array.
 * @return Lowercase string on success.
 */
template<typename CharT, std::size_t N>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] inline auto strtolower(
    const CharT (&source)[N]) -> result<std::basic_string<std::remove_cv_t<CharT>>>
{
    return xer::strtoctrans(source, ctrans_id::lower);
}

/**
 * @brief Converts a string to uppercase using ASCII uppercase conversion.
 *
 * @tparam CharT String character type.
 * @param source Source string.
 * @return Uppercase string on success.
 */
template<detail::supported_string_character CharT>
[[nodiscard]] inline auto strtoupper(
    const std::basic_string_view<CharT> source) -> result<std::basic_string<CharT>>
{
    return xer::strtoctrans(source, ctrans_id::upper);
}

/**
 * @brief Converts a pointer-sized string to uppercase.
 *
 * @tparam CharT String character type.
 * @param source Source pointer.
 * @param size Source size in code units.
 * @return Uppercase string on success.
 */
template<typename CharT>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] inline auto strtoupper(
    const CharT* source,
    const std::size_t size) -> result<std::basic_string<std::remove_cv_t<CharT>>>
{
    return xer::strtoctrans(source, size, ctrans_id::upper);
}

/**
 * @brief Converts an array string to uppercase.
 *
 * @tparam CharT String character type.
 * @tparam N Source array size.
 * @param source Source array.
 * @return Uppercase string on success.
 */
template<typename CharT, std::size_t N>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] inline auto strtoupper(
    const CharT (&source)[N]) -> result<std::basic_string<std::remove_cv_t<CharT>>>
{
    return xer::strtoctrans(source, ctrans_id::upper);
}

} // namespace xer

#endif /* XER_BITS_STRING_CASE_H_INCLUDED_ */
