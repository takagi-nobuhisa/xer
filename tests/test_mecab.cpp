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


[[nodiscard]] auto parse_mecab_test_tokens(
    std::u8string_view output) -> std::vector<xer::mecab_token>
{
    auto result = xer::detail::mecab_parse_output(output);

    xer_assert(result.has_value());
    return *result;
}

[[nodiscard]] auto phrase_surface(
    const std::vector<xer::mecab_token>& tokens,
    const xer::mecab_phrase& phrase) -> std::u8string
{
    std::u8string result;

    for (std::size_t i = 0; i < phrase.count; ++i) {
        result += tokens.at(phrase.index + i).surface;
    }

    return result;
}

void test_mecab_split_phrases_empty()
{
    const auto phrases = xer::mecab_split_phrases({});

    xer_assert(phrases.empty());
}

void test_mecab_split_phrases_basic_bunsetsu_and_symbols()
{
    const auto tokens = parse_mecab_test_tokens(
        u8"私\t名詞,代名詞,一般,*,*,*,私,ワタシ,ワタシ\n"
        u8"は\t助詞,係助詞,*,*,*,*,は,ハ,ワ\n"
        u8"明日\t名詞,副詞可能,*,*,*,*,明日,アシタ,アシタ\n"
        u8"、\t記号,読点,*,*,*,*,、,、,、\n"
        u8"学校\t名詞,一般,*,*,*,*,学校,ガッコウ,ガッコー\n"
        u8"へ\t助詞,格助詞,一般,*,*,*,へ,ヘ,エ\n"
        u8"行き\t動詞,自立,*,*,五段・カ行促音便,連用形,行く,イキ,イキ\n"
        u8"ます\t助動詞,*,*,*,特殊・マス,基本形,ます,マス,マス\n"
        u8"。\t記号,句点,*,*,*,*,。,。,。\n"
        u8"EOS\n");

    const auto phrases = xer::mecab_split_phrases(tokens);

    xer_assert_eq(phrases.size(), 6uz);

    xer_assert_eq(phrases.at(0).kind, xer::mecab_phrase_kind::bunsetsu);
    xer_assert_eq(phrases.at(0).index, 0uz);
    xer_assert_eq(phrases.at(0).count, 2uz);
    xer_assert_eq(phrase_surface(tokens, phrases.at(0)), u8"私は");

    xer_assert_eq(phrases.at(1).kind, xer::mecab_phrase_kind::bunsetsu);
    xer_assert_eq(phrases.at(1).index, 2uz);
    xer_assert_eq(phrases.at(1).count, 1uz);
    xer_assert_eq(phrase_surface(tokens, phrases.at(1)), u8"明日");

    xer_assert_eq(phrases.at(2).kind, xer::mecab_phrase_kind::symbol);
    xer_assert_eq(phrases.at(2).index, 3uz);
    xer_assert_eq(phrases.at(2).count, 1uz);
    xer_assert_eq(phrase_surface(tokens, phrases.at(2)), u8"、");

    xer_assert_eq(phrases.at(3).kind, xer::mecab_phrase_kind::bunsetsu);
    xer_assert_eq(phrases.at(3).index, 4uz);
    xer_assert_eq(phrases.at(3).count, 2uz);
    xer_assert_eq(phrase_surface(tokens, phrases.at(3)), u8"学校へ");

    xer_assert_eq(phrases.at(4).kind, xer::mecab_phrase_kind::bunsetsu);
    xer_assert_eq(phrases.at(4).index, 6uz);
    xer_assert_eq(phrases.at(4).count, 2uz);
    xer_assert_eq(phrase_surface(tokens, phrases.at(4)), u8"行きます");

    xer_assert_eq(phrases.at(5).kind, xer::mecab_phrase_kind::symbol);
    xer_assert_eq(phrases.at(5).index, 8uz);
    xer_assert_eq(phrases.at(5).count, 1uz);
    xer_assert_eq(phrase_surface(tokens, phrases.at(5)), u8"。");
}

void test_mecab_split_phrases_groups_consecutive_symbols()
{
    const auto tokens = parse_mecab_test_tokens(
        u8"えっ\t感動詞,*,*,*,*,*,えっ,エッ,エッ\n"
        u8"！\t記号,一般,*,*,*,*,！,！,！\n"
        u8"？\t記号,一般,*,*,*,*,？,？,？\n"
        u8"EOS\n");

    const auto phrases = xer::mecab_split_phrases(tokens);

    xer_assert_eq(phrases.size(), 2uz);
    xer_assert_eq(phrases.at(0).kind, xer::mecab_phrase_kind::bunsetsu);
    xer_assert_eq(phrases.at(0).index, 0uz);
    xer_assert_eq(phrases.at(0).count, 1uz);
    xer_assert_eq(phrase_surface(tokens, phrases.at(0)), u8"えっ");

    xer_assert_eq(phrases.at(1).kind, xer::mecab_phrase_kind::symbol);
    xer_assert_eq(phrases.at(1).index, 1uz);
    xer_assert_eq(phrases.at(1).count, 2uz);
    xer_assert_eq(phrase_surface(tokens, phrases.at(1)), u8"！？");
}

void test_mecab_split_phrases_keeps_compound_nouns()
{
    const auto tokens = parse_mecab_test_tokens(
        u8"形態素\t名詞,一般,*,*,*,*,形態素,ケイタイソ,ケイタイソ\n"
        u8"解析\t名詞,サ変接続,*,*,*,*,解析,カイセキ,カイセキ\n"
        u8"結果\t名詞,一般,*,*,*,*,結果,ケッカ,ケッカ\n"
        u8"を\t助詞,格助詞,一般,*,*,*,を,ヲ,ヲ\n"
        u8"使う\t動詞,自立,*,*,五段・ワ行促音便,基本形,使う,ツカウ,ツカウ\n"
        u8"EOS\n");

    const auto phrases = xer::mecab_split_phrases(tokens);

    xer_assert_eq(phrases.size(), 2uz);
    xer_assert_eq(phrases.at(0).kind, xer::mecab_phrase_kind::bunsetsu);
    xer_assert_eq(phrases.at(0).index, 0uz);
    xer_assert_eq(phrases.at(0).count, 4uz);
    xer_assert_eq(phrase_surface(tokens, phrases.at(0)), u8"形態素解析結果を");

    xer_assert_eq(phrases.at(1).kind, xer::mecab_phrase_kind::bunsetsu);
    xer_assert_eq(phrases.at(1).index, 4uz);
    xer_assert_eq(phrases.at(1).count, 1uz);
    xer_assert_eq(phrase_surface(tokens, phrases.at(1)), u8"使う");
}

void test_mecab_split_phrases_keeps_renyou_plus_independent_word()
{
    const auto tokens = parse_mecab_test_tokens(
        u8"読み\t動詞,自立,*,*,五段・マ行,連用形,読む,ヨミ,ヨミ\n"
        u8"方\t名詞,接尾,一般,*,*,*,方,カタ,カタ\n"
        u8"を\t助詞,格助詞,一般,*,*,*,を,ヲ,ヲ\n"
        u8"考える\t動詞,自立,*,*,一段,基本形,考える,カンガエル,カンガエル\n"
        u8"EOS\n");

    const auto phrases = xer::mecab_split_phrases(tokens);

    xer_assert_eq(phrases.size(), 2uz);
    xer_assert_eq(phrases.at(0).kind, xer::mecab_phrase_kind::bunsetsu);
    xer_assert_eq(phrases.at(0).index, 0uz);
    xer_assert_eq(phrases.at(0).count, 3uz);
    xer_assert_eq(phrase_surface(tokens, phrases.at(0)), u8"読み方を");

    xer_assert_eq(phrases.at(1).kind, xer::mecab_phrase_kind::bunsetsu);
    xer_assert_eq(phrases.at(1).index, 3uz);
    xer_assert_eq(phrases.at(1).count, 1uz);
    xer_assert_eq(phrase_surface(tokens, phrases.at(1)), u8"考える");
}

void test_mecab_split_phrases_keeps_prefix_with_following_word()
{
    const auto tokens = parse_mecab_test_tokens(
        u8"お\t接頭詞,名詞接続,*,*,*,*,お,オ,オ\n"
        u8"茶\t名詞,一般,*,*,*,*,茶,チャ,チャ\n"
        u8"を\t助詞,格助詞,一般,*,*,*,を,ヲ,ヲ\n"
        u8"飲む\t動詞,自立,*,*,五段・マ行,基本形,飲む,ノム,ノム\n"
        u8"EOS\n");

    const auto phrases = xer::mecab_split_phrases(tokens);

    xer_assert_eq(phrases.size(), 2uz);
    xer_assert_eq(phrases.at(0).kind, xer::mecab_phrase_kind::bunsetsu);
    xer_assert_eq(phrases.at(0).index, 0uz);
    xer_assert_eq(phrases.at(0).count, 3uz);
    xer_assert_eq(phrase_surface(tokens, phrases.at(0)), u8"お茶を");

    xer_assert_eq(phrases.at(1).kind, xer::mecab_phrase_kind::bunsetsu);
    xer_assert_eq(phrases.at(1).index, 3uz);
    xer_assert_eq(phrases.at(1).count, 1uz);
    xer_assert_eq(phrase_surface(tokens, phrases.at(1)), u8"飲む");
}

void test_mecab_split_phrases_handles_leading_symbols()
{
    const auto tokens = parse_mecab_test_tokens(
        u8"「\t記号,括弧開,*,*,*,*,「,「,「\n"
        u8"はい\t感動詞,*,*,*,*,*,はい,ハイ,ハイ\n"
        u8"」\t記号,括弧閉,*,*,*,*,」,」,」\n"
        u8"EOS\n");

    const auto phrases = xer::mecab_split_phrases(tokens);

    xer_assert_eq(phrases.size(), 3uz);
    xer_assert_eq(phrases.at(0).kind, xer::mecab_phrase_kind::symbol);
    xer_assert_eq(phrases.at(0).index, 0uz);
    xer_assert_eq(phrases.at(0).count, 1uz);
    xer_assert_eq(phrase_surface(tokens, phrases.at(0)), u8"「");

    xer_assert_eq(phrases.at(1).kind, xer::mecab_phrase_kind::bunsetsu);
    xer_assert_eq(phrases.at(1).index, 1uz);
    xer_assert_eq(phrases.at(1).count, 1uz);
    xer_assert_eq(phrase_surface(tokens, phrases.at(1)), u8"はい");

    xer_assert_eq(phrases.at(2).kind, xer::mecab_phrase_kind::symbol);
    xer_assert_eq(phrases.at(2).index, 2uz);
    xer_assert_eq(phrases.at(2).count, 1uz);
    xer_assert_eq(phrase_surface(tokens, phrases.at(2)), u8"」");
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

    test_mecab_split_phrases_empty();
    test_mecab_split_phrases_basic_bunsetsu_and_symbols();
    test_mecab_split_phrases_groups_consecutive_symbols();
    test_mecab_split_phrases_keeps_compound_nouns();
    test_mecab_split_phrases_keeps_renyou_plus_independent_word();
    test_mecab_split_phrases_keeps_prefix_with_following_word();
    test_mecab_split_phrases_handles_leading_symbols();
    test_mecab_parse_rejects_invalid_utf8();

    return 0;
}
