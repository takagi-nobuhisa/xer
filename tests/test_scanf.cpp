/**
 * @file tests/test_scanf.cpp
 * @brief Tests for xer/bits/scanf_format.h and xer/bits/scanf.h.
 */

#include <string>
#include <string_view>

#include <xer/assert.h>
#include <xer/bits/scanf.h>
#include <xer/bits/scanf_format.h>

namespace {

void test_parse_literal_and_whitespace() {
    const auto parsed = xer::detail::parse_scan_format(u8"abc  def");
    xer_assert(parsed.has_value());
    xer_assert_eq(parsed->tokens.size(), static_cast<std::size_t>(3));

    xer_assert(std::holds_alternative<xer::detail::scan_literal_token>(parsed->tokens[0]));
    xer_assert(std::holds_alternative<xer::detail::scan_whitespace_token>(parsed->tokens[1]));
    xer_assert(std::holds_alternative<xer::detail::scan_literal_token>(parsed->tokens[2]));

    const auto& first = std::get<xer::detail::scan_literal_token>(parsed->tokens[0]);
    const auto& third = std::get<xer::detail::scan_literal_token>(parsed->tokens[2]);

    xer_assert_eq(first.text, std::u8string(u8"abc"));
    xer_assert_eq(third.text, std::u8string(u8"def"));
}

void test_parse_simple_conversion() {
    const auto parsed = xer::detail::parse_scan_format(u8"%d %s %c");
    xer_assert(parsed.has_value());
    xer_assert_eq(parsed->argument_mode, xer::detail::scan_argument_mode_t::sequential);
    xer_assert_eq(parsed->tokens.size(), static_cast<std::size_t>(5));

    const auto& first = std::get<xer::detail::scan_conversion_token>(parsed->tokens[0]);
    const auto& second = std::get<xer::detail::scan_conversion_token>(parsed->tokens[2]);
    const auto& third = std::get<xer::detail::scan_conversion_token>(parsed->tokens[4]);

    xer_assert_eq(first.conversion, xer::detail::scan_conversion_t::d);
    xer_assert_eq(second.conversion, xer::detail::scan_conversion_t::s);
    xer_assert_eq(third.conversion, xer::detail::scan_conversion_t::c);
}

void test_parse_control_tokens() {
    const auto parsed1 = xer::detail::parse_scan_format(u8"%@ %d");
    xer_assert(parsed1.has_value());
    xer_assert_eq(parsed1->argument_mode, xer::detail::scan_argument_mode_t::sequential);
    xer_assert(std::holds_alternative<xer::detail::scan_control_token>(parsed1->tokens[0]));
    {
        const auto& token = std::get<xer::detail::scan_control_token>(parsed1->tokens[0]);
        xer_assert_eq(token.argument_mode, xer::detail::scan_argument_mode_t::sequential);
        xer_assert_eq(token.argument_index, static_cast<std::size_t>(0));
    }

    const auto parsed2 = xer::detail::parse_scan_format(u8"%2$d %1$@ %1$s");
    xer_assert(parsed2.has_value());
    xer_assert_eq(parsed2->argument_mode, xer::detail::scan_argument_mode_t::positional);
    xer_assert(std::holds_alternative<xer::detail::scan_control_token>(parsed2->tokens[2]));
    {
        const auto& token = std::get<xer::detail::scan_control_token>(parsed2->tokens[2]);
        xer_assert_eq(token.argument_mode, xer::detail::scan_argument_mode_t::positional);
        xer_assert_eq(token.argument_index, static_cast<std::size_t>(1));
    }
}

void test_parse_rejects_mixed_positional_and_nonpositional() {
    const auto result1 = xer::detail::parse_scan_format(u8"%2$d %s");
    xer_assert(!result1.has_value());

    const auto result2 = xer::detail::parse_scan_format(u8"%2$d %@ %d");
    xer_assert(!result2.has_value());
}

void test_parse_scanset() {
    const auto parsed = xer::detail::parse_scan_format(u8"%[^a-c] %[]x]");
    xer_assert(parsed.has_value());
    xer_assert_eq(parsed->tokens.size(), static_cast<std::size_t>(3));

    const auto& first = std::get<xer::detail::scan_conversion_token>(parsed->tokens[0]);
    xer_assert_eq(first.conversion, xer::detail::scan_conversion_t::scanset);
    xer_assert(first.scanset.negated);
    xer_assert(first.scanset.ascii_table[static_cast<unsigned char>('a')]);
    xer_assert(first.scanset.ascii_table[static_cast<unsigned char>('b')]);
    xer_assert(first.scanset.ascii_table[static_cast<unsigned char>('c')]);

    const auto& second = std::get<xer::detail::scan_conversion_token>(parsed->tokens[2]);
    xer_assert_eq(second.conversion, xer::detail::scan_conversion_t::scanset);
    xer_assert(!second.scanset.negated);
    xer_assert(second.scanset.ascii_table[static_cast<unsigned char>(']')]);
    xer_assert(second.scanset.ascii_table[static_cast<unsigned char>('x')]);
}

void test_parse_rejects_invalid_scanset() {
    const auto result = xer::detail::parse_scan_format(u8"%[abc");
    xer_assert(!result.has_value());
}

void test_sscanf_signed_and_unsigned() {
    int a = 0;
    unsigned int b = 0;
    unsigned int c = 0;
    unsigned int d = 0;

    const auto result = xer::sscanf(u8"-12 34 ff 17", u8"%d %u %x %o", &a, &b, &c, &d);
    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(4));

    xer_assert_eq(a, -12);
    xer_assert_eq(b, 34u);
    xer_assert_eq(c, 0xffu);
    xer_assert_eq(d, 15u);
}

void test_sscanf_floating_point() {
    double a = 0.0;
    float b = 0.0f;

    const auto result = xer::sscanf(u8"3.25 -1.5e2", u8"%f %g", &a, &b);
    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(2));

    xer_assert_eq(a, 3.25);
    xer_assert_eq(b, -150.0f);
}

void test_sscanf_string_and_char32() {
    std::u8string s;
    char32_t ch = U'\0';

    const auto result = xer::sscanf(u8"hello あ", u8"%s %c", &s, &ch);
    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(2));

    xer_assert_eq(s, std::u8string(u8"hello"));
    xer_assert_eq(ch, U'あ');
}

void test_sscanf_char_variants() {
    char c1 = '\0';
    signed char c2 = 0;
    unsigned char c3 = 0;
    char16_t c4 = 0;
    wchar_t c5 = 0;
    char8_t c6 = 0;

    const auto result =
        xer::sscanf(u8"A B C D E F", u8"%c %c %c %c %c %c", &c1, &c2, &c3, &c4, &c5, &c6);
    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(6));

    xer_assert_eq(c1, 'A');
    xer_assert_eq(c2, static_cast<signed char>('B'));
    xer_assert_eq(c3, static_cast<unsigned char>('C'));
    xer_assert_eq(c4, static_cast<char16_t>(U'D'));
    xer_assert_eq(c5, static_cast<wchar_t>(U'E'));
    xer_assert_eq(c6, static_cast<char8_t>(U'F'));
}

void test_sscanf_width() {
    std::u8string s;
    int value = 0;

    const auto result = xer::sscanf(u8"abc 12345", u8"%3s %2d", &s, &value);
    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(2));

    xer_assert_eq(s, std::u8string(u8"abc"));
    xer_assert_eq(value, 12);
}

void test_sscanf_scanset() {
    std::u8string a;
    std::u8string b;

    const auto result = xer::sscanf(u8"abc123,XYZ", u8"%[a-z0-9],%[^,]", &a, &b);
    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(2));

    xer_assert_eq(a, std::u8string(u8"abc123"));
    xer_assert_eq(b, std::u8string(u8"XYZ"));
}

void test_sscanf_suppressed_assignment() {
    int value = 0;

    const auto result = xer::sscanf(u8"10 20", u8"%*d %d", &value);
    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(1));
    xer_assert_eq(value, 20);
}

void test_sscanf_nullptr_discards_value() {
    int value = 0;

    const auto result = xer::sscanf(u8"10 20", u8"%d %d", static_cast<int*>(nullptr), &value);
    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(1));
    xer_assert_eq(value, 20);
}

void test_sscanf_positional_arguments() {
    int a = 0;
    int b = 0;
    std::u8string s;

    const auto result = xer::sscanf(u8"11 abc 22", u8"%3$u %2$s %1$d", &a, &s, &b);
    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(3));

    xer_assert_eq(a, 22);
    xer_assert_eq(s, std::u8string(u8"abc"));
    xer_assert_eq(b, 11);
}

void test_sscanf_positional_control_token() {
    int a = 0;
    int b = 0;

    const auto result = xer::sscanf(u8"10 20", u8"%2$d %1$@ %d", &a, &b);
    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(2));

    xer_assert_eq(a, 20);
    xer_assert_eq(b, 10);
}

void test_sscanf_string_wide_targets() {
    std::u8string utf8;
    std::u16string utf16;
    std::u32string utf32;
    std::wstring wide;

    const auto result = xer::sscanf(
        u8"alpha 猫 犬 鳥",
        u8"%s %s %s %s",
        &utf8,
        &utf16,
        &utf32,
        &wide);
    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(4));

    xer_assert_eq(utf8, std::u8string(u8"alpha"));
    xer_assert_eq(utf16, std::u16string(u"猫"));
    xer_assert_eq(utf32, std::u32string(U"犬"));
    xer_assert_eq(wide, std::wstring(L"鳥"));
}

void test_sscanf_scanset_wide_targets() {
    std::u16string utf16;
    std::u32string utf32;
    std::wstring wide;

    const auto result = xer::sscanf(
        u8"あいう えお かき",
        u8"%[あいう] %[えお] %[かき]",
        &utf16,
        &utf32,
        &wide);
    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(3));

    xer_assert_eq(utf16, std::u16string(u"あいう"));
    xer_assert_eq(utf32, std::u32string(U"えお"));
    xer_assert_eq(wide, std::wstring(L"かき"));
}


void test_sscanf_percent_literal() {
    int value = 0;

    const auto result = xer::sscanf(u8"% 42", u8"%% %d", &value);
    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(1));
    xer_assert_eq(value, 42);
}

void test_sscanf_match_failure_returns_partial_count() {
    int a = 0;
    int b = 0;

    const auto result = xer::sscanf(u8"10 xx", u8"%d %d", &a, &b);
    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(1));
    xer_assert_eq(a, 10);
    xer_assert_eq(b, 0);
}

void test_sscanf_size_modifier_affects_intermediate_type() {
    int a = 0;
    long long b = 0;
    unsigned int c = 0;

    const auto result = xer::sscanf(u8"300 255 65535", u8"%hhd %hhu %hu", &a, &b, &c);
    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(3));

    xer_assert_eq(a, static_cast<int>(static_cast<signed char>(300)));
    xer_assert_eq(b, static_cast<long long>(static_cast<unsigned char>(255)));
    xer_assert_eq(c, static_cast<unsigned int>(static_cast<unsigned short>(65535)));
}

void test_sscanf_utf8_scanset_nonascii_individual_chars() {
    std::u8string s;

    const auto result = xer::sscanf(u8"あいうx", u8"%[あいう]", &s);
    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(1));
    xer_assert_eq(s, std::u8string(u8"あいう"));
}

void test_sscanf_rejects_type_mismatch() {
    std::u8string s;

    const auto result = xer::sscanf(u8"123", u8"%d", &s);
    xer_assert(!result.has_value());
}

} // namespace

int main() {
    test_parse_literal_and_whitespace();
    test_parse_simple_conversion();
    test_parse_control_tokens();
    test_parse_rejects_mixed_positional_and_nonpositional();
    test_parse_scanset();
    test_parse_rejects_invalid_scanset();

    test_sscanf_signed_and_unsigned();
    test_sscanf_floating_point();
    test_sscanf_string_and_char32();
    test_sscanf_char_variants();
    test_sscanf_width();
    test_sscanf_scanset();
    test_sscanf_suppressed_assignment();
    test_sscanf_nullptr_discards_value();
    test_sscanf_positional_arguments();
    test_sscanf_positional_control_token();
    test_sscanf_percent_literal();
    test_sscanf_match_failure_returns_partial_count();
    test_sscanf_size_modifier_affects_intermediate_type();
    test_sscanf_utf8_scanset_nonascii_individual_chars();
    test_sscanf_rejects_type_mismatch();

    return 0;
}
