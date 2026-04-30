/**
 * @file xer/bits/bytes.h
 * @brief Internal byte-sequence conversion helper implementations.
 */

#pragma once

#ifndef XER_BITS_BYTES_H_INCLUDED_
#define XER_BITS_BYTES_H_INCLUDED_

#include <cstddef>
#include <span>
#include <string_view>
#include <vector>

namespace xer {

/**
 * @brief Views a `char` string as a byte sequence without copying.
 *
 * The returned span refers to the same storage as the input view. The caller
 * must ensure that the input storage outlives the returned span.
 *
 * @param value String view to reinterpret as bytes.
 * @return Non-owning byte view.
 */
[[nodiscard]] constexpr auto to_bytes_view(std::string_view value) noexcept
    -> std::span<const std::byte>
{
    return std::as_bytes(std::span{value.data(), value.size()});
}

/**
 * @brief Views a UTF-8 string as a byte sequence without copying.
 *
 * The returned span refers to the same storage as the input view. The caller
 * must ensure that the input storage outlives the returned span.
 *
 * @param value UTF-8 string view to reinterpret as bytes.
 * @return Non-owning byte view.
 */
[[nodiscard]] constexpr auto to_bytes_view(std::u8string_view value) noexcept
    -> std::span<const std::byte>
{
    return std::as_bytes(std::span{value.data(), value.size()});
}

/**
 * @brief Views a `char` span as a byte sequence without copying.
 *
 * @param value Span to reinterpret as bytes.
 * @return Non-owning byte view.
 */
[[nodiscard]] constexpr auto to_bytes_view(std::span<const char> value) noexcept
    -> std::span<const std::byte>
{
    return std::as_bytes(value);
}

/**
 * @brief Views a `char8_t` span as a byte sequence without copying.
 *
 * @param value Span to reinterpret as bytes.
 * @return Non-owning byte view.
 */
[[nodiscard]] constexpr auto to_bytes_view(std::span<const char8_t> value) noexcept
    -> std::span<const std::byte>
{
    return std::as_bytes(value);
}

/**
 * @brief Views an `unsigned char` span as a byte sequence without copying.
 *
 * @param value Span to reinterpret as bytes.
 * @return Non-owning byte view.
 */
[[nodiscard]] constexpr auto to_bytes_view(
    std::span<const unsigned char> value) noexcept -> std::span<const std::byte>
{
    return std::as_bytes(value);
}

/**
 * @brief Returns an existing byte span unchanged.
 *
 * @param value Byte span.
 * @return The same byte span.
 */
[[nodiscard]] constexpr auto to_bytes_view(
    std::span<const std::byte> value) noexcept -> std::span<const std::byte>
{
    return value;
}

/**
 * @brief Copies a `char` string into an owning byte vector.
 *
 * @param value String view to copy as bytes.
 * @return Owning byte vector.
 */
[[nodiscard]] inline auto to_bytes(std::string_view value)
    -> std::vector<std::byte>
{
    const auto view = to_bytes_view(value);
    return {view.begin(), view.end()};
}

/**
 * @brief Copies a UTF-8 string into an owning byte vector.
 *
 * @param value UTF-8 string view to copy as bytes.
 * @return Owning byte vector.
 */
[[nodiscard]] inline auto to_bytes(std::u8string_view value)
    -> std::vector<std::byte>
{
    const auto view = to_bytes_view(value);
    return {view.begin(), view.end()};
}

/**
 * @brief Copies a `char` span into an owning byte vector.
 *
 * @param value Span to copy as bytes.
 * @return Owning byte vector.
 */
[[nodiscard]] inline auto to_bytes(std::span<const char> value)
    -> std::vector<std::byte>
{
    const auto view = to_bytes_view(value);
    return {view.begin(), view.end()};
}

/**
 * @brief Copies a `char8_t` span into an owning byte vector.
 *
 * @param value Span to copy as bytes.
 * @return Owning byte vector.
 */
[[nodiscard]] inline auto to_bytes(std::span<const char8_t> value)
    -> std::vector<std::byte>
{
    const auto view = to_bytes_view(value);
    return {view.begin(), view.end()};
}

/**
 * @brief Copies an `unsigned char` span into an owning byte vector.
 *
 * @param value Span to copy as bytes.
 * @return Owning byte vector.
 */
[[nodiscard]] inline auto to_bytes(std::span<const unsigned char> value)
    -> std::vector<std::byte>
{
    const auto view = to_bytes_view(value);
    return {view.begin(), view.end()};
}

/**
 * @brief Copies a byte span into an owning byte vector.
 *
 * @param value Byte span to copy.
 * @return Owning byte vector.
 */
[[nodiscard]] inline auto to_bytes(std::span<const std::byte> value)
    -> std::vector<std::byte>
{
    return {value.begin(), value.end()};
}

} // namespace xer

#endif /* XER_BITS_BYTES_H_INCLUDED_ */
