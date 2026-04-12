/**
 * @file xer/bits/string_write.h
 * @brief Internal string write function implementations.
 */

#pragma once

#ifndef XER_BITS_STRING_WRITE_H_INCLUDED_
#define XER_BITS_STRING_WRITE_H_INCLUDED_

#include <cstddef>
#include <expected>
#include <ranges>
#include <string_view>
#include <type_traits>

#include <xer/bits/common.h>
#include <xer/bits/contiguous_range.h>
#include <xer/bits/string_character.h>
#include <xer/error.h>

namespace xer::detail {


/**
 * @brief Checks whether a pointer/size pair is valid.
 *
 * @param ptr Pointer to check.
 * @param count Number of elements to access.
 * @return true if the pair is valid.
 * @return false otherwise.
 */
[[nodiscard]] constexpr auto is_valid_string_argument(
    const void* ptr,
    const std::size_t count) noexcept -> bool
{
    return ptr != nullptr || count == 0;
}

/**
 * @brief Searches for the first terminating NUL character in the destination
 *        buffer.
 *
 * @tparam CharT Character type.
 * @param destination Destination buffer.
 * @param destination_size Destination buffer size.
 * @return Index of the terminating NUL character on success.
 */
template<supported_string_character CharT>
[[nodiscard]] constexpr auto find_terminator(
    const CharT* destination,
    const std::size_t destination_size) noexcept -> result<std::size_t>
{
    if (!is_valid_string_argument(destination, destination_size)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    for (std::size_t index = 0; index < destination_size; ++index) {
        if (destination[index] == static_cast<CharT>(0)) {
            return index;
        }
    }

    return std::unexpected(make_error(error_t::invalid_argument));
}

} // namespace xer::detail

namespace xer {

/**
 * @brief Copies the source string to the destination buffer including the
 *        terminating NUL character.
 *
 * @tparam CharT Character type.
 * @param destination Destination buffer.
 * @param destination_size Destination buffer size.
 * @param source Source string.
 * @return Destination pointer on success.
 */
template<detail::supported_string_character CharT>
[[nodiscard]] constexpr auto strcpy(
    CharT* destination,
    const std::size_t destination_size,
    const std::basic_string_view<CharT> source) noexcept -> result<CharT*>
{
    if (!detail::is_valid_string_argument(destination, destination_size)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    const std::size_t required_size = source.size() + 1;

    if (destination_size < required_size) {
        return std::unexpected(make_error(error_t::length_error));
    }

    for (std::size_t index = 0; index < source.size(); ++index) {
        destination[index] = source[index];
    }

    destination[source.size()] = static_cast<CharT>(0);

    return destination;
}

/**
 * @brief Copies the source string to the destination range including the
 *        terminating NUL character.
 *
 * @tparam Destination Destination range type.
 * @tparam CharT Character type.
 * @param destination Destination range.
 * @param source Source string.
 * @return Iterator to the beginning of the destination range on success.
 */
template<typename Destination, typename CharT>
    requires detail::mutable_character_contiguous_range<Destination, CharT>
[[nodiscard]] constexpr auto strcpy(
    Destination& destination,
    const std::basic_string_view<CharT> source) noexcept -> result<std::ranges::iterator_t<Destination>>
{
    const auto result = xer::strcpy(
        std::ranges::data(destination),
        static_cast<std::size_t>(std::ranges::size(destination)),
        source);

    if (!result.has_value()) {
        return std::unexpected(result.error());
    }

    return std::ranges::begin(destination);
}

/**
 * @brief Copies at most @p count characters from the source string to the
 *        destination buffer.
 *
 * If the source string is shorter than @p count, the remaining elements are
 * filled with NUL characters. This function does not guarantee appending a
 * terminating NUL character when the source string length is greater than or
 * equal to @p count.
 *
 * @tparam CharT Character type.
 * @param destination Destination buffer.
 * @param destination_size Destination buffer size.
 * @param source Source string.
 * @param count Maximum number of characters to write.
 * @return Destination pointer on success.
 */
template<detail::supported_string_character CharT>
[[nodiscard]] constexpr auto strncpy(
    CharT* destination,
    const std::size_t destination_size,
    const std::basic_string_view<CharT> source,
    const std::size_t count) noexcept -> result<CharT*>
{
    if (!detail::is_valid_string_argument(destination, destination_size)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (destination_size < count) {
        return std::unexpected(make_error(error_t::length_error));
    }

    const std::size_t copy_count = source.size() < count ? source.size() : count;

    for (std::size_t index = 0; index < copy_count; ++index) {
        destination[index] = source[index];
    }

    for (std::size_t index = copy_count; index < count; ++index) {
        destination[index] = static_cast<CharT>(0);
    }

    return destination;
}

/**
 * @brief Copies at most @p count characters from the source string to the
 *        destination range.
 *
 * @tparam Destination Destination range type.
 * @tparam CharT Character type.
 * @param destination Destination range.
 * @param source Source string.
 * @param count Maximum number of characters to write.
 * @return Iterator to the beginning of the destination range on success.
 */
template<typename Destination, typename CharT>
    requires detail::mutable_character_contiguous_range<Destination, CharT>
[[nodiscard]] constexpr auto strncpy(
    Destination& destination,
    const std::basic_string_view<CharT> source,
    const std::size_t count) noexcept -> result<std::ranges::iterator_t<Destination>>
{
    const auto result = xer::strncpy(
        std::ranges::data(destination),
        static_cast<std::size_t>(std::ranges::size(destination)),
        source,
        count);

    if (!result.has_value()) {
        return std::unexpected(result.error());
    }

    return std::ranges::begin(destination);
}

/**
 * @brief Appends the source string to the destination buffer and writes a
 *        terminating NUL character.
 *
 * The destination buffer must already contain a terminating NUL character
 * within the specified buffer size.
 *
 * @tparam CharT Character type.
 * @param destination Destination buffer.
 * @param destination_size Destination buffer size.
 * @param source Source string.
 * @return Destination pointer on success.
 */
template<detail::supported_string_character CharT>
[[nodiscard]] constexpr auto strcat(
    CharT* destination,
    const std::size_t destination_size,
    const std::basic_string_view<CharT> source) noexcept -> result<CharT*>
{
    const auto terminator_result =
        detail::find_terminator(destination, destination_size);

    if (!terminator_result.has_value()) {
        return std::unexpected(terminator_result.error());
    }

    const std::size_t destination_length = terminator_result.value();
    const std::size_t required_size = destination_length + source.size() + 1;

    if (destination_size < required_size) {
        return std::unexpected(make_error(error_t::length_error));
    }

    for (std::size_t index = 0; index < source.size(); ++index) {
        destination[destination_length + index] = source[index];
    }

    destination[destination_length + source.size()] = static_cast<CharT>(0);

    return destination;
}

/**
 * @brief Appends the source string to the destination range and writes a
 *        terminating NUL character.
 *
 * @tparam Destination Destination range type.
 * @tparam CharT Character type.
 * @param destination Destination range.
 * @param source Source string.
 * @return Iterator to the beginning of the destination range on success.
 */
template<typename Destination, typename CharT>
    requires detail::mutable_character_contiguous_range<Destination, CharT>
[[nodiscard]] constexpr auto strcat(
    Destination& destination,
    const std::basic_string_view<CharT> source) noexcept -> result<std::ranges::iterator_t<Destination>>
{
    const auto result = xer::strcat(
        std::ranges::data(destination),
        static_cast<std::size_t>(std::ranges::size(destination)),
        source);

    if (!result.has_value()) {
        return std::unexpected(result.error());
    }

    return std::ranges::begin(destination);
}

/**
 * @brief Appends at most @p count characters from the source string to the
 *        destination buffer and writes a terminating NUL character.
 *
 * The destination buffer must already contain a terminating NUL character
 * within the specified buffer size.
 *
 * @tparam CharT Character type.
 * @param destination Destination buffer.
 * @param destination_size Destination buffer size.
 * @param source Source string.
 * @param count Maximum number of characters to append.
 * @return Destination pointer on success.
 */
template<detail::supported_string_character CharT>
[[nodiscard]] constexpr auto strncat(
    CharT* destination,
    const std::size_t destination_size,
    const std::basic_string_view<CharT> source,
    const std::size_t count) noexcept -> result<CharT*>
{
    const auto terminator_result =
        detail::find_terminator(destination, destination_size);

    if (!terminator_result.has_value()) {
        return std::unexpected(terminator_result.error());
    }

    const std::size_t destination_length = terminator_result.value();
    const std::size_t append_count = source.size() < count ? source.size() : count;
    const std::size_t required_size = destination_length + append_count + 1;

    if (destination_size < required_size) {
        return std::unexpected(make_error(error_t::length_error));
    }

    for (std::size_t index = 0; index < append_count; ++index) {
        destination[destination_length + index] = source[index];
    }

    destination[destination_length + append_count] = static_cast<CharT>(0);

    return destination;
}

/**
 * @brief Appends at most @p count characters from the source string to the
 *        destination range and writes a terminating NUL character.
 *
 * @tparam Destination Destination range type.
 * @tparam CharT Character type.
 * @param destination Destination range.
 * @param source Source string.
 * @param count Maximum number of characters to append.
 * @return Iterator to the beginning of the destination range on success.
 */
template<typename Destination, typename CharT>
    requires detail::mutable_character_contiguous_range<Destination, CharT>
[[nodiscard]] constexpr auto strncat(
    Destination& destination,
    const std::basic_string_view<CharT> source,
    const std::size_t count) noexcept -> result<std::ranges::iterator_t<Destination>>
{
    const auto result = xer::strncat(
        std::ranges::data(destination),
        static_cast<std::size_t>(std::ranges::size(destination)),
        source,
        count);

    if (!result.has_value()) {
        return std::unexpected(result.error());
    }

    return std::ranges::begin(destination);
}

} // namespace xer

#endif /* XER_BITS_STRING_WRITE_H_INCLUDED_ */
