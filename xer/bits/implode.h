/**
 * @file xer/bits/implode.h
 * @brief Internal implode function implementations.
 */

#pragma once

#ifndef XER_BITS_IMPLODE_H_INCLUDED_
#define XER_BITS_IMPLODE_H_INCLUDED_

#include <concepts>
#include <expected>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>

#include <xer/bits/common.h>
#include <xer/error.h>

namespace xer::detail {

/**
 * @brief Checks whether a value is convertible to UTF-8 string view.
 *
 * @tparam T Target type.
 */
template<typename T>
concept u8string_view_compatible =
    std::constructible_from<std::u8string_view, T>;

/**
 * @brief Concatenates UTF-8 string pieces with a separator.
 *
 * @tparam Range Input range type.
 * @param separator Separator string.
 * @param pieces Input pieces.
 * @return Concatenated string on success.
 */
template<std::ranges::input_range Range>
    requires u8string_view_compatible<std::ranges::range_reference_t<Range>>
[[nodiscard]] inline std::expected<std::u8string, error<void>> implode_impl(
    const std::u8string_view separator,
    Range&& pieces)
{
    std::u8string result;

    auto first = std::ranges::begin(pieces);
    const auto last = std::ranges::end(pieces);

    if (first == last) {
        return result;
    }

    {
        const std::u8string_view first_piece = *first;
        result.append(first_piece.data(), first_piece.size());
        ++first;
    }

    for (; first != last; ++first) {
        const std::u8string_view piece = *first;
        result.append(separator.data(), separator.size());
        result.append(piece.data(), piece.size());
    }

    return result;
}

} // namespace xer::detail

namespace xer {

/**
 * @brief Joins UTF-8 string pieces with a separator.
 *
 * This function follows PHP implode() semantics for the normal case, while
 * adapting the API to XER's UTF-8 and std::expected based design.
 *
 * @tparam Range Input range type whose elements are convertible to
 * std::u8string_view.
 * @param separator Separator string inserted between pieces.
 * @param pieces Input pieces.
 * @return Concatenated string on success.
 */
template<std::ranges::input_range Range>
    requires detail::u8string_view_compatible<
        std::ranges::range_reference_t<Range>>
[[nodiscard]] inline std::expected<std::u8string, error<void>> implode(
    const std::u8string_view separator,
    Range&& pieces)
{
    return detail::implode_impl(separator, std::forward<Range>(pieces));
}

} // namespace xer

#endif /* XER_BITS_IMPLODE_H_INCLUDED_ */
