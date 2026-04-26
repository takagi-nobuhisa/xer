/**
 * @file tests/test_string_replace.cpp
 * @brief Tests for xer::str_replace.
 */

#include <cstddef>
#include <string>

#include <xer/assert.h>
#include <xer/string.h>

namespace {

void test_str_replace_basic()
{
    const auto result = xer::str_replace(u8"world", u8"XER", u8"hello world");

    xer_assert(result.has_value());
    xer_assert(*result == std::u8string(u8"hello XER"));
}

void test_str_replace_multiple_occurrences()
{
    const auto result = xer::str_replace(u8"cat", u8"dog", u8"cat cat cat");

    xer_assert(result.has_value());
    xer_assert(*result == std::u8string(u8"dog dog dog"));
}

void test_str_replace_non_overlapping_occurrences()
{
    const auto result = xer::str_replace(u8"aa", u8"b", u8"aaaaa");

    xer_assert(result.has_value());
    xer_assert(*result == std::u8string(u8"bba"));
}

void test_str_replace_remove_occurrences()
{
    const auto result = xer::str_replace(u8"-", u8"", u8"a-b-c");

    xer_assert(result.has_value());
    xer_assert(*result == std::u8string(u8"abc"));
}

void test_str_replace_expand_occurrences()
{
    const auto result = xer::str_replace(u8"/", u8"::", u8"a/b/c");

    xer_assert(result.has_value());
    xer_assert(*result == std::u8string(u8"a::b::c"));
}

void test_str_replace_not_found_returns_copy()
{
    const auto result = xer::str_replace(u8"x", u8"y", u8"abc");

    xer_assert(result.has_value());
    xer_assert(*result == std::u8string(u8"abc"));
}

void test_str_replace_empty_search_returns_copy()
{
    std::size_t count = 999;
    const auto result = xer::str_replace(u8"", u8"x", u8"abc", &count);

    xer_assert(result.has_value());
    xer_assert(*result == std::u8string(u8"abc"));
    xer_assert_eq(count, static_cast<std::size_t>(0));
}

void test_str_replace_count()
{
    std::size_t count = 0;
    const auto result = xer::str_replace(u8"a", u8"A", u8"banana", &count);

    xer_assert(result.has_value());
    xer_assert(*result == std::u8string(u8"bAnAnA"));
    xer_assert_eq(count, static_cast<std::size_t>(3));
}

void test_str_replace_utf8_substring()
{
    const auto result = xer::str_replace(u8"猫", u8"犬", u8"猫と猫");

    xer_assert(result.has_value());
    xer_assert(*result == std::u8string(u8"犬と犬"));
}

void test_str_replace_embedded_nul()
{
    const std::u8string subject({u8'a', u8'\0', u8'b', u8'\0', u8'c'});
    const std::u8string search({u8'\0'});
    const auto result = xer::str_replace(search, u8"_", subject);

    xer_assert(result.has_value());
    xer_assert_eq(result->size(), static_cast<std::size_t>(5));
    xer_assert((*result)[0] == u8'a');
    xer_assert((*result)[1] == u8'_');
    xer_assert((*result)[2] == u8'b');
    xer_assert((*result)[3] == u8'_');
    xer_assert((*result)[4] == u8'c');
}

} // namespace

auto main() -> int
{
    test_str_replace_basic();
    test_str_replace_multiple_occurrences();
    test_str_replace_non_overlapping_occurrences();
    test_str_replace_remove_occurrences();
    test_str_replace_expand_occurrences();
    test_str_replace_not_found_returns_copy();
    test_str_replace_empty_search_returns_copy();
    test_str_replace_count();
    test_str_replace_utf8_substring();
    test_str_replace_embedded_nul();

    return 0;
}
