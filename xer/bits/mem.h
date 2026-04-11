/**
 * @file xer/bits/mem.h
 * @brief Internal memory function implementations.
 */

#pragma once

#ifndef XER_BITS_MEM_H_INCLUDED_
#define XER_BITS_MEM_H_INCLUDED_

#include <cstddef>
#include <expected>
#include <ranges>
#include <type_traits>

#include <xer/bits/common.h>
#include <xer/bits/contiguous_range.h>
#include <xer/error.h>

namespace xer::detail {

/**
 * @brief Checks whether the specified pointer/count pair is valid.
 *
 * @param ptr Pointer to check.
 * @param count Number of elements to access.
 * @return true if the pair is valid.
 * @return false otherwise.
 */
[[nodiscard]] constexpr bool is_valid_memory_argument(
    const void* ptr,
    const std::size_t count) noexcept
{
    return ptr != nullptr || count == 0;
}

/**
 * @brief Checks whether two byte ranges overlap.
 *
 * @param lhs_begin Beginning of the first range.
 * @param lhs_size Size of the first range.
 * @param rhs_begin Beginning of the second range.
 * @param rhs_size Size of the second range.
 * @return true if the ranges overlap.
 * @return false otherwise.
 */
[[nodiscard]] constexpr bool byte_ranges_overlap(
    const std::byte* lhs_begin,
    const std::size_t lhs_size,
    const std::byte* rhs_begin,
    const std::size_t rhs_size) noexcept
{
    if (lhs_size == 0 || rhs_size == 0) {
        return false;
    }

    const std::byte* lhs_end = lhs_begin + lhs_size;
    const std::byte* rhs_end = rhs_begin + rhs_size;

    return lhs_begin < rhs_end && rhs_begin < lhs_end;
}

/**
 * @brief Compares two bytes as unsigned values.
 *
 * @param lhs Left-hand byte.
 * @param rhs Right-hand byte.
 * @return Negative value if lhs < rhs.
 * @return Positive value if lhs > rhs.
 * @return Zero if lhs == rhs.
 */
[[nodiscard]] constexpr int compare_byte(
    const std::byte lhs,
    const std::byte rhs) noexcept
{
    const auto lhs_value = std::to_integer<unsigned int>(lhs);
    const auto rhs_value = std::to_integer<unsigned int>(rhs);

    if (lhs_value < rhs_value) {
        return -1;
    }

    if (lhs_value > rhs_value) {
        return 1;
    }

    return 0;
}

} // namespace xer::detail

namespace xer {

/**
 * @brief Copies bytes from source to destination.
 *
 * The destination buffer must be at least as large as the source buffer.
 * Overlapping ranges are rejected; use memmove for overlapping copies.
 *
 * @param destination Destination buffer.
 * @param destination_size Size of the destination buffer.
 * @param source Source buffer.
 * @param source_size Size of the source buffer.
 * @return Destination pointer on success.
 */
[[nodiscard]] constexpr std::expected<std::byte*, error<void>> memcpy(
    std::byte* destination,
    const std::size_t destination_size,
    const std::byte* source,
    const std::size_t source_size) noexcept
{
    if (!detail::is_valid_memory_argument(destination, destination_size) ||
        !detail::is_valid_memory_argument(source, source_size)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (destination_size < source_size) {
        return std::unexpected(make_error(error_t::length_error));
    }

    if (detail::byte_ranges_overlap(destination, source_size, source, source_size)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    for (std::size_t index = 0; index < source_size; ++index) {
        destination[index] = source[index];
    }

    return destination;
}

/**
 * @brief Copies bytes from source to destination.
 *
 * @tparam Destination Destination range type.
 * @tparam Source Source range type.
 * @param destination Destination range.
 * @param source Source range.
 * @return Iterator to the beginning of the destination range on success.
 */
template<typename Destination, typename Source>
    requires detail::mutable_byte_contiguous_range<Destination> &&
             detail::byte_contiguous_range<Source>
[[nodiscard]] constexpr std::expected<std::ranges::iterator_t<Destination>, error<void>>
memcpy(Destination& destination, const Source& source) noexcept
{
    const auto result = xer::memcpy(
        std::ranges::data(destination),
        static_cast<std::size_t>(std::ranges::size(destination)),
        std::ranges::data(source),
        static_cast<std::size_t>(std::ranges::size(source)));

    if (!result.has_value()) {
        return std::unexpected(result.error());
    }

    return std::ranges::begin(destination);
}

/**
 * @brief Moves bytes from source to destination.
 *
 * The destination buffer must be at least as large as the source buffer.
 * Overlapping ranges are allowed.
 *
 * @param destination Destination buffer.
 * @param destination_size Size of the destination buffer.
 * @param source Source buffer.
 * @param source_size Size of the source buffer.
 * @return Destination pointer on success.
 */
[[nodiscard]] constexpr std::expected<std::byte*, error<void>> memmove(
    std::byte* destination,
    const std::size_t destination_size,
    const std::byte* source,
    const std::size_t source_size) noexcept
{
    if (!detail::is_valid_memory_argument(destination, destination_size) ||
        !detail::is_valid_memory_argument(source, source_size)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (destination_size < source_size) {
        return std::unexpected(make_error(error_t::length_error));
    }

    if (source_size == 0 || destination == source) {
        return destination;
    }

    if (destination < source || destination >= source + source_size) {
        for (std::size_t index = 0; index < source_size; ++index) {
            destination[index] = source[index];
        }
    } else {
        for (std::size_t index = source_size; index > 0; --index) {
            destination[index - 1] = source[index - 1];
        }
    }

    return destination;
}

/**
 * @brief Moves bytes from source to destination.
 *
 * @tparam Destination Destination range type.
 * @tparam Source Source range type.
 * @param destination Destination range.
 * @param source Source range.
 * @return Iterator to the beginning of the destination range on success.
 */
template<typename Destination, typename Source>
    requires detail::mutable_byte_contiguous_range<Destination> &&
             detail::byte_contiguous_range<Source>
[[nodiscard]] constexpr std::expected<std::ranges::iterator_t<Destination>, error<void>>
memmove(Destination& destination, const Source& source) noexcept
{
    const auto result = xer::memmove(
        std::ranges::data(destination),
        static_cast<std::size_t>(std::ranges::size(destination)),
        std::ranges::data(source),
        static_cast<std::size_t>(std::ranges::size(source)));

    if (!result.has_value()) {
        return std::unexpected(result.error());
    }

    return std::ranges::begin(destination);
}

/**
 * @brief Searches for the specified byte in a mutable buffer.
 *
 * @param source Source buffer.
 * @param source_size Size of the source buffer.
 * @param value Byte value to search for.
 * @return Pointer to the matched byte on success.
 */
[[nodiscard]] constexpr std::expected<std::byte*, error<void>> memchr(
    std::byte* source,
    const std::size_t source_size,
    const std::byte value) noexcept
{
    if (!detail::is_valid_memory_argument(source, source_size)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    for (std::size_t index = 0; index < source_size; ++index) {
        if (source[index] == value) {
            return source + index;
        }
    }

    return std::unexpected(make_error(error_t::not_found));
}

/**
 * @brief Searches for the specified byte in an immutable buffer.
 *
 * @param source Source buffer.
 * @param source_size Size of the source buffer.
 * @param value Byte value to search for.
 * @return Pointer to the matched byte on success.
 */
[[nodiscard]] constexpr std::expected<const std::byte*, error<void>> memchr(
    const std::byte* source,
    const std::size_t source_size,
    const std::byte value) noexcept
{
    if (!detail::is_valid_memory_argument(source, source_size)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    for (std::size_t index = 0; index < source_size; ++index) {
        if (source[index] == value) {
            return source + index;
        }
    }

    return std::unexpected(make_error(error_t::not_found));
}

/**
 * @brief Searches for the specified byte in a mutable contiguous range.
 *
 * @tparam Range Range type.
 * @param source Source range.
 * @param value Byte value to search for.
 * @return Iterator to the matched byte on success.
 */
template<typename Range>
    requires detail::mutable_byte_contiguous_range<Range>
[[nodiscard]] constexpr std::expected<std::ranges::iterator_t<Range>, error<void>>
memchr(Range& source, const std::byte value) noexcept
{
    const auto result = xer::memchr(
        std::ranges::data(source),
        static_cast<std::size_t>(std::ranges::size(source)),
        value);

    if (!result.has_value()) {
        return std::unexpected(result.error());
    }

    return std::ranges::begin(source) +
           static_cast<std::ranges::range_difference_t<Range>>(
               result.value() - std::ranges::data(source));
}

/**
 * @brief Searches for the specified byte in an immutable contiguous range.
 *
 * @tparam Range Range type.
 * @param source Source range.
 * @param value Byte value to search for.
 * @return Iterator to the matched byte on success.
 */
template<typename Range>
    requires detail::byte_contiguous_range<Range>
[[nodiscard]] constexpr std::expected<std::ranges::iterator_t<const Range>, error<void>>
memchr(const Range& source, const std::byte value) noexcept
{
    const auto result = xer::memchr(
        std::ranges::data(source),
        static_cast<std::size_t>(std::ranges::size(source)),
        value);

    if (!result.has_value()) {
        return std::unexpected(result.error());
    }

    return std::ranges::begin(source) +
           static_cast<std::ranges::range_difference_t<std::ranges::ref_view<const Range>>>(
               result.value() - std::ranges::data(source));
}

/**
 * @brief Compares two byte buffers lexicographically.
 *
 * If the common prefix is equal, the shorter buffer is treated as smaller.
 *
 * @param lhs Left-hand buffer.
 * @param lhs_size Size of the left-hand buffer.
 * @param rhs Right-hand buffer.
 * @param rhs_size Size of the right-hand buffer.
 * @return Negative value if lhs < rhs.
 * @return Positive value if lhs > rhs.
 * @return Zero if lhs == rhs.
 */
[[nodiscard]] constexpr std::expected<int, error<void>> memcmp(
    const std::byte* lhs,
    const std::size_t lhs_size,
    const std::byte* rhs,
    const std::size_t rhs_size) noexcept
{
    if (!detail::is_valid_memory_argument(lhs, lhs_size) ||
        !detail::is_valid_memory_argument(rhs, rhs_size)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    const std::size_t minimum_size = lhs_size < rhs_size ? lhs_size : rhs_size;

    for (std::size_t index = 0; index < minimum_size; ++index) {
        const int compare_result = detail::compare_byte(lhs[index], rhs[index]);

        if (compare_result != 0) {
            return compare_result;
        }
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
 * @brief Compares two contiguous byte ranges lexicographically.
 *
 * @tparam Left Left-hand range type.
 * @tparam Right Right-hand range type.
 * @param lhs Left-hand range.
 * @param rhs Right-hand range.
 * @return Comparison result.
 */
template<typename Left, typename Right>
    requires detail::byte_contiguous_range<Left> &&
             detail::byte_contiguous_range<Right>
[[nodiscard]] constexpr std::expected<int, error<void>> memcmp(
    const Left& lhs,
    const Right& rhs) noexcept
{
    return xer::memcmp(
        std::ranges::data(lhs),
        static_cast<std::size_t>(std::ranges::size(lhs)),
        std::ranges::data(rhs),
        static_cast<std::size_t>(std::ranges::size(rhs)));
}

/**
 * @brief Fills a mutable buffer with the specified byte value.
 *
 * @param destination Destination buffer.
 * @param destination_size Size of the destination buffer.
 * @param value Byte value to write.
 * @return Destination pointer on success.
 */
[[nodiscard]] constexpr std::expected<std::byte*, error<void>> memset(
    std::byte* destination,
    const std::size_t destination_size,
    const std::byte value) noexcept
{
    if (!detail::is_valid_memory_argument(destination, destination_size)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    for (std::size_t index = 0; index < destination_size; ++index) {
        destination[index] = value;
    }

    return destination;
}

/**
 * @brief Fills a mutable contiguous range with the specified byte value.
 *
 * @tparam Destination Destination range type.
 * @param destination Destination range.
 * @param value Byte value to write.
 * @return Iterator to the beginning of the destination range on success.
 */
template<typename Destination>
    requires detail::mutable_byte_contiguous_range<Destination>
[[nodiscard]] constexpr std::expected<std::ranges::iterator_t<Destination>, error<void>>
memset(Destination& destination, const std::byte value) noexcept
{
    const auto result = xer::memset(
        std::ranges::data(destination),
        static_cast<std::size_t>(std::ranges::size(destination)),
        value);

    if (!result.has_value()) {
        return std::unexpected(result.error());
    }

    return std::ranges::begin(destination);
}

} // namespace xer

#endif /* XER_BITS_MEM_H_INCLUDED_ */
