/**
 * @file tests/test_string_case.cpp
 * @brief Tests for xer::strtoctrans, xer::strtolower, and xer::strtoupper.
 */

#include <cstddef>
#include <string>
#include <string_view>

#include <xer/assert.h>
#include <xer/string.h>

namespace {

void test_strtolower_utf8_string_view_ascii() {
    constexpr std::u8string_view source = u8"HeLLo-123_!?";

    const auto result = xer::strtolower(source);

    xer_assert(result.has_value());
    xer_assert(*result == std::u8string(u8"hello-123_!?"));
}

void test_strtoupper_utf8_string_view_ascii() {
    constexpr std::u8string_view source = u8"HeLLo-123_!?";

    const auto result = xer::strtoupper(source);

    xer_assert(result.has_value());
    xer_assert(*result == std::u8string(u8"HELLO-123_!?"));
}

void test_strtolower_string_literal() {
    const auto result = xer::strtolower(u8"Hello");

    xer_assert(result.has_value());
    xer_assert(*result == std::u8string(u8"hello"));
}

void test_strtoupper_string_literal() {
    const auto result = xer::strtoupper(u8"Hello");

    xer_assert(result.has_value());
    xer_assert(*result == std::u8string(u8"HELLO"));
}

void test_strtoctrans_latin1_lower_utf8() {
    constexpr std::u8string_view source = u8"ÀÁÄÖÜ ẞ";

    const auto result = xer::strtoctrans(source, xer::ctrans_id::latin1_lower);

    xer_assert(result.has_value());
    xer_assert(*result == std::u8string(u8"àáäöü ß"));
}

void test_strtoctrans_latin1_upper_utf8() {
    constexpr std::u8string_view source = u8"àáäöü ß";

    const auto result = xer::strtoctrans(source, xer::ctrans_id::latin1_upper);

    xer_assert(result.has_value());
    xer_assert(*result == std::u8string(u8"ÀÁÄÖÜ ẞ"));
}

void test_strtolower_ascii_rejects_non_ascii_utf8() {
    constexpr std::u8string_view source = u8"ABCあ";

    const auto result = xer::strtolower(source);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_strtoupper_ascii_rejects_non_ascii_utf8() {
    constexpr std::u8string_view source = u8"abcあ";

    const auto result = xer::strtoupper(source);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_strtolower_char_string() {
    constexpr std::string_view source = "AbC";

    const auto result = xer::strtolower(source);

    xer_assert(result.has_value());
    xer_assert(*result == std::string("abc"));
}

void test_strtoupper_char_string() {
    constexpr std::string_view source = "AbC";

    const auto result = xer::strtoupper(source);

    xer_assert(result.has_value());
    xer_assert(*result == std::string("ABC"));
}

void test_strtolower_pointer_and_size_uses_size_directly() {
    constexpr char8_t source[] = {u8'A', u8'\0', u8'B', u8'\0'};

    const auto result = xer::strtolower(source, 4);

    xer_assert(result.has_value());
    xer_assert_eq(result->size(), static_cast<std::size_t>(4));
    xer_assert((*result)[0] == u8'a');
    xer_assert((*result)[1] == u8'\0');
    xer_assert((*result)[2] == u8'b');
    xer_assert((*result)[3] == u8'\0');
}

void test_strtolower_array_excludes_trailing_nul_only() {
    constexpr char8_t source[] = {u8'A', u8'\0', u8'B', u8'\0'};

    const auto result = xer::strtolower(source);

    xer_assert(result.has_value());
    xer_assert_eq(result->size(), static_cast<std::size_t>(3));
    xer_assert((*result)[0] == u8'a');
    xer_assert((*result)[1] == u8'\0');
    xer_assert((*result)[2] == u8'b');
}

void test_strtoupper_array_without_trailing_nul_uses_whole_array() {
    constexpr char8_t source[] = {u8'a', u8'b', u8'c'};

    const auto result = xer::strtoupper(source);

    xer_assert(result.has_value());
    xer_assert(*result == std::u8string(u8"ABC"));
}

void test_strtoctrans_utf16_latin1_upper() {
    constexpr std::u16string_view source = u"àßz";

    const auto result = xer::strtoctrans(source, xer::ctrans_id::latin1_upper);

    xer_assert(result.has_value());
    xer_assert(*result == std::u16string(u"ÀẞZ"));
}

void test_strtoctrans_utf32_latin1_lower() {
    constexpr std::u32string_view source = U"ÀẞZ";

    const auto result = xer::strtoctrans(source, xer::ctrans_id::latin1_lower);

    xer_assert(result.has_value());
    xer_assert(*result == std::u32string(U"àßz"));
}

void test_strtoctrans_unsigned_char_ascii() {
    constexpr unsigned char source[] = {
        static_cast<unsigned char>('A'),
        static_cast<unsigned char>('b'),
        static_cast<unsigned char>('C'),
    };

    const auto result = xer::strtoctrans(source, xer::ctrans_id::lower);

    xer_assert(result.has_value());
    xer_assert_eq(result->size(), static_cast<std::size_t>(3));
    xer_assert_eq((*result)[0], static_cast<unsigned char>('a'));
    xer_assert_eq((*result)[1], static_cast<unsigned char>('b'));
    xer_assert_eq((*result)[2], static_cast<unsigned char>('c'));
}

void test_strtoctrans_pointer_null_with_nonzero_size_is_error() {
    const char8_t* const source = nullptr;

    const auto result = xer::strtoctrans(source, 1, xer::ctrans_id::lower);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_strtoctrans_pointer_null_with_zero_size_is_empty() {
    const char8_t* const source = nullptr;

    const auto result = xer::strtoctrans(source, 0, xer::ctrans_id::lower);

    xer_assert(result.has_value());
    xer_assert(result->empty());
}

void test_strtoctrans_invalid_utf8_is_error() {
    constexpr char8_t source[] = {static_cast<char8_t>(0xC3)};

    const auto result = xer::strtoctrans(source, 1, xer::ctrans_id::lower);

    xer_assert(!result.has_value());
}

void test_strtoctrans_invalid_utf16_is_error() {
    constexpr char16_t source[] = {static_cast<char16_t>(0xD800)};

    const auto result = xer::strtoctrans(source, 1, xer::ctrans_id::lower);

    xer_assert(!result.has_value());
}

} // namespace

auto main() -> int {
    test_strtolower_utf8_string_view_ascii();
    test_strtoupper_utf8_string_view_ascii();
    test_strtolower_string_literal();
    test_strtoupper_string_literal();
    test_strtoctrans_latin1_lower_utf8();
    test_strtoctrans_latin1_upper_utf8();
    test_strtolower_ascii_rejects_non_ascii_utf8();
    test_strtoupper_ascii_rejects_non_ascii_utf8();
    test_strtolower_char_string();
    test_strtoupper_char_string();
    test_strtolower_pointer_and_size_uses_size_directly();
    test_strtolower_array_excludes_trailing_nul_only();
    test_strtoupper_array_without_trailing_nul_uses_whole_array();
    test_strtoctrans_utf16_latin1_upper();
    test_strtoctrans_utf32_latin1_lower();
    test_strtoctrans_unsigned_char_ascii();
    test_strtoctrans_pointer_null_with_nonzero_size_is_error();
    test_strtoctrans_pointer_null_with_zero_size_is_empty();
    test_strtoctrans_invalid_utf8_is_error();
    test_strtoctrans_invalid_utf16_is_error();
    return 0;
}
