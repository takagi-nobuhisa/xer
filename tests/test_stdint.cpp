/**
 * @file tests/test_stdint.cpp
 * @brief Tests for xer/stdint.h.
 */

#include <concepts>
#include <type_traits>

#include <xer/stdint.h>

using namespace xer::literals::integer_literals;

namespace {

/**
 * @brief Tests integer type aliases.
 */
void test_integer_type_aliases()
{
    static_assert(std::same_as<xer::int8_t, std::int8_t>);
    static_assert(std::same_as<xer::int16_t, std::int16_t>);
    static_assert(std::same_as<xer::int32_t, std::int32_t>);
    static_assert(std::same_as<xer::int64_t, std::int64_t>);

    static_assert(std::same_as<xer::uint8_t, std::uint8_t>);
    static_assert(std::same_as<xer::uint16_t, std::uint16_t>);
    static_assert(std::same_as<xer::uint32_t, std::uint32_t>);
    static_assert(std::same_as<xer::uint64_t, std::uint64_t>);

    static_assert(std::same_as<xer::int_least8_t, std::int_least8_t>);
    static_assert(std::same_as<xer::int_least16_t, std::int_least16_t>);
    static_assert(std::same_as<xer::int_least32_t, std::int_least32_t>);
    static_assert(std::same_as<xer::int_least64_t, std::int_least64_t>);

    static_assert(std::same_as<xer::uint_least8_t, std::uint_least8_t>);
    static_assert(std::same_as<xer::uint_least16_t, std::uint_least16_t>);
    static_assert(std::same_as<xer::uint_least32_t, std::uint_least32_t>);
    static_assert(std::same_as<xer::uint_least64_t, std::uint_least64_t>);

    static_assert(std::same_as<xer::int_fast8_t, std::int_fast8_t>);
    static_assert(std::same_as<xer::int_fast16_t, std::int_fast16_t>);
    static_assert(std::same_as<xer::int_fast32_t, std::int_fast32_t>);
    static_assert(std::same_as<xer::int_fast64_t, std::int_fast64_t>);

    static_assert(std::same_as<xer::uint_fast8_t, std::uint_fast8_t>);
    static_assert(std::same_as<xer::uint_fast16_t, std::uint_fast16_t>);
    static_assert(std::same_as<xer::uint_fast32_t, std::uint_fast32_t>);
    static_assert(std::same_as<xer::uint_fast64_t, std::uint_fast64_t>);

    static_assert(std::same_as<xer::intptr_t, std::intptr_t>);
    static_assert(std::same_as<xer::uintptr_t, std::uintptr_t>);

#if defined(__SIZEOF_INT128__)
    static_assert(sizeof(xer::int128_t) == 16);
    static_assert(sizeof(xer::uint128_t) == 16);
    static_assert(std::signed_integral<xer::int128_t>);
    static_assert(std::unsigned_integral<xer::uint128_t>);
    static_assert(std::same_as<xer::intmax_t, xer::int128_t>);
    static_assert(std::same_as<xer::uintmax_t, xer::uint128_t>);
#else
    static_assert(std::same_as<xer::intmax_t, std::intmax_t>);
    static_assert(std::same_as<xer::uintmax_t, std::uintmax_t>);
#endif
}

/**
 * @brief Tests min/max/bit-width helper templates.
 */
void test_numeric_limit_helpers()
{
    static_assert(xer::min_of<xer::int8_t> == static_cast<xer::int8_t>(-128));
    static_assert(xer::max_of<xer::int8_t> == static_cast<xer::int8_t>(127));
    static_assert(xer::max_of<xer::uint8_t> == static_cast<xer::uint8_t>(255));

    static_assert(xer::min_of<xer::int16_t> == static_cast<xer::int16_t>(-32768));
    static_assert(xer::max_of<xer::int16_t> == static_cast<xer::int16_t>(32767));
    static_assert(xer::max_of<xer::uint16_t> == static_cast<xer::uint16_t>(65535));

    static_assert(xer::bit_width_of<xer::int8_t> == 8);
    static_assert(xer::bit_width_of<xer::uint8_t> == 8);
    static_assert(xer::bit_width_of<xer::int16_t> == 16);
    static_assert(xer::bit_width_of<xer::uint16_t> == 16);
    static_assert(xer::bit_width_of<xer::int32_t> == 32);
    static_assert(xer::bit_width_of<xer::uint32_t> == 32);
    static_assert(xer::bit_width_of<xer::int64_t> == 64);
    static_assert(xer::bit_width_of<xer::uint64_t> == 64);

#if defined(__SIZEOF_INT128__)
    static_assert(xer::bit_width_of<xer::int128_t> == 128);
    static_assert(xer::bit_width_of<xer::uint128_t> == 128);
#endif
}

/**
 * @brief Tests integer user-defined literals for signed types.
 */
void test_signed_integer_literals()
{
    static_assert(std::same_as<decltype(0_i8), xer::int8_t>);
    static_assert(std::same_as<decltype(127_i8), xer::int8_t>);
    static_assert(0_i8 == static_cast<xer::int8_t>(0));
    static_assert(127_i8 == static_cast<xer::int8_t>(127));

    static_assert(std::same_as<decltype(0_i16), xer::int16_t>);
    static_assert(std::same_as<decltype(32767_i16), xer::int16_t>);
    static_assert(32767_i16 == static_cast<xer::int16_t>(32767));

    static_assert(std::same_as<decltype(0_i32), xer::int32_t>);
    static_assert(std::same_as<decltype(2147483647_i32), xer::int32_t>);
    static_assert(2147483647_i32 == static_cast<xer::int32_t>(2147483647));

    static_assert(std::same_as<decltype(0_i64), xer::int64_t>);
    static_assert(std::same_as<decltype(9223372036854775807_i64), xer::int64_t>);
    static_assert(
        9223372036854775807_i64 ==
        static_cast<xer::int64_t>(9223372036854775807LL));

    static_assert(0x7f_i8 == static_cast<xer::int8_t>(127));
    static_assert(0b1111111_i8 == static_cast<xer::int8_t>(127));
    static_assert(0177_i8 == static_cast<xer::int8_t>(127));

    static_assert(0x7fff_i16 == static_cast<xer::int16_t>(32767));
    static_assert(0x7fff'ffff_i32 == static_cast<xer::int32_t>(2147483647));
    static_assert(
        0x7fff'ffff'ffff'ffff_i64 ==
        static_cast<xer::int64_t>(9223372036854775807LL));

    static_assert(12'345_i32 == static_cast<xer::int32_t>(12345));
    static_assert(0b1010'0101_u8 == static_cast<xer::uint8_t>(0xa5));

#if defined(__SIZEOF_INT128__)
    static_assert(std::same_as<decltype(0_i128), xer::int128_t>);
    static_assert(0_i128 == static_cast<xer::int128_t>(0));
    static_assert(
        170141183460469231731687303715884105727_i128 ==
        xer::max_of<xer::int128_t>);
    static_assert(
        0x7fff'ffff'ffff'ffff'ffff'ffff'ffff'ffff_i128 ==
        xer::max_of<xer::int128_t>);
#endif
}

/**
 * @brief Tests integer user-defined literals for unsigned types.
 */
void test_unsigned_integer_literals()
{
    static_assert(std::same_as<decltype(0_u8), xer::uint8_t>);
    static_assert(std::same_as<decltype(255_u8), xer::uint8_t>);
    static_assert(255_u8 == static_cast<xer::uint8_t>(255));
    static_assert(0xff_u8 == static_cast<xer::uint8_t>(255));
    static_assert(0b11111111_u8 == static_cast<xer::uint8_t>(255));
    static_assert(0377_u8 == static_cast<xer::uint8_t>(255));

    static_assert(std::same_as<decltype(65535_u16), xer::uint16_t>);
    static_assert(65535_u16 == static_cast<xer::uint16_t>(65535));
    static_assert(0xffff_u16 == static_cast<xer::uint16_t>(65535));

    static_assert(std::same_as<decltype(4294967295_u32), xer::uint32_t>);
    static_assert(
        4294967295_u32 == static_cast<xer::uint32_t>(4294967295ULL));
    static_assert(0xffff'ffff_u32 == static_cast<xer::uint32_t>(4294967295ULL));

    static_assert(std::same_as<decltype(18446744073709551615_u64), xer::uint64_t>);
    static_assert(
        18446744073709551615_u64 ==
        static_cast<xer::uint64_t>(18446744073709551615ULL));
    static_assert(
        0xffff'ffff'ffff'ffff_u64 ==
        static_cast<xer::uint64_t>(18446744073709551615ULL));

#if defined(__SIZEOF_INT128__)
    static_assert(std::same_as<decltype(0_u128), xer::uint128_t>);
    static_assert(0_u128 == static_cast<xer::uint128_t>(0));
    static_assert(
        340282366920938463463374607431768211455_u128 ==
        xer::max_of<xer::uint128_t>);
    static_assert(
        0xffff'ffff'ffff'ffff'ffff'ffff'ffff'ffff_u128 ==
        xer::max_of<xer::uint128_t>);
#endif
}

/**
 * @brief Tests note-worthy behavior of unary minus after integer promotions.
 */
void test_unary_minus_behavior()
{
    static_assert(std::same_as<decltype(-1_i8), int>);
    static_assert(std::same_as<decltype(-1_i16), int>);
    static_assert(std::same_as<decltype(-1_i32), xer::int32_t>);
    static_assert(std::same_as<decltype(-1_i64), xer::int64_t>);

    static_assert(-1_i8 == -1);
    static_assert(-1_i16 == -1);
    static_assert(-1_i32 == static_cast<xer::int32_t>(-1));
    static_assert(-1_i64 == static_cast<xer::int64_t>(-1));

#if defined(__SIZEOF_INT128__)
    static_assert(std::same_as<decltype(-1_i128), xer::int128_t>);
    static_assert(-1_i128 == static_cast<xer::int128_t>(-1));
#endif
}

} // namespace

int main()
{
    test_integer_type_aliases();
    test_numeric_limit_helpers();
    test_signed_integer_literals();
    test_unsigned_integer_literals();
    test_unary_minus_behavior();

    return 0;
}
