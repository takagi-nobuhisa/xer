/**
 * @file tests/test_stdint_least_literals.cpp
 * @brief Tests for least-width integer literal suffixes.
 */

#include <type_traits>

#include <xer/assert.h>
#include <xer/stdint.h>

namespace {

void test_signed_least_literals_types() {
    using namespace xer::literals::integer_literals;

    constexpr auto i8 = 12_il8;
    constexpr auto i16 = 1234_il16;
    constexpr auto i32 = 123456_il32;
    constexpr auto i64 = 123456789_il64;

    static_assert(std::is_same_v<decltype(i8), const xer::int_least8_t>);
    static_assert(std::is_same_v<decltype(i16), const xer::int_least16_t>);
    static_assert(std::is_same_v<decltype(i32), const xer::int_least32_t>);
    static_assert(std::is_same_v<decltype(i64), const xer::int_least64_t>);

    xer_assert_eq(i8, static_cast<xer::int_least8_t>(12));
    xer_assert_eq(i16, static_cast<xer::int_least16_t>(1234));
    xer_assert_eq(i32, static_cast<xer::int_least32_t>(123456));
    xer_assert_eq(i64, static_cast<xer::int_least64_t>(123456789));
}

void test_unsigned_least_literals_types() {
    using namespace xer::literals::integer_literals;

    constexpr auto u8 = 12_ul8;
    constexpr auto u16 = 1234_ul16;
    constexpr auto u32 = 123456_ul32;
    constexpr auto u64 = 123456789_ul64;

    static_assert(std::is_same_v<decltype(u8), const xer::uint_least8_t>);
    static_assert(std::is_same_v<decltype(u16), const xer::uint_least16_t>);
    static_assert(std::is_same_v<decltype(u32), const xer::uint_least32_t>);
    static_assert(std::is_same_v<decltype(u64), const xer::uint_least64_t>);

    xer_assert_eq(u8, static_cast<xer::uint_least8_t>(12));
    xer_assert_eq(u16, static_cast<xer::uint_least16_t>(1234));
    xer_assert_eq(u32, static_cast<xer::uint_least32_t>(123456));
    xer_assert_eq(u64, static_cast<xer::uint_least64_t>(123456789));
}

void test_least_literals_accept_common_bases() {
    using namespace xer::literals::integer_literals;

    constexpr auto decimal = 255_ul16;
    constexpr auto hexadecimal = 0xFF_ul16;
    constexpr auto binary = 0b11111111_ul16;
    constexpr auto octal = 0377_ul16;

    static_assert(decimal == static_cast<xer::uint_least16_t>(255));
    static_assert(hexadecimal == static_cast<xer::uint_least16_t>(255));
    static_assert(binary == static_cast<xer::uint_least16_t>(255));
    static_assert(octal == static_cast<xer::uint_least16_t>(255));

    xer_assert_eq(decimal, hexadecimal);
    xer_assert_eq(decimal, binary);
    xer_assert_eq(decimal, octal);
}

void test_least_literals_accept_digit_separators() {
    using namespace xer::literals::integer_literals;

    constexpr auto signed_value = 1'234_il32;
    constexpr auto unsigned_value = 12'345_ul32;

    static_assert(signed_value == static_cast<xer::int_least32_t>(1234));
    static_assert(unsigned_value == static_cast<xer::uint_least32_t>(12345));

    xer_assert_eq(signed_value, static_cast<xer::int_least32_t>(1234));
    xer_assert_eq(unsigned_value, static_cast<xer::uint_least32_t>(12345));
}

void test_least_literals_at_representable_limits() {
    using namespace xer::literals::integer_literals;

    constexpr auto i8_max = 127_il8;
    constexpr auto u8_max = 255_ul8;
    constexpr auto i16_max = 32767_il16;
    constexpr auto u16_max = 65535_ul16;

    static_assert(i8_max == static_cast<xer::int_least8_t>(127));
    static_assert(u8_max == static_cast<xer::uint_least8_t>(255));
    static_assert(i16_max == static_cast<xer::int_least16_t>(32767));
    static_assert(u16_max == static_cast<xer::uint_least16_t>(65535));

    xer_assert_eq(i8_max, static_cast<xer::int_least8_t>(127));
    xer_assert_eq(u8_max, static_cast<xer::uint_least8_t>(255));
    xer_assert_eq(i16_max, static_cast<xer::int_least16_t>(32767));
    xer_assert_eq(u16_max, static_cast<xer::uint_least16_t>(65535));
}

void test_least_literals_can_be_used_in_constant_expressions() {
    using namespace xer::literals::integer_literals;

    constexpr xer::int_least32_t width = 40_il32;
    constexpr xer::int_least32_t height = 30_il32;
    constexpr xer::int_least32_t area = width * height;

    static_assert(area == static_cast<xer::int_least32_t>(1200));
    xer_assert_eq(area, static_cast<xer::int_least32_t>(1200));
}

} // namespace

auto main() -> int {
    test_signed_least_literals_types();
    test_unsigned_least_literals_types();
    test_least_literals_accept_common_bases();
    test_least_literals_accept_digit_separators();
    test_least_literals_at_representable_limits();
    test_least_literals_can_be_used_in_constant_expressions();
    return 0;
}
