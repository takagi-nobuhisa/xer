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
        xer_assert_not(token.features.項目.empty());
        xer_assert_eq(token.features.項目.front(), token.features.品詞);
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
        xer_assert_not(token.features.項目.empty());
        xer_assert_eq(token.features.項目.front(), token.features.品詞);
    }
}

void test_mecab_parse_output_splits_ipadic_features()
{
    constexpr std::u8string_view output =
        u8"私\t名詞,代名詞,一般,*,*,*,私,ワタシ,ワタシ\n"
        u8"は\t助詞,係助詞,*,*,*,*,は,ハ,ワ\n"
        u8"EOS\n";

    const auto result = xer::detail::mecab_parse_output(output);

    xer_assert(result.has_value());
    xer_assert_eq(result->size(), 2uz);

    const auto& first = result->at(0);
    xer_assert_eq(first.surface, u8"私");
    xer_assert_eq(first.feature, u8"名詞,代名詞,一般,*,*,*,私,ワタシ,ワタシ");
    xer_assert_eq(first.features.品詞, u8"名詞");
    xer_assert_eq(first.features.品詞細分類1, u8"代名詞");
    xer_assert_eq(first.features.品詞細分類2, u8"一般");
    xer_assert_eq(first.features.品詞細分類3, u8"*");
    xer_assert_eq(first.features.活用型, u8"*");
    xer_assert_eq(first.features.活用形, u8"*");
    xer_assert_eq(first.features.原形, u8"私");
    xer_assert_eq(first.features.読み, u8"ワタシ");
    xer_assert_eq(first.features.発音, u8"ワタシ");
    xer_assert_eq(first.features.項目.size(), 9uz);
    xer_assert_eq(first.features.項目.at(7), first.features.読み);

    const auto& second = result->at(1);
    xer_assert_eq(second.surface, u8"は");
    xer_assert_eq(second.features.品詞, u8"助詞");
    xer_assert_eq(second.features.品詞細分類1, u8"係助詞");
    xer_assert_eq(second.features.原形, u8"は");
    xer_assert_eq(second.features.読み, u8"ハ");
    xer_assert_eq(second.features.発音, u8"ワ");
}

void test_mecab_parse_output_preserves_short_features()
{
    constexpr std::u8string_view output =
        u8"未知語\t名詞,一般\n"
        u8"EOS\n";

    const auto result = xer::detail::mecab_parse_output(output);

    xer_assert(result.has_value());
    xer_assert_eq(result->size(), 1uz);

    const auto& token = result->at(0);
    xer_assert_eq(token.surface, u8"未知語");
    xer_assert_eq(token.feature, u8"名詞,一般");
    xer_assert_eq(token.features.品詞, u8"名詞");
    xer_assert_eq(token.features.品詞細分類1, u8"一般");
    xer_assert(token.features.品詞細分類2.empty());
    xer_assert(token.features.活用型.empty());
    xer_assert(token.features.原形.empty());
    xer_assert(token.features.読み.empty());
    xer_assert(token.features.発音.empty());
    xer_assert_eq(token.features.項目.size(), 2uz);
}

void test_mecab_parse_output_keeps_literal_feature_fields()
{
    constexpr std::u8string_view output =
        u8"記号\t名詞,読点,一般\n"
        u8"EOS\n";

    const auto result = xer::detail::mecab_parse_output(output);

    xer_assert(result.has_value());
    xer_assert_eq(result->size(), 1uz);

    const auto& token = result->at(0);
    xer_assert_eq(token.features.項目.size(), 3uz);
    xer_assert_eq(token.features.項目.at(0), u8"名詞");
    xer_assert_eq(token.features.項目.at(1), u8"読点");
    xer_assert_eq(token.features.項目.at(2), u8"一般");
    xer_assert_eq(token.features.品詞, u8"名詞");
    xer_assert_eq(token.features.品詞細分類1, u8"読点");
    xer_assert_eq(token.features.品詞細分類2, u8"一般");
}

void test_mecab_token_copy_keeps_features_independent()
{
    constexpr std::u8string_view output =
        u8"私\t名詞,代名詞,一般,*,*,*,私,ワタシ,ワタシ\n"
        u8"EOS\n";

    auto result = xer::detail::mecab_parse_output(output);

    xer_assert(result.has_value());
    xer_assert_eq(result->size(), 1uz);

    xer::mecab_token copied = result->at(0);
    result->at(0).feature = u8"変更後";
    result->at(0).features.品詞 = u8"変更後";
    result->at(0).features.項目.at(0) = u8"変更後";

    xer_assert_eq(copied.surface, u8"私");
    xer_assert_eq(copied.feature, u8"名詞,代名詞,一般,*,*,*,私,ワタシ,ワタシ");
    xer_assert_eq(copied.features.品詞, u8"名詞");
    xer_assert_eq(copied.features.品詞細分類1, u8"代名詞");
    xer_assert_eq(copied.features.読み, u8"ワタシ");
    xer_assert_eq(copied.features.項目.at(0), u8"名詞");
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
    test_mecab_parse_output_splits_ipadic_features();
    test_mecab_parse_output_preserves_short_features();
    test_mecab_parse_output_keeps_literal_feature_fields();
    test_mecab_token_copy_keeps_features_independent();
    test_mecab_parse_rejects_invalid_utf8();

    return 0;
}
