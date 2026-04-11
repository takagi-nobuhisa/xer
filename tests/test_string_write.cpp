/**
 * @file tests/test_string_write.cpp
 * @brief Tests for xer/bits/string_write.h.
 */

#include <array>
#include <expected>
#include <string>
#include <string_view>
#include <vector>

#include <xer/assert.h>
#include <xer/bits/string_write.h>
#include <xer/error.h>

namespace {

void test_strcpy_pointer_char8_t_success()
{
    std::array<char8_t, 8> destination {u8'\0', u8'\0', u8'\0', u8'\0',
                                        u8'\0', u8'\0', u8'\0', u8'\0'};
    constexpr std::u8string_view source = u8"abc";

    const auto result =
        xer::strcpy(destination.data(), destination.size(), source);

    xer_assert(result.has_value());
    xer_assert_eq(result.value(), destination.data());
    xer_assert_eq(destination[0], u8'a');
    xer_assert_eq(destination[1], u8'b');
    xer_assert_eq(destination[2], u8'c');
    xer_assert_eq(destination[3], u8'\0');
}

void test_strcpy_pointer_length_error()
{
    std::array<char8_t, 3> destination {u8'\0', u8'\0', u8'\0'};
    constexpr std::u8string_view source = u8"abc";

    const auto result =
        xer::strcpy(destination.data(), destination.size(), source);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::length_error);
}

void test_strcpy_pointer_invalid_argument()
{
    constexpr std::u8string_view source = u8"abc";

    const auto result =
        xer::strcpy(static_cast<char8_t*>(nullptr), 4, source);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_strcpy_container_success()
{
    std::array<char8_t, 8> destination {u8'\0', u8'\0', u8'\0', u8'\0',
                                        u8'\0', u8'\0', u8'\0', u8'\0'};
    constexpr std::u8string_view source = u8"xyz";

    const auto result = xer::strcpy(destination, source);

    xer_assert(result.has_value());
    xer_assert_eq(result.value(), destination.begin());
    xer_assert_eq(destination[0], u8'x');
    xer_assert_eq(destination[1], u8'y');
    xer_assert_eq(destination[2], u8'z');
    xer_assert_eq(destination[3], u8'\0');
}

void test_strncpy_pointer_shorter_source_zero_fill()
{
    std::array<char8_t, 8> destination {u8'X', u8'X', u8'X', u8'X',
                                        u8'X', u8'X', u8'X', u8'X'};
    constexpr std::u8string_view source = u8"ab";

    const auto result =
        xer::strncpy(destination.data(), destination.size(), source, 5);

    xer_assert(result.has_value());
    xer_assert_eq(result.value(), destination.data());
    xer_assert_eq(destination[0], u8'a');
    xer_assert_eq(destination[1], u8'b');
    xer_assert_eq(destination[2], u8'\0');
    xer_assert_eq(destination[3], u8'\0');
    xer_assert_eq(destination[4], u8'\0');
    xer_assert_eq(destination[5], u8'X');
}

void test_strncpy_pointer_no_terminator_guarantee()
{
    std::array<char8_t, 4> destination {u8'X', u8'X', u8'X', u8'X'};
    constexpr std::u8string_view source = u8"abcd";

    const auto result =
        xer::strncpy(destination.data(), destination.size(), source, 3);

    xer_assert(result.has_value());
    xer_assert_eq(result.value(), destination.data());
    xer_assert_eq(destination[0], u8'a');
    xer_assert_eq(destination[1], u8'b');
    xer_assert_eq(destination[2], u8'c');
    xer_assert_eq(destination[3], u8'X');
}

void test_strncpy_pointer_length_error()
{
    std::array<char8_t, 3> destination {u8'\0', u8'\0', u8'\0'};
    constexpr std::u8string_view source = u8"abc";

    const auto result =
        xer::strncpy(destination.data(), destination.size(), source, 4);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::length_error);
}

void test_strncpy_container_success()
{
    std::vector<char8_t> destination(6, u8'X');
    constexpr std::u8string_view source = u8"pq";

    const auto result = xer::strncpy(destination, source, 4);

    xer_assert(result.has_value());
    xer_assert_eq(result.value(), destination.begin());
    xer_assert_eq(destination[0], u8'p');
    xer_assert_eq(destination[1], u8'q');
    xer_assert_eq(destination[2], u8'\0');
    xer_assert_eq(destination[3], u8'\0');
    xer_assert_eq(destination[4], u8'X');
}

void test_strcat_pointer_success()
{
    std::array<char8_t, 8> destination {u8'a', u8'b', u8'\0', u8'X',
                                        u8'X', u8'X', u8'X', u8'X'};
    constexpr std::u8string_view source = u8"cd";

    const auto result =
        xer::strcat(destination.data(), destination.size(), source);

    xer_assert(result.has_value());
    xer_assert_eq(result.value(), destination.data());
    xer_assert_eq(destination[0], u8'a');
    xer_assert_eq(destination[1], u8'b');
    xer_assert_eq(destination[2], u8'c');
    xer_assert_eq(destination[3], u8'd');
    xer_assert_eq(destination[4], u8'\0');
}

void test_strcat_pointer_not_found()
{
    std::array<char8_t, 4> destination {u8'a', u8'b', u8'c', u8'd'};
    constexpr std::u8string_view source = u8"x";

    const auto result =
        xer::strcat(destination.data(), destination.size(), source);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_strcat_pointer_length_error()
{
    std::array<char8_t, 4> destination {u8'a', u8'b', u8'\0', u8'X'};
    constexpr std::u8string_view source = u8"cd";

    const auto result =
        xer::strcat(destination.data(), destination.size(), source);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::length_error);
}

void test_strcat_container_success()
{
    std::array<char8_t, 8> destination {u8'h', u8'i', u8'\0', u8'X',
                                        u8'X', u8'X', u8'X', u8'X'};
    constexpr std::u8string_view source = u8"!";

    const auto result = xer::strcat(destination, source);

    xer_assert(result.has_value());
    xer_assert_eq(result.value(), destination.begin());
    xer_assert_eq(destination[0], u8'h');
    xer_assert_eq(destination[1], u8'i');
    xer_assert_eq(destination[2], u8'!');
    xer_assert_eq(destination[3], u8'\0');
}

void test_strncat_pointer_success_full_append()
{
    std::array<char8_t, 8> destination {u8'a', u8'b', u8'\0', u8'X',
                                        u8'X', u8'X', u8'X', u8'X'};
    constexpr std::u8string_view source = u8"cdef";

    const auto result =
        xer::strncat(destination.data(), destination.size(), source, 2);

    xer_assert(result.has_value());
    xer_assert_eq(result.value(), destination.data());
    xer_assert_eq(destination[0], u8'a');
    xer_assert_eq(destination[1], u8'b');
    xer_assert_eq(destination[2], u8'c');
    xer_assert_eq(destination[3], u8'd');
    xer_assert_eq(destination[4], u8'\0');
}

void test_strncat_pointer_source_shorter_than_count()
{
    std::array<char8_t, 8> destination {u8'a', u8'\0', u8'X', u8'X',
                                        u8'X', u8'X', u8'X', u8'X'};
    constexpr std::u8string_view source = u8"bc";

    const auto result =
        xer::strncat(destination.data(), destination.size(), source, 5);

    xer_assert(result.has_value());
    xer_assert_eq(result.value(), destination.data());
    xer_assert_eq(destination[0], u8'a');
    xer_assert_eq(destination[1], u8'b');
    xer_assert_eq(destination[2], u8'c');
    xer_assert_eq(destination[3], u8'\0');
}

void test_strncat_pointer_not_found()
{
    std::array<char8_t, 4> destination {u8'a', u8'b', u8'c', u8'd'};
    constexpr std::u8string_view source = u8"x";

    const auto result =
        xer::strncat(destination.data(), destination.size(), source, 1);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_strncat_pointer_length_error()
{
    std::array<char8_t, 4> destination {u8'a', u8'b', u8'\0', u8'X'};
    constexpr std::u8string_view source = u8"cdef";

    const auto result =
        xer::strncat(destination.data(), destination.size(), source, 2);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::length_error);
}

void test_strncat_container_success()
{
    std::vector<char8_t> destination {u8'm', u8'n', u8'\0', u8'X', u8'X', u8'X'};
    constexpr std::u8string_view source = u8"opq";

    const auto result = xer::strncat(destination, source, 2);

    xer_assert(result.has_value());
    xer_assert_eq(result.value(), destination.begin());
    xer_assert_eq(destination[0], u8'm');
    xer_assert_eq(destination[1], u8'n');
    xer_assert_eq(destination[2], u8'o');
    xer_assert_eq(destination[3], u8'p');
    xer_assert_eq(destination[4], u8'\0');
}

void test_unsigned_char_strcpy_success()
{
    std::array<unsigned char, 6> destination {0, 0, 0, 0, 0, 0};
    const std::basic_string_view<unsigned char> source {
        reinterpret_cast<const unsigned char*>("ab"), 2};

    const auto result =
        xer::strcpy(destination.data(), destination.size(), source);

    xer_assert(result.has_value());
    xer_assert_eq(result.value(), destination.data());
    xer_assert_eq(destination[0], static_cast<unsigned char>('a'));
    xer_assert_eq(destination[1], static_cast<unsigned char>('b'));
    xer_assert_eq(destination[2], static_cast<unsigned char>(0));
}

void test_char16_t_strcat_success()
{
    std::array<char16_t, 8> destination {u'a', u'\0', u'X', u'X',
                                         u'X', u'X', u'X', u'X'};
    constexpr std::basic_string_view<char16_t> source = u"bc";

    const auto result =
        xer::strcat(destination.data(), destination.size(), source);

    xer_assert(result.has_value());
    xer_assert_eq(result.value(), destination.data());
    xer_assert_eq(destination[0], u'a');
    xer_assert_eq(destination[1], u'b');
    xer_assert_eq(destination[2], u'c');
    xer_assert_eq(destination[3], u'\0');
}

} // namespace

int main()
{
    test_strcpy_pointer_char8_t_success();
    test_strcpy_pointer_length_error();
    test_strcpy_pointer_invalid_argument();
    test_strcpy_container_success();

    test_strncpy_pointer_shorter_source_zero_fill();
    test_strncpy_pointer_no_terminator_guarantee();
    test_strncpy_pointer_length_error();
    test_strncpy_container_success();

    test_strcat_pointer_success();
    test_strcat_pointer_not_found();
    test_strcat_pointer_length_error();
    test_strcat_container_success();

    test_strncat_pointer_success_full_append();
    test_strncat_pointer_source_shorter_than_count();
    test_strncat_pointer_not_found();
    test_strncat_pointer_length_error();
    test_strncat_container_success();

    test_unsigned_char_strcpy_success();
    test_char16_t_strcat_success();

    return 0;
}
