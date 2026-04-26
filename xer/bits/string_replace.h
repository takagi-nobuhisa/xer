/**
 * @file xer/bits/string_replace.h
 * @brief Internal string replacement function implementations.
 */

#pragma once

#ifndef XER_BITS_STRING_REPLACE_H_INCLUDED_
#define XER_BITS_STRING_REPLACE_H_INCLUDED_

#include <cstddef>
#include <expected>
#include <string>
#include <string_view>

#include <xer/bits/common.h>
#include <xer/error.h>

namespace xer::detail {

/**
 * @brief Counts non-overlapping occurrences of a UTF-8 byte sequence.
 *
 * The search is performed in std::u8string_view code units. This matches
 * ordinary substring replacement semantics and does not decode UTF-8 code
 * points. An empty search string has no replaceable occurrences.
 *
 * @param search Search string.
 * @param subject Subject string.
 * @return Number of non-overlapping occurrences.
 */
[[nodiscard]] inline auto count_str_replace_occurrences(
    const std::u8string_view search,
    const std::u8string_view subject) noexcept -> std::size_t
{
    if (search.empty()) {
        return 0;
    }

    std::size_t count = 0;
    std::size_t offset = 0;

    while (offset <= subject.size()) {
        const std::size_t position = subject.find(search, offset);
        if (position == std::u8string_view::npos) {
            break;
        }

        ++count;
        offset = position + search.size();
    }

    return count;
}

/**
 * @brief Calculates the resulting string size for str_replace().
 *
 * @param search Search string.
 * @param replace Replacement string.
 * @param subject Subject string.
 * @param occurrence_count Number of replacements to perform.
 * @return Result size on success.
 */
[[nodiscard]] inline auto calculate_str_replace_size(
    const std::u8string_view search,
    const std::u8string_view replace,
    const std::u8string_view subject,
    const std::size_t occurrence_count) noexcept -> result<std::size_t>
{
    const std::size_t max_size = std::u8string().max_size();

    if (replace.size() <= search.size()) {
        const std::size_t shrink_per_occurrence = search.size() - replace.size();
        return subject.size() - (shrink_per_occurrence * occurrence_count);
    }

    const std::size_t growth_per_occurrence = replace.size() - search.size();
    if (occurrence_count != 0 &&
        growth_per_occurrence >
            (max_size - subject.size()) / occurrence_count) {
        return std::unexpected(make_error(error_t::length_error));
    }

    return subject.size() + (growth_per_occurrence * occurrence_count);
}

/**
 * @brief Replaces all non-overlapping occurrences in a UTF-8 string.
 *
 * @param search Search string. An empty string performs no replacement.
 * @param replace Replacement string.
 * @param subject Subject string.
 * @param count Optional destination for the number of replacements.
 * @return Replaced string on success.
 */
[[nodiscard]] inline auto str_replace_impl(
    const std::u8string_view search,
    const std::u8string_view replace,
    const std::u8string_view subject,
    std::size_t* const count) -> result<std::u8string>
{
    const std::size_t occurrence_count =
        count_str_replace_occurrences(search, subject);

    if (count != nullptr) {
        *count = occurrence_count;
    }

    const auto result_size = calculate_str_replace_size(
        search,
        replace,
        subject,
        occurrence_count);
    if (!result_size.has_value()) {
        return std::unexpected(result_size.error());
    }

    std::u8string result;
    result.reserve(*result_size);

    if (search.empty() || occurrence_count == 0) {
        result.assign(subject.data(), subject.size());
        return result;
    }

    std::size_t copied = 0;
    while (copied < subject.size()) {
        const std::size_t position = subject.find(search, copied);
        if (position == std::u8string_view::npos) {
            break;
        }

        result.append(subject.data() + copied, position - copied);
        result.append(replace.data(), replace.size());
        copied = position + search.size();
    }

    result.append(subject.data() + copied, subject.size() - copied);
    return result;
}

} // namespace xer::detail

namespace xer {

/**
 * @brief Replaces all non-overlapping occurrences of a UTF-8 substring.
 *
 * This function follows the basic argument order of PHP str_replace(): search,
 * replace, then subject. Replacement is performed on UTF-8 code units, not on
 * decoded code points. This is appropriate for substring replacement because
 * the searched and replaced values are themselves UTF-8 strings.
 *
 * If @p search is empty, the subject is returned unchanged and the replacement
 * count is zero.
 *
 * @param search Search string.
 * @param replace Replacement string.
 * @param subject Subject string.
 * @param count Optional destination for the number of replacements.
 * @return Replaced string on success.
 */
[[nodiscard]] inline auto str_replace(
    const std::u8string_view search,
    const std::u8string_view replace,
    const std::u8string_view subject,
    std::size_t* const count = nullptr) -> result<std::u8string>
{
    return detail::str_replace_impl(search, replace, subject, count);
}

} // namespace xer

#endif /* XER_BITS_STRING_REPLACE_H_INCLUDED_ */
