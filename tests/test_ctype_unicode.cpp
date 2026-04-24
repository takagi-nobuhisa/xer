/**
 * @file tests/test_ctype_unicode.cpp
 * @brief Tests for Unicode-related xer::isctype categories.
 */

#include <xer/assert.h>
#include <xer/ctype.h>

namespace {

void test_unicode_accepts_ascii() {
    xer_assert(xer::isctype(U'A', xer::ctype_id::unicode));
    xer_assert(xer::isctype(U'0', xer::ctype_id::unicode));
    xer_assert(xer::isctype(U' ', xer::ctype_id::unicode));
}

void test_unicode_accepts_bmp_non_ascii() {
    xer_assert(xer::isctype(U'あ', xer::ctype_id::unicode));
    xer_assert(xer::isctype(U'Ａ', xer::ctype_id::unicode));
    xer_assert(xer::isctype(U'ｶ', xer::ctype_id::unicode));
}

void test_unicode_accepts_non_bmp_scalar_value() {
    xer_assert(xer::isctype(U'𠮷', xer::ctype_id::unicode));
    xer_assert(xer::isctype(U'😀', xer::ctype_id::unicode));
}

void test_unicode_rejects_surrogates() {
    xer_assert_not(xer::isctype(
        static_cast<char32_t>(0xD800),
        xer::ctype_id::unicode));

    xer_assert_not(xer::isctype(
        static_cast<char32_t>(0xDBFF),
        xer::ctype_id::unicode));

    xer_assert_not(xer::isctype(
        static_cast<char32_t>(0xDC00),
        xer::ctype_id::unicode));

    xer_assert_not(xer::isctype(
        static_cast<char32_t>(0xDFFF),
        xer::ctype_id::unicode));
}

void test_unicode_rejects_values_above_unicode_range() {
    xer_assert_not(xer::isctype(
        static_cast<char32_t>(0x110000),
        xer::ctype_id::unicode));

    xer_assert_not(xer::isctype(
        static_cast<char32_t>(0xFFFFFFFFu),
        xer::ctype_id::unicode));
}

void test_unicode_bmp_accepts_ascii() {
    xer_assert(xer::isctype(U'A', xer::ctype_id::unicode_bmp));
    xer_assert(xer::isctype(U'0', xer::ctype_id::unicode_bmp));
    xer_assert(xer::isctype(U' ', xer::ctype_id::unicode_bmp));
}

void test_unicode_bmp_accepts_bmp_non_ascii() {
    xer_assert(xer::isctype(U'あ', xer::ctype_id::unicode_bmp));
    xer_assert(xer::isctype(U'Ａ', xer::ctype_id::unicode_bmp));
    xer_assert(xer::isctype(U'ｶ', xer::ctype_id::unicode_bmp));
}

void test_unicode_bmp_accepts_bmp_boundary_values() {
    xer_assert(xer::isctype(
        static_cast<char32_t>(0x0000),
        xer::ctype_id::unicode_bmp));

    xer_assert(xer::isctype(
        static_cast<char32_t>(0xD7FF),
        xer::ctype_id::unicode_bmp));

    xer_assert(xer::isctype(
        static_cast<char32_t>(0xE000),
        xer::ctype_id::unicode_bmp));

    xer_assert(xer::isctype(
        static_cast<char32_t>(0xFFFF),
        xer::ctype_id::unicode_bmp));
}

void test_unicode_bmp_rejects_surrogates() {
    xer_assert_not(xer::isctype(
        static_cast<char32_t>(0xD800),
        xer::ctype_id::unicode_bmp));

    xer_assert_not(xer::isctype(
        static_cast<char32_t>(0xDBFF),
        xer::ctype_id::unicode_bmp));

    xer_assert_not(xer::isctype(
        static_cast<char32_t>(0xDC00),
        xer::ctype_id::unicode_bmp));

    xer_assert_not(xer::isctype(
        static_cast<char32_t>(0xDFFF),
        xer::ctype_id::unicode_bmp));
}

void test_unicode_bmp_rejects_non_bmp_scalar_values() {
    xer_assert_not(xer::isctype(U'𠮷', xer::ctype_id::unicode_bmp));
    xer_assert_not(xer::isctype(U'😀', xer::ctype_id::unicode_bmp));

    xer_assert_not(xer::isctype(
        static_cast<char32_t>(0x10000),
        xer::ctype_id::unicode_bmp));

    xer_assert_not(xer::isctype(
        static_cast<char32_t>(0x10FFFF),
        xer::ctype_id::unicode_bmp));
}

void test_unicode_bmp_rejects_values_above_unicode_range() {
    xer_assert_not(xer::isctype(
        static_cast<char32_t>(0x110000),
        xer::ctype_id::unicode_bmp));

    xer_assert_not(xer::isctype(
        static_cast<char32_t>(0xFFFFFFFFu),
        xer::ctype_id::unicode_bmp));
}

} // namespace

auto main() -> int {
    test_unicode_accepts_ascii();
    test_unicode_accepts_bmp_non_ascii();
    test_unicode_accepts_non_bmp_scalar_value();
    test_unicode_rejects_surrogates();
    test_unicode_rejects_values_above_unicode_range();

    test_unicode_bmp_accepts_ascii();
    test_unicode_bmp_accepts_bmp_non_ascii();
    test_unicode_bmp_accepts_bmp_boundary_values();
    test_unicode_bmp_rejects_surrogates();
    test_unicode_bmp_rejects_non_bmp_scalar_values();
    test_unicode_bmp_rejects_values_above_unicode_range();

    return 0;
}
