/**
 * @file tests/test_string_write.cpp
 * @brief Tests for xer/bits/string_write.h.
 */

#include <array>
#include <string>
#include <string_view>
#include <vector>

#include <xer/assert.h>
#include <xer/bits/string_write.h>
#include <xer/error.h>

namespace {

void test_strcpy_pointer_char8_t_success()
{
    std::array<char8_t, 8> destination{};
    constexpr std::u8string_view source = u8"abc";

    const auto result = xer::strcpy(destination.data(), destination.size(), source);

    xer_assert(result.has_value());
    xer_assert_eq(*result, destination.data());
    xer_assert_eq(destination[0], u8'a');
    xer_assert_eq(destination[1], u8'b');
    xer_assert_eq(destination[2], u8'c');
    xer_assert_eq(destination[3], u8'\0');
}

void test_strcpy_pointer_length_error()
{
    std::array<char8_t, 3> destination{};
    constexpr std::u8string_view source = u8"abc";

    const auto result = xer::strcpy(destination.data(), destination.size(), source);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::length_error);
}

void test_strcpy_raw_array_success()
{
    char8_t destination[8] {};
    constexpr std::u8string_view source = u8"xyz";

    const auto result = xer::strcpy(destination, source);

    xer_assert(result.has_value());
    xer_assert_eq(*result, destination);
    xer_assert_eq(destination[0], u8'x');
    xer_assert_eq(destination[1], u8'y');
    xer_assert_eq(destination[2], u8'z');
    xer_assert_eq(destination[3], u8'\0');
}

void test_strcpy_vector_auto_resize()
{
    std::vector<char8_t> destination {u8'X'};
    constexpr std::u8string_view source = u8"hello";

    const auto result = xer::strcpy(destination, source);

    xer_assert(result.has_value());
    xer_assert_eq(destination.size(), static_cast<std::size_t>(6));
    xer_assert_eq(destination[0], u8'h');
    xer_assert_eq(destination[1], u8'e');
    xer_assert_eq(destination[2], u8'l');
    xer_assert_eq(destination[3], u8'l');
    xer_assert_eq(destination[4], u8'o');
    xer_assert_eq(destination[5], u8'\0');
}

void test_strcpy_basic_string_auto_resize()
{
    std::u8string destination = u8"X";
    constexpr std::u8string_view source = u8"hello";

    const auto result = xer::strcpy(destination, source);

    xer_assert(result.has_value());
    xer_assert_eq(destination, std::u8string(u8"hello"));
    xer_assert_eq(destination.size(), static_cast<std::size_t>(5));
}

void test_strncpy_pointer_shorter_source_zero_fill()
{
    std::array<char8_t, 8> destination {u8'X', u8'X', u8'X', u8'X',
                                        u8'X', u8'X', u8'X', u8'X'};
    constexpr std::u8string_view source = u8"ab";

    const auto result = xer::strncpy(destination.data(), destination.size(), source, 5);

    xer_assert(result.has_value());
    xer_assert_eq(destination[0], u8'a');
    xer_assert_eq(destination[1], u8'b');
    xer_assert_eq(destination[2], u8'\0');
    xer_assert_eq(destination[3], u8'\0');
    xer_assert_eq(destination[4], u8'\0');
}

void test_strncpy_vector_auto_resize()
{
    std::vector<char8_t> destination {u8'X'};
    constexpr std::u8string_view source = u8"pq";

    const auto result = xer::strncpy(destination, source, 4);

    xer_assert(result.has_value());
    xer_assert_eq(destination.size(), static_cast<std::size_t>(4));
    xer_assert_eq(destination[0], u8'p');
    xer_assert_eq(destination[1], u8'q');
    xer_assert_eq(destination[2], u8'\0');
    xer_assert_eq(destination[3], u8'\0');
}

void test_strncpy_basic_string_auto_resize()
{
    std::u8string destination = u8"X";
    constexpr std::u8string_view source = u8"pq";

    const auto result = xer::strncpy(destination, source, 4);

    xer_assert(result.has_value());
    xer_assert_eq(destination.size(), static_cast<std::size_t>(4));
    xer_assert_eq(destination[0], u8'p');
    xer_assert_eq(destination[1], u8'q');
    xer_assert_eq(destination[2], u8'\0');
    xer_assert_eq(destination[3], u8'\0');
}

void test_strcat_pointer_success()
{
    std::array<char8_t, 8> destination {u8'a', u8'b', u8'\0', u8'X',
                                        u8'X', u8'X', u8'X', u8'X'};
    constexpr std::u8string_view source = u8"cd";

    const auto result = xer::strcat(destination.data(), destination.size(), source);

    xer_assert(result.has_value());
    xer_assert_eq(destination[0], u8'a');
    xer_assert_eq(destination[1], u8'b');
    xer_assert_eq(destination[2], u8'c');
    xer_assert_eq(destination[3], u8'd');
    xer_assert_eq(destination[4], u8'\0');
}

void test_strcat_vector_auto_resize_with_terminal_nul()
{
    std::vector<char8_t> destination {u8'h', u8'i', u8'\0'};
    constexpr std::u8string_view source = u8"!";

    const auto result = xer::strcat(destination, source);

    xer_assert(result.has_value());
    xer_assert_eq(destination.size(), static_cast<std::size_t>(4));
    xer_assert_eq(destination[0], u8'h');
    xer_assert_eq(destination[1], u8'i');
    xer_assert_eq(destination[2], u8'!');
    xer_assert_eq(destination[3], u8'\0');
}

void test_strcat_vector_auto_resize_without_terminal_nul()
{
    std::vector<char8_t> destination {u8'h', u8'i'};
    constexpr std::u8string_view source = u8"!";

    const auto result = xer::strcat(destination, source);

    xer_assert(result.has_value());
    xer_assert_eq(destination.size(), static_cast<std::size_t>(4));
    xer_assert_eq(destination[0], u8'h');
    xer_assert_eq(destination[1], u8'i');
    xer_assert_eq(destination[2], u8'!');
    xer_assert_eq(destination[3], u8'\0');
}

void test_strcat_basic_string_auto_resize()
{
    std::u8string destination = u8"hi";
    constexpr std::u8string_view source = u8"!";

    const auto result = xer::strcat(destination, source);

    xer_assert(result.has_value());
    xer_assert_eq(destination, std::u8string(u8"hi!"));
    xer_assert_eq(destination.size(), static_cast<std::size_t>(3));
}

void test_strncat_pointer_success()
{
    std::array<char8_t, 8> destination {u8'a', u8'b', u8'\0', u8'X',
                                        u8'X', u8'X', u8'X', u8'X'};
    constexpr std::u8string_view source = u8"cdef";

    const auto result = xer::strncat(destination.data(), destination.size(), source, 2);

    xer_assert(result.has_value());
    xer_assert_eq(destination[0], u8'a');
    xer_assert_eq(destination[1], u8'b');
    xer_assert_eq(destination[2], u8'c');
    xer_assert_eq(destination[3], u8'd');
    xer_assert_eq(destination[4], u8'\0');
}

void test_strncat_vector_auto_resize_with_terminal_nul()
{
    std::vector<char8_t> destination {u8'm', u8'n', u8'\0'};
    constexpr std::u8string_view source = u8"opq";

    const auto result = xer::strncat(destination, source, 2);

    xer_assert(result.has_value());
    xer_assert_eq(destination.size(), static_cast<std::size_t>(5));
    xer_assert_eq(destination[0], u8'm');
    xer_assert_eq(destination[1], u8'n');
    xer_assert_eq(destination[2], u8'o');
    xer_assert_eq(destination[3], u8'p');
    xer_assert_eq(destination[4], u8'\0');
}

void test_strncat_vector_auto_resize_without_terminal_nul()
{
    std::vector<char8_t> destination {u8'm', u8'n'};
    constexpr std::u8string_view source = u8"opq";

    const auto result = xer::strncat(destination, source, 2);

    xer_assert(result.has_value());
    xer_assert_eq(destination.size(), static_cast<std::size_t>(5));
    xer_assert_eq(destination[0], u8'm');
    xer_assert_eq(destination[1], u8'n');
    xer_assert_eq(destination[2], u8'o');
    xer_assert_eq(destination[3], u8'p');
    xer_assert_eq(destination[4], u8'\0');
}

void test_strncat_basic_string_auto_resize()
{
    std::u8string destination = u8"mn";
    constexpr std::u8string_view source = u8"opq";

    const auto result = xer::strncat(destination, source, 2);

    xer_assert(result.has_value());
    xer_assert_eq(destination, std::u8string(u8"mnop"));
    xer_assert_eq(destination.size(), static_cast<std::size_t>(4));
}

} // namespace

auto main() -> int
{
    test_strcpy_pointer_char8_t_success();
    test_strcpy_pointer_length_error();
    test_strcpy_raw_array_success();
    test_strcpy_vector_auto_resize();
    test_strcpy_basic_string_auto_resize();

    test_strncpy_pointer_shorter_source_zero_fill();
    test_strncpy_vector_auto_resize();
    test_strncpy_basic_string_auto_resize();

    test_strcat_pointer_success();
    test_strcat_vector_auto_resize_with_terminal_nul();
    test_strcat_vector_auto_resize_without_terminal_nul();
    test_strcat_basic_string_auto_resize();

    test_strncat_pointer_success();
    test_strncat_vector_auto_resize_with_terminal_nul();
    test_strncat_vector_auto_resize_without_terminal_nul();
    test_strncat_basic_string_auto_resize();

    return 0;
}
