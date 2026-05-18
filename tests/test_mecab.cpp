/**
 * @file tests/test_mecab.cpp
 * @brief Tests for MeCab-based raw morphological analysis.
 */

#include <string>
#include <string_view>
#include <vector>

#include <xer/assert.h>
#include <xer/error.h>
#include <xer/mecab.h>

namespace {

[[nodiscard]] auto join_surfaces(
    const std::vector<xer::mecab_token>& tokens) -> std::u8string
{
    std::u8string result;

    for (const auto& token : tokens) {
        result += token.surface;
    }

    return result;
}

void test_mecab_parse_empty_text()
{
    const auto result = xer::mecab_parse(u8"");

    xer_assert(result.has_value());
    xer_assert(result->empty());
}

void test_mecab_parse_returns_raw_tokens()
{
    constexpr std::u8string_view text = u8"私は猫です。";

    const auto result = xer::mecab_parse(text);

    xer_assert(result.has_value());
    xer_assert_not(result->empty());
    xer_assert_eq(join_surfaces(*result), text);

    for (const auto& token : *result) {
        xer_assert_not(token.surface.empty());
        xer_assert_not(token.feature.empty());
    }
}

void test_mecab_parse_keeps_surface_sequence()
{
    constexpr std::u8string_view text = u8"すもももももももものうち";

    const auto result = xer::mecab_parse(text);

    xer_assert(result.has_value());
    xer_assert_not(result->empty());
    xer_assert_eq(join_surfaces(*result), text);

    for (const auto& token : *result) {
        xer_assert_not(token.surface.empty());
        xer_assert_not(token.feature.empty());
    }
}

void test_mecab_parse_rejects_invalid_utf8()
{
    const std::u8string invalid {
        static_cast<char8_t>(0xE3),
        static_cast<char8_t>(0x81),
    };

    const auto result = xer::mecab_parse(invalid);

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::encoding_error);
}

} // namespace

auto main() -> int
{
    test_mecab_parse_empty_text();
    test_mecab_parse_returns_raw_tokens();
    test_mecab_parse_keeps_surface_sequence();
    test_mecab_parse_rejects_invalid_utf8();

    return 0;
}
