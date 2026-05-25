#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>

#include <xer/binary.h>
#include <xer/stdio.h>

// XER_EXAMPLE_BEGIN: binary_basic
//
// This example splits and composes fixed-width unsigned integers, uses
// xer::byteswap, reverses bit order, and calculates simple checksums.
//
// Expected output:
// high_u8: 12
// low_u8: 34
// make_u16: 1234
// byteswap u32: 78563412
// reverse_bits u8: 48
// checksum8: 0a
// checksum_xor16_be: 0206
// checksum32_le: 04030201
// crc16: bb3d
// crc32: cbf43926
// bin2hex: 01020304
// md5 abc: 900150983cd24fb0d6963f7d28e17f72

auto main() -> int
{
    constexpr std::uint16_t value16 = 0x1234u;
    constexpr std::uint32_t value32 = 0x12345678u;
    constexpr std::uint8_t bits8 = 0b00010010u;
    constexpr std::array<std::byte, 4> bytes{
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0x04},
    };
    constexpr std::array<std::byte, 9> crc_bytes{
        std::byte{'1'},
        std::byte{'2'},
        std::byte{'3'},
        std::byte{'4'},
        std::byte{'5'},
        std::byte{'6'},
        std::byte{'7'},
        std::byte{'8'},
        std::byte{'9'},
    };
    constexpr std::array<std::byte, 3> md5_bytes{
        std::byte{'a'},
        std::byte{'b'},
        std::byte{'c'},
    };

    if (!xer::printf(u8"high_u8: %02x\n", xer::high_u8(value16)).has_value()) {
        return 1;
    }

    if (!xer::printf(u8"low_u8: %02x\n", xer::low_u8(value16)).has_value()) {
        return 1;
    }

    if (!xer::printf(
        u8"make_u16: %04x\n",
        xer::make_u16(xer::high_u8(value16), xer::low_u8(value16))).has_value()) {
        return 1;
    }

    if (!xer::printf(u8"byteswap u32: %08x\n", xer::byteswap(value32)).has_value()) {
        return 1;
    }

    if (!xer::printf(u8"reverse_bits u8: %02x\n", xer::reverse_bits(bits8)).has_value()) {
        return 1;
    }

    if (!xer::printf(
        u8"checksum8: %02x\n",
        xer::checksum8(std::span<const std::byte>(bytes))).has_value()) {
        return 1;
    }

    if (!xer::printf(
        u8"checksum_xor16_be: %04x\n",
        xer::checksum_xor16(
            std::span<const std::byte>(bytes),
            xer::byte_order::big_endian)).has_value()) {
        return 1;
    }

    if (!xer::printf(
        u8"checksum32_le: %08x\n",
        xer::checksum32(
            std::span<const std::byte>(bytes),
            xer::byte_order::little_endian)).has_value()) {
        return 1;
    }

    if (!xer::printf(
        u8"crc16: %04x\n",
        xer::crc16(std::span<const std::byte>(crc_bytes))).has_value()) {
        return 1;
    }

    if (!xer::printf(
        u8"crc32: %08x\n",
        xer::crc32(std::span<const std::byte>(crc_bytes))).has_value()) {
        return 1;
    }

    const auto hex = xer::bin2hex(std::span<const std::byte>(bytes));
    if (!xer::printf(u8"bin2hex: %@\n", hex).has_value()) {
        return 1;
    }

    const auto digest = xer::md5(std::span<const std::byte>(md5_bytes));
    const auto digest_hex = xer::bin2hex(digest.begin(), digest.end());
    if (!xer::printf(u8"md5 abc: %@\n", digest_hex).has_value()) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: binary_basic
