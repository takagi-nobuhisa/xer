/**
 * @file tests/test_rewind.cpp
 * @brief Tests for xer::rewind.
 */

#include <array>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <span>
#include <string>
#include <string_view>

#ifdef _WIN32
#    include <windows.h>
#else
#    include <unistd.h>
#endif

#include <xer/assert.h>
#include <xer/stdio.h>
#include "test_helpers.h"

namespace {

using xer_test::filesystem_path_to_u8string;
using xer_test::make_unique_test_root;
using xer_test::test_directory_guard;
using xer_test::write_binary_file;

namespace fs = std::filesystem;

void test_binary_rewind_empty_stream_is_error() {
    xer::binary_stream stream;

    const auto result = xer::rewind(stream);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_text_rewind_empty_stream_is_error() {
    xer::text_stream stream;

    const auto result = xer::rewind(stream);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_binary_rewind_memory_stream_returns_to_beginning() {
    std::array<std::byte, 3> memory{
        std::byte{'a'},
        std::byte{'b'},
        std::byte{'c'},
    };

    auto stream_result = xer::memopen(std::span<std::byte>(memory), "r+");
    xer_assert(stream_result.has_value());

    auto& stream = *stream_result;

    const auto first = xer::fgetb(stream);
    xer_assert(first.has_value());
    xer_assert_eq(*first, std::byte{'a'});

    const auto second = xer::fgetb(stream);
    xer_assert(second.has_value());
    xer_assert_eq(*second, std::byte{'b'});

    stream.set_eof(true);
    stream.set_error(true);

    const auto rewind_result = xer::rewind(stream);
    xer_assert(rewind_result.has_value());
    xer_assert(!xer::feof(stream));
    xer_assert(!xer::ferror(stream));

    const auto again = xer::fgetb(stream);
    xer_assert(again.has_value());
    xer_assert_eq(*again, std::byte{'a'});
}

void test_text_rewind_string_view_stream_returns_to_beginning() {
    constexpr std::u8string_view input = u8"aあb";

    auto stream_result = xer::stropen(input, "r");
    xer_assert(stream_result.has_value());

    auto& stream = *stream_result;

    const auto first = xer::fgetc(stream);
    xer_assert(first.has_value());
    xer_assert_eq(*first, U'a');

    const auto second = xer::fgetc(stream);
    xer_assert(second.has_value());
    xer_assert_eq(*second, U'あ');

    const auto rewind_result = xer::rewind(stream);
    xer_assert(rewind_result.has_value());

    const auto again = xer::fgetc(stream);
    xer_assert(again.has_value());
    xer_assert_eq(*again, U'a');
}

void test_text_rewind_clears_eof_and_error_indicators() {
    constexpr std::u8string_view input = u8"x";

    auto stream_result = xer::stropen(input, "r");
    xer_assert(stream_result.has_value());

    auto& stream = *stream_result;

    const auto first = xer::fgetc(stream);
    xer_assert(first.has_value());
    xer_assert_eq(*first, U'x');

    const auto eof = xer::fgetc(stream);
    xer_assert(!eof.has_value());
    xer_assert_eq(eof.error().code, xer::error_t::end_of_file);
    xer_assert(xer::feof(stream));

    stream.set_error(true);
    xer_assert(xer::ferror(stream));

    const auto rewind_result = xer::rewind(stream);
    xer_assert(rewind_result.has_value());
    xer_assert(!xer::feof(stream));
    xer_assert(!xer::ferror(stream));

    const auto again = xer::fgetc(stream);
    xer_assert(again.has_value());
    xer_assert_eq(*again, U'x');
}

void test_text_rewind_discards_ungetc_character() {
    constexpr std::u8string_view input = u8"abc";

    auto stream_result = xer::stropen(input, "r");
    xer_assert(stream_result.has_value());

    auto& stream = *stream_result;

    const auto first = xer::fgetc(stream);
    xer_assert(first.has_value());
    xer_assert_eq(*first, U'a');

    const auto unget_result = xer::ungetc(U'z', stream);
    xer_assert(unget_result.has_value());

    const auto rewind_result = xer::rewind(stream);
    xer_assert(rewind_result.has_value());

    const auto again = xer::fgetc(stream);
    xer_assert(again.has_value());
    xer_assert_eq(*again, U'a');
}

void test_text_rewind_auto_detect_file_discards_lookahead() {
    const test_directory_guard guard(make_unique_test_root());
    const fs::path file_path = guard.path / "auto_detect.txt";

    write_binary_file(file_path, u8"abcdefあいう");

    const xer::path target(filesystem_path_to_u8string(file_path));
    auto stream_result = xer::fopen(target, "r", xer::encoding_t::auto_detect);
    xer_assert(stream_result.has_value());

    auto& stream = *stream_result;

    const auto first = xer::fgetc(stream);
    xer_assert(first.has_value());
    xer_assert_eq(*first, U'a');

    const auto rewind_result = xer::rewind(stream);
    xer_assert(rewind_result.has_value());

    const auto again = xer::fgetc(stream);
    xer_assert(again.has_value());
    xer_assert_eq(*again, U'a');
}

} // namespace

auto main() -> int {
    test_binary_rewind_empty_stream_is_error();
    test_text_rewind_empty_stream_is_error();
    test_binary_rewind_memory_stream_returns_to_beginning();
    test_text_rewind_string_view_stream_returns_to_beginning();
    test_text_rewind_clears_eof_and_error_indicators();
    test_text_rewind_discards_ungetc_character();
    test_text_rewind_auto_detect_file_discards_lookahead();
    return 0;
}
