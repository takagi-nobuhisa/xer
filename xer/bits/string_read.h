/**
 * @file xer/bits/string_read.h
 * @brief Internal string reading function implementations.
 */

#pragma once

#ifndef XER_BITS_STRING_READ_H_INCLUDED_
#define XER_BITS_STRING_READ_H_INCLUDED_

#include <cstddef>
#include <cstdint>
#include <expected>
#include <string_view>
#include <type_traits>

#include <xer/bits/advanced_encoding.h>
#include <xer/bits/common.h>
#include <xer/bits/string_character.h>
#include <xer/error.h>

namespace xer::detail {

/**
 * @brief Decoded code point information.
 */
struct decoded_code_point {
    /**
     * @brief Decoded code point.
     */
    char32_t value;

    /**
     * @brief Number of code units consumed.
     */
    std::size_t size;
};

/**
 * @brief Creates an unexpected error result.
 *
 * @tparam T Value type of expected.
 * @param code Error code.
 * @return Unexpected error result.
 */
template<typename T>
[[nodiscard]] inline auto unexpected_string_error(
    const error_t code) -> result<T>
{
    return std::unexpected(make_error(code));
}

/**
 * @brief Tests whether a code point is Unicode-valid.
 *
 * @param value Code point to test.
 * @return true if valid, otherwise false.
 */
[[nodiscard]] constexpr auto is_valid_code_point(const char32_t value) noexcept -> bool
{
    return value <= static_cast<char32_t>(0x10FFFF) &&
           !xer::advanced::detail::is_surrogate(value);
}

/**
 * @brief Decodes one UTF-8 code point from a code-unit position.
 *
 * @param source Source UTF-8 string view.
 * @param index Starting index.
 * @return Decoded code point on success.
 */
[[nodiscard]] inline auto decode_utf8_at(
    const std::u8string_view source,
    const std::size_t index) -> result<decoded_code_point>
{
    if (index >= source.size()) {
        return unexpected_string_error<decoded_code_point>(error_t::out_of_range);
    }

    const std::uint8_t b1 = static_cast<std::uint8_t>(source[index]);

    if (b1 <= 0x7Fu) {
        return decoded_code_point {static_cast<char32_t>(b1), 1};
    }

    std::uint32_t packed = static_cast<std::uint32_t>(b1);
    std::size_t count = 0;

    if (b1 >= 0xC2u && b1 <= 0xDFu) {
        count = 2;
    } else if (b1 >= 0xE0u && b1 <= 0xEFu) {
        count = 3;
    } else if (b1 >= 0xF0u && b1 <= 0xF4u) {
        count = 4;
    } else {
        return unexpected_string_error<decoded_code_point>(error_t::encoding_error);
    }

    if (index + count > source.size()) {
        return unexpected_string_error<decoded_code_point>(error_t::encoding_error);
    }

    for (std::size_t i = 1; i < count; ++i) {
        packed |= static_cast<std::uint32_t>(
                      static_cast<std::uint8_t>(source[index + i]))
                  << static_cast<unsigned int>(i * 8);
    }

    const char32_t code_point = xer::advanced::packed_utf8_to_utf32(packed);
    if (code_point == xer::advanced::detail::invalid_utf32) {
        return unexpected_string_error<decoded_code_point>(error_t::encoding_error);
    }

    return decoded_code_point {code_point, count};
}

/**
 * @brief Decodes one UTF-16 code point from a code-unit position.
 *
 * @param source Source UTF-16 string view.
 * @param index Starting index.
 * @return Decoded code point on success.
 */
[[nodiscard]] inline auto decode_utf16_at(
    const std::u16string_view source,
    const std::size_t index) -> result<decoded_code_point>
{
    if (index >= source.size()) {
        return unexpected_string_error<decoded_code_point>(error_t::out_of_range);
    }

    const char16_t first = source[index];

    if (first < 0xD800u || first > 0xDFFFu) {
        return decoded_code_point {static_cast<char32_t>(first), 1};
    }

    if (first >= 0xDC00u) {
        return unexpected_string_error<decoded_code_point>(error_t::encoding_error);
    }

    if (index + 1 >= source.size()) {
        return unexpected_string_error<decoded_code_point>(error_t::encoding_error);
    }

    const char16_t second = source[index + 1];
    if (second < 0xDC00u || second > 0xDFFFu) {
        return unexpected_string_error<decoded_code_point>(error_t::encoding_error);
    }

    const std::uint32_t packed =
        static_cast<std::uint32_t>(first) |
        (static_cast<std::uint32_t>(second) << 16);
    const char32_t code_point = xer::advanced::packed_utf16_to_utf32(packed);
    if (code_point == xer::advanced::detail::invalid_utf32) {
        return unexpected_string_error<decoded_code_point>(error_t::encoding_error);
    }

    return decoded_code_point {code_point, 2};
}

/**
 * @brief Decodes one UTF-32 code point from a code-unit position.
 *
 * @param source Source UTF-32 string view.
 * @param index Starting index.
 * @return Decoded code point on success.
 */
[[nodiscard]] inline auto decode_utf32_at(
    const std::u32string_view source,
    const std::size_t index) -> result<decoded_code_point>
{
    if (index >= source.size()) {
        return unexpected_string_error<decoded_code_point>(error_t::out_of_range);
    }

    const char32_t value = source[index];
    if (!is_valid_code_point(value)) {
        return unexpected_string_error<decoded_code_point>(error_t::encoding_error);
    }

    return decoded_code_point {value, 1};
}

/**
 * @brief Tests whether a character exists in a string view.
 *
 * @tparam CharT Character type.
 * @param set Character set to test.
 * @param value Value to search for.
 * @return true if found, otherwise false.
 */
template<supported_string_character CharT>
[[nodiscard]] constexpr auto contains_char(
    const std::basic_string_view<CharT> set,
    const CharT value) noexcept -> bool
{
    for (const CharT candidate : set) {
        if (candidate == value) {
            return true;
        }
    }

    return false;
}

} // namespace xer::detail

namespace xer {

/**
 * @brief Returns the length of a NUL-terminated string in code units.
 *
 * This overload accepts a pointer and an explicit buffer size because XER does
 * not provide a separate strnlen-style function at this stage.
 *
 * @tparam CharT Character type.
 * @param source Source pointer.
 * @param size Maximum number of code units to inspect.
 * @return Code-unit length of the source string.
 */
template<typename CharT>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] constexpr auto strlen(
    const CharT* source,
    const std::size_t size) -> result<std::size_t>
{
    using bare_char_t = std::remove_cv_t<CharT>;

    if (source == nullptr && size != 0) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    for (std::size_t index = 0; index < size; ++index) {
        if (source[index] == static_cast<bare_char_t>(0)) {
            return index;
        }
    }

    return std::unexpected(make_error(error_t::not_found));
}

/**
 * @brief Returns the length of a NUL-terminated array string in code units.
 *
 * This overload is intended to make string literals work naturally.
 *
 * @tparam CharT Character type.
 * @tparam N Array size.
 * @param source Source array.
 * @return Code-unit length of the source string.
 */
template<typename CharT, std::size_t N>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] constexpr auto strlen(const CharT (&source)[N]) -> result<std::size_t>
{
    return xer::strlen(source, N);
}

/**
 * @brief Returns the length of a string view in code units.
 *
 * @tparam CharT Character type.
 * @param source Source string view.
 * @return Code-unit length of the source string.
 */
template<detail::supported_string_character CharT>
[[nodiscard]] constexpr auto strlen(
    const std::basic_string_view<CharT> source) -> result<std::size_t>
{
    return source.size();
}

/**
 * @brief Compares two strings lexicographically.
 *
 * The common prefix up to the smaller size is compared first.
 * If no difference is found there, the shorter string is considered smaller.
 *
 * @tparam CharT Character type.
 * @param lhs Left-hand string.
 * @param rhs Right-hand string.
 * @return Negative value if lhs < rhs, zero if equal, positive value if lhs > rhs.
 */
template<detail::supported_string_character CharT>
[[nodiscard]] constexpr auto strcmp(
    const std::basic_string_view<CharT> lhs,
    const std::basic_string_view<CharT> rhs) -> result<int>
{
    const std::size_t common_size = lhs.size() < rhs.size() ? lhs.size() : rhs.size();

    for (std::size_t i = 0; i < common_size; ++i) {
        if (lhs[i] < rhs[i]) {
            return -1;
        }

        if (lhs[i] > rhs[i]) {
            return 1;
        }
    }

    if (lhs.size() < rhs.size()) {
        return -1;
    }

    if (lhs.size() > rhs.size()) {
        return 1;
    }

    return 0;
}

/**
 * @brief Compares two strings lexicographically with explicit sizes.
 *
 * @tparam CharT Character type.
 * @param lhs Left-hand string pointer.
 * @param lhs_size Left-hand size in code units.
 * @param rhs Right-hand string pointer.
 * @param rhs_size Right-hand size in code units.
 * @return Negative value if lhs < rhs, zero if equal, positive value if lhs > rhs.
 */
template<typename CharT>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] constexpr auto strcmp(
    const CharT* lhs,
    const std::size_t lhs_size,
    const CharT* rhs,
    const std::size_t rhs_size) -> result<int>
{
    if ((lhs == nullptr && lhs_size != 0) || (rhs == nullptr && rhs_size != 0)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    using bare_char_t = std::remove_cv_t<CharT>;

    return xer::strcmp(
        std::basic_string_view<bare_char_t>(lhs, lhs_size),
        std::basic_string_view<bare_char_t>(rhs, rhs_size));
}

/**
 * @brief Compares two array strings lexicographically.
 *
 * This overload is intended to make string literals work naturally.
 *
 * @tparam CharT Character type.
 * @tparam N1 Left-hand array size.
 * @tparam N2 Right-hand array size.
 * @param lhs Left-hand array.
 * @param rhs Right-hand array.
 * @return Negative value if lhs < rhs, zero if equal, positive value if lhs > rhs.
 */
template<typename CharT, std::size_t N1, std::size_t N2>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] constexpr auto strcmp(
    const CharT (&lhs)[N1],
    const CharT (&rhs)[N2]) -> result<int>
{
    const auto lhs_length = xer::strlen(lhs);
    if (!lhs_length) {
        return std::unexpected(lhs_length.error());
    }

    const auto rhs_length = xer::strlen(rhs);
    if (!rhs_length) {
        return std::unexpected(rhs_length.error());
    }

    return xer::strcmp(lhs, *lhs_length, rhs, *rhs_length);
}

/**
 * @brief Compares up to the specified number of code units lexicographically.
 *
 * @tparam CharT Character type.
 * @param lhs Left-hand string.
 * @param rhs Right-hand string.
 * @param count Maximum number of code units to compare.
 * @return Negative value if lhs < rhs, zero if equal, positive value if lhs > rhs.
 */
template<detail::supported_string_character CharT>
[[nodiscard]] constexpr auto strncmp(
    const std::basic_string_view<CharT> lhs,
    const std::basic_string_view<CharT> rhs,
    const std::size_t count) -> result<int>
{
    const std::size_t lhs_size = lhs.size() < count ? lhs.size() : count;
    const std::size_t rhs_size = rhs.size() < count ? rhs.size() : count;
    const std::size_t common_size = lhs_size < rhs_size ? lhs_size : rhs_size;

    for (std::size_t i = 0; i < common_size; ++i) {
        if (lhs[i] < rhs[i]) {
            return -1;
        }

        if (lhs[i] > rhs[i]) {
            return 1;
        }
    }

    if (lhs_size == count && rhs_size == count) {
        return 0;
    }

    if (lhs_size < rhs_size) {
        return -1;
    }

    if (lhs_size > rhs_size) {
        return 1;
    }

    return 0;
}

/**
 * @brief Compares up to the specified number of code units lexicographically
 *        with explicit sizes.
 *
 * @tparam CharT Character type.
 * @param lhs Left-hand string pointer.
 * @param lhs_size Left-hand size in code units.
 * @param rhs Right-hand string pointer.
 * @param rhs_size Right-hand size in code units.
 * @param count Maximum number of code units to compare.
 * @return Negative value if lhs < rhs, zero if equal, positive value if lhs > rhs.
 */
template<typename CharT>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] constexpr auto strncmp(
    const CharT* lhs,
    const std::size_t lhs_size,
    const CharT* rhs,
    const std::size_t rhs_size,
    const std::size_t count) -> result<int>
{
    if ((lhs == nullptr && lhs_size != 0) || (rhs == nullptr && rhs_size != 0)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    using bare_char_t = std::remove_cv_t<CharT>;

    return xer::strncmp(
        std::basic_string_view<bare_char_t>(lhs, lhs_size),
        std::basic_string_view<bare_char_t>(rhs, rhs_size),
        count);
}

/**
 * @brief Compares two array strings lexicographically up to the specified
 *        number of code units.
 *
 * This overload is intended to make string literals work naturally.
 *
 * @tparam CharT Character type.
 * @tparam N1 Left-hand array size.
 * @tparam N2 Right-hand array size.
 * @param lhs Left-hand array.
 * @param rhs Right-hand array.
 * @param count Maximum number of code units to compare.
 * @return Negative value if lhs < rhs, zero if equal, positive value if lhs > rhs.
 */
template<typename CharT, std::size_t N1, std::size_t N2>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] constexpr auto strncmp(
    const CharT (&lhs)[N1],
    const CharT (&rhs)[N2],
    const std::size_t count) -> result<int>
{
    const auto lhs_length = xer::strlen(lhs);
    if (!lhs_length) {
        return std::unexpected(lhs_length.error());
    }

    const auto rhs_length = xer::strlen(rhs);
    if (!rhs_length) {
        return std::unexpected(rhs_length.error());
    }

    return xer::strncmp(lhs, *lhs_length, rhs, *rhs_length, count);
}

/**
 * @brief Searches for the first occurrence of a code unit.
 *
 * @tparam CharT Character type.
 * @param source Source string.
 * @param value Code unit to search for.
 * @return Iterator pointing to the found code unit.
 */
template<detail::supported_string_character CharT>
[[nodiscard]] constexpr auto strchr(
    const std::basic_string_view<CharT> source,
    const CharT value) -> result<typename std::basic_string_view<CharT>::const_iterator>
{
    for (auto it = source.begin(); it != source.end(); ++it) {
        if (*it == value) {
            return it;
        }
    }

    return std::unexpected(make_error(error_t::not_found));
}

/**
 * @brief Searches for the first occurrence of a code unit with explicit size.
 *
 * @tparam CharT Character type.
 * @param source Source pointer.
 * @param source_size Source size in code units.
 * @param value Code unit to search for.
 * @return Iterator pointing to the found code unit.
 */
template<typename CharT>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] constexpr auto strchr(
    const CharT* source,
    const std::size_t source_size,
    const std::remove_cv_t<CharT> value) -> result<const CharT*>
{
    if (source == nullptr && source_size != 0) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    for (std::size_t index = 0; index < source_size; ++index) {
        if (source[index] == value) {
            return source + index;
        }
    }

    return std::unexpected(make_error(error_t::not_found));
}

/**
 * @brief Searches for the first occurrence of a code unit in an array string.
 *
 * This overload is intended to make string literals work naturally.
 *
 * @tparam CharT Character type.
 * @tparam N Array size.
 * @param source Source array.
 * @param value Code unit to search for.
 * @return Iterator pointing to the found code unit.
 */
template<typename CharT, std::size_t N>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] constexpr auto strchr(
    const CharT (&source)[N],
    const std::remove_cv_t<CharT> value) -> result<const CharT*>
{
    const auto source_length = xer::strlen(source);
    if (!source_length) {
        return std::unexpected(source_length.error());
    }

    return xer::strchr(source, *source_length, value);
}

/**
 * @brief Searches for the last occurrence of a code unit.
 *
 * @tparam CharT Character type.
 * @param source Source string.
 * @param value Code unit to search for.
 * @return Iterator pointing to the found code unit.
 */
template<detail::supported_string_character CharT>
[[nodiscard]] constexpr auto strrchr(
    const std::basic_string_view<CharT> source,
    const CharT value) -> result<typename std::basic_string_view<CharT>::const_iterator>
{
    for (auto it = source.end(); it != source.begin();) {
        --it;
        if (*it == value) {
            return it;
        }
    }

    return std::unexpected(make_error(error_t::not_found));
}

/**
 * @brief Searches for the last occurrence of a code unit with explicit size.
 *
 * @tparam CharT Character type.
 * @param source Source pointer.
 * @param source_size Source size in code units.
 * @param value Code unit to search for.
 * @return Iterator pointing to the found code unit.
 */
template<typename CharT>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] constexpr auto strrchr(
    const CharT* source,
    const std::size_t source_size,
    const std::remove_cv_t<CharT> value) -> result<const CharT*>
{
    if (source == nullptr && source_size != 0) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    for (std::size_t index = source_size; index > 0; --index) {
        if (source[index - 1] == value) {
            return source + (index - 1);
        }
    }

    return std::unexpected(make_error(error_t::not_found));
}

/**
 * @brief Searches for the last occurrence of a code unit in an array string.
 *
 * This overload is intended to make string literals work naturally.
 *
 * @tparam CharT Character type.
 * @tparam N Array size.
 * @param source Source array.
 * @param value Code unit to search for.
 * @return Iterator pointing to the found code unit.
 */
template<typename CharT, std::size_t N>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] constexpr auto strrchr(
    const CharT (&source)[N],
    const std::remove_cv_t<CharT> value) -> result<const CharT*>
{
    const auto source_length = xer::strlen(source);
    if (!source_length) {
        return std::unexpected(source_length.error());
    }

    return xer::strrchr(source, *source_length, value);
}

/**
 * @brief Searches for the first occurrence of a code point in a UTF-8 string.
 *
 * @param source Source UTF-8 string.
 * @param value Code point to search for.
 * @return Iterator pointing to the first code unit of the found code point.
 */
[[nodiscard]] inline auto strchr(
    const std::u8string_view source,
    const char32_t value) -> result<std::u8string_view::const_iterator>
{
    if (!detail::is_valid_code_point(value)) {
        return std::unexpected(make_error(error_t::encoding_error));
    }

    for (std::size_t index = 0; index < source.size();) {
        auto decoded = detail::decode_utf8_at(source, index);
        if (!decoded.has_value()) {
            return std::unexpected(decoded.error());
        }

        if (decoded->value == value) {
            return source.begin() + static_cast<std::ptrdiff_t>(index);
        }

        index += decoded->size;
    }

    return std::unexpected(make_error(error_t::not_found));
}

/**
 * @brief Searches for the first occurrence of a code point in a UTF-16 string.
 *
 * @param source Source UTF-16 string.
 * @param value Code point to search for.
 * @return Iterator pointing to the first code unit of the found code point.
 */
[[nodiscard]] inline auto strchr(
    const std::u16string_view source,
    const char32_t value) -> result<std::u16string_view::const_iterator>
{
    if (!detail::is_valid_code_point(value)) {
        return std::unexpected(make_error(error_t::encoding_error));
    }

    for (std::size_t index = 0; index < source.size();) {
        auto decoded = detail::decode_utf16_at(source, index);
        if (!decoded.has_value()) {
            return std::unexpected(decoded.error());
        }

        if (decoded->value == value) {
            return source.begin() + static_cast<std::ptrdiff_t>(index);
        }

        index += decoded->size;
    }

    return std::unexpected(make_error(error_t::not_found));
}

/**
 * @brief Searches for the first occurrence of a code point in a UTF-32 string.
 *
 * @param source Source UTF-32 string.
 * @param value Code point to search for.
 * @return Iterator pointing to the found code point.
 */
[[nodiscard]] inline auto strchr(
    const std::u32string_view source,
    const char32_t value) -> result<std::u32string_view::const_iterator>
{
    if (!detail::is_valid_code_point(value)) {
        return std::unexpected(make_error(error_t::encoding_error));
    }

    for (auto it = source.begin(); it != source.end(); ++it) {
        if (!detail::is_valid_code_point(*it)) {
            return std::unexpected(make_error(error_t::encoding_error));
        }

        if (*it == value) {
            return it;
        }
    }

    return std::unexpected(make_error(error_t::not_found));
}

/**
 * @brief Searches for the last occurrence of a code point in a UTF-8 string.
 *
 * @param source Source UTF-8 string.
 * @param value Code point to search for.
 * @return Iterator pointing to the first code unit of the found code point.
 */
[[nodiscard]] inline auto strrchr(
    const std::u8string_view source,
    const char32_t value) -> result<std::u8string_view::const_iterator>
{
    if (!detail::is_valid_code_point(value)) {
        return std::unexpected(make_error(error_t::encoding_error));
    }

    bool found = false;
    std::size_t found_index = 0;

    for (std::size_t index = 0; index < source.size();) {
        auto decoded = detail::decode_utf8_at(source, index);
        if (!decoded.has_value()) {
            return std::unexpected(decoded.error());
        }

        if (decoded->value == value) {
            found = true;
            found_index = index;
        }

        index += decoded->size;
    }

    if (!found) {
        return std::unexpected(make_error(error_t::not_found));
    }

    return source.begin() + static_cast<std::ptrdiff_t>(found_index);
}

/**
 * @brief Searches for the last occurrence of a code point in a UTF-16 string.
 *
 * @param source Source UTF-16 string.
 * @param value Code point to search for.
 * @return Iterator pointing to the first code unit of the found code point.
 */
[[nodiscard]] inline auto strrchr(
    const std::u16string_view source,
    const char32_t value) -> result<std::u16string_view::const_iterator>
{
    if (!detail::is_valid_code_point(value)) {
        return std::unexpected(make_error(error_t::encoding_error));
    }

    bool found = false;
    std::size_t found_index = 0;

    for (std::size_t index = 0; index < source.size();) {
        auto decoded = detail::decode_utf16_at(source, index);
        if (!decoded.has_value()) {
            return std::unexpected(decoded.error());
        }

        if (decoded->value == value) {
            found = true;
            found_index = index;
        }

        index += decoded->size;
    }

    if (!found) {
        return std::unexpected(make_error(error_t::not_found));
    }

    return source.begin() + static_cast<std::ptrdiff_t>(found_index);
}

/**
 * @brief Searches for the last occurrence of a code point in a UTF-32 string.
 *
 * @param source Source UTF-32 string.
 * @param value Code point to search for.
 * @return Iterator pointing to the found code point.
 */
[[nodiscard]] inline auto strrchr(
    const std::u32string_view source,
    const char32_t value) -> result<std::u32string_view::const_iterator>
{
    if (!detail::is_valid_code_point(value)) {
        return std::unexpected(make_error(error_t::encoding_error));
    }

    for (auto it = source.end(); it != source.begin();) {
        --it;

        if (!detail::is_valid_code_point(*it)) {
            return std::unexpected(make_error(error_t::encoding_error));
        }

        if (*it == value) {
            return it;
        }
    }

    return std::unexpected(make_error(error_t::not_found));
}

/**
 * @brief Searches for the first occurrence of a substring.
 *
 * @tparam CharT Character type.
 * @param source Source string.
 * @param pattern Pattern string.
 * @return Iterator pointing to the beginning of the found substring.
 */
template<detail::supported_string_character CharT>
[[nodiscard]] constexpr auto strstr(
    const std::basic_string_view<CharT> source,
    const std::basic_string_view<CharT> pattern) -> result<typename std::basic_string_view<CharT>::const_iterator>
{
    if (pattern.empty()) {
        return source.begin();
    }

    if (pattern.size() > source.size()) {
        return std::unexpected(make_error(error_t::not_found));
    }

    const std::size_t last = source.size() - pattern.size();
    for (std::size_t i = 0; i <= last; ++i) {
        bool matched = true;

        for (std::size_t j = 0; j < pattern.size(); ++j) {
            if (source[i + j] != pattern[j]) {
                matched = false;
                break;
            }
        }

        if (matched) {
            return source.begin() + static_cast<std::ptrdiff_t>(i);
        }
    }

    return std::unexpected(make_error(error_t::not_found));
}

/**
 * @brief Searches for the first occurrence of a substring with explicit sizes.
 *
 * @tparam CharT Character type.
 * @param source Source pointer.
 * @param source_size Source size in code units.
 * @param pattern Pattern pointer.
 * @param pattern_size Pattern size in code units.
 * @return Iterator pointing to the beginning of the found substring.
 */
template<typename CharT>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] constexpr auto strstr(
    const CharT* source,
    const std::size_t source_size,
    const CharT* pattern,
    const std::size_t pattern_size) -> result<const CharT*>
{
    if ((source == nullptr && source_size != 0) ||
        (pattern == nullptr && pattern_size != 0)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    using bare_char_t = std::remove_cv_t<CharT>;

    if (pattern_size == 0) {
        return source;
    }

    if (pattern_size > source_size) {
        return std::unexpected(make_error(error_t::not_found));
    }

    const auto source_view = std::basic_string_view<bare_char_t>(source, source_size);
    const auto result = xer::strstr(
        source_view,
        std::basic_string_view<bare_char_t>(pattern, pattern_size));
    if (!result) {
        return std::unexpected(result.error());
    }

    return source + (*result - source_view.begin());
}

/**
 * @brief Searches for the first occurrence of a substring in array strings.
 *
 * This overload is intended to make string literals work naturally.
 *
 * @tparam CharT Character type.
 * @tparam N1 Source array size.
 * @tparam N2 Pattern array size.
 * @param source Source array.
 * @param pattern Pattern array.
 * @return Iterator pointing to the beginning of the found substring.
 */
template<typename CharT, std::size_t N1, std::size_t N2>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] constexpr auto strstr(
    const CharT (&source)[N1],
    const CharT (&pattern)[N2]) -> result<const CharT*>
{
    const auto source_length = xer::strlen(source);
    if (!source_length) {
        return std::unexpected(source_length.error());
    }

    const auto pattern_length = xer::strlen(pattern);
    if (!pattern_length) {
        return std::unexpected(pattern_length.error());
    }

    return xer::strstr(source, *source_length, pattern, *pattern_length);
}

/**
 * @brief Searches for the last occurrence of a substring.
 *
 * @tparam CharT Character type.
 * @param source Source string.
 * @param pattern Pattern string.
 * @return Iterator pointing to the beginning of the found substring.
 */
template<detail::supported_string_character CharT>
[[nodiscard]] constexpr auto strrstr(
    const std::basic_string_view<CharT> source,
    const std::basic_string_view<CharT> pattern) -> result<typename std::basic_string_view<CharT>::const_iterator>
{
    if (pattern.empty()) {
        return source.end();
    }

    const std::size_t pos = source.rfind(pattern);
    if (pos == std::basic_string_view<CharT>::npos) {
        return std::unexpected(make_error(error_t::not_found));
    }

    return source.begin() + static_cast<std::ptrdiff_t>(pos);
}

/**
 * @brief Searches for the last occurrence of a substring with explicit sizes.
 *
 * @tparam CharT Character type.
 * @param source Source pointer.
 * @param source_size Source size in code units.
 * @param pattern Pattern pointer.
 * @param pattern_size Pattern size in code units.
 * @return Iterator pointing to the beginning of the found substring.
 */
template<typename CharT>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] constexpr auto strrstr(
    const CharT* source,
    const std::size_t source_size,
    const CharT* pattern,
    const std::size_t pattern_size) -> result<const CharT*>
{
    if ((source == nullptr && source_size != 0) ||
        (pattern == nullptr && pattern_size != 0)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    using bare_char_t = std::remove_cv_t<CharT>;

    if (pattern_size == 0) {
        return source + source_size;
    }

    if (pattern_size > source_size) {
        return std::unexpected(make_error(error_t::not_found));
    }

    const auto source_view = std::basic_string_view<bare_char_t>(source, source_size);
    const auto result = xer::strrstr(
        source_view,
        std::basic_string_view<bare_char_t>(pattern, pattern_size));
    if (!result) {
        return std::unexpected(result.error());
    }

    return source + (*result - source_view.begin());
}

/**
 * @brief Searches for the last occurrence of a substring in array strings.
 *
 * This overload is intended to make string literals work naturally.
 *
 * @tparam CharT Character type.
 * @tparam N1 Source array size.
 * @tparam N2 Pattern array size.
 * @param source Source array.
 * @param pattern Pattern array.
 * @return Iterator pointing to the beginning of the found substring.
 */
template<typename CharT, std::size_t N1, std::size_t N2>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] constexpr auto strrstr(
    const CharT (&source)[N1],
    const CharT (&pattern)[N2]) -> result<const CharT*>
{
    const auto source_length = xer::strlen(source);
    if (!source_length) {
        return std::unexpected(source_length.error());
    }

    const auto pattern_length = xer::strlen(pattern);
    if (!pattern_length) {
        return std::unexpected(pattern_length.error());
    }

    return xer::strrstr(source, *source_length, pattern, *pattern_length);
}

/**
 * @brief Returns the first position of a substring.
 *
 * @tparam CharT Character type.
 * @param source Source string.
 * @param pattern Pattern string.
 * @return Zero-based code-unit position of the found substring.
 */
template<detail::supported_string_character CharT>
[[nodiscard]] constexpr auto strpos(
    const std::basic_string_view<CharT> source,
    const std::basic_string_view<CharT> pattern) -> result<std::size_t>
{
    const std::size_t pos = source.find(pattern);
    if (pos == std::basic_string_view<CharT>::npos) {
        return std::unexpected(make_error(error_t::not_found));
    }

    return pos;
}

/**
 * @brief Returns the first position of a substring with explicit sizes.
 *
 * @tparam CharT Character type.
 * @param source Source pointer.
 * @param source_size Source size in code units.
 * @param pattern Pattern pointer.
 * @param pattern_size Pattern size in code units.
 * @return Zero-based code-unit position of the found substring.
 */
template<typename CharT>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] constexpr auto strpos(
    const CharT* source,
    const std::size_t source_size,
    const CharT* pattern,
    const std::size_t pattern_size) -> result<std::size_t>
{
    if ((source == nullptr && source_size != 0) ||
        (pattern == nullptr && pattern_size != 0)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    using bare_char_t = std::remove_cv_t<CharT>;

    return xer::strpos(
        std::basic_string_view<bare_char_t>(source, source_size),
        std::basic_string_view<bare_char_t>(pattern, pattern_size));
}

/**
 * @brief Returns the first position of a substring in array strings.
 *
 * This overload is intended to make string literals work naturally.
 *
 * @tparam CharT Character type.
 * @tparam N1 Source array size.
 * @tparam N2 Pattern array size.
 * @param source Source array.
 * @param pattern Pattern array.
 * @return Zero-based code-unit position of the found substring.
 */
template<typename CharT, std::size_t N1, std::size_t N2>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] constexpr auto strpos(
    const CharT (&source)[N1],
    const CharT (&pattern)[N2]) -> result<std::size_t>
{
    const auto source_length = xer::strlen(source);
    if (!source_length) {
        return std::unexpected(source_length.error());
    }

    const auto pattern_length = xer::strlen(pattern);
    if (!pattern_length) {
        return std::unexpected(pattern_length.error());
    }

    return xer::strpos(source, *source_length, pattern, *pattern_length);
}

/**
 * @brief Returns the last position of a substring.
 *
 * @tparam CharT Character type.
 * @param source Source string.
 * @param pattern Pattern string.
 * @return Zero-based code-unit position of the found substring.
 */
template<detail::supported_string_character CharT>
[[nodiscard]] constexpr auto strrpos(
    const std::basic_string_view<CharT> source,
    const std::basic_string_view<CharT> pattern) -> result<std::size_t>
{
    const std::size_t pos = source.rfind(pattern);
    if (pos == std::basic_string_view<CharT>::npos) {
        return std::unexpected(make_error(error_t::not_found));
    }

    return pos;
}

/**
 * @brief Returns the last position of a substring with explicit sizes.
 *
 * @tparam CharT Character type.
 * @param source Source pointer.
 * @param source_size Source size in code units.
 * @param pattern Pattern pointer.
 * @param pattern_size Pattern size in code units.
 * @return Zero-based code-unit position of the found substring.
 */
template<typename CharT>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] constexpr auto strrpos(
    const CharT* source,
    const std::size_t source_size,
    const CharT* pattern,
    const std::size_t pattern_size) -> result<std::size_t>
{
    if ((source == nullptr && source_size != 0) ||
        (pattern == nullptr && pattern_size != 0)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    using bare_char_t = std::remove_cv_t<CharT>;

    return xer::strrpos(
        std::basic_string_view<bare_char_t>(source, source_size),
        std::basic_string_view<bare_char_t>(pattern, pattern_size));
}

/**
 * @brief Returns the last position of a substring in array strings.
 *
 * This overload is intended to make string literals work naturally.
 *
 * @tparam CharT Character type.
 * @tparam N1 Source array size.
 * @tparam N2 Pattern array size.
 * @param source Source array.
 * @param pattern Pattern array.
 * @return Zero-based code-unit position of the found substring.
 */
template<typename CharT, std::size_t N1, std::size_t N2>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] constexpr auto strrpos(
    const CharT (&source)[N1],
    const CharT (&pattern)[N2]) -> result<std::size_t>
{
    const auto source_length = xer::strlen(source);
    if (!source_length) {
        return std::unexpected(source_length.error());
    }

    const auto pattern_length = xer::strlen(pattern);
    if (!pattern_length) {
        return std::unexpected(pattern_length.error());
    }

    return xer::strrpos(source, *source_length, pattern, *pattern_length);
}

/**
 * @brief Searches for the first code unit that is contained in the accept set.
 *
 * @tparam CharT Character type.
 * @param source Source string.
 * @param accept Accept set.
 * @return Iterator pointing to the found code unit.
 */
template<detail::supported_string_character CharT>
[[nodiscard]] constexpr auto strpbrk(
    const std::basic_string_view<CharT> source,
    const std::basic_string_view<CharT> accept) -> result<typename std::basic_string_view<CharT>::const_iterator>
{
    for (auto it = source.begin(); it != source.end(); ++it) {
        if (detail::contains_char(accept, *it)) {
            return it;
        }
    }

    return std::unexpected(make_error(error_t::not_found));
}

/**
 * @brief Searches for the first code unit that is contained in the accept set
 *        with explicit sizes.
 *
 * @tparam CharT Character type.
 * @param source Source pointer.
 * @param source_size Source size in code units.
 * @param accept Accept-set pointer.
 * @param accept_size Accept-set size in code units.
 * @return Pointer to the found code unit.
 */
template<typename CharT>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] constexpr auto strpbrk(
    const CharT* source,
    const std::size_t source_size,
    const CharT* accept,
    const std::size_t accept_size) -> result<const CharT*>
{
    if ((source == nullptr && source_size != 0) ||
        (accept == nullptr && accept_size != 0)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    using bare_char_t = std::remove_cv_t<CharT>;

    const auto source_view = std::basic_string_view<bare_char_t>(source, source_size);
    const auto accept_view = std::basic_string_view<bare_char_t>(accept, accept_size);
    const auto result = xer::strpbrk(source_view, accept_view);
    if (!result) {
        return std::unexpected(result.error());
    }

    return source + (*result - source_view.begin());
}

/**
 * @brief Searches for the first code unit that is contained in the accept set
 *        in array strings.
 *
 * This overload is intended to make string literals work naturally.
 *
 * @tparam CharT Character type.
 * @tparam N1 Source array size.
 * @tparam N2 Accept-set array size.
 * @param source Source array.
 * @param accept Accept-set array.
 * @return Pointer to the found code unit.
 */
template<typename CharT, std::size_t N1, std::size_t N2>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] constexpr auto strpbrk(
    const CharT (&source)[N1],
    const CharT (&accept)[N2]) -> result<const CharT*>
{
    const auto source_length = xer::strlen(source);
    if (!source_length) {
        return std::unexpected(source_length.error());
    }

    const auto accept_length = xer::strlen(accept);
    if (!accept_length) {
        return std::unexpected(accept_length.error());
    }

    return xer::strpbrk(source, *source_length, accept, *accept_length);
}

/**
 * @brief Returns the length of the maximum initial segment consisting only of
 *        accepted code units.
 *
 * @tparam CharT Character type.
 * @param source Source string.
 * @param accept Accept set.
 * @return Length of the initial accepted segment.
 */
template<detail::supported_string_character CharT>
[[nodiscard]] constexpr auto strspn(
    const std::basic_string_view<CharT> source,
    const std::basic_string_view<CharT> accept) -> result<std::size_t>
{
    std::size_t count = 0;

    for (const CharT value : source) {
        if (!detail::contains_char(accept, value)) {
            break;
        }

        ++count;
    }

    return count;
}

/**
 * @brief Returns the length of the maximum initial segment consisting only of
 *        accepted code units with explicit sizes.
 *
 * @tparam CharT Character type.
 * @param source Source pointer.
 * @param source_size Source size in code units.
 * @param accept Accept-set pointer.
 * @param accept_size Accept-set size in code units.
 * @return Length of the initial accepted segment.
 */
template<typename CharT>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] constexpr auto strspn(
    const CharT* source,
    const std::size_t source_size,
    const CharT* accept,
    const std::size_t accept_size) -> result<std::size_t>
{
    if ((source == nullptr && source_size != 0) ||
        (accept == nullptr && accept_size != 0)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    using bare_char_t = std::remove_cv_t<CharT>;

    return xer::strspn(
        std::basic_string_view<bare_char_t>(source, source_size),
        std::basic_string_view<bare_char_t>(accept, accept_size));
}

/**
 * @brief Returns the length of the maximum initial segment consisting only of
 *        accepted code units in array strings.
 *
 * This overload is intended to make string literals work naturally.
 *
 * @tparam CharT Character type.
 * @tparam N1 Source array size.
 * @tparam N2 Accept-set array size.
 * @param source Source array.
 * @param accept Accept-set array.
 * @return Length of the initial accepted segment.
 */
template<typename CharT, std::size_t N1, std::size_t N2>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] constexpr auto strspn(
    const CharT (&source)[N1],
    const CharT (&accept)[N2]) -> result<std::size_t>
{
    const auto source_length = xer::strlen(source);
    if (!source_length) {
        return std::unexpected(source_length.error());
    }

    const auto accept_length = xer::strlen(accept);
    if (!accept_length) {
        return std::unexpected(accept_length.error());
    }

    return xer::strspn(source, *source_length, accept, *accept_length);
}

/**
 * @brief Returns the length of the maximum initial segment consisting only of
 *        rejected code units.
 *
 * @tparam CharT Character type.
 * @param source Source string.
 * @param reject Reject set.
 * @return Length of the initial rejected segment.
 */
template<detail::supported_string_character CharT>
[[nodiscard]] constexpr auto strcspn(
    const std::basic_string_view<CharT> source,
    const std::basic_string_view<CharT> reject) -> result<std::size_t>
{
    std::size_t count = 0;

    for (const CharT value : source) {
        if (detail::contains_char(reject, value)) {
            break;
        }

        ++count;
    }

    return count;
}

/**
 * @brief Returns the length of the maximum initial segment consisting only of
 *        rejected code units with explicit sizes.
 *
 * @tparam CharT Character type.
 * @param source Source pointer.
 * @param source_size Source size in code units.
 * @param reject Reject-set pointer.
 * @param reject_size Reject-set size in code units.
 * @return Length of the initial rejected segment.
 */
template<typename CharT>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] constexpr auto strcspn(
    const CharT* source,
    const std::size_t source_size,
    const CharT* reject,
    const std::size_t reject_size) -> result<std::size_t>
{
    if ((source == nullptr && source_size != 0) ||
        (reject == nullptr && reject_size != 0)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    using bare_char_t = std::remove_cv_t<CharT>;

    return xer::strcspn(
        std::basic_string_view<bare_char_t>(source, source_size),
        std::basic_string_view<bare_char_t>(reject, reject_size));
}

/**
 * @brief Returns the length of the maximum initial segment consisting only of
 *        rejected code units in array strings.
 *
 * This overload is intended to make string literals work naturally.
 *
 * @tparam CharT Character type.
 * @tparam N1 Source array size.
 * @tparam N2 Reject-set array size.
 * @param source Source array.
 * @param reject Reject-set array.
 * @return Length of the initial rejected segment.
 */
template<typename CharT, std::size_t N1, std::size_t N2>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] constexpr auto strcspn(
    const CharT (&source)[N1],
    const CharT (&reject)[N2]) -> result<std::size_t>
{
    const auto source_length = xer::strlen(source);
    if (!source_length) {
        return std::unexpected(source_length.error());
    }

    const auto reject_length = xer::strlen(reject);
    if (!reject_length) {
        return std::unexpected(reject_length.error());
    }

    return xer::strcspn(source, *source_length, reject, *reject_length);
}

} // namespace xer

#endif /* XER_BITS_STRING_READ_H_INCLUDED_ */
