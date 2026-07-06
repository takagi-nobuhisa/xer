/**
 * @file xer/bits/binary.h
 * @brief Small binary integer helper implementations.
 */

#pragma once

#ifndef XER_BITS_BINARY_H_INCLUDED_
#define XER_BITS_BINARY_H_INCLUDED_

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <iterator>
#include <numeric>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include <xer/bits/file_contents.h>
#include <xer/error.h>
#include <xer/path.h>
#include <xer/stdint.h>

namespace xer {

using std::byteswap;


/**
 * @brief Byte order used when grouping bytes into 16-bit or 32-bit words.
 */
enum class byte_order {
    little_endian,
    big_endian,
};


/**
 * @brief Extracts the high 8 bits of a 16-bit unsigned integer.
 * @param value Input value.
 * @return High 8-bit part.
 */
[[nodiscard]] constexpr auto high_u8(std::uint16_t value) noexcept -> std::uint8_t
{
    return static_cast<std::uint8_t>((value >> 8) & 0xffu);
}

/**
 * @brief Extracts the low 8 bits of a 16-bit unsigned integer.
 * @param value Input value.
 * @return Low 8-bit part.
 */
[[nodiscard]] constexpr auto low_u8(std::uint16_t value) noexcept -> std::uint8_t
{
    return static_cast<std::uint8_t>(value & 0xffu);
}

/**
 * @brief Composes a 16-bit unsigned integer from high and low 8-bit parts.
 * @param high High 8-bit part.
 * @param low Low 8-bit part.
 * @return Composed 16-bit value.
 */
[[nodiscard]] constexpr auto make_u16(
    std::uint8_t high,
    std::uint8_t low) noexcept -> std::uint16_t
{
    return static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(high) << 8) |
        static_cast<std::uint16_t>(low));
}

/**
 * @brief Extracts the high 16 bits of a 32-bit unsigned integer.
 * @param value Input value.
 * @return High 16-bit part.
 */
[[nodiscard]] constexpr auto high_u16(std::uint32_t value) noexcept -> std::uint16_t
{
    return static_cast<std::uint16_t>((value >> 16) & 0xffffu);
}

/**
 * @brief Extracts the low 16 bits of a 32-bit unsigned integer.
 * @param value Input value.
 * @return Low 16-bit part.
 */
[[nodiscard]] constexpr auto low_u16(std::uint32_t value) noexcept -> std::uint16_t
{
    return static_cast<std::uint16_t>(value & 0xffffu);
}

/**
 * @brief Composes a 32-bit unsigned integer from high and low 16-bit parts.
 * @param high High 16-bit part.
 * @param low Low 16-bit part.
 * @return Composed 32-bit value.
 */
[[nodiscard]] constexpr auto make_u32(
    std::uint16_t high,
    std::uint16_t low) noexcept -> std::uint32_t
{
    return (static_cast<std::uint32_t>(high) << 16) |
        static_cast<std::uint32_t>(low);
}

/**
 * @brief Extracts the high 32 bits of a 64-bit unsigned integer.
 * @param value Input value.
 * @return High 32-bit part.
 */
[[nodiscard]] constexpr auto high_u32(std::uint64_t value) noexcept -> std::uint32_t
{
    return static_cast<std::uint32_t>((value >> 32) & 0xffffffffull);
}

/**
 * @brief Extracts the low 32 bits of a 64-bit unsigned integer.
 * @param value Input value.
 * @return Low 32-bit part.
 */
[[nodiscard]] constexpr auto low_u32(std::uint64_t value) noexcept -> std::uint32_t
{
    return static_cast<std::uint32_t>(value & 0xffffffffull);
}

/**
 * @brief Composes a 64-bit unsigned integer from high and low 32-bit parts.
 * @param high High 32-bit part.
 * @param low Low 32-bit part.
 * @return Composed 64-bit value.
 */
[[nodiscard]] constexpr auto make_u64(
    std::uint32_t high,
    std::uint32_t low) noexcept -> std::uint64_t
{
    return (static_cast<std::uint64_t>(high) << 32) |
        static_cast<std::uint64_t>(low);
}

#if defined(XER_HAS_INT128)

/**
 * @brief Extracts the high 64 bits of a 128-bit unsigned integer.
 * @param value Input value.
 * @return High 64-bit part.
 */
[[nodiscard]] constexpr auto high_u64(xer::uint128_t value) noexcept -> std::uint64_t
{
    return static_cast<std::uint64_t>(value >> 64);
}

/**
 * @brief Extracts the low 64 bits of a 128-bit unsigned integer.
 * @param value Input value.
 * @return Low 64-bit part.
 */
[[nodiscard]] constexpr auto low_u64(xer::uint128_t value) noexcept -> std::uint64_t
{
    return static_cast<std::uint64_t>(value & static_cast<xer::uint128_t>(0xffffffffffffffffull));
}

/**
 * @brief Composes a 128-bit unsigned integer from high and low 64-bit parts.
 * @param high High 64-bit part.
 * @param low Low 64-bit part.
 * @return Composed 128-bit value.
 */
[[nodiscard]] constexpr auto make_u128(
    std::uint64_t high,
    std::uint64_t low) noexcept -> xer::uint128_t
{
    return (static_cast<xer::uint128_t>(high) << 64) |
        static_cast<xer::uint128_t>(low);
}

/**
 * @brief Reverses the byte order of a 128-bit unsigned integer.
 * @param value Input value.
 * @return Value with its byte order reversed.
 */
[[nodiscard]] constexpr auto byteswap(xer::uint128_t value) noexcept -> xer::uint128_t
{
    return make_u128(
        xer::byteswap(low_u64(value)),
        xer::byteswap(high_u64(value)));
}

#endif

namespace detail {

template<typename T>
[[nodiscard]] constexpr auto reverse_bits_unsigned(T value) noexcept -> T
{
    T result = 0;

    for (int i = 0; i < xer::bit_width_of<T>; ++i) {
        result = static_cast<T>((result << 1) | (value & static_cast<T>(1)));
        value = static_cast<T>(value >> 1);
    }

    return result;
}

} // namespace detail

/**
 * @brief Reverses the bit order of an 8-bit unsigned integer.
 * @param value Input value.
 * @return Value with its bit order reversed.
 */
[[nodiscard]] constexpr auto reverse_bits(std::uint8_t value) noexcept -> std::uint8_t
{
    return detail::reverse_bits_unsigned(value);
}

/**
 * @brief Reverses the bit order of a 16-bit unsigned integer.
 * @param value Input value.
 * @return Value with its bit order reversed.
 */
[[nodiscard]] constexpr auto reverse_bits(std::uint16_t value) noexcept -> std::uint16_t
{
    return detail::reverse_bits_unsigned(value);
}

/**
 * @brief Reverses the bit order of a 32-bit unsigned integer.
 * @param value Input value.
 * @return Value with its bit order reversed.
 */
[[nodiscard]] constexpr auto reverse_bits(std::uint32_t value) noexcept -> std::uint32_t
{
    return detail::reverse_bits_unsigned(value);
}

/**
 * @brief Reverses the bit order of a 64-bit unsigned integer.
 * @param value Input value.
 * @return Value with its bit order reversed.
 */
[[nodiscard]] constexpr auto reverse_bits(std::uint64_t value) noexcept -> std::uint64_t
{
    return detail::reverse_bits_unsigned(value);
}

#if defined(XER_HAS_INT128)

/**
 * @brief Reverses the bit order of a 128-bit unsigned integer.
 * @param value Input value.
 * @return Value with its bit order reversed.
 */
[[nodiscard]] constexpr auto reverse_bits(xer::uint128_t value) noexcept -> xer::uint128_t
{
    return detail::reverse_bits_unsigned(value);
}

#endif


namespace detail {

template<typename T>
[[nodiscard]] constexpr auto checksum_byte_value(T&& value) noexcept -> std::uint8_t
{
    using value_type = std::remove_cvref_t<T>;

    if constexpr (std::is_same_v<value_type, std::byte>) {
        return std::to_integer<std::uint8_t>(value);
    } else {
        return static_cast<std::uint8_t>(value);
    }
}

template<typename InputIt>
[[nodiscard]] constexpr auto checksum_add8_iter(InputIt first, InputIt last) -> std::uint8_t
{
    return std::accumulate(
        first,
        last,
        std::uint8_t{0},
        [](std::uint8_t sum, const auto& value) -> std::uint8_t {
            return static_cast<std::uint8_t>(
                static_cast<unsigned int>(sum) +
                static_cast<unsigned int>(checksum_byte_value(value)));
        });
}

template<typename InputIt>
[[nodiscard]] constexpr auto checksum_xor8_iter(InputIt first, InputIt last) -> std::uint8_t
{
    return std::accumulate(
        first,
        last,
        std::uint8_t{0},
        [](std::uint8_t result, const auto& value) -> std::uint8_t {
            return static_cast<std::uint8_t>(result ^ checksum_byte_value(value));
        });
}

template<typename InputIt>
[[nodiscard]] constexpr auto checksum_next_byte(
    InputIt& first,
    InputIt last) -> std::uint8_t
{
    if (first == last) {
        return 0;
    }

    const auto value = checksum_byte_value(*first);
    ++first;
    return value;
}

template<typename InputIt>
[[nodiscard]] constexpr auto checksum_read_u16(
    InputIt& first,
    InputIt last,
    const byte_order order) -> std::uint16_t
{
    const auto b0 = checksum_next_byte(first, last);
    const auto b1 = checksum_next_byte(first, last);

    if (order == byte_order::little_endian) {
        return make_u16(b1, b0);
    }

    return make_u16(b0, b1);
}

template<typename InputIt>
[[nodiscard]] constexpr auto checksum_read_u32(
    InputIt& first,
    InputIt last,
    const byte_order order) -> std::uint32_t
{
    const auto b0 = checksum_next_byte(first, last);
    const auto b1 = checksum_next_byte(first, last);
    const auto b2 = checksum_next_byte(first, last);
    const auto b3 = checksum_next_byte(first, last);

    if (order == byte_order::little_endian) {
        return make_u32(make_u16(b3, b2), make_u16(b1, b0));
    }

    return make_u32(make_u16(b0, b1), make_u16(b2, b3));
}

template<typename InputIt>
[[nodiscard]] constexpr auto checksum_add16_iter(
    InputIt first,
    InputIt last,
    const byte_order order) -> std::uint16_t
{
    std::uint16_t result = 0;

    while (first != last) {
        result = static_cast<std::uint16_t>(
            static_cast<unsigned int>(result) +
            static_cast<unsigned int>(checksum_read_u16(first, last, order)));
    }

    return result;
}

template<typename InputIt>
[[nodiscard]] constexpr auto checksum_xor16_iter(
    InputIt first,
    InputIt last,
    const byte_order order) -> std::uint16_t
{
    std::uint16_t result = 0;

    while (first != last) {
        result = static_cast<std::uint16_t>(
            result ^ checksum_read_u16(first, last, order));
    }

    return result;
}

template<typename InputIt>
[[nodiscard]] constexpr auto checksum_add32_iter(
    InputIt first,
    InputIt last,
    const byte_order order) -> std::uint32_t
{
    std::uint32_t result = 0;

    while (first != last) {
        result = static_cast<std::uint32_t>(
            static_cast<std::uint64_t>(result) +
            static_cast<std::uint64_t>(checksum_read_u32(first, last, order)));
    }

    return result;
}

template<typename InputIt>
[[nodiscard]] constexpr auto checksum_xor32_iter(
    InputIt first,
    InputIt last,
    const byte_order order) -> std::uint32_t
{
    std::uint32_t result = 0;

    while (first != last) {
        result = static_cast<std::uint32_t>(
            result ^ checksum_read_u32(first, last, order));
    }

    return result;
}



template<typename InputIt>
[[nodiscard]] constexpr auto crc16_iter(InputIt first, InputIt last) -> std::uint16_t
{
    return std::accumulate(
        first,
        last,
        std::uint16_t{0x0000u},
        [](std::uint16_t crc, const auto& value) -> std::uint16_t {
            crc = static_cast<std::uint16_t>(
                crc ^ static_cast<std::uint16_t>(checksum_byte_value(value)));

            for (int i = 0; i < 8; ++i) {
                if ((crc & 0x0001u) != 0) {
                    crc = static_cast<std::uint16_t>((crc >> 1) ^ 0xa001u);
                } else {
                    crc = static_cast<std::uint16_t>(crc >> 1);
                }
            }

            return crc;
        });
}

template<typename InputIt>
[[nodiscard]] constexpr auto crc32_iter(InputIt first, InputIt last) -> std::uint32_t
{
    const auto crc = std::accumulate(
        first,
        last,
        std::uint32_t{0xffffffffu},
        [](std::uint32_t value, const auto& byte) -> std::uint32_t {
            value ^= static_cast<std::uint32_t>(checksum_byte_value(byte));

            for (int i = 0; i < 8; ++i) {
                if ((value & 0x00000001u) != 0) {
                    value = (value >> 1) ^ 0xedb88320u;
                } else {
                    value >>= 1;
                }
            }

            return value;
        });

    return crc ^ 0xffffffffu;
}

[[nodiscard]] inline auto checksum_bytes_from_pointer(
    const void* data,
    const std::size_t size) noexcept -> result<std::span<const std::byte>>
{
    if (data == nullptr && size != 0) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return std::span<const std::byte>(static_cast<const std::byte*>(data), size);
}

} // namespace detail

/**
 * @brief Calculates an 8-bit additive checksum for a byte span.
 * @param bytes Source bytes.
 * @return 8-bit additive checksum.
 */
[[nodiscard]] constexpr auto checksum_add8(std::span<const std::byte> bytes) noexcept -> std::uint8_t
{
    return detail::checksum_add8_iter(bytes.begin(), bytes.end());
}

/**
 * @brief Calculates an 8-bit additive checksum for a byte span.
 * @param bytes Source bytes.
 * @return 8-bit additive checksum.
 */
[[nodiscard]] constexpr auto checksum8(std::span<const std::byte> bytes) noexcept -> std::uint8_t
{
    return checksum_add8(bytes);
}

/**
 * @brief Calculates an 8-bit XOR checksum for a byte span.
 * @param bytes Source bytes.
 * @return 8-bit XOR checksum.
 */
[[nodiscard]] constexpr auto checksum_xor8(std::span<const std::byte> bytes) noexcept -> std::uint8_t
{
    return detail::checksum_xor8_iter(bytes.begin(), bytes.end());
}

/**
 * @brief Calculates a 16-bit additive checksum for a byte span.
 * @param bytes Source bytes.
 * @param order Byte order used to group bytes into 16-bit words.
 * @return 16-bit additive checksum.
 */
[[nodiscard]] constexpr auto checksum_add16(
    std::span<const std::byte> bytes,
    byte_order order) noexcept -> std::uint16_t
{
    return detail::checksum_add16_iter(bytes.begin(), bytes.end(), order);
}

/**
 * @brief Calculates a 16-bit additive checksum for a byte span.
 * @param bytes Source bytes.
 * @param order Byte order used to group bytes into 16-bit words.
 * @return 16-bit additive checksum.
 */
[[nodiscard]] constexpr auto checksum16(
    std::span<const std::byte> bytes,
    byte_order order) noexcept -> std::uint16_t
{
    return checksum_add16(bytes, order);
}

/**
 * @brief Calculates a 16-bit XOR checksum for a byte span.
 * @param bytes Source bytes.
 * @param order Byte order used to group bytes into 16-bit words.
 * @return 16-bit XOR checksum.
 */
[[nodiscard]] constexpr auto checksum_xor16(
    std::span<const std::byte> bytes,
    byte_order order) noexcept -> std::uint16_t
{
    return detail::checksum_xor16_iter(bytes.begin(), bytes.end(), order);
}

/**
 * @brief Calculates a 32-bit additive checksum for a byte span.
 * @param bytes Source bytes.
 * @param order Byte order used to group bytes into 32-bit words.
 * @return 32-bit additive checksum.
 */
[[nodiscard]] constexpr auto checksum_add32(
    std::span<const std::byte> bytes,
    byte_order order) noexcept -> std::uint32_t
{
    return detail::checksum_add32_iter(bytes.begin(), bytes.end(), order);
}

/**
 * @brief Calculates a 32-bit additive checksum for a byte span.
 * @param bytes Source bytes.
 * @param order Byte order used to group bytes into 32-bit words.
 * @return 32-bit additive checksum.
 */
[[nodiscard]] constexpr auto checksum32(
    std::span<const std::byte> bytes,
    byte_order order) noexcept -> std::uint32_t
{
    return checksum_add32(bytes, order);
}

/**
 * @brief Calculates a 32-bit XOR checksum for a byte span.
 * @param bytes Source bytes.
 * @param order Byte order used to group bytes into 32-bit words.
 * @return 32-bit XOR checksum.
 */
[[nodiscard]] constexpr auto checksum_xor32(
    std::span<const std::byte> bytes,
    byte_order order) noexcept -> std::uint32_t
{
    return detail::checksum_xor32_iter(bytes.begin(), bytes.end(), order);
}

/**
 * @brief Calculates an 8-bit additive checksum for a pointer and byte size.
 * @param data Source byte pointer.
 * @param size Number of bytes.
 * @return 8-bit additive checksum on success.
 */
[[nodiscard]] inline auto checksum_add8(
    const void* data,
    std::size_t size) noexcept -> result<std::uint8_t>
{
    const auto bytes = detail::checksum_bytes_from_pointer(data, size);
    if (!bytes.has_value()) {
        return std::unexpected(bytes.error());
    }

    return checksum_add8(*bytes);
}

/**
 * @brief Calculates an 8-bit additive checksum for a pointer and byte size.
 * @param data Source byte pointer.
 * @param size Number of bytes.
 * @return 8-bit additive checksum on success.
 */
[[nodiscard]] inline auto checksum8(
    const void* data,
    std::size_t size) noexcept -> result<std::uint8_t>
{
    return checksum_add8(data, size);
}

/**
 * @brief Calculates an 8-bit XOR checksum for a pointer and byte size.
 * @param data Source byte pointer.
 * @param size Number of bytes.
 * @return 8-bit XOR checksum on success.
 */
[[nodiscard]] inline auto checksum_xor8(
    const void* data,
    std::size_t size) noexcept -> result<std::uint8_t>
{
    const auto bytes = detail::checksum_bytes_from_pointer(data, size);
    if (!bytes.has_value()) {
        return std::unexpected(bytes.error());
    }

    return checksum_xor8(*bytes);
}

/**
 * @brief Calculates a 16-bit additive checksum for a pointer and byte size.
 * @param data Source byte pointer.
 * @param size Number of bytes.
 * @param order Byte order used to group bytes into 16-bit words.
 * @return 16-bit additive checksum on success.
 */
[[nodiscard]] inline auto checksum_add16(
    const void* data,
    std::size_t size,
    byte_order order) noexcept -> result<std::uint16_t>
{
    const auto bytes = detail::checksum_bytes_from_pointer(data, size);
    if (!bytes.has_value()) {
        return std::unexpected(bytes.error());
    }

    return checksum_add16(*bytes, order);
}

/**
 * @brief Calculates a 16-bit additive checksum for a pointer and byte size.
 * @param data Source byte pointer.
 * @param size Number of bytes.
 * @param order Byte order used to group bytes into 16-bit words.
 * @return 16-bit additive checksum on success.
 */
[[nodiscard]] inline auto checksum16(
    const void* data,
    std::size_t size,
    byte_order order) noexcept -> result<std::uint16_t>
{
    return checksum_add16(data, size, order);
}

/**
 * @brief Calculates a 16-bit XOR checksum for a pointer and byte size.
 * @param data Source byte pointer.
 * @param size Number of bytes.
 * @param order Byte order used to group bytes into 16-bit words.
 * @return 16-bit XOR checksum on success.
 */
[[nodiscard]] inline auto checksum_xor16(
    const void* data,
    std::size_t size,
    byte_order order) noexcept -> result<std::uint16_t>
{
    const auto bytes = detail::checksum_bytes_from_pointer(data, size);
    if (!bytes.has_value()) {
        return std::unexpected(bytes.error());
    }

    return checksum_xor16(*bytes, order);
}

/**
 * @brief Calculates a 32-bit additive checksum for a pointer and byte size.
 * @param data Source byte pointer.
 * @param size Number of bytes.
 * @param order Byte order used to group bytes into 32-bit words.
 * @return 32-bit additive checksum on success.
 */
[[nodiscard]] inline auto checksum_add32(
    const void* data,
    std::size_t size,
    byte_order order) noexcept -> result<std::uint32_t>
{
    const auto bytes = detail::checksum_bytes_from_pointer(data, size);
    if (!bytes.has_value()) {
        return std::unexpected(bytes.error());
    }

    return checksum_add32(*bytes, order);
}

/**
 * @brief Calculates a 32-bit additive checksum for a pointer and byte size.
 * @param data Source byte pointer.
 * @param size Number of bytes.
 * @param order Byte order used to group bytes into 32-bit words.
 * @return 32-bit additive checksum on success.
 */
[[nodiscard]] inline auto checksum32(
    const void* data,
    std::size_t size,
    byte_order order) noexcept -> result<std::uint32_t>
{
    return checksum_add32(data, size, order);
}

/**
 * @brief Calculates a 32-bit XOR checksum for a pointer and byte size.
 * @param data Source byte pointer.
 * @param size Number of bytes.
 * @param order Byte order used to group bytes into 32-bit words.
 * @return 32-bit XOR checksum on success.
 */
[[nodiscard]] inline auto checksum_xor32(
    const void* data,
    std::size_t size,
    byte_order order) noexcept -> result<std::uint32_t>
{
    const auto bytes = detail::checksum_bytes_from_pointer(data, size);
    if (!bytes.has_value()) {
        return std::unexpected(bytes.error());
    }

    return checksum_xor32(*bytes, order);
}

/**
 * @brief Calculates an 8-bit additive checksum for an iterator range.
 * @param first First byte iterator.
 * @param last End iterator.
 * @return 8-bit additive checksum.
 */
template<std::input_iterator InputIt>
[[nodiscard]] constexpr auto checksum_add8(InputIt first, InputIt last) -> std::uint8_t
{
    return detail::checksum_add8_iter(first, last);
}

/**
 * @brief Calculates an 8-bit additive checksum for an iterator range.
 * @param first First byte iterator.
 * @param last End iterator.
 * @return 8-bit additive checksum.
 */
template<std::input_iterator InputIt>
[[nodiscard]] constexpr auto checksum8(InputIt first, InputIt last) -> std::uint8_t
{
    return checksum_add8(first, last);
}

/**
 * @brief Calculates an 8-bit XOR checksum for an iterator range.
 * @param first First byte iterator.
 * @param last End iterator.
 * @return 8-bit XOR checksum.
 */
template<std::input_iterator InputIt>
[[nodiscard]] constexpr auto checksum_xor8(InputIt first, InputIt last) -> std::uint8_t
{
    return detail::checksum_xor8_iter(first, last);
}

/**
 * @brief Calculates a 16-bit additive checksum for an iterator range.
 * @param first First byte iterator.
 * @param last End iterator.
 * @param order Byte order used to group bytes into 16-bit words.
 * @return 16-bit additive checksum.
 */
template<std::input_iterator InputIt>
[[nodiscard]] constexpr auto checksum_add16(
    InputIt first,
    InputIt last,
    byte_order order) -> std::uint16_t
{
    return detail::checksum_add16_iter(first, last, order);
}

/**
 * @brief Calculates a 16-bit additive checksum for an iterator range.
 * @param first First byte iterator.
 * @param last End iterator.
 * @param order Byte order used to group bytes into 16-bit words.
 * @return 16-bit additive checksum.
 */
template<std::input_iterator InputIt>
[[nodiscard]] constexpr auto checksum16(
    InputIt first,
    InputIt last,
    byte_order order) -> std::uint16_t
{
    return checksum_add16(first, last, order);
}

/**
 * @brief Calculates a 16-bit XOR checksum for an iterator range.
 * @param first First byte iterator.
 * @param last End iterator.
 * @param order Byte order used to group bytes into 16-bit words.
 * @return 16-bit XOR checksum.
 */
template<std::input_iterator InputIt>
[[nodiscard]] constexpr auto checksum_xor16(
    InputIt first,
    InputIt last,
    byte_order order) -> std::uint16_t
{
    return detail::checksum_xor16_iter(first, last, order);
}

/**
 * @brief Calculates a 32-bit additive checksum for an iterator range.
 * @param first First byte iterator.
 * @param last End iterator.
 * @param order Byte order used to group bytes into 32-bit words.
 * @return 32-bit additive checksum.
 */
template<std::input_iterator InputIt>
[[nodiscard]] constexpr auto checksum_add32(
    InputIt first,
    InputIt last,
    byte_order order) -> std::uint32_t
{
    return detail::checksum_add32_iter(first, last, order);
}

/**
 * @brief Calculates a 32-bit additive checksum for an iterator range.
 * @param first First byte iterator.
 * @param last End iterator.
 * @param order Byte order used to group bytes into 32-bit words.
 * @return 32-bit additive checksum.
 */
template<std::input_iterator InputIt>
[[nodiscard]] constexpr auto checksum32(
    InputIt first,
    InputIt last,
    byte_order order) -> std::uint32_t
{
    return checksum_add32(first, last, order);
}

/**
 * @brief Calculates a 32-bit XOR checksum for an iterator range.
 * @param first First byte iterator.
 * @param last End iterator.
 * @param order Byte order used to group bytes into 32-bit words.
 * @return 32-bit XOR checksum.
 */
template<std::input_iterator InputIt>
[[nodiscard]] constexpr auto checksum_xor32(
    InputIt first,
    InputIt last,
    byte_order order) -> std::uint32_t
{
    return detail::checksum_xor32_iter(first, last, order);
}

/**
 * @brief Calculates an 8-bit additive checksum for a file.
 * @param filename Source file path.
 * @return 8-bit additive checksum on success.
 */
[[nodiscard]] inline auto checksum_add8(const path& filename) -> result<std::uint8_t>
{
    const auto bytes = file_get_contents(filename);
    if (!bytes.has_value()) {
        return std::unexpected(bytes.error());
    }

    return checksum_add8(std::span<const std::byte>(*bytes));
}

/**
 * @brief Calculates an 8-bit additive checksum for a file.
 * @param filename Source file path.
 * @return 8-bit additive checksum on success.
 */
[[nodiscard]] inline auto checksum8(const path& filename) -> result<std::uint8_t>
{
    return checksum_add8(filename);
}

/**
 * @brief Calculates an 8-bit XOR checksum for a file.
 * @param filename Source file path.
 * @return 8-bit XOR checksum on success.
 */
[[nodiscard]] inline auto checksum_xor8(const path& filename) -> result<std::uint8_t>
{
    const auto bytes = file_get_contents(filename);
    if (!bytes.has_value()) {
        return std::unexpected(bytes.error());
    }

    return checksum_xor8(std::span<const std::byte>(*bytes));
}

/**
 * @brief Calculates a 16-bit additive checksum for a file.
 * @param filename Source file path.
 * @param order Byte order used to group bytes into 16-bit words.
 * @return 16-bit additive checksum on success.
 */
[[nodiscard]] inline auto checksum_add16(
    const path& filename,
    byte_order order) -> result<std::uint16_t>
{
    const auto bytes = file_get_contents(filename);
    if (!bytes.has_value()) {
        return std::unexpected(bytes.error());
    }

    return checksum_add16(std::span<const std::byte>(*bytes), order);
}

/**
 * @brief Calculates a 16-bit additive checksum for a file.
 * @param filename Source file path.
 * @param order Byte order used to group bytes into 16-bit words.
 * @return 16-bit additive checksum on success.
 */
[[nodiscard]] inline auto checksum16(
    const path& filename,
    byte_order order) -> result<std::uint16_t>
{
    return checksum_add16(filename, order);
}

/**
 * @brief Calculates a 16-bit XOR checksum for a file.
 * @param filename Source file path.
 * @param order Byte order used to group bytes into 16-bit words.
 * @return 16-bit XOR checksum on success.
 */
[[nodiscard]] inline auto checksum_xor16(
    const path& filename,
    byte_order order) -> result<std::uint16_t>
{
    const auto bytes = file_get_contents(filename);
    if (!bytes.has_value()) {
        return std::unexpected(bytes.error());
    }

    return checksum_xor16(std::span<const std::byte>(*bytes), order);
}

/**
 * @brief Calculates a 32-bit additive checksum for a file.
 * @param filename Source file path.
 * @param order Byte order used to group bytes into 32-bit words.
 * @return 32-bit additive checksum on success.
 */
[[nodiscard]] inline auto checksum_add32(
    const path& filename,
    byte_order order) -> result<std::uint32_t>
{
    const auto bytes = file_get_contents(filename);
    if (!bytes.has_value()) {
        return std::unexpected(bytes.error());
    }

    return checksum_add32(std::span<const std::byte>(*bytes), order);
}

/**
 * @brief Calculates a 32-bit additive checksum for a file.
 * @param filename Source file path.
 * @param order Byte order used to group bytes into 32-bit words.
 * @return 32-bit additive checksum on success.
 */
[[nodiscard]] inline auto checksum32(
    const path& filename,
    byte_order order) -> result<std::uint32_t>
{
    return checksum_add32(filename, order);
}

/**
 * @brief Calculates a 32-bit XOR checksum for a file.
 * @param filename Source file path.
 * @param order Byte order used to group bytes into 32-bit words.
 * @return 32-bit XOR checksum on success.
 */
[[nodiscard]] inline auto checksum_xor32(
    const path& filename,
    byte_order order) -> result<std::uint32_t>
{
    const auto bytes = file_get_contents(filename);
    if (!bytes.has_value()) {
        return std::unexpected(bytes.error());
    }

    return checksum_xor32(std::span<const std::byte>(*bytes), order);
}


/**
 * @brief Calculates a CRC-16/ARC value for a byte span.
 * @param bytes Source bytes.
 * @return CRC-16/ARC value.
 */
[[nodiscard]] constexpr auto crc16(std::span<const std::byte> bytes) noexcept -> std::uint16_t
{
    return detail::crc16_iter(bytes.begin(), bytes.end());
}

/**
 * @brief Calculates a CRC-32/ISO-HDLC value for a byte span.
 * @param bytes Source bytes.
 * @return CRC-32/ISO-HDLC value.
 */
[[nodiscard]] constexpr auto crc32(std::span<const std::byte> bytes) noexcept -> std::uint32_t
{
    return detail::crc32_iter(bytes.begin(), bytes.end());
}

/**
 * @brief Calculates a CRC-16/ARC value for a pointer and byte size.
 * @param data Source byte pointer.
 * @param size Number of bytes.
 * @return CRC-16/ARC value on success.
 */
[[nodiscard]] inline auto crc16(
    const void* data,
    std::size_t size) noexcept -> result<std::uint16_t>
{
    const auto bytes = detail::checksum_bytes_from_pointer(data, size);
    if (!bytes.has_value()) {
        return std::unexpected(bytes.error());
    }

    return crc16(*bytes);
}

/**
 * @brief Calculates a CRC-32/ISO-HDLC value for a pointer and byte size.
 * @param data Source byte pointer.
 * @param size Number of bytes.
 * @return CRC-32/ISO-HDLC value on success.
 */
[[nodiscard]] inline auto crc32(
    const void* data,
    std::size_t size) noexcept -> result<std::uint32_t>
{
    const auto bytes = detail::checksum_bytes_from_pointer(data, size);
    if (!bytes.has_value()) {
        return std::unexpected(bytes.error());
    }

    return crc32(*bytes);
}

/**
 * @brief Calculates a CRC-16/ARC value for an iterator range.
 * @param first First byte iterator.
 * @param last End iterator.
 * @return CRC-16/ARC value.
 */
template<std::input_iterator InputIt>
[[nodiscard]] constexpr auto crc16(InputIt first, InputIt last) -> std::uint16_t
{
    return detail::crc16_iter(first, last);
}

/**
 * @brief Calculates a CRC-32/ISO-HDLC value for an iterator range.
 * @param first First byte iterator.
 * @param last End iterator.
 * @return CRC-32/ISO-HDLC value.
 */
template<std::input_iterator InputIt>
[[nodiscard]] constexpr auto crc32(InputIt first, InputIt last) -> std::uint32_t
{
    return detail::crc32_iter(first, last);
}

/**
 * @brief Calculates a CRC-16/ARC value for a file.
 * @param filename Source file path.
 * @return CRC-16/ARC value on success.
 */
[[nodiscard]] inline auto crc16(const path& filename) -> result<std::uint16_t>
{
    const auto bytes = file_get_contents(filename);
    if (!bytes.has_value()) {
        return std::unexpected(bytes.error());
    }

    return crc16(std::span<const std::byte>(*bytes));
}

/**
 * @brief Calculates a CRC-32/ISO-HDLC value for a file.
 * @param filename Source file path.
 * @return CRC-32/ISO-HDLC value on success.
 */
[[nodiscard]] inline auto crc32(const path& filename) -> result<std::uint32_t>
{
    const auto bytes = file_get_contents(filename);
    if (!bytes.has_value()) {
        return std::unexpected(bytes.error());
    }

    return crc32(std::span<const std::byte>(*bytes));
}


namespace detail {

[[nodiscard]] constexpr auto hex_digit(unsigned int value) noexcept -> char8_t
{
    constexpr char8_t digits[] = u8"0123456789abcdef";
    return digits[value & 0x0fu];
}

[[nodiscard]] constexpr auto hex_value(char8_t ch) noexcept -> int
{
    if (ch >= u8'0' && ch <= u8'9') {
        return static_cast<int>(ch - u8'0');
    }

    if (ch >= u8'a' && ch <= u8'f') {
        return static_cast<int>(ch - u8'a') + 10;
    }

    if (ch >= u8'A' && ch <= u8'F') {
        return static_cast<int>(ch - u8'A') + 10;
    }

    return -1;
}

template<typename InputIt>
[[nodiscard]] auto bin2hex_iter(InputIt first, InputIt last) -> std::u8string
{
    std::u8string result;

    if constexpr (std::forward_iterator<InputIt>) {
        const auto length = std::distance(first, last);
        if (length > 0) {
            result.reserve(static_cast<std::size_t>(length) * 2u);
        }
    }

    for (; first != last; ++first) {
        const auto value = checksum_byte_value(*first);
        result.push_back(hex_digit(static_cast<unsigned int>(value >> 4u)));
        result.push_back(hex_digit(static_cast<unsigned int>(value & 0x0fu)));
    }

    return result;
}

[[nodiscard]] constexpr auto md5_read_u32_le(
    const std::array<std::byte, 64>& block,
    std::size_t offset) noexcept -> std::uint32_t
{
    return static_cast<std::uint32_t>(checksum_byte_value(block[offset])) |
        (static_cast<std::uint32_t>(checksum_byte_value(block[offset + 1])) << 8) |
        (static_cast<std::uint32_t>(checksum_byte_value(block[offset + 2])) << 16) |
        (static_cast<std::uint32_t>(checksum_byte_value(block[offset + 3])) << 24);
}

constexpr auto md5_write_u32_le(
    std::array<std::byte, 16>& output,
    std::size_t offset,
    std::uint32_t value) noexcept -> void
{
    output[offset] = static_cast<std::byte>(value & 0xffu);
    output[offset + 1] = static_cast<std::byte>((value >> 8) & 0xffu);
    output[offset + 2] = static_cast<std::byte>((value >> 16) & 0xffu);
    output[offset + 3] = static_cast<std::byte>((value >> 24) & 0xffu);
}

class md5_context {
public:
    constexpr auto update_byte(std::uint8_t value) noexcept -> void
    {
        buffer_[buffer_size_] = static_cast<std::byte>(value);
        ++buffer_size_;
        bit_count_ += 8u;

        if (buffer_size_ == buffer_.size()) {
            transform(buffer_);
            buffer_size_ = 0;
        }
    }

    template<typename InputIt>
    constexpr auto update(InputIt first, InputIt last) -> void
    {
        for (; first != last; ++first) {
            update_byte(checksum_byte_value(*first));
        }
    }

    [[nodiscard]] constexpr auto final() noexcept -> std::array<std::byte, 16>
    {
        const auto message_bit_count = bit_count_;

        update_byte(0x80u);
        while (buffer_size_ != 56u) {
            update_byte(0x00u);
        }

        for (int i = 0; i < 8; ++i) {
            update_byte(static_cast<std::uint8_t>((message_bit_count >> (i * 8)) & 0xffu));
        }

        std::array<std::byte, 16> digest{};
        md5_write_u32_le(digest, 0, a_);
        md5_write_u32_le(digest, 4, b_);
        md5_write_u32_le(digest, 8, c_);
        md5_write_u32_le(digest, 12, d_);
        return digest;
    }

private:
    constexpr auto transform(const std::array<std::byte, 64>& block) noexcept -> void
    {
        constexpr std::array<std::uint32_t, 64> k{
            0xd76aa478u, 0xe8c7b756u, 0x242070dbu, 0xc1bdceeeu,
            0xf57c0fafu, 0x4787c62au, 0xa8304613u, 0xfd469501u,
            0x698098d8u, 0x8b44f7afu, 0xffff5bb1u, 0x895cd7beu,
            0x6b901122u, 0xfd987193u, 0xa679438eu, 0x49b40821u,
            0xf61e2562u, 0xc040b340u, 0x265e5a51u, 0xe9b6c7aau,
            0xd62f105du, 0x02441453u, 0xd8a1e681u, 0xe7d3fbc8u,
            0x21e1cde6u, 0xc33707d6u, 0xf4d50d87u, 0x455a14edu,
            0xa9e3e905u, 0xfcefa3f8u, 0x676f02d9u, 0x8d2a4c8au,
            0xfffa3942u, 0x8771f681u, 0x6d9d6122u, 0xfde5380cu,
            0xa4beea44u, 0x4bdecfa9u, 0xf6bb4b60u, 0xbebfbc70u,
            0x289b7ec6u, 0xeaa127fau, 0xd4ef3085u, 0x04881d05u,
            0xd9d4d039u, 0xe6db99e5u, 0x1fa27cf8u, 0xc4ac5665u,
            0xf4292244u, 0x432aff97u, 0xab9423a7u, 0xfc93a039u,
            0x655b59c3u, 0x8f0ccc92u, 0xffeff47du, 0x85845dd1u,
            0x6fa87e4fu, 0xfe2ce6e0u, 0xa3014314u, 0x4e0811a1u,
            0xf7537e82u, 0xbd3af235u, 0x2ad7d2bbu, 0xeb86d391u,
        };

        constexpr std::array<int, 64> s{
            7, 12, 17, 22, 7, 12, 17, 22,
            7, 12, 17, 22, 7, 12, 17, 22,
            5, 9, 14, 20, 5, 9, 14, 20,
            5, 9, 14, 20, 5, 9, 14, 20,
            4, 11, 16, 23, 4, 11, 16, 23,
            4, 11, 16, 23, 4, 11, 16, 23,
            6, 10, 15, 21, 6, 10, 15, 21,
            6, 10, 15, 21, 6, 10, 15, 21,
        };

        std::array<std::uint32_t, 16> m{};
        for (std::size_t i = 0; i < m.size(); ++i) {
            m[i] = md5_read_u32_le(block, i * 4u);
        }

        auto a = a_;
        auto b = b_;
        auto c = c_;
        auto d = d_;

        for (std::size_t i = 0; i < 64u; ++i) {
            std::uint32_t f = 0;
            std::size_t g = 0;

            if (i < 16u) {
                f = (b & c) | ((~b) & d);
                g = i;
            } else if (i < 32u) {
                f = (d & b) | ((~d) & c);
                g = (5u * i + 1u) % 16u;
            } else if (i < 48u) {
                f = b ^ c ^ d;
                g = (3u * i + 5u) % 16u;
            } else {
                f = c ^ (b | (~d));
                g = (7u * i) % 16u;
            }

            const auto temp = d;
            d = c;
            c = b;
            b += std::rotl(a + f + k[i] + m[g], s[i]);
            a = temp;
        }

        a_ += a;
        b_ += b;
        c_ += c;
        d_ += d;
    }

    std::uint32_t a_ = 0x67452301u;
    std::uint32_t b_ = 0xefcdab89u;
    std::uint32_t c_ = 0x98badcfeu;
    std::uint32_t d_ = 0x10325476u;
    std::uint64_t bit_count_ = 0;
    std::array<std::byte, 64> buffer_{};
    std::size_t buffer_size_ = 0;
};



[[nodiscard]] constexpr auto sha1_read_u32_be(
    const std::array<std::byte, 64>& block,
    std::size_t offset) noexcept -> std::uint32_t
{
    return (static_cast<std::uint32_t>(checksum_byte_value(block[offset])) << 24) |
        (static_cast<std::uint32_t>(checksum_byte_value(block[offset + 1])) << 16) |
        (static_cast<std::uint32_t>(checksum_byte_value(block[offset + 2])) << 8) |
        static_cast<std::uint32_t>(checksum_byte_value(block[offset + 3]));
}

constexpr auto sha1_write_u32_be(
    std::array<std::byte, 20>& output,
    std::size_t offset,
    std::uint32_t value) noexcept -> void
{
    output[offset] = static_cast<std::byte>((value >> 24) & 0xffu);
    output[offset + 1] = static_cast<std::byte>((value >> 16) & 0xffu);
    output[offset + 2] = static_cast<std::byte>((value >> 8) & 0xffu);
    output[offset + 3] = static_cast<std::byte>(value & 0xffu);
}

class sha1_context {
public:
    constexpr auto update_byte(std::uint8_t value) noexcept -> void
    {
        buffer_[buffer_size_] = static_cast<std::byte>(value);
        ++buffer_size_;
        bit_count_ += 8u;

        if (buffer_size_ == buffer_.size()) {
            transform(buffer_);
            buffer_size_ = 0;
        }
    }

    template<typename InputIt>
    constexpr auto update(InputIt first, InputIt last) -> void
    {
        for (; first != last; ++first) {
            update_byte(checksum_byte_value(*first));
        }
    }

    [[nodiscard]] constexpr auto final() noexcept -> std::array<std::byte, 20>
    {
        const auto message_bit_count = bit_count_;

        update_byte(0x80u);
        while (buffer_size_ != 56u) {
            update_byte(0x00u);
        }

        for (int i = 7; i >= 0; --i) {
            update_byte(static_cast<std::uint8_t>((message_bit_count >> (i * 8)) & 0xffu));
        }

        std::array<std::byte, 20> digest{};
        for (std::size_t i = 0; i < h_.size(); ++i) {
            sha1_write_u32_be(digest, i * 4u, h_[i]);
        }
        return digest;
    }

private:
    constexpr auto transform(const std::array<std::byte, 64>& block) noexcept -> void
    {
        std::array<std::uint32_t, 80> w{};
        for (std::size_t i = 0; i < 16u; ++i) {
            w[i] = sha1_read_u32_be(block, i * 4u);
        }
        for (std::size_t i = 16u; i < w.size(); ++i) {
            w[i] = std::rotl(w[i - 3u] ^ w[i - 8u] ^ w[i - 14u] ^ w[i - 16u], 1);
        }

        auto a = h_[0];
        auto b = h_[1];
        auto c = h_[2];
        auto d = h_[3];
        auto e = h_[4];

        for (std::size_t i = 0; i < 80u; ++i) {
            std::uint32_t f = 0;
            std::uint32_t k = 0;

            if (i < 20u) {
                f = (b & c) | ((~b) & d);
                k = 0x5a827999u;
            }
            else if (i < 40u) {
                f = b ^ c ^ d;
                k = 0x6ed9eba1u;
            }
            else if (i < 60u) {
                f = (b & c) | (b & d) | (c & d);
                k = 0x8f1bbcdcu;
            }
            else {
                f = b ^ c ^ d;
                k = 0xca62c1d6u;
            }

            const auto temp = std::rotl(a, 5) + f + e + k + w[i];
            e = d;
            d = c;
            c = std::rotl(b, 30);
            b = a;
            a = temp;
        }

        h_[0] += a;
        h_[1] += b;
        h_[2] += c;
        h_[3] += d;
        h_[4] += e;
    }

    std::array<std::uint32_t, 5> h_{
        0x67452301u,
        0xefcdab89u,
        0x98badcfeu,
        0x10325476u,
        0xc3d2e1f0u,
    };
    std::uint64_t bit_count_ = 0;
    std::array<std::byte, 64> buffer_{};
    std::size_t buffer_size_ = 0;
};

[[nodiscard]] constexpr auto sha256_read_u32_be(
    const std::array<std::byte, 64>& block,
    std::size_t offset) noexcept -> std::uint32_t
{
    return (static_cast<std::uint32_t>(checksum_byte_value(block[offset])) << 24) |
        (static_cast<std::uint32_t>(checksum_byte_value(block[offset + 1])) << 16) |
        (static_cast<std::uint32_t>(checksum_byte_value(block[offset + 2])) << 8) |
        static_cast<std::uint32_t>(checksum_byte_value(block[offset + 3]));
}

constexpr auto sha256_write_u32_be(
    std::array<std::byte, 32>& output,
    std::size_t offset,
    std::uint32_t value) noexcept -> void
{
    output[offset] = static_cast<std::byte>((value >> 24) & 0xffu);
    output[offset + 1] = static_cast<std::byte>((value >> 16) & 0xffu);
    output[offset + 2] = static_cast<std::byte>((value >> 8) & 0xffu);
    output[offset + 3] = static_cast<std::byte>(value & 0xffu);
}

class sha256_context {
public:
    constexpr auto update_byte(std::uint8_t value) noexcept -> void
    {
        buffer_[buffer_size_] = static_cast<std::byte>(value);
        ++buffer_size_;
        bit_count_ += 8u;

        if (buffer_size_ == buffer_.size()) {
            transform(buffer_);
            buffer_size_ = 0;
        }
    }

    template<typename InputIt>
    constexpr auto update(InputIt first, InputIt last) -> void
    {
        for (; first != last; ++first) {
            update_byte(checksum_byte_value(*first));
        }
    }

    [[nodiscard]] constexpr auto final() noexcept -> std::array<std::byte, 32>
    {
        const auto message_bit_count = bit_count_;

        update_byte(0x80u);
        while (buffer_size_ != 56u) {
            update_byte(0x00u);
        }

        for (int i = 7; i >= 0; --i) {
            update_byte(static_cast<std::uint8_t>((message_bit_count >> (i * 8)) & 0xffu));
        }

        std::array<std::byte, 32> digest{};
        for (std::size_t i = 0; i < h_.size(); ++i) {
            sha256_write_u32_be(digest, i * 4u, h_[i]);
        }
        return digest;
    }

private:
    [[nodiscard]] static constexpr auto ch(
        std::uint32_t x,
        std::uint32_t y,
        std::uint32_t z) noexcept -> std::uint32_t
    {
        return (x & y) ^ ((~x) & z);
    }

    [[nodiscard]] static constexpr auto maj(
        std::uint32_t x,
        std::uint32_t y,
        std::uint32_t z) noexcept -> std::uint32_t
    {
        return (x & y) ^ (x & z) ^ (y & z);
    }

    [[nodiscard]] static constexpr auto big_sigma0(std::uint32_t x) noexcept -> std::uint32_t
    {
        return std::rotr(x, 2) ^ std::rotr(x, 13) ^ std::rotr(x, 22);
    }

    [[nodiscard]] static constexpr auto big_sigma1(std::uint32_t x) noexcept -> std::uint32_t
    {
        return std::rotr(x, 6) ^ std::rotr(x, 11) ^ std::rotr(x, 25);
    }

    [[nodiscard]] static constexpr auto small_sigma0(std::uint32_t x) noexcept -> std::uint32_t
    {
        return std::rotr(x, 7) ^ std::rotr(x, 18) ^ (x >> 3);
    }

    [[nodiscard]] static constexpr auto small_sigma1(std::uint32_t x) noexcept -> std::uint32_t
    {
        return std::rotr(x, 17) ^ std::rotr(x, 19) ^ (x >> 10);
    }

    constexpr auto transform(const std::array<std::byte, 64>& block) noexcept -> void
    {
        constexpr std::array<std::uint32_t, 64> k{
            0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u,
            0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
            0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u,
            0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
            0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu,
            0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
            0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u,
            0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
            0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u,
            0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
            0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u,
            0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
            0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u,
            0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
            0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u,
            0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u,
        };

        std::array<std::uint32_t, 64> w{};
        for (std::size_t i = 0; i < 16u; ++i) {
            w[i] = sha256_read_u32_be(block, i * 4u);
        }
        for (std::size_t i = 16u; i < w.size(); ++i) {
            w[i] = small_sigma1(w[i - 2u]) + w[i - 7u] +
                small_sigma0(w[i - 15u]) + w[i - 16u];
        }

        auto a = h_[0];
        auto b = h_[1];
        auto c = h_[2];
        auto d = h_[3];
        auto e = h_[4];
        auto f = h_[5];
        auto g = h_[6];
        auto h = h_[7];

        for (std::size_t i = 0; i < 64u; ++i) {
            const auto temp1 = h + big_sigma1(e) + ch(e, f, g) + k[i] + w[i];
            const auto temp2 = big_sigma0(a) + maj(a, b, c);
            h = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }

        h_[0] += a;
        h_[1] += b;
        h_[2] += c;
        h_[3] += d;
        h_[4] += e;
        h_[5] += f;
        h_[6] += g;
        h_[7] += h;
    }

    std::array<std::uint32_t, 8> h_{
        0x6a09e667u,
        0xbb67ae85u,
        0x3c6ef372u,
        0xa54ff53au,
        0x510e527fu,
        0x9b05688cu,
        0x1f83d9abu,
        0x5be0cd19u,
    };
    std::uint64_t bit_count_ = 0;
    std::array<std::byte, 64> buffer_{};
    std::size_t buffer_size_ = 0;
};


template<typename InputIt>
[[nodiscard]] constexpr auto sha1_iter(InputIt first, InputIt last) -> std::array<std::byte, 20>
{
    sha1_context context;
    context.update(first, last);
    return context.final();
}

template<typename InputIt>
[[nodiscard]] constexpr auto sha256_iter(InputIt first, InputIt last) -> std::array<std::byte, 32>
{
    sha256_context context;
    context.update(first, last);
    return context.final();
}

template<typename InputIt>
[[nodiscard]] constexpr auto md5_iter(InputIt first, InputIt last) -> std::array<std::byte, 16>
{
    md5_context context;
    context.update(first, last);
    return context.final();
}

} // namespace detail

/**
 * @brief Converts binary data to a lowercase hexadecimal string.
 * @param bytes Source bytes.
 * @return Lowercase hexadecimal string.
 */
[[nodiscard]] inline auto bin2hex(std::span<const std::byte> bytes) -> std::u8string
{
    return detail::bin2hex_iter(bytes.begin(), bytes.end());
}

/**
 * @brief Converts binary data to a lowercase hexadecimal string.
 * @param data Source byte pointer.
 * @param size Number of bytes.
 * @return Lowercase hexadecimal string on success.
 */
[[nodiscard]] inline auto bin2hex(
    const void* data,
    std::size_t size) -> result<std::u8string>
{
    const auto bytes = detail::checksum_bytes_from_pointer(data, size);
    if (!bytes.has_value()) {
        return std::unexpected(bytes.error());
    }

    return bin2hex(*bytes);
}

/**
 * @brief Converts an iterator range of bytes to a lowercase hexadecimal string.
 * @param first First byte iterator.
 * @param last End iterator.
 * @return Lowercase hexadecimal string.
 */
template<std::input_iterator InputIt>
[[nodiscard]] auto bin2hex(InputIt first, InputIt last) -> std::u8string
{
    return detail::bin2hex_iter(first, last);
}

/**
 * @brief Converts a hexadecimal string to binary data.
 * @param hex Source hexadecimal string.
 * @return Binary data on success.
 */
[[nodiscard]] inline auto hex2bin(std::u8string_view hex) -> result<std::vector<std::byte>>
{
    if ((hex.size() % 2u) != 0u) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    std::vector<std::byte> result;
    result.reserve(hex.size() / 2u);

    for (std::size_t i = 0; i < hex.size(); i += 2u) {
        const auto high = detail::hex_value(hex[i]);
        const auto low = detail::hex_value(hex[i + 1u]);
        if (high < 0 || low < 0) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        result.push_back(static_cast<std::byte>((high << 4) | low));
    }

    return result;
}

/**
 * @brief Calculates the MD5 digest for a byte span.
 * @param bytes Source bytes.
 * @return 16-byte MD5 digest.
 */
[[nodiscard]] constexpr auto md5(std::span<const std::byte> bytes) noexcept -> std::array<std::byte, 16>
{
    return detail::md5_iter(bytes.begin(), bytes.end());
}

/**
 * @brief Calculates the MD5 digest for a pointer and byte size.
 * @param data Source byte pointer.
 * @param size Number of bytes.
 * @return 16-byte MD5 digest on success.
 */
[[nodiscard]] inline auto md5(
    const void* data,
    std::size_t size) noexcept -> result<std::array<std::byte, 16>>
{
    const auto bytes = detail::checksum_bytes_from_pointer(data, size);
    if (!bytes.has_value()) {
        return std::unexpected(bytes.error());
    }

    return md5(*bytes);
}

/**
 * @brief Calculates the MD5 digest for an iterator range.
 * @param first First byte iterator.
 * @param last End iterator.
 * @return 16-byte MD5 digest.
 */
template<std::input_iterator InputIt>
[[nodiscard]] constexpr auto md5(InputIt first, InputIt last) -> std::array<std::byte, 16>
{
    return detail::md5_iter(first, last);
}

/**
 * @brief Calculates the MD5 digest for a file.
 * @param filename Source file path.
 * @return 16-byte MD5 digest on success.
 */
[[nodiscard]] inline auto md5(const path& filename) -> result<std::array<std::byte, 16>>
{
    const auto bytes = file_get_contents(filename);
    if (!bytes.has_value()) {
        return std::unexpected(bytes.error());
    }

    return md5(std::span<const std::byte>(*bytes));
}



/**
 * @brief Calculates the SHA-1 digest for a byte span.
 * @param bytes Source bytes.
 * @return 20-byte SHA-1 digest.
 */
[[nodiscard]] constexpr auto sha1(std::span<const std::byte> bytes) noexcept -> std::array<std::byte, 20>
{
    return detail::sha1_iter(bytes.begin(), bytes.end());
}

/**
 * @brief Calculates the SHA-1 digest for a pointer and byte size.
 * @param data Source byte pointer.
 * @param size Number of bytes.
 * @return 20-byte SHA-1 digest on success.
 */
[[nodiscard]] inline auto sha1(
    const void* data,
    std::size_t size) noexcept -> result<std::array<std::byte, 20>>
{
    const auto bytes = detail::checksum_bytes_from_pointer(data, size);
    if (!bytes.has_value()) {
        return std::unexpected(bytes.error());
    }

    return sha1(*bytes);
}

/**
 * @brief Calculates the SHA-1 digest for an iterator range.
 * @param first First byte iterator.
 * @param last End iterator.
 * @return 20-byte SHA-1 digest.
 */
template<std::input_iterator InputIt>
[[nodiscard]] constexpr auto sha1(InputIt first, InputIt last) -> std::array<std::byte, 20>
{
    return detail::sha1_iter(first, last);
}

/**
 * @brief Calculates the SHA-1 digest for a file.
 * @param filename Source file path.
 * @return 20-byte SHA-1 digest on success.
 */
[[nodiscard]] inline auto sha1(const path& filename) -> result<std::array<std::byte, 20>>
{
    const auto bytes = file_get_contents(filename);
    if (!bytes.has_value()) {
        return std::unexpected(bytes.error());
    }

    return sha1(std::span<const std::byte>(*bytes));
}


/**
 * @brief Calculates the SHA-256 digest for a byte span.
 * @param bytes Source bytes.
 * @return 32-byte SHA-256 digest.
 */
[[nodiscard]] constexpr auto sha256(std::span<const std::byte> bytes) noexcept -> std::array<std::byte, 32>
{
    return detail::sha256_iter(bytes.begin(), bytes.end());
}

/**
 * @brief Calculates the SHA-256 digest for a pointer and byte size.
 * @param data Source byte pointer.
 * @param size Number of bytes.
 * @return 32-byte SHA-256 digest on success.
 */
[[nodiscard]] inline auto sha256(
    const void* data,
    std::size_t size) noexcept -> result<std::array<std::byte, 32>>
{
    const auto bytes = detail::checksum_bytes_from_pointer(data, size);
    if (!bytes.has_value()) {
        return std::unexpected(bytes.error());
    }

    return sha256(*bytes);
}

/**
 * @brief Calculates the SHA-256 digest for an iterator range.
 * @param first First byte iterator.
 * @param last End iterator.
 * @return 32-byte SHA-256 digest.
 */
template<std::input_iterator InputIt>
[[nodiscard]] constexpr auto sha256(InputIt first, InputIt last) -> std::array<std::byte, 32>
{
    return detail::sha256_iter(first, last);
}

/**
 * @brief Calculates the SHA-256 digest for a file.
 * @param filename Source file path.
 * @return 32-byte SHA-256 digest on success.
 */
[[nodiscard]] inline auto sha256(const path& filename) -> result<std::array<std::byte, 32>>
{
    const auto bytes = file_get_contents(filename);
    if (!bytes.has_value()) {
        return std::unexpected(bytes.error());
    }

    return sha256(std::span<const std::byte>(*bytes));
}

} // namespace xer

#endif /* XER_BITS_BINARY_H_INCLUDED_ */
