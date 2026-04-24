/**
 * @file tests/test_string_prefix_suffix.cpp
 * @brief Tests for xer::starts_with and xer::ends_with.
 */

#include <cstddef>
#include <string_view>

#include <xer/assert.h>
#include <xer/string.h>

namespace {

void test_starts_with_string_view_basic() {
    constexpr std::u8string_view source = u8"hello";

    xer_assert(xer::starts_with(source, std::u8string_view(u8"he")));
    xer_assert(xer::starts_with(source, std::u8string_view(u8"hello")));
    xer_assert(xer::starts_with(source, std::u8string_view{}));

    xer_assert_not(xer::starts_with(source, std::u8string_view(u8"el")));
    xer_assert_not(xer::starts_with(source, std::u8string_view(u8"hello!")));
}

void test_ends_with_string_view_basic() {
    constexpr std::u8string_view source = u8"hello";

    xer_assert(xer::ends_with(source, std::u8string_view(u8"lo")));
    xer_assert(xer::ends_with(source, std::u8string_view(u8"hello")));
    xer_assert(xer::ends_with(source, std::u8string_view{}));

    xer_assert_not(xer::ends_with(source, std::u8string_view(u8"el")));
    xer_assert_not(xer::ends_with(source, std::u8string_view(u8"!hello")));
}

void test_starts_with_string_literals() {
    xer_assert(xer::starts_with(u8"hello", u8"he"));
    xer_assert(xer::starts_with(u8"hello", u8"hello"));
    xer_assert(xer::starts_with(u8"hello", u8""));

    xer_assert_not(xer::starts_with(u8"hello", u8"el"));
    xer_assert_not(xer::starts_with(u8"hello", u8"hello!"));
}

void test_ends_with_string_literals() {
    xer_assert(xer::ends_with(u8"hello", u8"lo"));
    xer_assert(xer::ends_with(u8"hello", u8"hello"));
    xer_assert(xer::ends_with(u8"hello", u8""));

    xer_assert_not(xer::ends_with(u8"hello", u8"el"));
    xer_assert_not(xer::ends_with(u8"hello", u8"!hello"));
}

void test_array_without_trailing_nul() {
    constexpr char8_t source[] = {u8'a', u8'b', u8'c'};
    constexpr char8_t prefix[] = {u8'a', u8'b'};
    constexpr char8_t suffix[] = {u8'b', u8'c'};
    constexpr char8_t too_long[] = {u8'a', u8'b', u8'c', u8'd'};

    xer_assert(xer::starts_with(source, prefix));
    xer_assert(xer::ends_with(source, suffix));

    xer_assert_not(xer::starts_with(source, too_long));
    xer_assert_not(xer::ends_with(source, too_long));
}

void test_array_with_trailing_nul_is_treated_like_string_literal() {
    constexpr char8_t source[] = {u8'a', u8'b', u8'c', u8'\0'};
    constexpr char8_t prefix[] = {u8'a', u8'b', u8'\0'};
    constexpr char8_t suffix[] = {u8'b', u8'c', u8'\0'};

    xer_assert(xer::starts_with(source, prefix));
    xer_assert(xer::ends_with(source, suffix));
}

void test_array_with_embedded_nul() {
    constexpr char8_t source[] = {u8'a', u8'\0', u8'b'};
    constexpr char8_t prefix[] = {u8'a', u8'\0'};
    constexpr char8_t suffix[] = {u8'\0', u8'b'};
    constexpr char8_t wrong_suffix[] = {u8'a', u8'b'};

    xer_assert(xer::starts_with(source, prefix));
    xer_assert(xer::ends_with(source, suffix));

    xer_assert_not(xer::ends_with(source, wrong_suffix));
}

void test_pointer_and_size_uses_size_directly() {
    constexpr char8_t source[] = {u8'a', u8'b', u8'c', u8'\0'};
    constexpr char8_t prefix[] = {u8'a', u8'b'};
    constexpr char8_t suffix_without_nul[] = {u8'b', u8'c'};
    constexpr char8_t suffix_with_nul[] = {u8'c', u8'\0'};

    xer_assert(xer::starts_with(source, 4, prefix, 2));
    xer_assert(xer::ends_with(source, 4, suffix_with_nul, 2));

    xer_assert_not(xer::ends_with(source, 4, suffix_without_nul, 2));
}

void test_pointer_and_size_null_with_nonzero_size_is_false() {
    const char8_t* const source = nullptr;
    constexpr char8_t prefix[] = {u8'a'};

    xer_assert_not(xer::starts_with(source, 1, prefix, 1));
    xer_assert_not(xer::ends_with(source, 1, prefix, 1));
    xer_assert_not(xer::starts_with(prefix, 1, source, 1));
    xer_assert_not(xer::ends_with(prefix, 1, source, 1));
}

void test_mixed_array_and_string_view() {
    constexpr char8_t source[] = {u8'a', u8'b', u8'c', u8'\0'};
    constexpr std::u8string_view prefix = u8"ab";
    constexpr std::u8string_view suffix = u8"bc";

    xer_assert(xer::starts_with(source, prefix));
    xer_assert(xer::ends_with(source, suffix));

    xer_assert(xer::starts_with(std::u8string_view(u8"abc"), u8"ab"));
    xer_assert(xer::ends_with(std::u8string_view(u8"abc"), u8"bc"));
}

void test_other_character_types() {
    xer_assert(xer::starts_with("hello", "he"));
    xer_assert(xer::ends_with("hello", "lo"));

    xer_assert(xer::starts_with(u"hello", u"he"));
    xer_assert(xer::ends_with(u"hello", u"lo"));

    xer_assert(xer::starts_with(U"hello", U"he"));
    xer_assert(xer::ends_with(U"hello", U"lo"));
}

} // namespace

auto main() -> int {
    test_starts_with_string_view_basic();
    test_ends_with_string_view_basic();
    test_starts_with_string_literals();
    test_ends_with_string_literals();
    test_array_without_trailing_nul();
    test_array_with_trailing_nul_is_treated_like_string_literal();
    test_array_with_embedded_nul();
    test_pointer_and_size_uses_size_directly();
    test_pointer_and_size_null_with_nonzero_size_is_false();
    test_mixed_array_and_string_view();
    test_other_character_types();
    return 0;
}
