/**
 * @file tests/test_multibyte_string.cpp
 * @brief Runtime tests for multibyte string conversion functions.
 */

#include <cstddef>
#include <cstdint>

#include <xer/assert.h>
#include <xer/bits/mbstate.h>
#include <xer/bits/multibyte_string.h>

namespace {

[[nodiscard]] constexpr bool wchar_is_utf16() noexcept {
    return sizeof(wchar_t) == 2;
}

[[nodiscard]] constexpr bool wchar_is_utf32() noexcept {
    return sizeof(wchar_t) == 4;
}

void test_mbstate_default_constructed() {
    xer::mbstate_t state;

    xer_assert_eq(state.size, static_cast<std::uint8_t>(0));
    xer_assert(state.empty());
}

void test_mblen_utf8_ascii() {
    constexpr char8_t text[] = u8"A";

    const auto result = xer::mblen(text, 1);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(1));
}

void test_mblen_utf8_multibyte() {
    constexpr char8_t text[] = u8"あ";

    const auto result = xer::mblen(text, 3);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(3));
}

void test_mblen_cp932_single_byte() {
    constexpr unsigned char text[] = {0x41, 0x00};

    const auto result = xer::mblen(text, 1);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(1));
}

void test_mblen_cp932_double_byte() {
    constexpr unsigned char text[] = {0x82, 0xA0, 0x00}; // "あ" in CP932

    const auto result = xer::mblen(text, 2);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(2));
}

void test_mbtotc_utf8_ascii_to_char32() {
    constexpr char8_t text[] = u8"A";
    char32_t out = U'\0';

    const auto result = xer::mbtotc(&out, text, 1);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(1));
    xer_assert_eq(out, U'A');
}

void test_mbtotc_utf8_multibyte_to_char32() {
    constexpr char8_t text[] = u8"あ";
    char32_t out = U'\0';

    const auto result = xer::mbtotc(&out, text, 3);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(3));
    xer_assert_eq(out, U'あ');
}

void test_mbtotc_cp932_single_byte_to_char32() {
    constexpr unsigned char text[] = {0x41, 0x00};
    char32_t out = U'\0';

    const auto result = xer::mbtotc(&out, text, 1);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(1));
    xer_assert_eq(out, U'A');
}

void test_mbtotc_cp932_double_byte_to_char32() {
    constexpr unsigned char text[] = {0x82, 0xA0, 0x00}; // "あ" in CP932
    char32_t out = U'\0';

    const auto result = xer::mbtotc(&out, text, 2);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(2));
    xer_assert_eq(out, U'あ');
}

void test_mbtotc_utf8_ascii_to_wchar() {
    constexpr char8_t text[] = u8"A";
    wchar_t out = 0;

    const auto result = xer::mbtotc(&out, text, 1);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(1));
    xer_assert_eq(static_cast<char32_t>(out), U'A');
}

void test_mbtotc_utf8_multibyte_to_wchar() {
    constexpr char8_t text[] = u8"あ";
    wchar_t out = 0;

    const auto result = xer::mbtotc(&out, text, 3);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(3));

    if constexpr (sizeof(wchar_t) == 2) {
        xer_assert_eq(static_cast<char16_t>(out), u'あ');
    } else {
        xer_assert_eq(static_cast<char32_t>(out), U'あ');
    }
}

void test_mbtotc_null_output_returns_only_length() {
    constexpr char8_t text[] = u8"あ";

    const auto result = xer::mbtotc(static_cast<char32_t*>(nullptr), text, 3);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(3));
}

void test_mbtotc_invalid_utf8() {
    constexpr char8_t text[] = {
        static_cast<char8_t>(0x80),
        static_cast<char8_t>(0x00),
    };

    const auto result = xer::mbtotc(static_cast<char32_t*>(nullptr), text, 1);

    xer_assert_not(result.has_value());
}

void test_tctomb_utf8_ascii_from_char32() {
    char8_t out[4] = {};
    const auto result = xer::tctomb(out, 4, U'A');

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(1));
    xer_assert_eq(out[0], u8'A');
}

void test_tctomb_utf8_multibyte_from_char32() {
    char8_t out[4] = {};
    const auto result = xer::tctomb(out, 4, U'あ');

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(3));
    xer_assert_eq(out[0], u8"あ"[0]);
    xer_assert_eq(out[1], u8"あ"[1]);
    xer_assert_eq(out[2], u8"あ"[2]);
}

void test_tctomb_cp932_single_byte_from_char32() {
    unsigned char out[4] = {};
    const auto result = xer::tctomb(out, 4, U'A');

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(1));
    xer_assert_eq(out[0], static_cast<unsigned char>(0x41));
}

void test_tctomb_cp932_double_byte_from_char32() {
    unsigned char out[4] = {};
    const auto result = xer::tctomb(out, 4, U'あ');

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(2));
    xer_assert_eq(out[0], static_cast<unsigned char>(0x82));
    xer_assert_eq(out[1], static_cast<unsigned char>(0xA0));
}

void test_tctomb_utf8_from_wchar() {
    char8_t out[4] = {};
    wchar_t in = 0;

    if constexpr (sizeof(wchar_t) == 2) {
        in = static_cast<wchar_t>(u'あ');
    } else {
        in = static_cast<wchar_t>(U'あ');
    }

    const auto result = xer::tctomb(out, 4, in);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(3));
    xer_assert_eq(out[0], u8"あ"[0]);
    xer_assert_eq(out[1], u8"あ"[1]);
    xer_assert_eq(out[2], u8"あ"[2]);
}

void test_tctomb_null_output_returns_only_length() {
    const auto result = xer::tctomb(static_cast<char8_t*>(nullptr), 0, U'あ');

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(3));
}

void test_tctomb_buffer_too_small() {
    char8_t out[2] = {};
    const auto result = xer::tctomb(out, 2, U'あ');

    xer_assert_not(result.has_value());
}

void test_mbstotcs_utf8_to_char32_string() {
    constexpr char8_t text[] = u8"ABCあ";
    char32_t out[8] = {};

    const auto result = xer::mbstotcs(out, text, 8);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(6));
    xer_assert_eq(out[0], U'A');
    xer_assert_eq(out[1], U'B');
    xer_assert_eq(out[2], U'C');
    xer_assert_eq(out[3], U'あ');
    xer_assert_eq(out[4], U'\0');
}

void test_mbstotcs_cp932_to_char32_string() {
    constexpr unsigned char text[] = {
        0x41,
        0x42,
        0x43,
        0x82,
        0xA0,
        0x00,
    };
    char32_t out[8] = {};

    const auto result = xer::mbstotcs(out, text, 8);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(5));
    xer_assert_eq(out[0], U'A');
    xer_assert_eq(out[1], U'B');
    xer_assert_eq(out[2], U'C');
    xer_assert_eq(out[3], U'あ');
    xer_assert_eq(out[4], U'\0');
}

void test_mbstotcs_null_output_returns_only_length() {
    constexpr char8_t text[] = u8"ABCあ";

    const auto result = xer::mbstotcs(static_cast<char32_t*>(nullptr), text, 8);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(6));
}

void test_tcstombs_char32_to_utf8_string() {
    constexpr char32_t text[] = U"ABCあ";
    char8_t out[16] = {};

    const auto result = xer::tcstombs(out, 16, text);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(6));
    xer_assert_eq(out[0], u8'A');
    xer_assert_eq(out[1], u8'B');
    xer_assert_eq(out[2], u8'C');
    xer_assert_eq(out[3], u8"あ"[0]);
    xer_assert_eq(out[4], u8"あ"[1]);
    xer_assert_eq(out[5], u8"あ"[2]);
}

void test_tcstombs_char32_to_cp932_string() {
    constexpr char32_t text[] = U"ABCあ";
    unsigned char out[16] = {};

    const auto result = xer::tcstombs(out, 16, text);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(5));
    xer_assert_eq(out[0], static_cast<unsigned char>(0x41));
    xer_assert_eq(out[1], static_cast<unsigned char>(0x42));
    xer_assert_eq(out[2], static_cast<unsigned char>(0x43));
    xer_assert_eq(out[3], static_cast<unsigned char>(0x82));
    xer_assert_eq(out[4], static_cast<unsigned char>(0xA0));
}

void test_tcstombs_null_output_returns_only_length() {
    constexpr char32_t text[] = U"ABCあ";

    const auto result = xer::tcstombs(static_cast<char8_t*>(nullptr), 0, text);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(6));
}

void test_tcstombs_unrepresentable_in_cp932() {
    constexpr char32_t text[] = {U'😀', U'\0'};
    unsigned char out[16] = {};

    const auto result = xer::tcstombs(out, 16, text);

    xer_assert_not(result.has_value());
}

void test_stateful_mblen_utf8_complete_sequence() {
    constexpr char8_t text[] = u8"あ";
    xer::mbstate_t state;

    const auto result = xer::mblen(text, 3, &state);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(3));
}

void test_stateful_mbtotc_utf8_complete_sequence() {
    constexpr char8_t text[] = u8"あ";
    xer::mbstate_t state;
    char32_t out = U'\0';

    const auto result = xer::mbtotc(&out, text, 3, &state);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(3));
    xer_assert_eq(out, U'あ');
}

void test_stateful_tctomb_utf8_complete_sequence() {
    xer::mbstate_t state;
    char8_t out[4] = {};

    const auto result = xer::tctomb(out, 4, U'あ', &state);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(3));
    xer_assert_eq(out[0], u8"あ"[0]);
    xer_assert_eq(out[1], u8"あ"[1]);
    xer_assert_eq(out[2], u8"あ"[2]);
}

void test_stateful_mbstotcs_utf8_string() {
    constexpr char8_t text[] = u8"ABCあ";
    xer::mbstate_t state;
    char32_t out[8] = {};

    const auto result = xer::mbstotcs(out, text, 8, &state);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(6));
    xer_assert_eq(out[0], U'A');
    xer_assert_eq(out[1], U'B');
    xer_assert_eq(out[2], U'C');
    xer_assert_eq(out[3], U'あ');
    xer_assert_eq(out[4], U'\0');
}

void test_stateful_tcstombs_utf8_string() {
    constexpr char32_t text[] = U"ABCあ";
    xer::mbstate_t state;
    char8_t out[16] = {};

    const auto result = xer::tcstombs(out, 16, text, &state);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(6));
    xer_assert_eq(out[0], u8'A');
    xer_assert_eq(out[1], u8'B');
    xer_assert_eq(out[2], u8'C');
    xer_assert_eq(out[3], u8"あ"[0]);
    xer_assert_eq(out[4], u8"あ"[1]);
    xer_assert_eq(out[5], u8"あ"[2]);
}

} // namespace

int main() {
    test_mbstate_default_constructed();

    test_mblen_utf8_ascii();
    test_mblen_utf8_multibyte();
    test_mblen_cp932_single_byte();
    test_mblen_cp932_double_byte();

    test_mbtotc_utf8_ascii_to_char32();
    test_mbtotc_utf8_multibyte_to_char32();
    test_mbtotc_cp932_single_byte_to_char32();
    test_mbtotc_cp932_double_byte_to_char32();
    test_mbtotc_utf8_ascii_to_wchar();
    test_mbtotc_utf8_multibyte_to_wchar();
    test_mbtotc_null_output_returns_only_length();
    test_mbtotc_invalid_utf8();

    test_tctomb_utf8_ascii_from_char32();
    test_tctomb_utf8_multibyte_from_char32();
    test_tctomb_cp932_single_byte_from_char32();
    test_tctomb_cp932_double_byte_from_char32();
    test_tctomb_utf8_from_wchar();
    test_tctomb_null_output_returns_only_length();
    test_tctomb_buffer_too_small();

    test_mbstotcs_utf8_to_char32_string();
    test_mbstotcs_cp932_to_char32_string();
    test_mbstotcs_null_output_returns_only_length();

    test_tcstombs_char32_to_utf8_string();
    test_tcstombs_char32_to_cp932_string();
    test_tcstombs_null_output_returns_only_length();
    test_tcstombs_unrepresentable_in_cp932();

    test_stateful_mblen_utf8_complete_sequence();
    test_stateful_mbtotc_utf8_complete_sequence();
    test_stateful_tctomb_utf8_complete_sequence();
    test_stateful_mbstotcs_utf8_string();
    test_stateful_tcstombs_utf8_string();

    return 0;
}
