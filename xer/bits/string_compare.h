/**
 * @file xer/bits/string_compare.h
 * @brief Internal string comparison and comparison-based search functions.
 */

#pragma once

#ifndef XER_BITS_STRING_COMPARE_H_INCLUDED_
#define XER_BITS_STRING_COMPARE_H_INCLUDED_

#include <cstddef>
#include <cstdint>
#include <expected>
#include <string_view>
#include <type_traits>
#include <vector>

#include <xer/bits/common.h>
#include <xer/bits/string_read.h>
#include <xer/bits/toctrans.h>
#include <xer/error.h>

namespace xer::detail {

/**
 * @brief Stores normalized code points and their source code-unit positions.
 */
struct normalized_string_view_data {
    /**
     * @brief Normalized code points in logical comparison order.
     */
    std::vector<char32_t> values;

    /**
     * @brief Starting code-unit position for each normalized code point.
     */
    std::vector<std::size_t> positions;
};

/**
 * @brief Converts a code point with ASCII lowercase folding.
 *
 * This helper is used by the case-prefixed comparison functions.
 * It directly follows the project rule that these functions normalize each
 * code point with xer::tolower before comparing.
 *
 * @param value Code point to normalize.
 * @return Normalized code point on success.
 */
[[nodiscard]] inline auto normalize_case_code_point(const char32_t value)
    -> result<char32_t>
{
    return xer::tolower(value);
}

/**
 * @brief Converts a code point with the specified dynamic transformation.
 *
 * This helper is used by the i-prefixed comparison functions.
 *
 * @param value Code point to normalize.
 * @param trans Transformation identifier.
 * @return Normalized code point on success.
 */
[[nodiscard]] inline auto normalize_ctrans_code_point(
    const char32_t value,
    const ctrans_id trans) -> result<char32_t>
{
    return xer::toctrans(value, trans);
}

/**
 * @brief Decodes and normalizes a string for comparison-based processing.
 *
 * The output keeps both the normalized code-point sequence and the original
 * starting code-unit position of each element so that substring search can
 * later return iterators or positions in the original source string.
 *
 * For char and unsigned char strings, each code unit is treated as a single
 * scalar value in the range 0..255. For UTF-8/16/32 strings, decoding is
 * performed code point by code point using the existing helpers from
 * xer/bits/string_read.h.
 *
 * @tparam CharT Source character type.
 * @tparam Normalizer Callable that converts one code point to another.
 * @param source Source string.
 * @param normalizer Code-point normalizer.
 * @return Normalized representation on success.
 */
template<supported_string_character CharT, typename Normalizer>
[[nodiscard]] inline auto decode_normalized_string(
    const std::basic_string_view<CharT> source,
    Normalizer&& normalizer) -> result<normalized_string_view_data>
{
    normalized_string_view_data decoded;
    decoded.values.reserve(source.size());
    decoded.positions.reserve(source.size());

    if constexpr (std::same_as<CharT, char> || std::same_as<CharT, unsigned char>) {
        using unsigned_char_type = std::make_unsigned_t<CharT>;
        for (std::size_t i = 0; i < source.size(); ++i) {
            const char32_t value = static_cast<char32_t>(
                static_cast<unsigned_char_type>(source[i]));
            auto normalized = normalizer(value);
            if (!normalized.has_value()) {
                return std::unexpected(normalized.error());
            }
            decoded.values.push_back(*normalized);
            decoded.positions.push_back(i);
        }
        return decoded;
    } else if constexpr (std::same_as<CharT, char8_t>) {
        for (std::size_t index = 0; index < source.size();) {
            auto cp = decode_utf8_at(source, index);
            if (!cp.has_value()) {
                return std::unexpected(cp.error());
            }
            auto normalized = normalizer(cp->value);
            if (!normalized.has_value()) {
                return std::unexpected(normalized.error());
            }
            decoded.values.push_back(*normalized);
            decoded.positions.push_back(index);
            index += cp->size;
        }
        return decoded;
    } else if constexpr (std::same_as<CharT, char16_t>) {
        for (std::size_t index = 0; index < source.size();) {
            auto cp = decode_utf16_at(source, index);
            if (!cp.has_value()) {
                return std::unexpected(cp.error());
            }
            auto normalized = normalizer(cp->value);
            if (!normalized.has_value()) {
                return std::unexpected(normalized.error());
            }
            decoded.values.push_back(*normalized);
            decoded.positions.push_back(index);
            index += cp->size;
        }
        return decoded;
    } else {
        static_assert(std::same_as<CharT, char32_t>);
        for (std::size_t index = 0; index < source.size(); ++index) {
            const char32_t value = source[index];
            if (!is_valid_code_point(value)) {
                return std::unexpected(make_error(error_t::encoding_error));
            }
            auto normalized = normalizer(value);
            if (!normalized.has_value()) {
                return std::unexpected(normalized.error());
            }
            decoded.values.push_back(*normalized);
            decoded.positions.push_back(index);
        }
        return decoded;
    }
}

/**
 * @brief Lexicographically compares normalized sequences.
 *
 * @param lhs Left normalized sequence.
 * @param rhs Right normalized sequence.
 * @param count Maximum number of code points to compare.
 * @return Negative value, zero, or positive value.
 */
[[nodiscard]] inline auto compare_normalized_sequences(
    const normalized_string_view_data& lhs,
    const normalized_string_view_data& rhs,
    const std::size_t count) noexcept -> int
{
    const std::size_t lhs_size = lhs.values.size() < count ? lhs.values.size() : count;
    const std::size_t rhs_size = rhs.values.size() < count ? rhs.values.size() : count;
    const std::size_t common_size = lhs_size < rhs_size ? lhs_size : rhs_size;

    for (std::size_t i = 0; i < common_size; ++i) {
        if (lhs.values[i] < rhs.values[i]) {
            return -1;
        }
        if (lhs.values[i] > rhs.values[i]) {
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
 * @brief Finds the first matching normalized subsequence.
 *
 * @param source Normalized source sequence.
 * @param pattern Normalized pattern sequence.
 * @return Starting code-unit position on success.
 */
[[nodiscard]] inline auto find_first_normalized_subsequence(
    const normalized_string_view_data& source,
    const normalized_string_view_data& pattern) -> result<std::size_t>
{
    if (pattern.values.empty()) {
        return 0;
    }

    if (pattern.values.size() > source.values.size()) {
        return std::unexpected(make_error(error_t::not_found));
    }

    const std::size_t last = source.values.size() - pattern.values.size();
    for (std::size_t i = 0; i <= last; ++i) {
        bool matched = true;
        for (std::size_t j = 0; j < pattern.values.size(); ++j) {
            if (source.values[i + j] != pattern.values[j]) {
                matched = false;
                break;
            }
        }
        if (matched) {
            return source.positions[i];
        }
    }

    return std::unexpected(make_error(error_t::not_found));
}

/**
 * @brief Finds the last matching normalized subsequence.
 *
 * @param source Normalized source sequence.
 * @param pattern Normalized pattern sequence.
 * @param source_code_unit_size Original source length in code units.
 * @return Starting code-unit position on success.
 */
[[nodiscard]] inline auto find_last_normalized_subsequence(
    const normalized_string_view_data& source,
    const normalized_string_view_data& pattern,
    const std::size_t source_code_unit_size) -> result<std::size_t>
{
    if (pattern.values.empty()) {
        return source_code_unit_size;
    }

    if (pattern.values.size() > source.values.size()) {
        return std::unexpected(make_error(error_t::not_found));
    }

    for (std::size_t i = source.values.size() - pattern.values.size() + 1; i > 0; --i) {
        const std::size_t start = i - 1;
        bool matched = true;
        for (std::size_t j = 0; j < pattern.values.size(); ++j) {
            if (source.values[start + j] != pattern.values[j]) {
                matched = false;
                break;
            }
        }
        if (matched) {
            return source.positions[start];
        }
    }

    return std::unexpected(make_error(error_t::not_found));
}

/**
 * @brief Normalizes one code unit for code-unit based single-character search.
 *
 * This helper is intentionally separate from UTF decoding. It is used for the
 * overloads whose signatures mirror raw code-unit strchr/strrchr semantics.
 *
 * @tparam CharT Code-unit type.
 * @tparam Normalizer Callable that converts one scalar value to another.
 * @param value Code unit to normalize.
 * @param normalizer Scalar normalizer.
 * @return Normalized scalar value on success.
 */
template<supported_string_character CharT, typename Normalizer>
[[nodiscard]] inline auto normalize_code_unit(
    const CharT value,
    Normalizer&& normalizer) -> result<char32_t>
{
    using unsigned_char_type = std::conditional_t<
        std::same_as<CharT, char>,
        unsigned char,
        std::make_unsigned_t<CharT>>;
    const char32_t code_point = static_cast<char32_t>(
        static_cast<unsigned_char_type>(value));
    return normalizer(code_point);
}

} // namespace xer::detail

namespace xer {

/**
 * @brief Compares two strings after ASCII lowercase normalization.
 *
 * Strings are decoded and compared code point by code point. Each code point is
 * normalized with xer::tolower before comparison.
 *
 * @tparam CharT Character type.
 * @param lhs Left-hand string.
 * @param rhs Right-hand string.
 * @return Negative value if lhs < rhs, zero if equal, positive value if lhs > rhs.
 */
template<detail::supported_string_character CharT>
[[nodiscard]] inline auto strcasecmp(
    const std::basic_string_view<CharT> lhs,
    const std::basic_string_view<CharT> rhs) -> result<int>
{
    auto lhs_decoded =
        detail::decode_normalized_string(lhs, detail::normalize_case_code_point);
    if (!lhs_decoded.has_value()) {
        return std::unexpected(lhs_decoded.error());
    }

    auto rhs_decoded =
        detail::decode_normalized_string(rhs, detail::normalize_case_code_point);
    if (!rhs_decoded.has_value()) {
        return std::unexpected(rhs_decoded.error());
    }

    return detail::compare_normalized_sequences(*lhs_decoded, *rhs_decoded, SIZE_MAX);
}

/**
 * @brief Compares two strings after ASCII lowercase normalization.
 *
 * This overload accepts explicit buffer sizes so that pointer-based strings can
 * be compared without relying on a separate strnlen-style helper.
 *
 * @tparam CharT Character type.
 * @param lhs Left-hand string pointer.
 * @param lhs_size Maximum number of code units to inspect in @p lhs.
 * @param rhs Right-hand string pointer.
 * @param rhs_size Maximum number of code units to inspect in @p rhs.
 * @return Negative value if lhs < rhs, zero if equal, positive value if lhs > rhs.
 */
template<typename CharT>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] inline auto strcasecmp(
    const CharT* lhs,
    const std::size_t lhs_size,
    const CharT* rhs,
    const std::size_t rhs_size) -> result<int>
{
    auto lhs_length = xer::strlen(lhs, lhs_size);
    if (!lhs_length) {
        return std::unexpected(lhs_length.error());
    }

    auto rhs_length = xer::strlen(rhs, rhs_size);
    if (!rhs_length) {
        return std::unexpected(rhs_length.error());
    }

    using bare_char_t = std::remove_cv_t<CharT>;
    return xer::strcasecmp(
        std::basic_string_view<bare_char_t>(lhs, *lhs_length),
        std::basic_string_view<bare_char_t>(rhs, *rhs_length));
}

/**
 * @brief Compares two array strings after ASCII lowercase normalization.
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
[[nodiscard]] inline auto strcasecmp(
    const CharT (&lhs)[N1],
    const CharT (&rhs)[N2]) -> result<int>
{
    return xer::strcasecmp(lhs, N1, rhs, N2);
}

/**
 * @brief Compares up to the specified number of code points after ASCII lowercase normalization.
 *
 * @tparam CharT Character type.
 * @param lhs Left-hand string.
 * @param rhs Right-hand string.
 * @param count Maximum number of code points to compare.
 * @return Negative value if lhs < rhs, zero if equal, positive value if lhs > rhs.
 */
template<detail::supported_string_character CharT>
[[nodiscard]] inline auto strncasecmp(
    const std::basic_string_view<CharT> lhs,
    const std::basic_string_view<CharT> rhs,
    const std::size_t count) -> result<int>
{
    auto lhs_decoded =
        detail::decode_normalized_string(lhs, detail::normalize_case_code_point);
    if (!lhs_decoded.has_value()) {
        return std::unexpected(lhs_decoded.error());
    }

    auto rhs_decoded =
        detail::decode_normalized_string(rhs, detail::normalize_case_code_point);
    if (!rhs_decoded.has_value()) {
        return std::unexpected(rhs_decoded.error());
    }

    return detail::compare_normalized_sequences(*lhs_decoded, *rhs_decoded, count);
}

/**
 * @brief Compares up to the specified number of code points after ASCII lowercase normalization.
 *
 * This overload accepts explicit buffer sizes so that pointer-based strings can
 * be compared without relying on a separate strnlen-style helper.
 *
 * @tparam CharT Character type.
 * @param lhs Left-hand string pointer.
 * @param lhs_size Maximum number of code units to inspect in @p lhs.
 * @param rhs Right-hand string pointer.
 * @param rhs_size Maximum number of code units to inspect in @p rhs.
 * @param count Maximum number of code points to compare.
 * @return Negative value if lhs < rhs, zero if equal, positive value if lhs > rhs.
 */
template<typename CharT>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] inline auto strncasecmp(
    const CharT* lhs,
    const std::size_t lhs_size,
    const CharT* rhs,
    const std::size_t rhs_size,
    const std::size_t count) -> result<int>
{
    auto lhs_length = xer::strlen(lhs, lhs_size);
    if (!lhs_length) {
        return std::unexpected(lhs_length.error());
    }

    auto rhs_length = xer::strlen(rhs, rhs_size);
    if (!rhs_length) {
        return std::unexpected(rhs_length.error());
    }

    using bare_char_t = std::remove_cv_t<CharT>;
    return xer::strncasecmp(
        std::basic_string_view<bare_char_t>(lhs, *lhs_length),
        std::basic_string_view<bare_char_t>(rhs, *rhs_length),
        count);
}

/**
 * @brief Compares up to the specified number of code points after ASCII lowercase normalization.
 *
 * This overload is intended to make string literals work naturally.
 *
 * @tparam CharT Character type.
 * @tparam N1 Left-hand array size.
 * @tparam N2 Right-hand array size.
 * @param lhs Left-hand array.
 * @param rhs Right-hand array.
 * @param count Maximum number of code points to compare.
 * @return Negative value if lhs < rhs, zero if equal, positive value if lhs > rhs.
 */
template<typename CharT, std::size_t N1, std::size_t N2>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] inline auto strncasecmp(
    const CharT (&lhs)[N1],
    const CharT (&rhs)[N2],
    const std::size_t count) -> result<int>
{
    return xer::strncasecmp(lhs, N1, rhs, N2, count);
}

/**
 * @brief Compares two strings after normalization with the specified transformation.
 *
 * @tparam CharT Character type.
 * @param lhs Left-hand string.
 * @param rhs Right-hand string.
 * @param trans Transformation identifier.
 * @return Negative value if lhs < rhs, zero if equal, positive value if lhs > rhs.
 */
template<detail::supported_string_character CharT>
[[nodiscard]] inline auto stricmp(
    const std::basic_string_view<CharT> lhs,
    const std::basic_string_view<CharT> rhs,
    const ctrans_id trans) -> result<int>
{
    const auto normalizer = [trans](const char32_t value) {
        return detail::normalize_ctrans_code_point(value, trans);
    };

    auto lhs_decoded = detail::decode_normalized_string(lhs, normalizer);
    if (!lhs_decoded.has_value()) {
        return std::unexpected(lhs_decoded.error());
    }

    auto rhs_decoded = detail::decode_normalized_string(rhs, normalizer);
    if (!rhs_decoded.has_value()) {
        return std::unexpected(rhs_decoded.error());
    }

    return detail::compare_normalized_sequences(*lhs_decoded, *rhs_decoded, SIZE_MAX);
}

/**
 * @brief Compares two strings after normalization with the specified transformation.
 *
 * This overload accepts explicit buffer sizes so that pointer-based strings can
 * be compared without relying on a separate strnlen-style helper.
 *
 * @tparam CharT Character type.
 * @param lhs Left-hand string pointer.
 * @param lhs_size Maximum number of code units to inspect in @p lhs.
 * @param rhs Right-hand string pointer.
 * @param rhs_size Maximum number of code units to inspect in @p rhs.
 * @param trans Transformation identifier.
 * @return Negative value if lhs < rhs, zero if equal, positive value if lhs > rhs.
 */
template<typename CharT>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] inline auto stricmp(
    const CharT* lhs,
    const std::size_t lhs_size,
    const CharT* rhs,
    const std::size_t rhs_size,
    const ctrans_id trans) -> result<int>
{
    auto lhs_length = xer::strlen(lhs, lhs_size);
    if (!lhs_length) {
        return std::unexpected(lhs_length.error());
    }

    auto rhs_length = xer::strlen(rhs, rhs_size);
    if (!rhs_length) {
        return std::unexpected(rhs_length.error());
    }

    using bare_char_t = std::remove_cv_t<CharT>;
    return xer::stricmp(
        std::basic_string_view<bare_char_t>(lhs, *lhs_length),
        std::basic_string_view<bare_char_t>(rhs, *rhs_length),
        trans);
}

/**
 * @brief Compares two array strings after normalization with the specified transformation.
 *
 * This overload is intended to make string literals work naturally.
 *
 * @tparam CharT Character type.
 * @tparam N1 Left-hand array size.
 * @tparam N2 Right-hand array size.
 * @param lhs Left-hand array.
 * @param rhs Right-hand array.
 * @param trans Transformation identifier.
 * @return Negative value if lhs < rhs, zero if equal, positive value if lhs > rhs.
 */
template<typename CharT, std::size_t N1, std::size_t N2>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] inline auto stricmp(
    const CharT (&lhs)[N1],
    const CharT (&rhs)[N2],
    const ctrans_id trans) -> result<int>
{
    return xer::stricmp(lhs, N1, rhs, N2, trans);
}

/**
 * @brief Compares up to the specified number of code points after ctrans normalization.
 *
 * @tparam CharT Character type.
 * @param lhs Left-hand string.
 * @param rhs Right-hand string.
 * @param count Maximum number of code points to compare.
 * @param trans Transformation identifier.
 * @return Negative value if lhs < rhs, zero if equal, positive value if lhs > rhs.
 */
template<detail::supported_string_character CharT>
[[nodiscard]] inline auto strnicmp(
    const std::basic_string_view<CharT> lhs,
    const std::basic_string_view<CharT> rhs,
    const std::size_t count,
    const ctrans_id trans) -> result<int>
{
    const auto normalizer = [trans](const char32_t value) {
        return detail::normalize_ctrans_code_point(value, trans);
    };

    auto lhs_decoded = detail::decode_normalized_string(lhs, normalizer);
    if (!lhs_decoded.has_value()) {
        return std::unexpected(lhs_decoded.error());
    }

    auto rhs_decoded = detail::decode_normalized_string(rhs, normalizer);
    if (!rhs_decoded.has_value()) {
        return std::unexpected(rhs_decoded.error());
    }

    return detail::compare_normalized_sequences(*lhs_decoded, *rhs_decoded, count);
}

/**
 * @brief Compares up to the specified number of code points after ctrans normalization.
 *
 * This overload accepts explicit buffer sizes so that pointer-based strings can
 * be compared without relying on a separate strnlen-style helper.
 *
 * @tparam CharT Character type.
 * @param lhs Left-hand string pointer.
 * @param lhs_size Maximum number of code units to inspect in @p lhs.
 * @param rhs Right-hand string pointer.
 * @param rhs_size Maximum number of code units to inspect in @p rhs.
 * @param count Maximum number of code points to compare.
 * @param trans Transformation identifier.
 * @return Negative value if lhs < rhs, zero if equal, positive value if lhs > rhs.
 */
template<typename CharT>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] inline auto strnicmp(
    const CharT* lhs,
    const std::size_t lhs_size,
    const CharT* rhs,
    const std::size_t rhs_size,
    const std::size_t count,
    const ctrans_id trans) -> result<int>
{
    auto lhs_length = xer::strlen(lhs, lhs_size);
    if (!lhs_length) {
        return std::unexpected(lhs_length.error());
    }

    auto rhs_length = xer::strlen(rhs, rhs_size);
    if (!rhs_length) {
        return std::unexpected(rhs_length.error());
    }

    using bare_char_t = std::remove_cv_t<CharT>;
    return xer::strnicmp(
        std::basic_string_view<bare_char_t>(lhs, *lhs_length),
        std::basic_string_view<bare_char_t>(rhs, *rhs_length),
        count,
        trans);
}

/**
 * @brief Compares up to the specified number of code points after ctrans normalization.
 *
 * This overload is intended to make string literals work naturally.
 *
 * @tparam CharT Character type.
 * @tparam N1 Left-hand array size.
 * @tparam N2 Right-hand array size.
 * @param lhs Left-hand array.
 * @param rhs Right-hand array.
 * @param count Maximum number of code points to compare.
 * @param trans Transformation identifier.
 * @return Negative value if lhs < rhs, zero if equal, positive value if lhs > rhs.
 */
template<typename CharT, std::size_t N1, std::size_t N2>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] inline auto strnicmp(
    const CharT (&lhs)[N1],
    const CharT (&rhs)[N2],
    const std::size_t count,
    const ctrans_id trans) -> result<int>
{
    return xer::strnicmp(lhs, N1, rhs, N2, count, trans);
}

/**
 * @brief Searches for the first code unit that matches after ASCII lowercase normalization.
 *
 * This overload preserves raw code-unit strchr semantics.
 *
 * @tparam CharT Character type.
 * @param source Source string.
 * @param value Code unit to search for.
 * @return Iterator pointing to the found code unit.
 */
template<detail::supported_string_character CharT>
[[nodiscard]] inline auto strcasechr(
    const std::basic_string_view<CharT> source,
    const CharT value) -> result<typename std::basic_string_view<CharT>::const_iterator>
{
    auto needle = detail::normalize_code_unit(value, detail::normalize_case_code_point);
    if (!needle.has_value()) {
        return std::unexpected(needle.error());
    }

    for (auto it = source.begin(); it != source.end(); ++it) {
        auto normalized =
            detail::normalize_code_unit(*it, detail::normalize_case_code_point);
        if (!normalized.has_value()) {
            return std::unexpected(normalized.error());
        }
        if (*normalized == *needle) {
            return it;
        }
    }

    return std::unexpected(make_error(error_t::not_found));
}

/**
 * @brief Searches for the first code point that matches after ASCII lowercase normalization.
 *
 * @param source Source UTF-8 string.
 * @param value Code point to search for.
 * @return Iterator pointing to the first code unit of the found code point.
 */
[[nodiscard]] inline auto strcasechr(
    const std::u8string_view source,
    const char32_t value) -> result<std::u8string_view::const_iterator>
{
    auto needle = detail::normalize_case_code_point(value);
    if (!needle.has_value()) {
        return std::unexpected(needle.error());
    }

    for (std::size_t index = 0; index < source.size();) {
        auto decoded = detail::decode_utf8_at(source, index);
        if (!decoded.has_value()) {
            return std::unexpected(decoded.error());
        }
        auto normalized = detail::normalize_case_code_point(decoded->value);
        if (!normalized.has_value()) {
            return std::unexpected(normalized.error());
        }
        if (*normalized == *needle) {
            return source.begin() + static_cast<std::ptrdiff_t>(index);
        }
        index += decoded->size;
    }

    return std::unexpected(make_error(error_t::not_found));
}

/**
 * @brief Searches for the first code point that matches after ASCII lowercase normalization.
 *
 * @param source Source UTF-16 string.
 * @param value Code point to search for.
 * @return Iterator pointing to the first code unit of the found code point.
 */
[[nodiscard]] inline auto strcasechr(
    const std::u16string_view source,
    const char32_t value) -> result<std::u16string_view::const_iterator>
{
    auto needle = detail::normalize_case_code_point(value);
    if (!needle.has_value()) {
        return std::unexpected(needle.error());
    }

    for (std::size_t index = 0; index < source.size();) {
        auto decoded = detail::decode_utf16_at(source, index);
        if (!decoded.has_value()) {
            return std::unexpected(decoded.error());
        }
        auto normalized = detail::normalize_case_code_point(decoded->value);
        if (!normalized.has_value()) {
            return std::unexpected(normalized.error());
        }
        if (*normalized == *needle) {
            return source.begin() + static_cast<std::ptrdiff_t>(index);
        }
        index += decoded->size;
    }

    return std::unexpected(make_error(error_t::not_found));
}

/**
 * @brief Searches for the last code unit that matches after ASCII lowercase normalization.
 *
 * This overload preserves raw code-unit strrchr semantics.
 *
 * @tparam CharT Character type.
 * @param source Source string.
 * @param value Code unit to search for.
 * @return Iterator pointing to the found code unit.
 */
template<detail::supported_string_character CharT>
[[nodiscard]] inline auto strcaserchr(
    const std::basic_string_view<CharT> source,
    const CharT value) -> result<typename std::basic_string_view<CharT>::const_iterator>
{
    auto needle = detail::normalize_code_unit(value, detail::normalize_case_code_point);
    if (!needle.has_value()) {
        return std::unexpected(needle.error());
    }

    for (auto it = source.end(); it != source.begin();) {
        --it;
        auto normalized =
            detail::normalize_code_unit(*it, detail::normalize_case_code_point);
        if (!normalized.has_value()) {
            return std::unexpected(normalized.error());
        }
        if (*normalized == *needle) {
            return it;
        }
    }

    return std::unexpected(make_error(error_t::not_found));
}

/**
 * @brief Searches for the last code point that matches after ASCII lowercase normalization.
 *
 * @param source Source UTF-8 string.
 * @param value Code point to search for.
 * @return Iterator pointing to the first code unit of the found code point.
 */
[[nodiscard]] inline auto strcaserchr(
    const std::u8string_view source,
    const char32_t value) -> result<std::u8string_view::const_iterator>
{
    auto needle = detail::normalize_case_code_point(value);
    if (!needle.has_value()) {
        return std::unexpected(needle.error());
    }

    bool found = false;
    std::size_t found_index = 0;
    for (std::size_t index = 0; index < source.size();) {
        auto decoded = detail::decode_utf8_at(source, index);
        if (!decoded.has_value()) {
            return std::unexpected(decoded.error());
        }
        auto normalized = detail::normalize_case_code_point(decoded->value);
        if (!normalized.has_value()) {
            return std::unexpected(normalized.error());
        }
        if (*normalized == *needle) {
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
 * @brief Searches for the last code point that matches after ASCII lowercase normalization.
 *
 * @param source Source UTF-16 string.
 * @param value Code point to search for.
 * @return Iterator pointing to the first code unit of the found code point.
 */
[[nodiscard]] inline auto strcaserchr(
    const std::u16string_view source,
    const char32_t value) -> result<std::u16string_view::const_iterator>
{
    auto needle = detail::normalize_case_code_point(value);
    if (!needle.has_value()) {
        return std::unexpected(needle.error());
    }

    bool found = false;
    std::size_t found_index = 0;
    for (std::size_t index = 0; index < source.size();) {
        auto decoded = detail::decode_utf16_at(source, index);
        if (!decoded.has_value()) {
            return std::unexpected(decoded.error());
        }
        auto normalized = detail::normalize_case_code_point(decoded->value);
        if (!normalized.has_value()) {
            return std::unexpected(normalized.error());
        }
        if (*normalized == *needle) {
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
 * @brief Searches for the first code unit that matches after ctrans normalization.
 *
 * This overload preserves raw code-unit strchr semantics.
 *
 * @tparam CharT Character type.
 * @param source Source string.
 * @param value Code unit to search for.
 * @param trans Transformation identifier.
 * @return Iterator pointing to the found code unit.
 */
template<detail::supported_string_character CharT>
[[nodiscard]] inline auto strichr(
    const std::basic_string_view<CharT> source,
    const CharT value,
    const ctrans_id trans) -> result<typename std::basic_string_view<CharT>::const_iterator>
{
    const auto normalizer = [trans](const char32_t cp) {
        return detail::normalize_ctrans_code_point(cp, trans);
    };

    auto needle = detail::normalize_code_unit(value, normalizer);
    if (!needle.has_value()) {
        return std::unexpected(needle.error());
    }

    for (auto it = source.begin(); it != source.end(); ++it) {
        auto normalized = detail::normalize_code_unit(*it, normalizer);
        if (!normalized.has_value()) {
            return std::unexpected(normalized.error());
        }
        if (*normalized == *needle) {
            return it;
        }
    }

    return std::unexpected(make_error(error_t::not_found));
}

/**
 * @brief Searches for the first code point that matches after ctrans normalization.
 *
 * @param source Source UTF-8 string.
 * @param value Code point to search for.
 * @param trans Transformation identifier.
 * @return Iterator pointing to the first code unit of the found code point.
 */
[[nodiscard]] inline auto strichr(
    const std::u8string_view source,
    const char32_t value,
    const ctrans_id trans) -> result<std::u8string_view::const_iterator>
{
    const auto normalizer = [trans](const char32_t cp) {
        return detail::normalize_ctrans_code_point(cp, trans);
    };

    auto needle = normalizer(value);
    if (!needle.has_value()) {
        return std::unexpected(needle.error());
    }

    for (std::size_t index = 0; index < source.size();) {
        auto decoded = detail::decode_utf8_at(source, index);
        if (!decoded.has_value()) {
            return std::unexpected(decoded.error());
        }
        auto normalized = normalizer(decoded->value);
        if (!normalized.has_value()) {
            return std::unexpected(normalized.error());
        }
        if (*normalized == *needle) {
            return source.begin() + static_cast<std::ptrdiff_t>(index);
        }
        index += decoded->size;
    }

    return std::unexpected(make_error(error_t::not_found));
}

/**
 * @brief Searches for the first code point that matches after ctrans normalization.
 *
 * @param source Source UTF-16 string.
 * @param value Code point to search for.
 * @param trans Transformation identifier.
 * @return Iterator pointing to the first code unit of the found code point.
 */
[[nodiscard]] inline auto strichr(
    const std::u16string_view source,
    const char32_t value,
    const ctrans_id trans) -> result<std::u16string_view::const_iterator>
{
    const auto normalizer = [trans](const char32_t cp) {
        return detail::normalize_ctrans_code_point(cp, trans);
    };

    auto needle = normalizer(value);
    if (!needle.has_value()) {
        return std::unexpected(needle.error());
    }

    for (std::size_t index = 0; index < source.size();) {
        auto decoded = detail::decode_utf16_at(source, index);
        if (!decoded.has_value()) {
            return std::unexpected(decoded.error());
        }
        auto normalized = normalizer(decoded->value);
        if (!normalized.has_value()) {
            return std::unexpected(normalized.error());
        }
        if (*normalized == *needle) {
            return source.begin() + static_cast<std::ptrdiff_t>(index);
        }
        index += decoded->size;
    }

    return std::unexpected(make_error(error_t::not_found));
}

/**
 * @brief Searches for the last code unit that matches after ctrans normalization.
 *
 * This overload preserves raw code-unit strrchr semantics.
 *
 * @tparam CharT Character type.
 * @param source Source string.
 * @param value Code unit to search for.
 * @param trans Transformation identifier.
 * @return Iterator pointing to the found code unit.
 */
template<detail::supported_string_character CharT>
[[nodiscard]] inline auto strirchr(
    const std::basic_string_view<CharT> source,
    const CharT value,
    const ctrans_id trans) -> result<typename std::basic_string_view<CharT>::const_iterator>
{
    const auto normalizer = [trans](const char32_t cp) {
        return detail::normalize_ctrans_code_point(cp, trans);
    };

    auto needle = detail::normalize_code_unit(value, normalizer);
    if (!needle.has_value()) {
        return std::unexpected(needle.error());
    }

    for (auto it = source.end(); it != source.begin();) {
        --it;
        auto normalized = detail::normalize_code_unit(*it, normalizer);
        if (!normalized.has_value()) {
            return std::unexpected(normalized.error());
        }
        if (*normalized == *needle) {
            return it;
        }
    }

    return std::unexpected(make_error(error_t::not_found));
}

/**
 * @brief Searches for the last code point that matches after ctrans normalization.
 *
 * @param source Source UTF-8 string.
 * @param value Code point to search for.
 * @param trans Transformation identifier.
 * @return Iterator pointing to the first code unit of the found code point.
 */
[[nodiscard]] inline auto strirchr(
    const std::u8string_view source,
    const char32_t value,
    const ctrans_id trans) -> result<std::u8string_view::const_iterator>
{
    const auto normalizer = [trans](const char32_t cp) {
        return detail::normalize_ctrans_code_point(cp, trans);
    };

    auto needle = normalizer(value);
    if (!needle.has_value()) {
        return std::unexpected(needle.error());
    }

    bool found = false;
    std::size_t found_index = 0;
    for (std::size_t index = 0; index < source.size();) {
        auto decoded = detail::decode_utf8_at(source, index);
        if (!decoded.has_value()) {
            return std::unexpected(decoded.error());
        }
        auto normalized = normalizer(decoded->value);
        if (!normalized.has_value()) {
            return std::unexpected(normalized.error());
        }
        if (*normalized == *needle) {
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
 * @brief Searches for the last code point that matches after ctrans normalization.
 *
 * @param source Source UTF-16 string.
 * @param value Code point to search for.
 * @param trans Transformation identifier.
 * @return Iterator pointing to the first code unit of the found code point.
 */
[[nodiscard]] inline auto strirchr(
    const std::u16string_view source,
    const char32_t value,
    const ctrans_id trans) -> result<std::u16string_view::const_iterator>
{
    const auto normalizer = [trans](const char32_t cp) {
        return detail::normalize_ctrans_code_point(cp, trans);
    };

    auto needle = normalizer(value);
    if (!needle.has_value()) {
        return std::unexpected(needle.error());
    }

    bool found = false;
    std::size_t found_index = 0;
    for (std::size_t index = 0; index < source.size();) {
        auto decoded = detail::decode_utf16_at(source, index);
        if (!decoded.has_value()) {
            return std::unexpected(decoded.error());
        }
        auto normalized = normalizer(decoded->value);
        if (!normalized.has_value()) {
            return std::unexpected(normalized.error());
        }
        if (*normalized == *needle) {
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
 * @brief Returns the first position of a substring after ASCII lowercase normalization.
 *
 * Returned positions are expressed in the original source code-unit index.
 *
 * @tparam CharT Character type.
 * @param source Source string.
 * @param pattern Pattern string.
 * @return Zero-based code-unit position of the found substring.
 */
template<detail::supported_string_character CharT>
[[nodiscard]] inline auto strcasepos(
    const std::basic_string_view<CharT> source,
    const std::basic_string_view<CharT> pattern) -> result<std::size_t>
{
    auto decoded_source =
        detail::decode_normalized_string(source, detail::normalize_case_code_point);
    if (!decoded_source.has_value()) {
        return std::unexpected(decoded_source.error());
    }

    auto decoded_pattern =
        detail::decode_normalized_string(pattern, detail::normalize_case_code_point);
    if (!decoded_pattern.has_value()) {
        return std::unexpected(decoded_pattern.error());
    }

    return detail::find_first_normalized_subsequence(*decoded_source, *decoded_pattern);
}

/**
 * @brief Returns the last position of a substring after ASCII lowercase normalization.
 *
 * Returned positions are expressed in the original source code-unit index.
 *
 * @tparam CharT Character type.
 * @param source Source string.
 * @param pattern Pattern string.
 * @return Zero-based code-unit position of the found substring.
 */
template<detail::supported_string_character CharT>
[[nodiscard]] inline auto strcaserpos(
    const std::basic_string_view<CharT> source,
    const std::basic_string_view<CharT> pattern) -> result<std::size_t>
{
    auto decoded_source =
        detail::decode_normalized_string(source, detail::normalize_case_code_point);
    if (!decoded_source.has_value()) {
        return std::unexpected(decoded_source.error());
    }

    auto decoded_pattern =
        detail::decode_normalized_string(pattern, detail::normalize_case_code_point);
    if (!decoded_pattern.has_value()) {
        return std::unexpected(decoded_pattern.error());
    }

    return detail::find_last_normalized_subsequence(
        *decoded_source, *decoded_pattern, source.size());
}

/**
 * @brief Searches for the first occurrence of a substring after ASCII lowercase normalization.
 *
 * @tparam CharT Character type.
 * @param source Source string.
 * @param pattern Pattern string.
 * @return Iterator pointing to the beginning of the found substring.
 */
template<detail::supported_string_character CharT>
[[nodiscard]] inline auto strcasestr(
    const std::basic_string_view<CharT> source,
    const std::basic_string_view<CharT> pattern)
    -> result<typename std::basic_string_view<CharT>::const_iterator>
{
    auto pos = strcasepos(source, pattern);
    if (!pos.has_value()) {
        return std::unexpected(pos.error());
    }
    return source.begin() + static_cast<std::ptrdiff_t>(*pos);
}

/**
 * @brief Searches for the last occurrence of a substring after ASCII lowercase normalization.
 *
 * @tparam CharT Character type.
 * @param source Source string.
 * @param pattern Pattern string.
 * @return Iterator pointing to the beginning of the found substring.
 */
template<detail::supported_string_character CharT>
[[nodiscard]] inline auto strcaserstr(
    const std::basic_string_view<CharT> source,
    const std::basic_string_view<CharT> pattern)
    -> result<typename std::basic_string_view<CharT>::const_iterator>
{
    auto pos = strcaserpos(source, pattern);
    if (!pos.has_value()) {
        return std::unexpected(pos.error());
    }
    return source.begin() + static_cast<std::ptrdiff_t>(*pos);
}

/**
 * @brief Returns the first position of a substring after ctrans normalization.
 *
 * Returned positions are expressed in the original source code-unit index.
 *
 * @tparam CharT Character type.
 * @param source Source string.
 * @param pattern Pattern string.
 * @param trans Transformation identifier.
 * @return Zero-based code-unit position of the found substring.
 */
template<detail::supported_string_character CharT>
[[nodiscard]] inline auto stripos(
    const std::basic_string_view<CharT> source,
    const std::basic_string_view<CharT> pattern,
    const ctrans_id trans) -> result<std::size_t>
{
    const auto normalizer = [trans](const char32_t cp) {
        return detail::normalize_ctrans_code_point(cp, trans);
    };

    auto decoded_source = detail::decode_normalized_string(source, normalizer);
    if (!decoded_source.has_value()) {
        return std::unexpected(decoded_source.error());
    }

    auto decoded_pattern = detail::decode_normalized_string(pattern, normalizer);
    if (!decoded_pattern.has_value()) {
        return std::unexpected(decoded_pattern.error());
    }

    return detail::find_first_normalized_subsequence(*decoded_source, *decoded_pattern);
}

/**
 * @brief Returns the last position of a substring after ctrans normalization.
 *
 * Returned positions are expressed in the original source code-unit index.
 *
 * @tparam CharT Character type.
 * @param source Source string.
 * @param pattern Pattern string.
 * @param trans Transformation identifier.
 * @return Zero-based code-unit position of the found substring.
 */
template<detail::supported_string_character CharT>
[[nodiscard]] inline auto strirpos(
    const std::basic_string_view<CharT> source,
    const std::basic_string_view<CharT> pattern,
    const ctrans_id trans) -> result<std::size_t>
{
    const auto normalizer = [trans](const char32_t cp) {
        return detail::normalize_ctrans_code_point(cp, trans);
    };

    auto decoded_source = detail::decode_normalized_string(source, normalizer);
    if (!decoded_source.has_value()) {
        return std::unexpected(decoded_source.error());
    }

    auto decoded_pattern = detail::decode_normalized_string(pattern, normalizer);
    if (!decoded_pattern.has_value()) {
        return std::unexpected(decoded_pattern.error());
    }

    return detail::find_last_normalized_subsequence(
        *decoded_source, *decoded_pattern, source.size());
}

/**
 * @brief Searches for the first occurrence of a substring after ctrans normalization.
 *
 * @tparam CharT Character type.
 * @param source Source string.
 * @param pattern Pattern string.
 * @param trans Transformation identifier.
 * @return Iterator pointing to the beginning of the found substring.
 */
template<detail::supported_string_character CharT>
[[nodiscard]] inline auto stristr(
    const std::basic_string_view<CharT> source,
    const std::basic_string_view<CharT> pattern,
    const ctrans_id trans)
    -> result<typename std::basic_string_view<CharT>::const_iterator>
{
    auto pos = stripos(source, pattern, trans);
    if (!pos.has_value()) {
        return std::unexpected(pos.error());
    }
    return source.begin() + static_cast<std::ptrdiff_t>(*pos);
}

/**
 * @brief Searches for the last occurrence of a substring after ctrans normalization.
 *
 * @tparam CharT Character type.
 * @param source Source string.
 * @param pattern Pattern string.
 * @param trans Transformation identifier.
 * @return Iterator pointing to the beginning of the found substring.
 */
template<detail::supported_string_character CharT>
[[nodiscard]] inline auto strirstr(
    const std::basic_string_view<CharT> source,
    const std::basic_string_view<CharT> pattern,
    const ctrans_id trans)
    -> result<typename std::basic_string_view<CharT>::const_iterator>
{
    auto pos = strirpos(source, pattern, trans);
    if (!pos.has_value()) {
        return std::unexpected(pos.error());
    }
    return source.begin() + static_cast<std::ptrdiff_t>(*pos);
}

} // namespace xer

#endif /* XER_BITS_STRING_COMPARE_H_INCLUDED_ */
