#include <string_view>
#include <vector>

#include <xer/assert.h>
#include <xer/error.h>
#include <xer/unicode.h>

// XER_TEST_FEATURES: icu

namespace {

void assert_grapheme_cluster(
    const xer::result<xer::grapheme_cluster>& result,
    std::size_t offset,
    std::size_t size)
{
    xer_assert(result.has_value());
    xer_assert_eq(result->offset, offset);
    xer_assert_eq(result->size, size);
}

void test_next_grapheme_cluster_u8string_view()
{
    constexpr std::u8string_view text = u8"A\u0301B";

    assert_grapheme_cluster(xer::next_grapheme_cluster(text, 0), 0, 3);
    assert_grapheme_cluster(xer::next_grapheme_cluster(text, 3), 3, 1);

    const auto end = xer::next_grapheme_cluster(text, text.size());
    xer_assert_not(end.has_value());
    xer_assert_eq(end.error().code, xer::error_t::out_of_range);
}

void test_prev_grapheme_cluster_u8string_view()
{
    constexpr std::u8string_view text = u8"A\u0301B";

    assert_grapheme_cluster(xer::prev_grapheme_cluster(text, text.size()), 3, 1);
    assert_grapheme_cluster(xer::prev_grapheme_cluster(text, 3), 0, 3);

    const auto begin = xer::prev_grapheme_cluster(text, 0);
    xer_assert_not(begin.has_value());
    xer_assert_eq(begin.error().code, xer::error_t::out_of_range);
}

void test_grapheme_cluster_emoji_zwj_u8string_view()
{
    constexpr std::u8string_view text = u8"👩‍💻!";

    assert_grapheme_cluster(xer::next_grapheme_cluster(text, 0), 0, 11);
    assert_grapheme_cluster(xer::next_grapheme_cluster(text, 11), 11, 1);
}

void test_grapheme_cluster_regional_indicator_u8string_view()
{
    constexpr std::u8string_view text = u8"🇯🇵🇺🇸";

    assert_grapheme_cluster(xer::next_grapheme_cluster(text, 0), 0, 8);
    assert_grapheme_cluster(xer::next_grapheme_cluster(text, 8), 8, 8);
    assert_grapheme_cluster(xer::prev_grapheme_cluster(text, text.size()), 8, 8);
}

void test_grapheme_cluster_crlf_u8string_view()
{
    constexpr std::u8string_view text = u8"\r\nX";

    assert_grapheme_cluster(xer::next_grapheme_cluster(text, 0), 0, 2);
    assert_grapheme_cluster(xer::next_grapheme_cluster(text, 2), 2, 1);
}

void test_grapheme_cluster_keycap_u8string_view()
{
    constexpr std::u8string_view text = u8"1️⃣2";

    assert_grapheme_cluster(xer::next_grapheme_cluster(text, 0), 0, 7);
    assert_grapheme_cluster(xer::next_grapheme_cluster(text, 7), 7, 1);
}

void test_next_grapheme_cluster_u16string_view()
{
    constexpr std::u16string_view text = u"A\u0301😀";

    assert_grapheme_cluster(xer::next_grapheme_cluster(text, 0), 0, 2);
    assert_grapheme_cluster(xer::next_grapheme_cluster(text, 2), 2, 2);
    assert_grapheme_cluster(xer::prev_grapheme_cluster(text, text.size()), 2, 2);
}

void test_next_grapheme_cluster_wstring_view()
{
    constexpr std::wstring_view text = L"A\u0301😀";

    assert_grapheme_cluster(xer::next_grapheme_cluster(text, 0), 0, 2);

    if constexpr (sizeof(wchar_t) == 2) {
        assert_grapheme_cluster(xer::next_grapheme_cluster(text, 2), 2, 2);
    } else {
        assert_grapheme_cluster(xer::next_grapheme_cluster(text, 2), 2, 1);
    }
}

void test_grapheme_clusters_range()
{
    constexpr std::u8string_view text = u8"A\u0301B👩‍💻";
    std::vector<std::size_t> offsets;
    std::vector<std::size_t> sizes;

    for (const auto& item : xer::grapheme_clusters(text)) {
        xer_assert(item.has_value());
        offsets.push_back(item->offset);
        sizes.push_back(item->size);
    }

    xer_assert_eq(offsets.size(), 3uz);
    xer_assert_eq(offsets[0], 0uz);
    xer_assert_eq(offsets[1], 3uz);
    xer_assert_eq(offsets[2], 4uz);
    xer_assert_eq(sizes[0], 3uz);
    xer_assert_eq(sizes[1], 1uz);
    xer_assert_eq(sizes[2], 11uz);
}

void test_prev_grapheme_cluster_requires_boundary()
{
    constexpr std::u8string_view text = u8"A\u0301B";

    const auto result = xer::prev_grapheme_cluster(text, 1);
    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::encoding_error);
}

void test_grapheme_clusters_range_reports_encoding_error()
{
    const char8_t bytes[] = {
        static_cast<char8_t>(0x41),
        static_cast<char8_t>(0xE3),
        static_cast<char8_t>(0x81),
        static_cast<char8_t>(0x42),
    };
    const std::u8string_view text{bytes, sizeof(bytes) / sizeof(bytes[0])};

    auto iterator = xer::grapheme_clusters(text).begin();
    xer_assert((*iterator).has_value());
    ++iterator;
    xer_assert_not((*iterator).has_value());
    xer_assert_eq((*iterator).error().code, xer::error_t::encoding_error);
}

} // namespace

auto main() -> int
{
    test_next_grapheme_cluster_u8string_view();
    test_prev_grapheme_cluster_u8string_view();
    test_grapheme_cluster_emoji_zwj_u8string_view();
    test_grapheme_cluster_regional_indicator_u8string_view();
    test_grapheme_cluster_crlf_u8string_view();
    test_grapheme_cluster_keycap_u8string_view();
    test_next_grapheme_cluster_u16string_view();
    test_next_grapheme_cluster_wstring_view();
    test_grapheme_clusters_range();
    test_prev_grapheme_cluster_requires_boundary();
    test_grapheme_clusters_range_reports_encoding_error();
}
