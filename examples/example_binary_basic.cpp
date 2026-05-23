#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

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
// checksum_add8: 0a
// checksum_xor16_be: 0206
// checksum_add32_le: 04030201

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
        u8"checksum_add8: %02x\n",
        xer::checksum_add8(std::span<const std::byte>(bytes))).has_value()) {
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
        u8"checksum_add32_le: %08x\n",
        xer::checksum_add32(
            std::span<const std::byte>(bytes),
            xer::byte_order::little_endian)).has_value()) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: binary_basic
