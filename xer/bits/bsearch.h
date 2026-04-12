/**
 * @file xer/bits/bsearch.h
 * @brief Internal bsearch implementations.
 */

#pragma once

#ifndef XER_BITS_BSEARCH_H_INCLUDED_
#define XER_BITS_BSEARCH_H_INCLUDED_

#include <concepts>
#include <cstddef>
#include <expected>
#include <ranges>
#include <type_traits>

#include <xer/bits/common.h>
#include <xer/bits/error.h>
#include <xer/bits/random_access_range.h>

namespace xer::detail {

/**
 * @brief Comparator concept for bsearch.
 *
 * @tparam Compare Comparator type.
 * @tparam T Element type.
 */
template<typename Compare, typename T>
concept bsearch_pointer_compare =
    std::regular_invocable<Compare&, const T*, const T*> &&
    std::same_as<std::invoke_result_t<Compare&, const T*, const T*>, int>;

/**
 * @brief Searches a sorted contiguous sequence and returns the found pointer.
 *
 * @tparam T Element type.
 * @tparam Compare Comparator type.
 * @param key Pointer to the search key.
 * @param base Base pointer.
 * @param count Element count.
 * @param comp Comparator.
 * @return Found pointer on success, or an error on failure.
 */
template<typename T, typename Compare>
[[nodiscard]] constexpr result<T*> bsearch_pointer_impl(
    const T* key,
    T* base,
    std::size_t count,
    Compare& comp)
{
    if (count == 0) {
        return std::unexpected(make_error(error_t::not_found));
    }

    if (key == nullptr || base == nullptr) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    std::size_t first = 0;
    std::size_t last = count;

    while (first < last) {
        const std::size_t middle = first + ((last - first) / 2);
        const int result = comp(key, base + middle);

        if (result < 0) {
            last = middle;
        } else if (result > 0) {
            first = middle + 1;
        } else {
            return base + middle;
        }
    }

    return std::unexpected(make_error(error_t::not_found));
}

/**
 * @brief Searches a sorted const contiguous sequence and returns the found pointer.
 *
 * @tparam T Element type.
 * @tparam Compare Comparator type.
 * @param key Pointer to the search key.
 * @param base Base pointer.
 * @param count Element count.
 * @param comp Comparator.
 * @return Found pointer on success, or an error on failure.
 */
template<typename T, typename Compare>
[[nodiscard]] constexpr result<const T*> bsearch_pointer_impl(
    const T* key,
    const T* base,
    std::size_t count,
    Compare& comp)
{
    if (count == 0) {
        return std::unexpected(make_error(error_t::not_found));
    }

    if (key == nullptr || base == nullptr) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    std::size_t first = 0;
    std::size_t last = count;

    while (first < last) {
        const std::size_t middle = first + ((last - first) / 2);
        const int result = comp(key, base + middle);

        if (result < 0) {
            last = middle;
        } else if (result > 0) {
            first = middle + 1;
        } else {
            return base + middle;
        }
    }

    return std::unexpected(make_error(error_t::not_found));
}

/**
 * @brief Searches a sorted mutable random-access range and returns the found iterator.
 *
 * @tparam Range Range type.
 * @tparam Compare Comparator type.
 * @param key Pointer to the search key.
 * @param base Target range.
 * @param comp Comparator.
 * @return Found iterator on success, or an error on failure.
 */
template<typename Range, typename Compare>
[[nodiscard]] constexpr result<std::ranges::iterator_t<Range>>
bsearch_range_impl(
    const std::ranges::range_value_t<Range>* key,
    Range& base,
    Compare& comp)
{
    const std::size_t count = static_cast<std::size_t>(std::ranges::size(base));
    if (count == 0) {
        return std::unexpected(make_error(error_t::not_found));
    }

    if (key == nullptr) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    std::size_t first = 0;
    std::size_t last = count;

    while (first < last) {
        const std::size_t middle = first + ((last - first) / 2);
        const int result = comp(key, &base[middle]);

        if (result < 0) {
            last = middle;
        } else if (result > 0) {
            first = middle + 1;
        } else {
            return std::ranges::begin(base) +
                static_cast<std::ranges::range_difference_t<Range>>(middle);
        }
    }

    return std::unexpected(make_error(error_t::not_found));
}

/**
 * @brief Searches a sorted const random-access range and returns the found iterator.
 *
 * @tparam Range Range type.
 * @tparam Compare Comparator type.
 * @param key Pointer to the search key.
 * @param base Target range.
 * @param comp Comparator.
 * @return Found iterator on success, or an error on failure.
 */
template<typename Range, typename Compare>
[[nodiscard]] constexpr result<std::ranges::iterator_t<const Range>>
bsearch_range_impl(
    const std::ranges::range_value_t<Range>* key,
    const Range& base,
    Compare& comp)
{
    const std::size_t count = static_cast<std::size_t>(std::ranges::size(base));
    if (count == 0) {
        return std::unexpected(make_error(error_t::not_found));
    }

    if (key == nullptr) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    std::size_t first = 0;
    std::size_t last = count;

    while (first < last) {
        const std::size_t middle = first + ((last - first) / 2);
        const int result = comp(key, &base[middle]);

        if (result < 0) {
            last = middle;
        } else if (result > 0) {
            first = middle + 1;
        } else {
            return std::ranges::begin(base) +
                static_cast<std::ranges::range_difference_t<const Range>>(middle);
        }
    }

    return std::unexpected(make_error(error_t::not_found));
}

} // namespace xer::detail

namespace xer {

/**
 * @brief Searches a sorted contiguous sequence.
 *
 * @tparam T Element type.
 * @tparam Compare Comparator type.
 * @param key Pointer to the search key.
 * @param base Base pointer.
 * @param count Element count.
 * @param comp Comparator.
 * @return Found pointer on success, or not_found / invalid_argument on failure.
 */
template<typename T, typename Compare>
    requires(detail::bsearch_pointer_compare<Compare, T>)
[[nodiscard]] constexpr result<T*> bsearch(
    const T* key,
    T* base,
    std::size_t count,
    Compare comp)
{
    return detail::bsearch_pointer_impl(key, base, count, comp);
}

/**
 * @brief Searches a sorted const contiguous sequence.
 *
 * @tparam T Element type.
 * @tparam Compare Comparator type.
 * @param key Pointer to the search key.
 * @param base Base pointer.
 * @param count Element count.
 * @param comp Comparator.
 * @return Found pointer on success, or not_found / invalid_argument on failure.
 */
template<typename T, typename Compare>
    requires(detail::bsearch_pointer_compare<Compare, T>)
[[nodiscard]] constexpr result<const T*> bsearch(
    const T* key,
    const T* base,
    std::size_t count,
    Compare comp)
{
    return detail::bsearch_pointer_impl(key, base, count, comp);
}

/**
 * @brief Searches a sorted built-in array.
 *
 * @tparam T Element type.
 * @tparam N Array extent.
 * @tparam Compare Comparator type.
 * @param key Pointer to the search key.
 * @param base Target array.
 * @param comp Comparator.
 * @return Found pointer on success, or not_found / invalid_argument on failure.
 */
template<typename T, std::size_t N, typename Compare>
    requires(detail::bsearch_pointer_compare<Compare, T>)
[[nodiscard]] constexpr result<T*> bsearch(
    const T* key,
    T (&base)[N],
    Compare comp)
{
    return detail::bsearch_pointer_impl(key, base, N, comp);
}

/**
 * @brief Searches a sorted const built-in array.
 *
 * @tparam T Element type.
 * @tparam N Array extent.
 * @tparam Compare Comparator type.
 * @param key Pointer to the search key.
 * @param base Target array.
 * @param comp Comparator.
 * @return Found pointer on success, or not_found / invalid_argument on failure.
 */
template<typename T, std::size_t N, typename Compare>
    requires(detail::bsearch_pointer_compare<Compare, T>)
[[nodiscard]] constexpr result<const T*> bsearch(
    const T* key,
    const T (&base)[N],
    Compare comp)
{
    return detail::bsearch_pointer_impl(key, base, N, comp);
}

/**
 * @brief Searches a sorted mutable random-access range.
 *
 * @tparam Range Range type.
 * @tparam Compare Comparator type.
 * @param key Pointer to the search key.
 * @param base Target range.
 * @param comp Comparator.
 * @return Found iterator on success, or not_found / invalid_argument on failure.
 */
template<typename Range, typename Compare>
    requires(
        detail::mutable_random_access_searchable_range<Range> &&
        detail::bsearch_pointer_compare<Compare, std::ranges::range_value_t<Range>>)
[[nodiscard]] constexpr result<std::ranges::iterator_t<Range>> bsearch(
    const std::ranges::range_value_t<Range>* key,
    Range& base,
    Compare comp)
{
    return detail::bsearch_range_impl(key, base, comp);
}

/**
 * @brief Searches a sorted const random-access range.
 *
 * @tparam Range Range type.
 * @tparam Compare Comparator type.
 * @param key Pointer to the search key.
 * @param base Target range.
 * @param comp Comparator.
 * @return Found iterator on success, or not_found / invalid_argument on failure.
 */
template<typename Range, typename Compare>
    requires(
        detail::random_access_searchable_range<const Range> &&
        detail::bsearch_pointer_compare<Compare, std::ranges::range_value_t<Range>>)
[[nodiscard]] constexpr result<std::ranges::iterator_t<const Range>> bsearch(
    const std::ranges::range_value_t<Range>* key,
    const Range& base,
    Compare comp)
{
    return detail::bsearch_range_impl(key, base, comp);
}

} // namespace xer

#endif /* XER_BITS_BSEARCH_H_INCLUDED_ */
