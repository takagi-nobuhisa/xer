#include <array>
#include <string_view>
#include <vector>

#include <xer/assert.h>
#include <xer/error.h>
#include <xer/unicode.h>

// XER_TEST_FEATURES: icu

namespace {

void assert_code_point(
    const xer::result<xer::code_point>& result,
    std::size_t offset,
    std::size_t size,
    char32_t value)
{
    xer_assert(result.has_value());
    xer_assert_eq(result->offset, offset);
    xer_assert_eq(result->size, size);
    xer_assert_eq(result->value, value);
}

void test_next_code_point_u8string_view()
{
    constexpr std::u8string_view text = u8"Aあ😀";

    assert_code_point(xer::next_code_point(text, 0), 0, 1, U'A');
    assert_code_point(xer::next_code_point(text, 1), 1, 3, U'あ');
    assert_code_point(xer::next_code_point(text, 4), 4, 4, U'😀');

    const auto end = xer::next_code_point(text, text.size());
    xer_assert_not(end.has_value());
    xer_assert_eq(end.error().code, xer::error_t::out_of_range);
}

void test_prev_code_point_u8string_view()
{
    constexpr std::u8string_view text = u8"Aあ😀";

    assert_code_point(xer::prev_code_point(text, text.size()), 4, 4, U'😀');
    assert_code_point(xer::prev_code_point(text, 4), 1, 3, U'あ');
    assert_code_point(xer::prev_code_point(text, 1), 0, 1, U'A');

    const auto begin = xer::prev_code_point(text, 0);
    xer_assert_not(begin.has_value());
    xer_assert_eq(begin.error().code, xer::error_t::out_of_range);
}

void test_next_code_point_u16string_view()
{
    constexpr std::u16string_view text = u"Aあ😀";

    assert_code_point(xer::next_code_point(text, 0), 0, 1, U'A');
    assert_code_point(xer::next_code_point(text, 1), 1, 1, U'あ');
    assert_code_point(xer::next_code_point(text, 2), 2, 2, U'😀');
}

void test_prev_code_point_u16string_view()
{
    constexpr std::u16string_view text = u"Aあ😀";

    assert_code_point(xer::prev_code_point(text, text.size()), 2, 2, U'😀');
    assert_code_point(xer::prev_code_point(text, 2), 1, 1, U'あ');
    assert_code_point(xer::prev_code_point(text, 1), 0, 1, U'A');
}

void test_next_code_point_wstring_view()
{
    constexpr std::wstring_view text = L"Aあ😀";

    assert_code_point(xer::next_code_point(text, 0), 0, 1, U'A');
    assert_code_point(xer::next_code_point(text, 1), 1, 1, U'あ');

    if constexpr (sizeof(wchar_t) == 2) {
        assert_code_point(xer::next_code_point(text, 2), 2, 2, U'😀');
    } else {
        assert_code_point(xer::next_code_point(text, 2), 2, 1, U'😀');
    }
}

void test_code_points_u8_range()
{
    constexpr std::u8string_view text = u8"Aあ😀";
    std::vector<char32_t> values;
    std::vector<std::size_t> offsets;
    std::vector<std::size_t> sizes;

    for (const auto& item : xer::code_points(text)) {
        xer_assert(item.has_value());
        values.push_back(item->value);
        offsets.push_back(item->offset);
        sizes.push_back(item->size);
    }

    xer_assert_eq(values.size(), 3uz);
    xer_assert_eq(values[0], U'A');
    xer_assert_eq(values[1], U'あ');
    xer_assert_eq(values[2], U'😀');
    xer_assert_eq(offsets[0], 0uz);
    xer_assert_eq(offsets[1], 1uz);
    xer_assert_eq(offsets[2], 4uz);
    xer_assert_eq(sizes[0], 1uz);
    xer_assert_eq(sizes[1], 3uz);
    xer_assert_eq(sizes[2], 4uz);
}

void test_code_points_u16_range()
{
    constexpr std::u16string_view text = u"Aあ😀";
    std::vector<char32_t> values;

    for (const auto& item : xer::code_points(text)) {
        xer_assert(item.has_value());
        values.push_back(item->value);
    }

    xer_assert_eq(values.size(), 3uz);
    xer_assert_eq(values[0], U'A');
    xer_assert_eq(values[1], U'あ');
    xer_assert_eq(values[2], U'😀');
}

void test_code_points_wide_range()
{
    constexpr std::wstring_view text = L"Aあ😀";
    std::vector<char32_t> values;

    for (const auto& item : xer::code_points(text)) {
        xer_assert(item.has_value());
        values.push_back(item->value);
    }

    xer_assert_eq(values.size(), 3uz);
    xer_assert_eq(values[0], U'A');
    xer_assert_eq(values[1], U'あ');
    xer_assert_eq(values[2], U'😀');
}

void test_invalid_utf8_is_reported()
{
    const std::array<char8_t, 2> bytes = {
        static_cast<char8_t>(0xC3),
        static_cast<char8_t>(0x28),
    };
    const std::u8string_view text{bytes.data(), bytes.size()};

    const auto result = xer::next_code_point(text, 0);
    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::encoding_error);

    auto range = xer::code_points(text);
    auto iterator = range.begin();
    xer_assert(iterator != range.end());
    const auto& item = *iterator;
    xer_assert_not(item.has_value());
    xer_assert_eq(item.error().code, xer::error_t::encoding_error);
}

void test_invalid_utf16_surrogate_is_reported()
{
    const std::array<char16_t, 1> units = {static_cast<char16_t>(0xD83D)};
    const std::u16string_view text{units.data(), units.size()};

    const auto result = xer::next_code_point(text, 0);
    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::encoding_error);
}

} // namespace

auto main() -> int
{
    test_next_code_point_u8string_view();
    test_prev_code_point_u8string_view();
    test_next_code_point_u16string_view();
    test_prev_code_point_u16string_view();
    test_next_code_point_wstring_view();
    test_code_points_u8_range();
    test_code_points_u16_range();
    test_code_points_wide_range();
    test_invalid_utf8_is_reported();
    test_invalid_utf16_surrogate_is_reported();

    return 0;
}
