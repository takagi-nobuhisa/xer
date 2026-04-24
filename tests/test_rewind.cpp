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

namespace {

namespace fs = std::filesystem;

[[nodiscard]] auto filesystem_path_to_u8string(const fs::path& value) -> std::u8string {
#ifdef _WIN32
    const std::wstring native = value.native();
    const auto converted = xer::from_native_path(std::wstring_view(native));
#else
    const std::string native = value.native();
    const auto converted = xer::from_native_path(std::string_view(native));
#endif

    xer_assert(converted.has_value());
    return std::u8string(converted->str());
}

[[nodiscard]] auto make_unique_test_root() -> fs::path {
    const fs::path base = fs::temp_directory_path();
    const auto now =
        std::chrono::high_resolution_clock::now().time_since_epoch().count();

#ifdef _WIN32
    const unsigned long pid = ::GetCurrentProcessId();
#else
    const auto pid = static_cast<unsigned long>(::getpid());
#endif

    return base / ("xer_test_rewind_" + std::to_string(pid) + "_" +
                   std::to_string(static_cast<long long>(now)));
}

struct test_directory_guard {
    fs::path path;

    explicit test_directory_guard(fs::path value)
        : path(std::move(value)) {
        fs::create_directories(path);
    }

    ~test_directory_guard() {
        std::error_code ec;
        fs::remove_all(path, ec);
    }
};

void write_binary_file(const fs::path& path, std::u8string_view contents) {
    std::ofstream stream(path, std::ios::binary);
    xer_assert(stream.good());

    stream.write(
        reinterpret_cast<const char*>(contents.data()),
        static_cast<std::streamsize>(contents.size()));

    xer_assert(stream.good());
}

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
    xer_assert_eq(eof.error().code, xer::error_t::not_found);
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
