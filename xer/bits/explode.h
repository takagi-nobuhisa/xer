/**
 * @file xer/bits/explode.h
 * @brief Internal explode function implementations.
 */

#pragma once

#ifndef XER_BITS_EXPLODE_H_INCLUDED_
#define XER_BITS_EXPLODE_H_INCLUDED_

#include <climits>
#include <cstddef>
#include <expected>
#include <string>
#include <string_view>
#include <vector>

#include <xer/bits/common.h>
#include <xer/error.h>

namespace xer::detail {

/**
 * @brief Creates an unexpected error result for explode functions.
 *
 * @tparam T Value type of expected.
 * @param code Error code.
 * @return Unexpected error result.
 */
template<typename T>
[[nodiscard]] inline result<T> unexpected_explode_error(
    const error_t code)
{
    return std::unexpected(make_error(code));
}

/**
 * @brief Splits a UTF-8 string and returns string views.
 *
 * This function follows PHP explode() semantics for the limit argument.
 *
 * @param separator Separator string. Must not be empty.
 * @param source Source string to split.
 * @param limit Maximum number of elements to return.
 * @return Vector of string views on success.
 */
[[nodiscard]] inline result<std::vector<std::u8string_view>>
explode_view_impl(
    const std::u8string_view separator,
    const std::u8string_view source,
    int limit)
{
    if (separator.empty()) {
        return unexpected_explode_error<std::vector<std::u8string_view>>(
            error_t::invalid_argument);
    }

    if (limit == 0) {
        limit = 1;
    }

    std::vector<std::u8string_view> result;

    if (limit > 0) {
        if (limit == 1) {
            result.emplace_back(source);
            return result;
        }

        std::size_t start = 0;
        int remaining = limit;

        while (remaining > 1) {
            const std::size_t position = source.find(separator, start);
            if (position == std::u8string_view::npos) {
                break;
            }

            result.emplace_back(source.substr(start, position - start));
            start = position + separator.size();
            --remaining;
        }

        result.emplace_back(source.substr(start));
        return result;
    }

    std::size_t start = 0;
    while (true) {
        const std::size_t position = source.find(separator, start);
        if (position == std::u8string_view::npos) {
            break;
        }

        result.emplace_back(source.substr(start, position - start));
        start = position + separator.size();
    }

    result.emplace_back(source.substr(start));

    const std::size_t remove_count =
        static_cast<std::size_t>(-(static_cast<long long>(limit)));

    if (remove_count >= result.size()) {
        result.clear();
        return result;
    }

    result.resize(result.size() - remove_count);
    return result;
}

/**
 * @brief Splits a UTF-8 string and returns owning strings.
 *
 * This function is implemented on top of explode_view_impl().
 *
 * @param separator Separator string. Must not be empty.
 * @param source Source string to split.
 * @param limit Maximum number of elements to return.
 * @return Vector of owning strings on success.
 */
[[nodiscard]] inline result<std::vector<std::u8string>>
explode_impl(
    const std::u8string_view separator,
    const std::u8string_view source,
    const int limit)
{
    const auto views = explode_view_impl(separator, source, limit);
    if (!views.has_value()) {
        return std::unexpected(views.error());
    }

    std::vector<std::u8string> result;
    result.reserve(views->size());

    for (const std::u8string_view part : *views) {
        result.emplace_back(part);
    }

    return result;
}

} // namespace xer::detail

namespace xer {

/**
 * @brief Splits a UTF-8 string by a separator and returns owning strings.
 *
 * This function follows PHP explode() semantics for the limit argument.
 *
 * - limit > 0: returns at most limit elements, with the last containing the
 *   remainder of the string.
 * - limit == 0: treated as 1.
 * - limit < 0: splits normally, then removes -limit elements from the end.
 *
 * @param separator Separator string. Must not be empty.
 * @param source Source string to split.
 * @param limit Maximum number of elements to return.
 * @return Vector of owning strings on success.
 */
[[nodiscard]] inline result<std::vector<std::u8string>>
explode(
    const std::u8string_view separator,
    const std::u8string_view source,
    const int limit = INT_MAX)
{
    return detail::explode_impl(separator, source, limit);
}

/**
 * @brief Splits a UTF-8 string by a separator and returns string views.
 *
 * This function follows PHP explode() semantics for the limit argument.
 *
 * The returned views refer to the storage of @p source. The caller must ensure
 * that the underlying character data remains alive while the returned views are
 * used.
 *
 * @param separator Separator string. Must not be empty.
 * @param source Source string to split.
 * @param limit Maximum number of elements to return.
 * @return Vector of string views on success.
 */
[[nodiscard]] inline result<std::vector<std::u8string_view>>
explode_view(
    const std::u8string_view separator,
    const std::u8string_view source,
    const int limit = INT_MAX)
{
    return detail::explode_view_impl(separator, source, limit);
}

} // namespace xer

#endif /* XER_BITS_EXPLODE_H_INCLUDED_ */
