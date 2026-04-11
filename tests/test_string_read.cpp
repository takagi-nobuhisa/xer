/**
 * @file tests/test_string_read.cpp
 * @brief Runtime tests for xer/bits/string_read.h.
 */

#include <cstddef>
#include <cstdint>
#include <string_view>

#include <xer/assert.h>
#include <xer/bits/string_read.h>
#include <xer/error.h>

namespace {

/**
 * @brief Tests strlen with a UTF-8 string.
 */
void test_strlen_u8_basic()
{
    constexpr std::u8string_view text = u8"hello";

    const auto result = xer::strlen(text);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(5));
}

/**
 * @brief Tests strlen with an empty UTF-8 string.
 */
void test_strlen_u8_empty()
{
    constexpr std::u8string_view text = u8"";

    const auto result = xer::strlen(text);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(0));
}

/**
 * @brief Tests strlen with an unsigned-char string.
 */
void test_strlen_unsigned_char_basic()
{
    constexpr unsigned char text[] = {
        static_cast<unsigned char>('A'),
        static_cast<unsigned char>('B'),
        static_cast<unsigned char>('C'),
    };
    const std::basic_string_view<unsigned char> view(text, 3);

    const auto result = xer::strlen(view);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(3));
}

/**
 * @brief Tests strcmp with equal strings.
 */
void test_strcmp_equal()
{
    constexpr std::u8string_view lhs = u8"alpha";
    constexpr std::u8string_view rhs = u8"alpha";

    const auto result = xer::strcmp(lhs, rhs);

    xer_assert(result.has_value());
    xer_assert_eq(*result, 0);
}

/**
 * @brief Tests strcmp when the left string is smaller.
 */
void test_strcmp_less()
{
    constexpr std::u8string_view lhs = u8"alpha";
    constexpr std::u8string_view rhs = u8"bravo";

    const auto result = xer::strcmp(lhs, rhs);

    xer_assert(result.has_value());
    xer_assert_eq(*result, -1);
}

/**
 * @brief Tests strcmp when the left string is larger because of length.
 */
void test_strcmp_greater_by_length()
{
    constexpr std::u8string_view lhs = u8"alphabet";
    constexpr std::u8string_view rhs = u8"alpha";

    const auto result = xer::strcmp(lhs, rhs);

    xer_assert(result.has_value());
    xer_assert_eq(*result, 1);
}

/**
 * @brief Tests strncmp with a matching prefix.
 */
void test_strncmp_prefix_equal()
{
    constexpr std::u8string_view lhs = u8"alphabet";
    constexpr std::u8string_view rhs = u8"alpha";
    constexpr std::size_t count = 5;

    const auto result = xer::strncmp(lhs, rhs, count);

    xer_assert(result.has_value());
    xer_assert_eq(*result, 0);
}

/**
 * @brief Tests strncmp with a difference inside the comparison range.
 */
void test_strncmp_difference_in_range()
{
    constexpr std::u8string_view lhs = u8"abcxef";
    constexpr std::u8string_view rhs = u8"abcyef";

    const auto result = xer::strncmp(lhs, rhs, 4);

    xer_assert(result.has_value());
    xer_assert_eq(*result, -1);
}

/**
 * @brief Tests strncmp when one side ends before the count.
 */
void test_strncmp_shorter_before_count()
{
    constexpr std::u8string_view lhs = u8"abc";
    constexpr std::u8string_view rhs = u8"abcd";

    const auto result = xer::strncmp(lhs, rhs, 4);

    xer_assert(result.has_value());
    xer_assert_eq(*result, -1);
}

/**
 * @brief Tests code-unit strchr with UTF-8 input.
 */
void test_strchr_code_unit_found()
{
    constexpr std::u8string_view text = u8"banana";

    const auto result = xer::strchr(text, u8'a');

    xer_assert(result.has_value());
    xer_assert_eq(*result.value(), u8'a');
    xer_assert_eq(result.value() - text.begin(), 1);
}

/**
 * @brief Tests code-unit strchr when the value is not found.
 */
void test_strchr_code_unit_not_found()
{
    constexpr std::u8string_view text = u8"banana";

    const auto result = xer::strchr(text, u8'z');

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::not_found);
}

/**
 * @brief Tests code-unit strrchr with UTF-8 input.
 */
void test_strrchr_code_unit_found()
{
    constexpr std::u8string_view text = u8"banana";

    const auto result = xer::strrchr(text, u8'a');

    xer_assert(result.has_value());
    xer_assert_eq(*result.value(), u8'a');
    xer_assert_eq(result.value() - text.begin(), 5);
}

/**
 * @brief Tests code-unit strrchr when the value is not found.
 */
void test_strrchr_code_unit_not_found()
{
    constexpr std::u8string_view text = u8"banana";

    const auto result = xer::strrchr(text, u8'x');

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::not_found);
}

/**
 * @brief Tests code-point strchr with UTF-8 input.
 */
void test_strchr_utf8_code_point_found()
{
    constexpr std::u8string_view text = u8"AあB";

    const auto result = xer::strchr(text, U'あ');

    xer_assert(result.has_value());
    xer_assert_eq(result.value() - text.begin(), 1);
    xer_assert_eq(*result.value(), u8"あ"[0]);
}

/**
 * @brief Tests code-point strchr with UTF-8 when not found.
 */
void test_strchr_utf8_code_point_not_found()
{
    constexpr std::u8string_view text = u8"AあB";

    const auto result = xer::strchr(text, U'漢');

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::not_found);
}

/**
 * @brief Tests code-point strchr with invalid UTF-8 input.
 */
void test_strchr_utf8_code_point_ilseq()
{
    constexpr char8_t text[] = {
        static_cast<char8_t>(0x80),
    };
    const std::u8string_view view(text, 1);

    const auto result = xer::strchr(view, U'A');

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::ilseq);
}

/**
 * @brief Tests code-point strchr with an invalid search code point.
 */
void test_strchr_utf8_invalid_code_point_argument()
{
    constexpr std::u8string_view text = u8"ABC";

    const auto result = xer::strchr(text, static_cast<char32_t>(0x110000));

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::ilseq);
}

/**
 * @brief Tests code-point strrchr with UTF-8 input.
 */
void test_strrchr_utf8_code_point_found()
{
    constexpr std::u8string_view text = u8"あAあB";

    const auto result = xer::strrchr(text, U'あ');

    xer_assert(result.has_value());
    xer_assert_eq(result.value() - text.begin(), 4);
    xer_assert_eq(*result.value(), u8"あ"[0]);
}

/**
 * @brief Tests code-point strrchr with invalid UTF-8 input.
 */
void test_strrchr_utf8_code_point_ilseq()
{
    constexpr char8_t text[] = {
        u8'A',
        static_cast<char8_t>(0xE3),
        static_cast<char8_t>(0x81),
    };
    const std::u8string_view view(text, 3);

    const auto result = xer::strrchr(view, U'あ');

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::ilseq);
}

/**
 * @brief Tests code-point strchr with UTF-16 input.
 */
void test_strchr_utf16_code_point_found()
{
    constexpr char16_t text[] = {u'A', u'\u3042', u'B'};
    const std::u16string_view view(text, 3);

    const auto result = xer::strchr(view, U'あ');

    xer_assert(result.has_value());
    xer_assert_eq(result.value() - view.begin(), 1);
    xer_assert_eq(*result.value(), u'あ');
}

/**
 * @brief Tests code-point strchr with an invalid UTF-16 sequence.
 */
void test_strchr_utf16_code_point_ilseq()
{
    constexpr char16_t text[] = {
        static_cast<char16_t>(0xD800),
    };
    const std::u16string_view view(text, 1);

    const auto result = xer::strchr(view, U'A');

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::ilseq);
}

/**
 * @brief Tests code-point strrchr with UTF-16 input.
 */
void test_strrchr_utf16_code_point_found()
{
    constexpr char16_t text[] = {u'\u3042', u'X', u'\u3042', u'Y'};
    const std::u16string_view view(text, 4);

    const auto result = xer::strrchr(view, U'あ');

    xer_assert(result.has_value());
    xer_assert_eq(result.value() - view.begin(), 2);
    xer_assert_eq(*result.value(), u'あ');
}

/**
 * @brief Tests code-point strchr with UTF-32 input.
 */
void test_strchr_utf32_code_point_found()
{
    constexpr char32_t text[] = {U'A', U'あ', U'B'};
    const std::u32string_view view(text, 3);

    const auto result = xer::strchr(view, U'あ');

    xer_assert(result.has_value());
    xer_assert_eq(result.value() - view.begin(), 1);
    xer_assert_eq(*result.value(), U'あ');
}

/**
 * @brief Tests code-point strchr with invalid UTF-32 input.
 */
void test_strchr_utf32_code_point_ilseq()
{
    constexpr char32_t text[] = {
        static_cast<char32_t>(0x110000),
    };
    const std::u32string_view view(text, 1);

    const auto result = xer::strchr(view, U'A');

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::ilseq);
}

/**
 * @brief Tests code-point strrchr with UTF-32 input.
 */
void test_strrchr_utf32_code_point_found()
{
    constexpr char32_t text[] = {U'あ', U'B', U'あ', U'C'};
    const std::u32string_view view(text, 4);

    const auto result = xer::strrchr(view, U'あ');

    xer_assert(result.has_value());
    xer_assert_eq(result.value() - view.begin(), 2);
    xer_assert_eq(*result.value(), U'あ');
}

/**
 * @brief Tests strstr with a found substring.
 */
void test_strstr_found()
{
    constexpr std::u8string_view text = u8"hello world";
    constexpr std::u8string_view pattern = u8"world";

    const auto result = xer::strstr(text, pattern);

    xer_assert(result.has_value());
    xer_assert_eq(result.value() - text.begin(), 6);
    xer_assert_eq(*result.value(), u8'w');
}

/**
 * @brief Tests strstr with an empty pattern.
 */
void test_strstr_empty_pattern()
{
    constexpr std::u8string_view text = u8"hello";
    constexpr std::u8string_view pattern = u8"";

    const auto result = xer::strstr(text, pattern);

    xer_assert(result.has_value());
    xer_assert_eq(result.value(), text.begin());
}

/**
 * @brief Tests strstr when the pattern is not found.
 */
void test_strstr_not_found()
{
    constexpr std::u8string_view text = u8"hello world";
    constexpr std::u8string_view pattern = u8"xer";

    const auto result = xer::strstr(text, pattern);

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::not_found);
}

/**
 * @brief Tests strpbrk with a found character.
 */
void test_strpbrk_found()
{
    constexpr std::u8string_view text = u8"token";
    constexpr std::u8string_view accept = u8"xyzke";

    const auto result = xer::strpbrk(text, accept);

    xer_assert(result.has_value());
    xer_assert_eq(result.value() - text.begin(), 2);
    xer_assert_eq(*result.value(), u8'k');
}

/**
 * @brief Tests strpbrk when no character matches.
 */
void test_strpbrk_not_found()
{
    constexpr std::u8string_view text = u8"token";
    constexpr std::u8string_view accept = u8"xyz";

    const auto result = xer::strpbrk(text, accept);

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::not_found);
}

/**
 * @brief Tests strspn with a matching prefix.
 */
void test_strspn_basic()
{
    constexpr std::u8string_view text = u8"abcde123";
    constexpr std::u8string_view accept = u8"abcde";

    const auto result = xer::strspn(text, accept);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(5));
}

/**
 * @brief Tests strspn with no initial match.
 */
void test_strspn_zero()
{
    constexpr std::u8string_view text = u8"123abc";
    constexpr std::u8string_view accept = u8"abc";

    const auto result = xer::strspn(text, accept);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(0));
}

/**
 * @brief Tests strcspn with a rejected character in the middle.
 */
void test_strcspn_basic()
{
    constexpr std::u8string_view text = u8"abcde123";
    constexpr std::u8string_view reject = u8"123";

    const auto result = xer::strcspn(text, reject);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(5));
}

/**
 * @brief Tests strcspn when the first character is rejected.
 */
void test_strcspn_zero()
{
    constexpr std::u8string_view text = u8"1abc";
    constexpr std::u8string_view reject = u8"123";

    const auto result = xer::strcspn(text, reject);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(0));
}

/**
 * @brief Tests generic functions with unsigned-char input.
 */
void test_unsigned_char_search_and_compare()
{
    constexpr unsigned char text[] = {
        static_cast<unsigned char>('A'),
        static_cast<unsigned char>('B'),
        static_cast<unsigned char>('C'),
        static_cast<unsigned char>('B'),
    };
    constexpr unsigned char pattern[] = {
        static_cast<unsigned char>('B'),
        static_cast<unsigned char>('C'),
    };

    const std::basic_string_view<unsigned char> text_view(text, 4);
    const std::basic_string_view<unsigned char> pattern_view(pattern, 2);

    const auto strcmp_result = xer::strcmp(text_view.substr(1, 2), pattern_view);
    const auto strchr_result = xer::strchr(text_view, static_cast<unsigned char>('B'));
    const auto strrchr_result = xer::strrchr(text_view, static_cast<unsigned char>('B'));
    const auto strstr_result = xer::strstr(text_view, pattern_view);

    xer_assert(strcmp_result.has_value());
    xer_assert_eq(*strcmp_result, 0);

    xer_assert(strchr_result.has_value());
    xer_assert_eq(strchr_result.value() - text_view.begin(), 1);

    xer_assert(strrchr_result.has_value());
    xer_assert_eq(strrchr_result.value() - text_view.begin(), 3);

    xer_assert(strstr_result.has_value());
    xer_assert_eq(strstr_result.value() - text_view.begin(), 1);
}

} // namespace

int main()
{
    test_strlen_u8_basic();
    test_strlen_u8_empty();
    test_strlen_unsigned_char_basic();

    test_strcmp_equal();
    test_strcmp_less();
    test_strcmp_greater_by_length();

    test_strncmp_prefix_equal();
    test_strncmp_difference_in_range();
    test_strncmp_shorter_before_count();

    test_strchr_code_unit_found();
    test_strchr_code_unit_not_found();
    test_strrchr_code_unit_found();
    test_strrchr_code_unit_not_found();

    test_strchr_utf8_code_point_found();
    test_strchr_utf8_code_point_not_found();
    test_strchr_utf8_code_point_ilseq();
    test_strchr_utf8_invalid_code_point_argument();
    test_strrchr_utf8_code_point_found();
    test_strrchr_utf8_code_point_ilseq();

    test_strchr_utf16_code_point_found();
    test_strchr_utf16_code_point_ilseq();
    test_strrchr_utf16_code_point_found();

    test_strchr_utf32_code_point_found();
    test_strchr_utf32_code_point_ilseq();
    test_strrchr_utf32_code_point_found();

    test_strstr_found();
    test_strstr_empty_pattern();
    test_strstr_not_found();

    test_strpbrk_found();
    test_strpbrk_not_found();

    test_strspn_basic();
    test_strspn_zero();
    test_strcspn_basic();
    test_strcspn_zero();

    test_unsigned_char_search_and_compare();

    return 0;
}
