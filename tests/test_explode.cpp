#include <climits>
#include <string>
#include <string_view>
#include <vector>

#include <xer/assert.h>
#include <xer/error.h>
#include <xer/string.h>

namespace {

auto to_sv_vector(const std::vector<std::u8string>& values)
    -> std::vector<std::u8string_view>
{
    std::vector<std::u8string_view> result;
    result.reserve(values.size());

    for (const auto& value : values) {
        result.emplace_back(value);
    }

    return result;
}

void test_explode_basic()
{
    const auto result = xer::explode(u8",", u8"a,b,c");
    xer_assert(result.has_value());

    const std::vector<std::u8string_view> expected = {
        u8"a",
        u8"b",
        u8"c",
    };
    xer_assert_eq(to_sv_vector(*result), expected);
}

void test_explode_separator_not_found()
{
    const auto result = xer::explode(u8",", u8"abc");
    xer_assert(result.has_value());

    const std::vector<std::u8string_view> expected = {
        u8"abc",
    };
    xer_assert_eq(to_sv_vector(*result), expected);
}

void test_explode_empty_source()
{
    const auto result = xer::explode(u8",", u8"");
    xer_assert(result.has_value());

    const std::vector<std::u8string_view> expected = {
        u8"",
    };
    xer_assert_eq(to_sv_vector(*result), expected);
}

void test_explode_empty_elements()
{
    const auto result = xer::explode(u8",", u8",a,,b,");
    xer_assert(result.has_value());

    const std::vector<std::u8string_view> expected = {
        u8"",
        u8"a",
        u8"",
        u8"b",
        u8"",
    };
    xer_assert_eq(to_sv_vector(*result), expected);
}

void test_explode_multi_character_separator()
{
    const auto result = xer::explode(u8"::", u8"a::b::c");
    xer_assert(result.has_value());

    const std::vector<std::u8string_view> expected = {
        u8"a",
        u8"b",
        u8"c",
    };
    xer_assert_eq(to_sv_vector(*result), expected);
}

void test_explode_limit_positive_one()
{
    const auto result = xer::explode(u8",", u8"a,b,c,d", 1);
    xer_assert(result.has_value());

    const std::vector<std::u8string_view> expected = {
        u8"a,b,c,d",
    };
    xer_assert_eq(to_sv_vector(*result), expected);
}

void test_explode_limit_positive_two()
{
    const auto result = xer::explode(u8",", u8"a,b,c,d", 2);
    xer_assert(result.has_value());

    const std::vector<std::u8string_view> expected = {
        u8"a",
        u8"b,c,d",
    };
    xer_assert_eq(to_sv_vector(*result), expected);
}

void test_explode_limit_positive_three()
{
    const auto result = xer::explode(u8",", u8"a,b,c,d", 3);
    xer_assert(result.has_value());

    const std::vector<std::u8string_view> expected = {
        u8"a",
        u8"b",
        u8"c,d",
    };
    xer_assert_eq(to_sv_vector(*result), expected);
}

void test_explode_limit_zero_is_treated_as_one()
{
    const auto result = xer::explode(u8",", u8"a,b,c,d", 0);
    xer_assert(result.has_value());

    const std::vector<std::u8string_view> expected = {
        u8"a,b,c,d",
    };
    xer_assert_eq(to_sv_vector(*result), expected);
}

void test_explode_limit_negative_one()
{
    const auto result = xer::explode(u8",", u8"a,b,c,d", -1);
    xer_assert(result.has_value());

    const std::vector<std::u8string_view> expected = {
        u8"a",
        u8"b",
        u8"c",
    };
    xer_assert_eq(to_sv_vector(*result), expected);
}

void test_explode_limit_negative_two()
{
    const auto result = xer::explode(u8",", u8"a,b,c,d", -2);
    xer_assert(result.has_value());

    const std::vector<std::u8string_view> expected = {
        u8"a",
        u8"b",
    };
    xer_assert_eq(to_sv_vector(*result), expected);
}

void test_explode_limit_negative_equal_size()
{
    const auto result = xer::explode(u8",", u8"a,b,c,d", -4);
    xer_assert(result.has_value());
    xer_assert(result->empty());
}

void test_explode_limit_negative_larger_than_size()
{
    const auto result = xer::explode(u8",", u8"a,b,c,d", -5);
    xer_assert(result.has_value());
    xer_assert(result->empty());
}

void test_explode_limit_negative_without_separator()
{
    const auto result = xer::explode(u8",", u8"abc", -1);
    xer_assert(result.has_value());
    xer_assert(result->empty());
}

void test_explode_empty_separator_is_error()
{
    const auto result = xer::explode(u8"", u8"a,b,c");
    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_explode_utf8_text()
{
    const auto result = xer::explode(u8"☆", u8"あ☆い☆う");
    xer_assert(result.has_value());

    const std::vector<std::u8string_view> expected = {
        u8"あ",
        u8"い",
        u8"う",
    };
    xer_assert_eq(to_sv_vector(*result), expected);
}

void test_explode_view_basic()
{
    const auto result = xer::explode_view(u8",", u8"a,b,c");
    xer_assert(result.has_value());

    const std::vector<std::u8string_view> expected = {
        u8"a",
        u8"b",
        u8"c",
    };
    xer_assert_eq(*result, expected);
}

void test_explode_view_limit_positive()
{
    const auto result = xer::explode_view(u8",", u8"a,b,c,d", 3);
    xer_assert(result.has_value());

    const std::vector<std::u8string_view> expected = {
        u8"a",
        u8"b",
        u8"c,d",
    };
    xer_assert_eq(*result, expected);
}

void test_explode_view_limit_negative()
{
    const auto result = xer::explode_view(u8",", u8"a,b,c,d", -2);
    xer_assert(result.has_value());

    const std::vector<std::u8string_view> expected = {
        u8"a",
        u8"b",
    };
    xer_assert_eq(*result, expected);
}

void test_explode_view_empty_separator_is_error()
{
    const auto result = xer::explode_view(u8"", u8"a,b,c");
    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_explode_view_refers_to_source_storage()
{
    std::u8string source = u8"left,right,center";

    const auto result = xer::explode_view(u8",", source);
    xer_assert(result.has_value());
    xer_assert_eq(result->size(), static_cast<std::size_t>(3));

    xer_assert_eq((*result)[0].data(), source.data());
    xer_assert_eq((*result)[0], std::u8string_view(u8"left"));
    xer_assert_eq((*result)[1], std::u8string_view(u8"right"));
    xer_assert_eq((*result)[2], std::u8string_view(u8"center"));
}

} // namespace

int main()
{
    test_explode_basic();
    test_explode_separator_not_found();
    test_explode_empty_source();
    test_explode_empty_elements();
    test_explode_multi_character_separator();
    test_explode_limit_positive_one();
    test_explode_limit_positive_two();
    test_explode_limit_positive_three();
    test_explode_limit_zero_is_treated_as_one();
    test_explode_limit_negative_one();
    test_explode_limit_negative_two();
    test_explode_limit_negative_equal_size();
    test_explode_limit_negative_larger_than_size();
    test_explode_limit_negative_without_separator();
    test_explode_empty_separator_is_error();
    test_explode_utf8_text();

    test_explode_view_basic();
    test_explode_view_limit_positive();
    test_explode_view_limit_negative();
    test_explode_view_empty_separator_is_error();
    test_explode_view_refers_to_source_storage();

    return 0;
}
