/**
 * @file xer/bits/binary.h
 * @brief Small binary integer helper implementations.
 */

#pragma once

#ifndef XER_BITS_BINARY_H_INCLUDED_
#define XER_BITS_BINARY_H_INCLUDED_

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

#if defined(__SIZEOF_INT128__)

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

#if defined(__SIZEOF_INT128__)

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


[[nodiscard]] constexpr auto hex_digit_value(char8_t value) noexcept -> int
{
    if (value >= u8'0' && value <= u8'9') {
        return static_cast<int>(value - u8'0');
    }

    if (value >= u8'a' && value <= u8'f') {
        return static_cast<int>(value - u8'a') + 10;
    }

    if (value >= u8'A' && value <= u8'F') {
        return static_cast<int>(value - u8'A') + 10;
    }

    return -1;
}

template<typename InputIt>
[[nodiscard]] auto bin2hex_iter(InputIt first, InputIt last) -> std::u8string
{
    constexpr char8_t table[] = u8"0123456789abcdef";

    std::u8string result;

    if constexpr (std::forward_iterator<InputIt>) {
        const auto size = std::distance(first, last);
        if (size > 0) {
            result.reserve(static_cast<std::size_t>(size) * 2u);
        }
    }

    for (; first != last; ++first) {
        const auto value = checksum_byte_value(*first);
        result.push_back(table[(value >> 4) & 0x0fu]);
        result.push_back(table[value & 0x0fu]);
    }

    return result;
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
 * @param hex Hexadecimal string. Both lowercase and uppercase letters are accepted.
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
        const auto high = detail::hex_digit_value(hex[i]);
        const auto low = detail::hex_digit_value(hex[i + 1u]);

        if (high < 0 || low < 0) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        result.push_back(static_cast<std::byte>((high << 4) | low));
    }

    return result;
}

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

} // namespace xer

#endif /* XER_BITS_BINARY_H_INCLUDED_ */
