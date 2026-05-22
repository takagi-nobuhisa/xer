/**
 * @file tests/test_braille_chars.cpp
 * @brief Tests for basic one-character braille conversion helpers.
 */

#include <string_view>

#include <xer/assert.h>
#include <xer/braille.h>
#include <xer/error.h>

namespace {

void assert_braille_result_eq(
    const xer::result<std::u8string_view>& result,
    std::u8string_view expected)
{
    xer_assert(result.has_value());
    xer_assert_eq(*result, expected);
}

void assert_invalid_argument(const xer::result<std::u8string_view>& result)
{
    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_alpha_to_braille_lowercase()
{
    assert_braille_result_eq(xer::braille::alpha_to_braille(U'a'), u8"⠁");
    assert_braille_result_eq(xer::braille::alpha_to_braille(U'b'), u8"⠃");
    assert_braille_result_eq(xer::braille::alpha_to_braille(U'c'), u8"⠉");
    assert_braille_result_eq(xer::braille::alpha_to_braille(U'j'), u8"⠚");
    assert_braille_result_eq(xer::braille::alpha_to_braille(U'k'), u8"⠅");
    assert_braille_result_eq(xer::braille::alpha_to_braille(U'w'), u8"⠺");
    assert_braille_result_eq(xer::braille::alpha_to_braille(U'z'), u8"⠵");
}

void test_alpha_to_braille_uppercase()
{
    assert_braille_result_eq(xer::braille::alpha_to_braille(U'A'), u8"⠁");
    assert_braille_result_eq(xer::braille::alpha_to_braille(U'J'), u8"⠚");
    assert_braille_result_eq(xer::braille::alpha_to_braille(U'W'), u8"⠺");
    assert_braille_result_eq(xer::braille::alpha_to_braille(U'Z'), u8"⠵");
}

void test_alpha_to_braille_rejects_non_alpha()
{
    assert_invalid_argument(xer::braille::alpha_to_braille(U'0'));
    assert_invalid_argument(xer::braille::alpha_to_braille(U'-'));
    assert_invalid_argument(xer::braille::alpha_to_braille(U'あ'));
}

void test_digit_to_braille()
{
    assert_braille_result_eq(xer::braille::digit_to_braille(U'1'), u8"⠁");
    assert_braille_result_eq(xer::braille::digit_to_braille(U'2'), u8"⠃");
    assert_braille_result_eq(xer::braille::digit_to_braille(U'3'), u8"⠉");
    assert_braille_result_eq(xer::braille::digit_to_braille(U'9'), u8"⠊");
    assert_braille_result_eq(xer::braille::digit_to_braille(U'0'), u8"⠚");
}

void test_digit_to_braille_rejects_non_digit()
{
    assert_invalid_argument(xer::braille::digit_to_braille(U'a'));
    assert_invalid_argument(xer::braille::digit_to_braille(U'-'));
    assert_invalid_argument(xer::braille::digit_to_braille(U'１'));
}

void test_alnum_to_braille()
{
    assert_braille_result_eq(xer::braille::alnum_to_braille(U'a'), u8"⠁");
    assert_braille_result_eq(xer::braille::alnum_to_braille(U'A'), u8"⠁");
    assert_braille_result_eq(xer::braille::alnum_to_braille(U'z'), u8"⠵");
    assert_braille_result_eq(xer::braille::alnum_to_braille(U'1'), u8"⠁");
    assert_braille_result_eq(xer::braille::alnum_to_braille(U'0'), u8"⠚");
}

void test_alnum_to_braille_rejects_non_alnum()
{
    assert_invalid_argument(xer::braille::alnum_to_braille(U'-'));
    assert_invalid_argument(xer::braille::alnum_to_braille(U' '));
    assert_invalid_argument(xer::braille::alnum_to_braille(U'あ'));
}

void test_punct_to_braille()
{
    assert_braille_result_eq(xer::braille::punct_to_braille(U','), u8"⠂");
    assert_braille_result_eq(xer::braille::punct_to_braille(U';'), u8"⠆");
    assert_braille_result_eq(xer::braille::punct_to_braille(U':'), u8"⠒");
    assert_braille_result_eq(xer::braille::punct_to_braille(U'.'), u8"⠲");
    assert_braille_result_eq(xer::braille::punct_to_braille(U'!'), u8"⠖");
    assert_braille_result_eq(xer::braille::punct_to_braille(U'('), u8"⠶");
    assert_braille_result_eq(xer::braille::punct_to_braille(U')'), u8"⠶");
    assert_braille_result_eq(xer::braille::punct_to_braille(U'?'), u8"⠦");
    assert_braille_result_eq(xer::braille::punct_to_braille(U'“'), u8"⠦");
    assert_braille_result_eq(xer::braille::punct_to_braille(U'*'), u8"⠔");
    assert_braille_result_eq(xer::braille::punct_to_braille(U'”'), u8"⠴");
    assert_braille_result_eq(xer::braille::punct_to_braille(U'\''), u8"⠄");
    assert_braille_result_eq(xer::braille::punct_to_braille(U'-'), u8"⠤");
    assert_braille_result_eq(xer::braille::punct_to_braille(U'‐'), u8"⠤");
}

void test_punct_to_braille_rejects_unsupported_input()
{
    assert_invalid_argument(xer::braille::punct_to_braille(U'a'));
    assert_invalid_argument(xer::braille::punct_to_braille(U'1'));
    assert_invalid_argument(xer::braille::punct_to_braille(U' '));
    assert_invalid_argument(xer::braille::punct_to_braille(U'"'));
    assert_invalid_argument(xer::braille::punct_to_braille(U'。'));
}

} // namespace

auto main() -> int
{
    test_alpha_to_braille_lowercase();
    test_alpha_to_braille_uppercase();
    test_alpha_to_braille_rejects_non_alpha();
    test_digit_to_braille();
    test_digit_to_braille_rejects_non_digit();
    test_alnum_to_braille();
    test_alnum_to_braille_rejects_non_alnum();
    test_punct_to_braille();
    test_punct_to_braille_rejects_unsupported_input();
    return 0;
}
