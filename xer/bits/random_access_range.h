/**
 * @file xer/bits/random_access_range.h
 * @brief Internal random-access range concepts.
 */

#pragma once

#ifndef XER_BITS_RANDOM_ACCESS_RANGE_H_INCLUDED_
#define XER_BITS_RANDOM_ACCESS_RANGE_H_INCLUDED_

#include <concepts>
#include <cstddef>
#include <iterator>
#include <ranges>
#include <type_traits>

#include <xer/bits/common.h>

namespace xer::detail {

/**
 * @brief Concept for a sized random-access range whose elements can be
 *        accessed by index and whose iterator can be advanced by offset.
 *
 * @tparam Range Range type.
 */
template<typename Range>
concept random_access_searchable_range =
    std::ranges::random_access_range<Range> &&
    std::ranges::sized_range<Range> &&
    requires(Range range, const Range const_range, std::size_t index) {
        typename std::ranges::range_value_t<Range>;
        typename std::ranges::iterator_t<Range>;
        typename std::ranges::range_difference_t<Range>;
        typename std::ranges::range_reference_t<Range>;
        typename std::ranges::range_reference_t<const Range>;

        {
            const_range[index]
        } -> std::same_as<std::ranges::range_reference_t<const Range>>;

        requires std::same_as<
            std::remove_cvref_t<std::ranges::range_reference_t<const Range>>,
            std::ranges::range_value_t<Range>>;

        {
            range.begin() +
            static_cast<std::ranges::range_difference_t<Range>>(index)
        } -> std::same_as<std::ranges::iterator_t<Range>>;
    };

/**
 * @brief Concept for a mutable sized random-access range whose elements can be
 *        accessed by index.
 *
 * @tparam Range Range type.
 */
template<typename Range>
concept mutable_random_access_searchable_range =
    random_access_searchable_range<Range> &&
    std::ranges::range<Range> &&
    !std::is_const_v<std::remove_reference_t<Range>> &&
    requires(Range range, std::size_t index) {
        { range[index] } -> std::same_as<std::ranges::range_reference_t<Range>>;

        requires std::same_as<
            std::remove_cvref_t<std::ranges::range_reference_t<Range>>,
            std::ranges::range_value_t<Range>>;
    };

} // namespace xer::detail

#endif /* XER_BITS_RANDOM_ACCESS_RANGE_H_INCLUDED_ */
