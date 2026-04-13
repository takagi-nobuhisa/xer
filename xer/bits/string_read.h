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

} // namespace xer

#endif /* XER_BITS_STRING_READ_H_INCLUDED_ */
