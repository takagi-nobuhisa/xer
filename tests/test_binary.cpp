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

#if defined(_MSC_VER)
#define XER_TEST_BINARY_STATIC_ASSERT(...) ((void)0)
#else
#define XER_TEST_BINARY_STATIC_ASSERT(...) static_assert(__VA_ARGS__)
#endif


void test_split_make_u16()
{
    constexpr std::uint16_t value = 0x1234u;

    XER_TEST_BINARY_STATIC_ASSERT(xer::high_u8(value) == 0x12u);
    XER_TEST_BINARY_STATIC_ASSERT(xer::low_u8(value) == 0x34u);
    XER_TEST_BINARY_STATIC_ASSERT(xer::make_u16(0x12u, 0x34u) == value);

    xer_assert_eq(xer::high_u8(value), 0x12u);
    xer_assert_eq(xer::low_u8(value), 0x34u);
    xer_assert_eq(xer::make_u16(0x12u, 0x34u), value);
}

void test_split_make_u32()
{
    constexpr std::uint32_t value = 0x12345678u;

    XER_TEST_BINARY_STATIC_ASSERT(xer::high_u16(value) == 0x1234u);
    XER_TEST_BINARY_STATIC_ASSERT(xer::low_u16(value) == 0x5678u);
    XER_TEST_BINARY_STATIC_ASSERT(xer::make_u32(0x1234u, 0x5678u) == value);

    xer_assert_eq(xer::high_u16(value), 0x1234u);
    xer_assert_eq(xer::low_u16(value), 0x5678u);
    xer_assert_eq(xer::make_u32(0x1234u, 0x5678u), value);
}

void test_split_make_u64()
{
    constexpr std::uint64_t value = 0x0123456789abcdefull;

    XER_TEST_BINARY_STATIC_ASSERT(xer::high_u32(value) == 0x01234567u);
    XER_TEST_BINARY_STATIC_ASSERT(xer::low_u32(value) == 0x89abcdefu);
    XER_TEST_BINARY_STATIC_ASSERT(xer::make_u64(0x01234567u, 0x89abcdefu) == value);

    xer_assert_eq(xer::high_u32(value), 0x01234567u);
    xer_assert_eq(xer::low_u32(value), 0x89abcdefu);
    xer_assert_eq(xer::make_u64(0x01234567u, 0x89abcdefu), value);
}

#if defined(XER_HAS_INT128)

void test_split_make_u128()
{
    constexpr xer::uint128_t value = xer::make_u128(
        0x0123456789abcdefull,
        0xfedcba9876543210ull);

    XER_TEST_BINARY_STATIC_ASSERT(xer::high_u64(value) == 0x0123456789abcdefull);
    XER_TEST_BINARY_STATIC_ASSERT(xer::low_u64(value) == 0xfedcba9876543210ull);
    XER_TEST_BINARY_STATIC_ASSERT(xer::make_u128(
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

    XER_TEST_BINARY_STATIC_ASSERT(xer::byteswap(value) == expected);
    xer_assert(xer::byteswap(value) == expected);
}

#endif

void test_byteswap_imported_from_std()
{
    constexpr std::uint32_t value = 0x12345678u;

    XER_TEST_BINARY_STATIC_ASSERT(xer::byteswap(value) == std::byteswap(value));
    XER_TEST_BINARY_STATIC_ASSERT(xer::byteswap(value) == 0x78563412u);

    xer_assert_eq(xer::byteswap(value), 0x78563412u);
}

void test_reverse_bits_u8()
{
    constexpr std::uint8_t value = 0b00010010u;
    constexpr std::uint8_t expected = 0b01001000u;

    XER_TEST_BINARY_STATIC_ASSERT(xer::reverse_bits(value) == expected);
    xer_assert_eq(xer::reverse_bits(value), expected);
}

void test_reverse_bits_u16()
{
    constexpr std::uint16_t value = 0x1234u;
    constexpr std::uint16_t expected = 0x2c48u;

    XER_TEST_BINARY_STATIC_ASSERT(xer::reverse_bits(value) == expected);
    xer_assert_eq(xer::reverse_bits(value), expected);
}

void test_reverse_bits_u32()
{
    constexpr std::uint32_t value = 0x12345678u;
    constexpr std::uint32_t expected = 0x1e6a2c48u;

    XER_TEST_BINARY_STATIC_ASSERT(xer::reverse_bits(value) == expected);
    xer_assert_eq(xer::reverse_bits(value), expected);
}

void test_reverse_bits_u64()
{
    constexpr std::uint64_t value = 0x0123456789abcdefull;
    constexpr std::uint64_t expected = 0xf7b3d591e6a2c480ull;

    XER_TEST_BINARY_STATIC_ASSERT(xer::reverse_bits(value) == expected);
    xer_assert_eq(xer::reverse_bits(value), expected);
}

#if defined(XER_HAS_INT128)

void test_reverse_bits_u128()
{
    constexpr xer::uint128_t value = xer::make_u128(
        0x0123456789abcdefull,
        0xfedcba9876543210ull);
    constexpr xer::uint128_t expected = xer::make_u128(
        0x084c2a6e195d3b7full,
        0xf7b3d591e6a2c480ull);

    XER_TEST_BINARY_STATIC_ASSERT(xer::reverse_bits(value) == expected);
    xer_assert(xer::reverse_bits(value) == expected);
}

#endif

void test_reverse_bits_round_trip()
{
    constexpr std::uint64_t value = 0x13579bdf2468ace0ull;

    XER_TEST_BINARY_STATIC_ASSERT(xer::reverse_bits(xer::reverse_bits(value)) == value);
    xer_assert_eq(xer::reverse_bits(xer::reverse_bits(value)), value);
}

#undef XER_TEST_BINARY_STATIC_ASSERT

} // namespace

auto main() -> int
{
    test_split_make_u16();
    test_split_make_u32();
    test_split_make_u64();
#if defined(XER_HAS_INT128)
    test_split_make_u128();
#endif
#if defined(XER_HAS_INT128)
    test_byteswap_u128();
#endif
    test_byteswap_imported_from_std();
    test_reverse_bits_u8();
    test_reverse_bits_u16();
    test_reverse_bits_u32();
    test_reverse_bits_u64();
#if defined(XER_HAS_INT128)
    test_reverse_bits_u128();
#endif
    test_reverse_bits_round_trip();

    return 0;
}
