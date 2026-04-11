/**
 * @file xer/bits/contiguous_range.h
 * @brief Internal contiguous-range concepts.
 */

#pragma once

#ifndef XER_BITS_CONTIGUOUS_RANGE_H_INCLUDED_
#define XER_BITS_CONTIGUOUS_RANGE_H_INCLUDED_

#include <concepts>
#include <ranges>
#include <type_traits>

#include <xer/bits/common.h>

namespace xer::detail {

/**
 * @brief Concept for a sized contiguous range whose element type matches the
 *        specified value type.
 *
 * @tparam Range Range type.
 * @tparam Value Element value type.
 */
template<typename Range, typename Value>
concept contiguous_range_of =
    std::ranges::contiguous_range<Range> &&
    std::ranges::sized_range<Range> &&
    std::same_as<std::remove_cv_t<std::ranges::range_value_t<Range>>, Value>;

/**
 * @brief Concept for a mutable sized contiguous range whose element type
 *        matches the specified value type.
 *
 * @tparam Range Range type.
 * @tparam Value Element value type.
 */
template<typename Range, typename Value>
concept mutable_contiguous_range_of =
    contiguous_range_of<Range, Value> &&
    std::same_as<std::remove_cvref_t<std::ranges::range_reference_t<Range>>, Value> &&
    !std::is_const_v<std::remove_reference_t<std::ranges::range_reference_t<Range>>>;

/**
 * @brief Concept for a sized contiguous range of bytes.
 *
 * @tparam Range Range type.
 */
template<typename Range>
concept byte_contiguous_range = contiguous_range_of<Range, std::byte>;

/**
 * @brief Concept for a mutable sized contiguous range of bytes.
 *
 * @tparam Range Range type.
 */
template<typename Range>
concept mutable_byte_contiguous_range =
    mutable_contiguous_range_of<Range, std::byte>;

/**
 * @brief Concept for a sized contiguous range of a specific character type.
 *
 * @tparam Range Range type.
 * @tparam CharT Character type.
 */
template<typename Range, typename CharT>
concept character_contiguous_range =
    contiguous_range_of<Range, CharT> &&
    (std::same_as<CharT, char> || std::same_as<CharT, unsigned char> ||
     std::same_as<CharT, char8_t> || std::same_as<CharT, char16_t> ||
     std::same_as<CharT, char32_t>);

/**
 * @brief Concept for a mutable sized contiguous range of a specific character
 *        type.
 *
 * @tparam Range Range type.
 * @tparam CharT Character type.
 */
template<typename Range, typename CharT>
concept mutable_character_contiguous_range =
    character_contiguous_range<Range, CharT> &&
    mutable_contiguous_range_of<Range, CharT>;

} // namespace xer::detail

#endif /* XER_BITS_CONTIGUOUS_RANGE_H_INCLUDED_ */
