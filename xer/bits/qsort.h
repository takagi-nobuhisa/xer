/**
 * @file xer/bits/qsort.h
 * @brief Internal qsort implementations.
 */

#pragma once

#ifndef XER_BITS_QSORT_H_INCLUDED_
#define XER_BITS_QSORT_H_INCLUDED_

#include <concepts>
#include <cstddef>
#include <functional>
#include <ranges>
#include <type_traits>
#include <utility>

#include <xer/assert.h>
#include <xer/bits/common.h>
#include <xer/bits/random_access_range.h>

namespace xer::detail {

/**
 * @brief Comparator concept for qsort and bsearch.
 *
 * @tparam Compare Comparator type.
 * @tparam T Element type.
 */
template<typename Compare, typename T>
concept pointer_compare =
    std::regular_invocable<Compare&, const T*, const T*> &&
    std::same_as<std::invoke_result_t<Compare&, const T*, const T*>, int>;

/**
 * @brief Swaps two indexed elements in a contiguous range.
 *
 * @tparam T Element type.
 * @param base Base pointer.
 * @param lhs Left index.
 * @param rhs Right index.
 */
template<typename T>
constexpr void indexed_swap(T* base, std::size_t lhs, std::size_t rhs) noexcept(
    noexcept(std::swap(base[lhs], base[rhs])))
{
    if (lhs == rhs) {
        return;
    }

    using std::swap;
    swap(base[lhs], base[rhs]);
}

/**
 * @brief Swaps two indexed elements in a random-access range.
 *
 * @tparam Range Range type.
 * @param range Target range.
 * @param lhs Left index.
 * @param rhs Right index.
 */
template<typename Range>
constexpr void indexed_swap(Range& range, std::size_t lhs, std::size_t rhs) noexcept(
    noexcept(std::swap(range[lhs], range[rhs])))
{
    if (lhs == rhs) {
        return;
    }

    using std::swap;
    swap(range[lhs], range[rhs]);
}

/**
 * @brief Restores the heap property for a contiguous range.
 *
 * @tparam T Element type.
 * @tparam Compare Comparator type.
 * @param base Base pointer.
 * @param root Root index relative to the heap start.
 * @param count Heap element count.
 * @param comp Comparator.
 */
template<typename T, typename Compare>
constexpr void sift_down(T* base, std::size_t root, std::size_t count, Compare& comp)
{
    while (true) {
        const std::size_t left = (root * 2) + 1;
        if (left >= count) {
            return;
        }

        const std::size_t right = left + 1;
        std::size_t largest = root;

        if (comp(base + largest, base + left) < 0) {
            largest = left;
        }

        if (right < count && comp(base + largest, base + right) < 0) {
            largest = right;
        }

        if (largest == root) {
            return;
        }

        indexed_swap(base, root, largest);
        root = largest;
    }
}

/**
 * @brief Sorts a contiguous range in place with heapsort.
 *
 * @tparam T Element type.
 * @tparam Compare Comparator type.
 * @param base Base pointer.
 * @param count Element count.
 * @param comp Comparator.
 */
template<typename T, typename Compare>
constexpr void heap_sort(T* base, std::size_t count, Compare& comp)
{
    if (count < 2) {
        return;
    }

    for (std::size_t i = count / 2; i > 0; --i) {
        sift_down(base, i - 1, count, comp);
    }

    for (std::size_t i = count; i > 1; --i) {
        indexed_swap(base, 0, i - 1);
        sift_down(base, 0, i - 1, comp);
    }
}

/**
 * @brief Restores the heap property for a random-access range.
 *
 * @tparam Range Range type.
 * @tparam Compare Comparator type.
 * @param range Target range.
 * @param root Root index relative to the heap start.
 * @param count Heap element count.
 * @param comp Comparator.
 */
template<typename Range, typename Compare>
constexpr void sift_down(Range& range, std::size_t root, std::size_t count, Compare& comp)
{
    while (true) {
        const std::size_t left = (root * 2) + 1;
        if (left >= count) {
            return;
        }

        const std::size_t right = left + 1;
        std::size_t largest = root;

        if (comp(&range[largest], &range[left]) < 0) {
            largest = left;
        }

        if (right < count && comp(&range[largest], &range[right]) < 0) {
            largest = right;
        }

        if (largest == root) {
            return;
        }

        indexed_swap(range, root, largest);
        root = largest;
    }
}

/**
 * @brief Sorts a random-access range in place with heapsort.
 *
 * @tparam Range Range type.
 * @tparam Compare Comparator type.
 * @param range Target range.
 * @param comp Comparator.
 */
template<typename Range, typename Compare>
constexpr void heap_sort(Range& range, Compare& comp)
{
    const std::size_t count = static_cast<std::size_t>(std::ranges::size(range));
    if (count < 2) {
        return;
    }

    for (std::size_t i = count / 2; i > 0; --i) {
        sift_down(range, i - 1, count, comp);
    }

    for (std::size_t i = count; i > 1; --i) {
        indexed_swap(range, 0, i - 1);
        sift_down(range, 0, i - 1, comp);
    }
}

} // namespace xer::detail

namespace xer {

/**
 * @brief Sorts a contiguous sequence in place.
 *
 * @tparam T Element type.
 * @tparam Compare Comparator type.
 * @param base Base pointer.
 * @param count Element count.
 * @param comp Comparator.
 */
template<typename T, typename Compare>
    requires(std::swappable<T> && detail::pointer_compare<Compare, T>)
constexpr void qsort(T* base, std::size_t count, Compare comp)
{
    if (count == 0) {
        return;
    }

    xer_assert(base != nullptr);
    detail::heap_sort(base, count, comp);
}

/**
 * @brief Sorts a built-in array in place.
 *
 * @tparam T Element type.
 * @tparam N Array extent.
 * @tparam Compare Comparator type.
 * @param base Target array.
 * @param comp Comparator.
 */
template<typename T, std::size_t N, typename Compare>
    requires(std::swappable<T> && detail::pointer_compare<Compare, T>)
constexpr void qsort(T (&base)[N], Compare comp)
{
    qsort(base, N, comp);
}

/**
 * @brief Sorts a mutable random-access range in place.
 *
 * @tparam Range Range type.
 * @tparam Compare Comparator type.
 * @param base Target range.
 * @param comp Comparator.
 */
template<typename Range, typename Compare>
    requires(
        detail::mutable_random_access_searchable_range<Range> &&
        std::swappable<std::ranges::range_value_t<Range>> &&
        detail::pointer_compare<Compare, std::ranges::range_value_t<Range>>)
constexpr void qsort(Range& base, Compare comp)
{
    detail::heap_sort(base, comp);
}

} // namespace xer

#endif /* XER_BITS_QSORT_H_INCLUDED_ */
