/**
 * @file tests/test_file_stat.cpp
 * @brief Tests for xer/bits/file_stat.h.
 */

#include <cstddef>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>

#ifdef _WIN32
#    include <windows.h>
#else
#    include <unistd.h>
#endif

#include <xer/assert.h>
#include <xer/error.h>
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

    return base / ("xer_test_file_stat_" + std::to_string(pid) + "_" +
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

void write_text_file(const fs::path& path, const std::string& contents) {
    std::ofstream stream(path, std::ios::binary);
    xer_assert(stream.good());
    stream.write(contents.data(), static_cast<std::streamsize>(contents.size()));
    xer_assert(stream.good());
}

void test_filesize_regular_file() {
    const test_directory_guard guard(make_unique_test_root());
    const fs::path file_path = guard.path / "size.txt";
    write_text_file(file_path, "hello");

    const xer::path target(filesystem_path_to_u8string(file_path));
    const auto size = xer::filesize(target);

    xer_assert(size.has_value());
    xer_assert_eq(*size, static_cast<std::uintmax_t>(5));
}

void test_lstat_regular_file() {
    const test_directory_guard guard(make_unique_test_root());
    const fs::path file_path = guard.path / "status.txt";
    write_text_file(file_path, "abcdef");

    const xer::path target(filesystem_path_to_u8string(file_path));
    const auto status = xer::lstat(target);

    xer_assert(status.has_value());
    xer_assert_eq(status->size, static_cast<std::uintmax_t>(6));
    xer_assert_ne(status->mode, static_cast<std::uintmax_t>(0));
}

void test_lstat_directory() {
    const test_directory_guard guard(make_unique_test_root());
    const fs::path dir_path = guard.path / "dir";
    xer_assert(fs::create_directory(dir_path));

    const xer::path target(filesystem_path_to_u8string(dir_path));
    const auto status = xer::lstat(target);

    xer_assert(status.has_value());
    xer_assert_ne(status->mode, static_cast<std::uintmax_t>(0));
}

void test_lstat_missing_file() {
    const test_directory_guard guard(make_unique_test_root());
    const fs::path file_path = guard.path / "missing.txt";

    const xer::path target(filesystem_path_to_u8string(file_path));
    const auto status = xer::lstat(target);

    xer_assert_not(status.has_value());
    xer_assert_eq(status.error().code, xer::error_t::noent);
}

void test_fstat_binary_stream() {
    const test_directory_guard guard(make_unique_test_root());
    const fs::path file_path = guard.path / "binary.txt";
    write_text_file(file_path, "binary-data");

    const xer::path target(filesystem_path_to_u8string(file_path));
    auto stream = xer::fopen(target, "r");
    xer_assert(stream.has_value());

    const auto status = xer::fstat(*stream);

    xer_assert(status.has_value());
    xer_assert_eq(status->size, static_cast<std::uintmax_t>(11));
}

void test_fstat_text_stream() {
    const test_directory_guard guard(make_unique_test_root());
    const fs::path file_path = guard.path / "text.txt";
    write_text_file(file_path, "text-data");

    const xer::path target(filesystem_path_to_u8string(file_path));
    auto stream = xer::fopen(target, "r", xer::encoding_t::utf8);
    xer_assert(stream.has_value());

    const auto status = xer::fstat(*stream);

    xer_assert(status.has_value());
    xer_assert_eq(status->size, static_cast<std::uintmax_t>(9));
}

void test_fstat_memory_stream_fails() {
    std::byte buffer[4] {};
    auto stream = xer::memopen(buffer, "r+");
    xer_assert(stream.has_value());

    const auto status = xer::fstat(*stream);

    xer_assert_not(status.has_value());
}

void test_filesize_invalid_path_encoding() {
    std::u8string invalid;
    invalid.push_back(static_cast<char8_t>(0x80));

    const xer::path target{std::u8string_view(invalid)};
    const auto size = xer::filesize(target);

    xer_assert_not(size.has_value());
}

} // namespace

int main() {
    test_filesize_regular_file();
    test_lstat_regular_file();
    test_lstat_directory();
    test_lstat_missing_file();
    test_fstat_binary_stream();
    test_fstat_text_stream();
    test_fstat_memory_stream_fails();
    test_filesize_invalid_path_encoding();

    return 0;
}
