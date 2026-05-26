/**
 * @file xer/bits/unicode_grapheme_string.h
 * @brief String operations based on Unicode grapheme clusters.
 */

#pragma once

#ifndef XER_BITS_UNICODE_GRAPHEME_STRING_H_INCLUDED_
#define XER_BITS_UNICODE_GRAPHEME_STRING_H_INCLUDED_

#include <cstddef>
#include <expected>
#include <string_view>

#include <xer/error.h>
#include <xer/bits/unicode_code_point.h>
#include <xer/bits/unicode_grapheme_cluster.h>

namespace xer {

namespace detail {

template<code_point_source_char CharType>
[[nodiscard]] inline auto grapheme_length_impl(
    std::basic_string_view<CharType> text) -> result<std::size_t>
{
    std::size_t count = 0;
    for (const auto& item : grapheme_clusters(text)) {
        if (!item.has_value()) {
            return std::unexpected(item.error());
        }
        ++count;
    }
    return count;
}

template<code_point_source_char CharType>
[[nodiscard]] inline auto grapheme_substr_impl(
    std::basic_string_view<CharType> text,
    std::size_t offset,
    std::size_t count) -> result<std::basic_string_view<CharType>>
{
    using string_view_type = std::basic_string_view<CharType>;

    std::size_t position = 0;
    std::size_t index = 0;

    while (index < offset) {
        if (position >= text.size()) {
            return std::unexpected(make_error(error_t::out_of_range));
        }

        auto item = next_grapheme_cluster(text, position);
        if (!item.has_value()) {
            return std::unexpected(item.error());
        }

        position = item->offset + item->size;
        ++index;
    }

    const std::size_t start = position;
    if (count == 0) {
        return text.substr(start, 0);
    }

    if (count == string_view_type::npos) {
        while (position < text.size()) {
            auto item = next_grapheme_cluster(text, position);
            if (!item.has_value()) {
                return std::unexpected(item.error());
            }
            position = item->offset + item->size;
        }
        return text.substr(start, position - start);
    }

    std::size_t taken = 0;
    while (taken < count && position < text.size()) {
        auto item = next_grapheme_cluster(text, position);
        if (!item.has_value()) {
            return std::unexpected(item.error());
        }
        position = item->offset + item->size;
        ++taken;
    }

    return text.substr(start, position - start);
}

template<code_point_source_char CharType>
[[nodiscard]] inline auto grapheme_left_impl(
    std::basic_string_view<CharType> text,
    std::size_t count) -> result<std::basic_string_view<CharType>>
{
    return grapheme_substr_impl(text, 0, count);
}

template<code_point_source_char CharType>
[[nodiscard]] inline auto grapheme_right_impl(
    std::basic_string_view<CharType> text,
    std::size_t count) -> result<std::basic_string_view<CharType>>
{
    const auto length = grapheme_length_impl(text);
    if (!length.has_value()) {
        return std::unexpected(length.error());
    }

    if (count >= *length) {
        return text;
    }

    return grapheme_substr_impl(text, *length - count, std::basic_string_view<CharType>::npos);
}

} // namespace detail

/**
 * @brief Counts extended grapheme clusters in a UTF-8 string view.
 * @param text Source UTF-8 string view.
 * @return Number of grapheme clusters or an error.
 */
[[nodiscard]] inline auto grapheme_length(std::u8string_view text)
    -> result<std::size_t>
{
    return detail::grapheme_length_impl(text);
}

/**
 * @brief Counts extended grapheme clusters in a UTF-16 string view.
 * @param text Source UTF-16 string view.
 * @return Number of grapheme clusters or an error.
 */
[[nodiscard]] inline auto grapheme_length(std::u16string_view text)
    -> result<std::size_t>
{
    return detail::grapheme_length_impl(text);
}

/**
 * @brief Counts extended grapheme clusters in a wide string view.
 * @param text Source wide string view.
 * @return Number of grapheme clusters or an error.
 */
[[nodiscard]] inline auto grapheme_length(std::wstring_view text)
    -> result<std::size_t>
{
    return detail::grapheme_length_impl(text);
}

/**
 * @brief Extracts a UTF-8 substring by grapheme cluster index.
 * @param text Source UTF-8 string view.
 * @param offset Grapheme cluster offset.
 * @param count Number of grapheme clusters, or `std::u8string_view::npos` for the rest.
 * @return Substring view or an error.
 */
[[nodiscard]] inline auto grapheme_substr(
    std::u8string_view text,
    std::size_t offset,
    std::size_t count = std::u8string_view::npos) -> result<std::u8string_view>
{
    return detail::grapheme_substr_impl(text, offset, count);
}

/**
 * @brief Extracts a UTF-16 substring by grapheme cluster index.
 * @param text Source UTF-16 string view.
 * @param offset Grapheme cluster offset.
 * @param count Number of grapheme clusters, or `std::u16string_view::npos` for the rest.
 * @return Substring view or an error.
 */
[[nodiscard]] inline auto grapheme_substr(
    std::u16string_view text,
    std::size_t offset,
    std::size_t count = std::u16string_view::npos) -> result<std::u16string_view>
{
    return detail::grapheme_substr_impl(text, offset, count);
}

/**
 * @brief Extracts a wide substring by grapheme cluster index.
 * @param text Source wide string view.
 * @param offset Grapheme cluster offset.
 * @param count Number of grapheme clusters, or `std::wstring_view::npos` for the rest.
 * @return Substring view or an error.
 */
[[nodiscard]] inline auto grapheme_substr(
    std::wstring_view text,
    std::size_t offset,
    std::size_t count = std::wstring_view::npos) -> result<std::wstring_view>
{
    return detail::grapheme_substr_impl(text, offset, count);
}

/**
 * @brief Extracts the left part of a UTF-8 string view by grapheme cluster count.
 * @param text Source UTF-8 string view.
 * @param count Number of grapheme clusters.
 * @return Substring view or an error.
 */
[[nodiscard]] inline auto grapheme_left(
    std::u8string_view text,
    std::size_t count) -> result<std::u8string_view>
{
    return detail::grapheme_left_impl(text, count);
}

/**
 * @brief Extracts the left part of a UTF-16 string view by grapheme cluster count.
 * @param text Source UTF-16 string view.
 * @param count Number of grapheme clusters.
 * @return Substring view or an error.
 */
[[nodiscard]] inline auto grapheme_left(
    std::u16string_view text,
    std::size_t count) -> result<std::u16string_view>
{
    return detail::grapheme_left_impl(text, count);
}

/**
 * @brief Extracts the left part of a wide string view by grapheme cluster count.
 * @param text Source wide string view.
 * @param count Number of grapheme clusters.
 * @return Substring view or an error.
 */
[[nodiscard]] inline auto grapheme_left(
    std::wstring_view text,
    std::size_t count) -> result<std::wstring_view>
{
    return detail::grapheme_left_impl(text, count);
}

/**
 * @brief Extracts the right part of a UTF-8 string view by grapheme cluster count.
 * @param text Source UTF-8 string view.
 * @param count Number of grapheme clusters.
 * @return Substring view or an error.
 */
[[nodiscard]] inline auto grapheme_right(
    std::u8string_view text,
    std::size_t count) -> result<std::u8string_view>
{
    return detail::grapheme_right_impl(text, count);
}

/**
 * @brief Extracts the right part of a UTF-16 string view by grapheme cluster count.
 * @param text Source UTF-16 string view.
 * @param count Number of grapheme clusters.
 * @return Substring view or an error.
 */
[[nodiscard]] inline auto grapheme_right(
    std::u16string_view text,
    std::size_t count) -> result<std::u16string_view>
{
    return detail::grapheme_right_impl(text, count);
}

/**
 * @brief Extracts the right part of a wide string view by grapheme cluster count.
 * @param text Source wide string view.
 * @param count Number of grapheme clusters.
 * @return Substring view or an error.
 */
[[nodiscard]] inline auto grapheme_right(
    std::wstring_view text,
    std::size_t count) -> result<std::wstring_view>
{
    return detail::grapheme_right_impl(text, count);
}

} // namespace xer

#endif /* XER_BITS_UNICODE_GRAPHEME_STRING_H_INCLUDED_ */
