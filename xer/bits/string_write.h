/**
 * @file xer/bits/string_write.h
 * @brief Internal string write function implementations.
 */

#pragma once

#ifndef XER_BITS_STRING_WRITE_H_INCLUDED_
#define XER_BITS_STRING_WRITE_H_INCLUDED_

#include <cstddef>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include <xer/bits/common.h>
#include <xer/bits/contiguous_range.h>
#include <xer/bits/string_character.h>
#include <xer/error.h>

namespace xer::detail {

[[nodiscard]] constexpr auto is_valid_string_argument(
    const void* ptr,
    const std::size_t count) noexcept -> bool
{
    return ptr != nullptr || count == 0;
}

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

template<typename T>
struct is_basic_string : std::false_type {
};

template<typename CharT, typename Traits, typename Allocator>
struct is_basic_string<std::basic_string<CharT, Traits, Allocator>> : std::true_type {
};

template<typename T>
inline constexpr bool is_basic_string_v = is_basic_string<std::remove_cvref_t<T>>::value;

template<typename T>
struct is_std_vector : std::false_type {
};

template<typename CharT, typename Allocator>
struct is_std_vector<std::vector<CharT, Allocator>> : std::true_type {
};

template<typename T>
inline constexpr bool is_std_vector_v = is_std_vector<std::remove_cvref_t<T>>::value;

} // namespace xer::detail

namespace xer {

/**
 * @brief Copies the source string to the destination buffer including the
 *        terminating NUL character.
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
 * @brief Copies the source string to the destination raw array including the
 *        terminating NUL character.
 */
template<detail::supported_string_character CharT, std::size_t N>
[[nodiscard]] constexpr auto strcpy(
    CharT (&destination)[N],
    const std::basic_string_view<CharT> source) noexcept -> result<CharT*>
{
    return xer::strcpy(destination, N, source);
}

/**
 * @brief Copies the source string to a fixed-size contiguous range including
 *        the terminating NUL character.
 */
template<typename Destination, typename CharT>
    requires detail::mutable_character_contiguous_range<Destination, CharT> &&
             (!detail::is_std_vector_v<Destination>) &&
             (!detail::is_basic_string_v<Destination>)
[[nodiscard]] constexpr auto strcpy(
    Destination& destination,
    const std::basic_string_view<CharT> source) noexcept
    -> result<std::ranges::iterator_t<Destination>>
{
    const auto result = xer::strcpy(
        std::ranges::data(destination),
        static_cast<std::size_t>(std::ranges::size(destination)),
        source);

    if (!result) {
        return std::unexpected(result.error());
    }

    return std::ranges::begin(destination);
}

/**
 * @brief Copies the source string to the destination vector including the
 *        terminating NUL character.
 */
template<detail::supported_string_character CharT, typename Allocator>
[[nodiscard]] inline auto strcpy(
    std::vector<CharT, Allocator>& destination,
    const std::basic_string_view<CharT> source) noexcept
    -> result<typename std::vector<CharT, Allocator>::iterator>
{
    destination.resize(source.size() + 1);

    for (std::size_t index = 0; index < source.size(); ++index) {
        destination[index] = source[index];
    }
    destination[source.size()] = static_cast<CharT>(0);

    return destination.begin();
}

/**
 * @brief Copies the source string to the destination basic_string.
 *
 * Unlike pointer-based overloads, this overload does not store an explicit
 * trailing NUL character as part of the string contents.
 */
template<detail::supported_string_character CharT, typename Traits, typename Allocator>
[[nodiscard]] inline auto strcpy(
    std::basic_string<CharT, Traits, Allocator>& destination,
    const std::basic_string_view<CharT> source) noexcept
    -> result<typename std::basic_string<CharT, Traits, Allocator>::iterator>
{
    destination.assign(source.begin(), source.end());
    return destination.begin();
}

/**
 * @brief Copies at most @p count characters from the source string to the
 *        destination buffer.
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
 *        destination raw array.
 */
template<detail::supported_string_character CharT, std::size_t N>
[[nodiscard]] constexpr auto strncpy(
    CharT (&destination)[N],
    const std::basic_string_view<CharT> source,
    const std::size_t count) noexcept -> result<CharT*>
{
    return xer::strncpy(destination, N, source, count);
}

/**
 * @brief Copies at most @p count characters to a fixed-size contiguous range.
 */
template<typename Destination, typename CharT>
    requires detail::mutable_character_contiguous_range<Destination, CharT> &&
             (!detail::is_std_vector_v<Destination>) &&
             (!detail::is_basic_string_v<Destination>)
[[nodiscard]] constexpr auto strncpy(
    Destination& destination,
    const std::basic_string_view<CharT> source,
    const std::size_t count) noexcept
    -> result<std::ranges::iterator_t<Destination>>
{
    const auto result = xer::strncpy(
        std::ranges::data(destination),
        static_cast<std::size_t>(std::ranges::size(destination)),
        source,
        count);

    if (!result) {
        return std::unexpected(result.error());
    }

    return std::ranges::begin(destination);
}

/**
 * @brief Copies at most @p count characters to the destination vector.
 */
template<detail::supported_string_character CharT, typename Allocator>
[[nodiscard]] inline auto strncpy(
    std::vector<CharT, Allocator>& destination,
    const std::basic_string_view<CharT> source,
    const std::size_t count) noexcept
    -> result<typename std::vector<CharT, Allocator>::iterator>
{
    destination.resize(count);

    const std::size_t copy_count = source.size() < count ? source.size() : count;
    for (std::size_t index = 0; index < copy_count; ++index) {
        destination[index] = source[index];
    }
    for (std::size_t index = copy_count; index < count; ++index) {
        destination[index] = static_cast<CharT>(0);
    }

    return destination.begin();
}

/**
 * @brief Copies at most @p count characters to the destination basic_string.
 *
 * Embedded NUL characters are kept inside the string size when padding is
 * required, but no extra terminal NUL element is appended beyond size().
 */
template<detail::supported_string_character CharT, typename Traits, typename Allocator>
[[nodiscard]] inline auto strncpy(
    std::basic_string<CharT, Traits, Allocator>& destination,
    const std::basic_string_view<CharT> source,
    const std::size_t count) noexcept
    -> result<typename std::basic_string<CharT, Traits, Allocator>::iterator>
{
    destination.resize(count);

    const std::size_t copy_count = source.size() < count ? source.size() : count;
    for (std::size_t index = 0; index < copy_count; ++index) {
        destination[index] = source[index];
    }
    for (std::size_t index = copy_count; index < count; ++index) {
        destination[index] = static_cast<CharT>(0);
    }

    return destination.begin();
}

/**
 * @brief Appends the source string to the destination buffer and writes a
 *        terminating NUL character.
 */
template<detail::supported_string_character CharT>
[[nodiscard]] constexpr auto strcat(
    CharT* destination,
    const std::size_t destination_size,
    const std::basic_string_view<CharT> source) noexcept -> result<CharT*>
{
    const auto terminator_result = detail::find_terminator(destination, destination_size);
    if (!terminator_result) {
        return std::unexpected(terminator_result.error());
    }

    const std::size_t destination_length = *terminator_result;
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
 * @brief Appends the source string to the destination raw array.
 */
template<detail::supported_string_character CharT, std::size_t N>
[[nodiscard]] constexpr auto strcat(
    CharT (&destination)[N],
    const std::basic_string_view<CharT> source) noexcept -> result<CharT*>
{
    return xer::strcat(destination, N, source);
}

/**
 * @brief Appends the source string to a fixed-size contiguous range.
 */
template<typename Destination, typename CharT>
    requires detail::mutable_character_contiguous_range<Destination, CharT> &&
             (!detail::is_std_vector_v<Destination>) &&
             (!detail::is_basic_string_v<Destination>)
[[nodiscard]] constexpr auto strcat(
    Destination& destination,
    const std::basic_string_view<CharT> source) noexcept
    -> result<std::ranges::iterator_t<Destination>>
{
    const auto result = xer::strcat(
        std::ranges::data(destination),
        static_cast<std::size_t>(std::ranges::size(destination)),
        source);

    if (!result) {
        return std::unexpected(result.error());
    }

    return std::ranges::begin(destination);
}

/**
 * @brief Appends the source string to the destination vector.
 *
 * The vector is automatically expanded and keeps an explicit trailing NUL.
 */
template<detail::supported_string_character CharT, typename Allocator>
[[nodiscard]] inline auto strcat(
    std::vector<CharT, Allocator>& destination,
    const std::basic_string_view<CharT> source) noexcept
    -> result<typename std::vector<CharT, Allocator>::iterator>
{
    std::size_t destination_length = 0;

    if (!destination.empty() && destination.back() == static_cast<CharT>(0)) {
        destination_length = destination.size() - 1;
        destination.resize(destination_length + source.size() + 1);
    } else {
        destination_length = destination.size();
        destination.resize(destination_length + source.size() + 1);
    }

    for (std::size_t index = 0; index < source.size(); ++index) {
        destination[destination_length + index] = source[index];
    }
    destination[destination_length + source.size()] = static_cast<CharT>(0);

    return destination.begin();
}

/**
 * @brief Appends the source string to the destination basic_string.
 *
 * No explicit trailing NUL element is stored as part of the string size.
 */
template<detail::supported_string_character CharT, typename Traits, typename Allocator>
[[nodiscard]] inline auto strcat(
    std::basic_string<CharT, Traits, Allocator>& destination,
    const std::basic_string_view<CharT> source) noexcept
    -> result<typename std::basic_string<CharT, Traits, Allocator>::iterator>
{
    destination.append(source.begin(), source.end());
    return destination.begin();
}

/**
 * @brief Appends at most @p count characters from the source string to the
 *        destination buffer and writes a terminating NUL character.
 */
template<detail::supported_string_character CharT>
[[nodiscard]] constexpr auto strncat(
    CharT* destination,
    const std::size_t destination_size,
    const std::basic_string_view<CharT> source,
    const std::size_t count) noexcept -> result<CharT*>
{
    const auto terminator_result = detail::find_terminator(destination, destination_size);
    if (!terminator_result) {
        return std::unexpected(terminator_result.error());
    }

    const std::size_t destination_length = *terminator_result;
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
 * @brief Appends at most @p count characters to the destination raw array.
 */
template<detail::supported_string_character CharT, std::size_t N>
[[nodiscard]] constexpr auto strncat(
    CharT (&destination)[N],
    const std::basic_string_view<CharT> source,
    const std::size_t count) noexcept -> result<CharT*>
{
    return xer::strncat(destination, N, source, count);
}

/**
 * @brief Appends at most @p count characters to a fixed-size contiguous range.
 */
template<typename Destination, typename CharT>
    requires detail::mutable_character_contiguous_range<Destination, CharT> &&
             (!detail::is_std_vector_v<Destination>) &&
             (!detail::is_basic_string_v<Destination>)
[[nodiscard]] constexpr auto strncat(
    Destination& destination,
    const std::basic_string_view<CharT> source,
    const std::size_t count) noexcept
    -> result<std::ranges::iterator_t<Destination>>
{
    const auto result = xer::strncat(
        std::ranges::data(destination),
        static_cast<std::size_t>(std::ranges::size(destination)),
        source,
        count);

    if (!result) {
        return std::unexpected(result.error());
    }

    return std::ranges::begin(destination);
}

/**
 * @brief Appends at most @p count characters to the destination vector.
 *
 * The vector is automatically expanded and keeps an explicit trailing NUL.
 */
template<detail::supported_string_character CharT, typename Allocator>
[[nodiscard]] inline auto strncat(
    std::vector<CharT, Allocator>& destination,
    const std::basic_string_view<CharT> source,
    const std::size_t count) noexcept
    -> result<typename std::vector<CharT, Allocator>::iterator>
{
    const std::size_t append_count = source.size() < count ? source.size() : count;
    std::size_t destination_length = 0;

    if (!destination.empty() && destination.back() == static_cast<CharT>(0)) {
        destination_length = destination.size() - 1;
        destination.resize(destination_length + append_count + 1);
    } else {
        destination_length = destination.size();
        destination.resize(destination_length + append_count + 1);
    }

    for (std::size_t index = 0; index < append_count; ++index) {
        destination[destination_length + index] = source[index];
    }
    destination[destination_length + append_count] = static_cast<CharT>(0);

    return destination.begin();
}

/**
 * @brief Appends at most @p count characters to the destination basic_string.
 *
 * No explicit trailing NUL element is stored as part of the string size.
 */
template<detail::supported_string_character CharT, typename Traits, typename Allocator>
[[nodiscard]] inline auto strncat(
    std::basic_string<CharT, Traits, Allocator>& destination,
    const std::basic_string_view<CharT> source,
    const std::size_t count) noexcept
    -> result<typename std::basic_string<CharT, Traits, Allocator>::iterator>
{
    const std::size_t append_count = source.size() < count ? source.size() : count;
    destination.append(source.data(), append_count);
    return destination.begin();
}

} // namespace xer

#endif /* XER_BITS_STRING_WRITE_H_INCLUDED_ */
