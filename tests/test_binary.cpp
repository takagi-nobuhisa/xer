#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <vector>

#include <xer/assert.h>
#include <xer/binary.h>
#include <xer/stdint.h>
#include <xer/stdio.h>

namespace {

void test_split_make_u16()
{
    constexpr std::uint16_t value = 0x1234u;

    static_assert(xer::high_u8(value) == 0x12u);
    static_assert(xer::low_u8(value) == 0x34u);
    static_assert(xer::make_u16(0x12u, 0x34u) == value);

    xer_assert_eq(xer::high_u8(value), 0x12u);
    xer_assert_eq(xer::low_u8(value), 0x34u);
    xer_assert_eq(xer::make_u16(0x12u, 0x34u), value);
}

void test_split_make_u32()
{
    constexpr std::uint32_t value = 0x12345678u;

    static_assert(xer::high_u16(value) == 0x1234u);
    static_assert(xer::low_u16(value) == 0x5678u);
    static_assert(xer::make_u32(0x1234u, 0x5678u) == value);

    xer_assert_eq(xer::high_u16(value), 0x1234u);
    xer_assert_eq(xer::low_u16(value), 0x5678u);
    xer_assert_eq(xer::make_u32(0x1234u, 0x5678u), value);
}

void test_split_make_u64()
{
    constexpr std::uint64_t value = 0x0123456789abcdefull;

    static_assert(xer::high_u32(value) == 0x01234567u);
    static_assert(xer::low_u32(value) == 0x89abcdefu);
    static_assert(xer::make_u64(0x01234567u, 0x89abcdefu) == value);

    xer_assert_eq(xer::high_u32(value), 0x01234567u);
    xer_assert_eq(xer::low_u32(value), 0x89abcdefu);
    xer_assert_eq(xer::make_u64(0x01234567u, 0x89abcdefu), value);
}

#if defined(__SIZEOF_INT128__)

void test_split_make_u128()
{
    constexpr xer::uint128_t value = xer::make_u128(
        0x0123456789abcdefull,
        0xfedcba9876543210ull);

    static_assert(xer::high_u64(value) == 0x0123456789abcdefull);
    static_assert(xer::low_u64(value) == 0xfedcba9876543210ull);
    static_assert(xer::make_u128(
        0x0123456789abcdefull,
        0xfedcba9876543210ull) == value);

    xer_assert_eq(xer::high_u64(value), 0x0123456789abcdefull);
    xer_assert_eq(xer::low_u64(value), 0xfedcba9876543210ull);
    xer_assert(xer::make_u128(
        0x0123456789abcdefull,
        0xfedcba9876543210ull) == value);
}

void test_byteswap_u128()
{
    constexpr xer::uint128_t value = xer::make_u128(
        0x0011223344556677ull,
        0x8899aabbccddeeffull);
    constexpr xer::uint128_t expected = xer::make_u128(
        0xffeeddccbbaa9988ull,
        0x7766554433221100ull);

    static_assert(xer::byteswap(value) == expected);
    xer_assert(xer::byteswap(value) == expected);
}

#endif

void test_byteswap_imported_from_std()
{
    constexpr std::uint32_t value = 0x12345678u;

    static_assert(xer::byteswap(value) == std::byteswap(value));
    static_assert(xer::byteswap(value) == 0x78563412u);

    xer_assert_eq(xer::byteswap(value), 0x78563412u);
}

void test_reverse_bits_u8()
{
    constexpr std::uint8_t value = 0b00010010u;
    constexpr std::uint8_t expected = 0b01001000u;

    static_assert(xer::reverse_bits(value) == expected);
    xer_assert_eq(xer::reverse_bits(value), expected);
}

void test_reverse_bits_u16()
{
    constexpr std::uint16_t value = 0x1234u;
    constexpr std::uint16_t expected = 0x2c48u;

    static_assert(xer::reverse_bits(value) == expected);
    xer_assert_eq(xer::reverse_bits(value), expected);
}

void test_reverse_bits_u32()
{
    constexpr std::uint32_t value = 0x12345678u;
    constexpr std::uint32_t expected = 0x1e6a2c48u;

    static_assert(xer::reverse_bits(value) == expected);
    xer_assert_eq(xer::reverse_bits(value), expected);
}

void test_reverse_bits_u64()
{
    constexpr std::uint64_t value = 0x0123456789abcdefull;
    constexpr std::uint64_t expected = 0xf7b3d591e6a2c480ull;

    static_assert(xer::reverse_bits(value) == expected);
    xer_assert_eq(xer::reverse_bits(value), expected);
}

#if defined(__SIZEOF_INT128__)

void test_reverse_bits_u128()
{
    constexpr xer::uint128_t value = xer::make_u128(
        0x0123456789abcdefull,
        0xfedcba9876543210ull);
    constexpr xer::uint128_t expected = xer::make_u128(
        0x084c2a6e195d3b7full,
        0xf7b3d591e6a2c480ull);

    static_assert(xer::reverse_bits(value) == expected);
    xer_assert(xer::reverse_bits(value) == expected);
}

#endif

void test_reverse_bits_round_trip()
{
    constexpr std::uint64_t value = 0x13579bdf2468ace0ull;

    static_assert(xer::reverse_bits(xer::reverse_bits(value)) == value);
    xer_assert_eq(xer::reverse_bits(xer::reverse_bits(value)), value);
}


void test_checksum_span()
{
    constexpr std::array<std::byte, 5> bytes{
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0x04},
        std::byte{0x05},
    };

    static_assert(xer::checksum_add8(std::span<const std::byte>(bytes)) == 0x0fu);
    static_assert(xer::checksum_xor8(std::span<const std::byte>(bytes)) == 0x01u);

    static_assert(xer::checksum_add16(
        std::span<const std::byte>(bytes),
        xer::byte_order::big_endian) == 0x0906u);
    static_assert(xer::checksum_add16(
        std::span<const std::byte>(bytes),
        xer::byte_order::little_endian) == 0x0609u);

    static_assert(xer::checksum_xor16(
        std::span<const std::byte>(bytes),
        xer::byte_order::big_endian) == 0x0706u);
    static_assert(xer::checksum_xor16(
        std::span<const std::byte>(bytes),
        xer::byte_order::little_endian) == 0x0607u);

    static_assert(xer::checksum_add32(
        std::span<const std::byte>(bytes),
        xer::byte_order::big_endian) == 0x06020304u);
    static_assert(xer::checksum_add32(
        std::span<const std::byte>(bytes),
        xer::byte_order::little_endian) == 0x04030206u);

    static_assert(xer::checksum_xor32(
        std::span<const std::byte>(bytes),
        xer::byte_order::big_endian) == 0x04020304u);
    static_assert(xer::checksum_xor32(
        std::span<const std::byte>(bytes),
        xer::byte_order::little_endian) == 0x04030204u);

    xer_assert_eq(xer::checksum_add8(std::span<const std::byte>(bytes)), 0x0fu);
    xer_assert_eq(xer::checksum_xor8(std::span<const std::byte>(bytes)), 0x01u);
    xer_assert_eq(xer::checksum_add16(
        std::span<const std::byte>(bytes),
        xer::byte_order::big_endian), 0x0906u);
    xer_assert_eq(xer::checksum_add16(
        std::span<const std::byte>(bytes),
        xer::byte_order::little_endian), 0x0609u);
    xer_assert_eq(xer::checksum_add32(
        std::span<const std::byte>(bytes),
        xer::byte_order::big_endian), 0x06020304u);
    xer_assert_eq(xer::checksum_add32(
        std::span<const std::byte>(bytes),
        xer::byte_order::little_endian), 0x04030206u);
}

void test_checksum_pointer()
{
    const std::array<std::byte, 4> bytes{
        std::byte{0x10},
        std::byte{0x20},
        std::byte{0x30},
        std::byte{0x40},
    };

    const auto add8 = xer::checksum_add8(bytes.data(), bytes.size());
    xer_assert(add8.has_value());
    xer_assert_eq(*add8, 0xa0u);

    const auto xor8 = xer::checksum_xor8(bytes.data(), bytes.size());
    xer_assert(xor8.has_value());
    xer_assert_eq(*xor8, 0x40u);

    const auto add16 = xer::checksum_add16(
        bytes.data(),
        bytes.size(),
        xer::byte_order::big_endian);
    xer_assert(add16.has_value());
    xer_assert_eq(*add16, 0x4060u);

    const auto add32 = xer::checksum_add32(
        bytes.data(),
        bytes.size(),
        xer::byte_order::big_endian);
    xer_assert(add32.has_value());
    xer_assert_eq(*add32, 0x10203040u);

    const auto invalid = xer::checksum_add8(nullptr, 1);
    xer_assert_not(invalid.has_value());
}

void test_checksum_iterators()
{
    const std::vector<unsigned char> bytes{0x01u, 0x02u, 0x03u, 0x04u};

    xer_assert_eq(xer::checksum_add8(bytes.begin(), bytes.end()), 0x0au);
    xer_assert_eq(xer::checksum_xor8(bytes.begin(), bytes.end()), 0x04u);
    xer_assert_eq(xer::checksum_add16(
        bytes.begin(),
        bytes.end(),
        xer::byte_order::big_endian), 0x0406u);
    xer_assert_eq(xer::checksum_xor16(
        bytes.begin(),
        bytes.end(),
        xer::byte_order::big_endian), 0x0206u);
    xer_assert_eq(xer::checksum_add32(
        bytes.begin(),
        bytes.end(),
        xer::byte_order::big_endian), 0x01020304u);
    xer_assert_eq(xer::checksum_xor32(
        bytes.begin(),
        bytes.end(),
        xer::byte_order::big_endian), 0x01020304u);
}

void test_checksum_file()
{
    const std::array<std::byte, 4> bytes{
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0x04},
    };

    const xer::path filename(u8"test_binary_checksum.tmp");
    const auto write_result = xer::file_put_contents(filename, std::span<const std::byte>(bytes));
    xer_assert(write_result.has_value());

    const auto add8 = xer::checksum_add8(filename);
    xer_assert(add8.has_value());
    xer_assert_eq(*add8, 0x0au);

    const auto xor8 = xer::checksum_xor8(filename);
    xer_assert(xor8.has_value());
    xer_assert_eq(*xor8, 0x04u);

    const auto add16 = xer::checksum_add16(filename, xer::byte_order::big_endian);
    xer_assert(add16.has_value());
    xer_assert_eq(*add16, 0x0406u);

    const auto xor32 = xer::checksum_xor32(filename, xer::byte_order::big_endian);
    xer_assert(xor32.has_value());
    xer_assert_eq(*xor32, 0x01020304u);

    const auto remove_result = xer::remove(filename);
    xer_assert(remove_result.has_value());
}

} // namespace

auto main() -> int
{
    test_split_make_u16();
    test_split_make_u32();
    test_split_make_u64();
#if defined(__SIZEOF_INT128__)
    test_split_make_u128();
    test_byteswap_u128();
#endif
    test_byteswap_imported_from_std();
    test_reverse_bits_u8();
    test_reverse_bits_u16();
    test_reverse_bits_u32();
    test_reverse_bits_u64();
#if defined(__SIZEOF_INT128__)
    test_reverse_bits_u128();
#endif
    test_reverse_bits_round_trip();
    test_checksum_span();
    test_checksum_pointer();
    test_checksum_iterators();
    test_checksum_file();

    return 0;
}
