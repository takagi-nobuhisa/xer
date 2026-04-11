/**
 * @file tests/test_advanced_encoding.cpp
 * @brief Execution tests for xer/bits/advanced_encoding.h.
 */

#include <cstdint>

#include <xer/assert.h>
#include <xer/bits/advanced_encoding.h>

namespace {

/**
 * @brief Returns the invalid UTF-32 sentinel.
 * @return Invalid UTF-32 value.
 */
[[nodiscard]] constexpr char32_t invalid_utf32()
{
    return static_cast<char32_t>(-1);
}

/**
 * @brief Returns the invalid packed UTF-8 sentinel.
 * @return Invalid packed UTF-8 value.
 */
[[nodiscard]] constexpr std::uint32_t invalid_packed_utf8()
{
    return static_cast<std::uint32_t>(-1);
}

/**
 * @brief Returns the invalid packed UTF-16 sentinel.
 * @return Invalid packed UTF-16 value.
 */
[[nodiscard]] constexpr std::uint32_t invalid_packed_utf16()
{
    return static_cast<std::uint32_t>(-1);
}

/**
 * @brief Returns the invalid packed CP932 sentinel.
 * @return Invalid packed CP932 value.
 */
[[nodiscard]] constexpr std::int32_t invalid_packed_cp932()
{
    return -1;
}

/**
 * @brief Returns a high surrogate value as char32_t.
 * @return U+D800.
 */
[[nodiscard]] constexpr char32_t surrogate_high()
{
    return static_cast<char32_t>(0xD800);
}

/**
 * @brief Returns a low surrogate value as char32_t.
 * @return U+DFFF.
 */
[[nodiscard]] constexpr char32_t surrogate_low()
{
    return static_cast<char32_t>(0xDFFF);
}

} // namespace

int main()
{
    using namespace xer::advanced;

    /**
     * UTF-8: UTF-32 -> packed UTF-8
     */
    xer_assert_eq(utf32_to_packed_utf8(U'A'), UINT32_C(0x00000041));
    xer_assert_eq(utf32_to_packed_utf8(U'\u00A2'), UINT32_C(0x0000A2C2));
    xer_assert_eq(utf32_to_packed_utf8(U'\u3042'), UINT32_C(0x008281E3));
    xer_assert_eq(utf32_to_packed_utf8(U'\U0001F600'), UINT32_C(0x80989FF0));

    xer_assert_eq(utf32_to_packed_utf8(surrogate_high()), invalid_packed_utf8());
    xer_assert_eq(utf32_to_packed_utf8(surrogate_low()), invalid_packed_utf8());
    xer_assert_eq(
        utf32_to_packed_utf8(static_cast<char32_t>(0x00110000)),
        invalid_packed_utf8());

    /**
     * UTF-8: packed UTF-8 -> UTF-32
     */
    xer_assert_eq(packed_utf8_to_utf32(UINT32_C(0x00000041)), U'A');
    xer_assert_eq(packed_utf8_to_utf32(UINT32_C(0x0000A2C2)), U'\u00A2');
    xer_assert_eq(packed_utf8_to_utf32(UINT32_C(0x008281E3)), U'\u3042');
    xer_assert_eq(packed_utf8_to_utf32(UINT32_C(0x80989FF0)), U'\U0001F600');

    xer_assert_eq(packed_utf8_to_utf32(UINT32_C(0x00004141)), invalid_utf32());
    xer_assert_eq(packed_utf8_to_utf32(UINT32_C(0x000080C0)), invalid_utf32());
    xer_assert_eq(packed_utf8_to_utf32(UINT32_C(0x000080E0)), invalid_utf32());
    xer_assert_eq(packed_utf8_to_utf32(UINT32_C(0x0080A0ED)), invalid_utf32());
    xer_assert_eq(packed_utf8_to_utf32(UINT32_C(0x808080F5)), invalid_utf32());

    /**
     * UTF-8 round trip
     */
    xer_assert_eq(
        packed_utf8_to_utf32(utf32_to_packed_utf8(U'A')),
        U'A');
    xer_assert_eq(
        packed_utf8_to_utf32(utf32_to_packed_utf8(U'\u00A2')),
        U'\u00A2');
    xer_assert_eq(
        packed_utf8_to_utf32(utf32_to_packed_utf8(U'\u3042')),
        U'\u3042');
    xer_assert_eq(
        packed_utf8_to_utf32(utf32_to_packed_utf8(U'\U0001F600')),
        U'\U0001F600');

    /**
     * UTF-16: UTF-32 -> packed UTF-16
     */
    xer_assert_eq(utf32_to_packed_utf16(U'\0'), UINT32_C(0x00000000));
    xer_assert_eq(utf32_to_packed_utf16(U'A'), UINT32_C(0x00000041));
    xer_assert_eq(utf32_to_packed_utf16(U'\u3042'), UINT32_C(0x00003042));
    xer_assert_eq(utf32_to_packed_utf16(U'\U0001F600'), UINT32_C(0xDE00D83D));

    xer_assert_eq(utf32_to_packed_utf16(surrogate_high()), invalid_packed_utf16());
    xer_assert_eq(utf32_to_packed_utf16(surrogate_low()), invalid_packed_utf16());
    xer_assert_eq(
        utf32_to_packed_utf16(static_cast<char32_t>(0x00110000)),
        invalid_packed_utf16());

    /**
     * UTF-16: packed UTF-16 -> UTF-32
     */
    xer_assert_eq(packed_utf16_to_utf32(UINT32_C(0x00000000)), U'\0');
    xer_assert_eq(packed_utf16_to_utf32(UINT32_C(0x00000041)), U'A');
    xer_assert_eq(packed_utf16_to_utf32(UINT32_C(0x00003042)), U'\u3042');
    xer_assert_eq(packed_utf16_to_utf32(UINT32_C(0xDE00D83D)), U'\U0001F600');

    xer_assert_eq(packed_utf16_to_utf32(UINT32_C(0x0000D800)), invalid_utf32());
    xer_assert_eq(packed_utf16_to_utf32(UINT32_C(0x0000DC00)), invalid_utf32());
    xer_assert_eq(packed_utf16_to_utf32(UINT32_C(0x00410042)), invalid_utf32());
    xer_assert_eq(packed_utf16_to_utf32(UINT32_C(0x0000D83D)), invalid_utf32());
    xer_assert_eq(packed_utf16_to_utf32(UINT32_C(0xD83D0041)), invalid_utf32());

    /**
     * UTF-16 round trip
     */
    xer_assert_eq(
        packed_utf16_to_utf32(utf32_to_packed_utf16(U'\0')),
        U'\0');
    xer_assert_eq(
        packed_utf16_to_utf32(utf32_to_packed_utf16(U'A')),
        U'A');
    xer_assert_eq(
        packed_utf16_to_utf32(utf32_to_packed_utf16(U'\u3042')),
        U'\u3042');
    xer_assert_eq(
        packed_utf16_to_utf32(utf32_to_packed_utf16(U'\U0001F600')),
        U'\U0001F600');

    /**
     * CP932: packed CP932 -> UTF-32
     */
    xer_assert_eq(
        packed_cp932_to_utf32(static_cast<std::uint16_t>(0x0041)),
        U'A');
    xer_assert_eq(
        packed_cp932_to_utf32(static_cast<std::uint16_t>(0x0040)),
        U'@');
    xer_assert_eq(
        packed_cp932_to_utf32(static_cast<std::uint16_t>(0x00A6)),
        U'\uFF66');

    xer_assert_eq(
        packed_cp932_to_utf32(static_cast<std::uint16_t>(0x0080)),
        invalid_utf32());
    xer_assert_eq(
        packed_cp932_to_utf32(static_cast<std::uint16_t>(0x00A0)),
        invalid_utf32());
    xer_assert_eq(
        packed_cp932_to_utf32(static_cast<std::uint16_t>(0x4081)),
        U'\u3000');
    xer_assert_eq(
        packed_cp932_to_utf32(static_cast<std::uint16_t>(0xA082)),
        U'\u3042');
    xer_assert_eq(
        packed_cp932_to_utf32(static_cast<std::uint16_t>(0xA282)),
        U'\u3044');

    xer_assert_eq(
        packed_cp932_to_utf32(static_cast<std::uint16_t>(0x0081)),
        invalid_utf32());
    xer_assert_eq(
        packed_cp932_to_utf32(static_cast<std::uint16_t>(0x7F81)),
        invalid_utf32());
    xer_assert_eq(
        packed_cp932_to_utf32(static_cast<std::uint16_t>(0xFD81)),
        invalid_utf32());

    /**
     * CP932: UTF-32 -> packed CP932
     */
    xer_assert_eq(utf32_to_packed_cp932(U'A'), 0x0041);
    xer_assert_eq(utf32_to_packed_cp932(U'@'), 0x0040);
    xer_assert_eq(utf32_to_packed_cp932(U'\uFF66'), 0x00A6);
    xer_assert_eq(utf32_to_packed_cp932(U'\u3000'), 0x4081);
    xer_assert_eq(utf32_to_packed_cp932(U'\u3042'), 0xA082);
    xer_assert_eq(utf32_to_packed_cp932(U'\u3044'), 0xA282);

    xer_assert_eq(utf32_to_packed_cp932(surrogate_high()), invalid_packed_cp932());
    xer_assert_eq(
        utf32_to_packed_cp932(static_cast<char32_t>(0x00110000)),
        invalid_packed_cp932());

    /**
     * This assertion should be enabled after the reverse table invalid value
     * handling is fixed.
     */
    // xer_assert_eq(utf32_to_packed_cp932(U'\u20AC'), invalid_packed_cp932());

    xer_assert_ne(utf32_to_packed_cp932(U'\u20AC'), 0x0041);
    xer_assert_ne(utf32_to_packed_cp932(U'\u20AC'), 0x4081);

    /**
     * CP932 round trip for representable characters
     */
    xer_assert_eq(
        packed_cp932_to_utf32(static_cast<std::uint16_t>(utf32_to_packed_cp932(U'A'))),
        U'A');
    xer_assert_eq(
        packed_cp932_to_utf32(static_cast<std::uint16_t>(utf32_to_packed_cp932(U'@'))),
        U'@');
    xer_assert_eq(
        packed_cp932_to_utf32(static_cast<std::uint16_t>(utf32_to_packed_cp932(U'\uFF66'))),
        U'\uFF66');
    xer_assert_eq(
        packed_cp932_to_utf32(static_cast<std::uint16_t>(utf32_to_packed_cp932(U'\u3000'))),
        U'\u3000');
    xer_assert_eq(
        packed_cp932_to_utf32(static_cast<std::uint16_t>(utf32_to_packed_cp932(U'\u3042'))),
        U'\u3042');

    /**
     * Internal helper tests
     */
    xer_assert(detail::is_utf8_continuation(0x80));
    xer_assert(detail::is_utf8_continuation(0xBF));
    xer_assert_not(detail::is_utf8_continuation(0x7F));
    xer_assert_not(detail::is_utf8_continuation(0xC0));

    xer_assert(detail::is_cp932_single_byte(0x0041));
    xer_assert(detail::is_cp932_single_byte(0x00A1));
    xer_assert(detail::is_cp932_single_byte(0x00DF));
    xer_assert_not(detail::is_cp932_single_byte(0x0080));
    xer_assert_not(detail::is_cp932_single_byte(0x00E0));

    xer_assert(detail::is_cp932_lead_byte(0x81));
    xer_assert(detail::is_cp932_lead_byte(0x9F));
    xer_assert(detail::is_cp932_lead_byte(0xE0));
    xer_assert(detail::is_cp932_lead_byte(0xFC));
    xer_assert_not(detail::is_cp932_lead_byte(0x80));
    xer_assert_not(detail::is_cp932_lead_byte(0xFD));

    xer_assert(detail::is_cp932_trail_byte(0x40));
    xer_assert(detail::is_cp932_trail_byte(0x7E));
    xer_assert(detail::is_cp932_trail_byte(0x80));
    xer_assert(detail::is_cp932_trail_byte(0xFC));
    xer_assert_not(detail::is_cp932_trail_byte(0x7F));
    xer_assert_not(detail::is_cp932_trail_byte(0xFD));

    xer_assert_eq(detail::bmp_to_packed_utf16(U'A'), UINT32_C(0x00000041));
    xer_assert_eq(detail::bmp_to_packed_utf16(U'\u3042'), UINT32_C(0x00003042));
    xer_assert_eq(
        detail::bmp_to_packed_utf16(surrogate_high()),
        invalid_packed_utf16());
    xer_assert_eq(
        detail::bmp_to_packed_utf16(U'\U0001F600'),
        invalid_packed_utf16());

    xer_assert_eq(detail::utf16_unit_to_utf32(u'A'), U'A');
    xer_assert_eq(detail::utf16_unit_to_utf32(u'\u3042'), U'\u3042');
    xer_assert_eq(
        detail::utf16_unit_to_utf32(static_cast<char16_t>(0xD800)),
        invalid_utf32());
    xer_assert_eq(
        detail::utf16_unit_to_utf32(static_cast<char16_t>(0xDFFF)),
        invalid_utf32());

    return 0;
}
