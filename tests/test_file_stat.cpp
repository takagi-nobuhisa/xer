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
#include "test_helpers.h"

namespace {

using xer_test::filesystem_path_to_u8string;
using xer_test::make_unique_test_root;
using xer_test::test_directory_guard;
using xer_test::write_text_file;

namespace fs = std::filesystem;

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


void test_file_time_helpers_match_lstat() {
    const test_directory_guard guard(make_unique_test_root());
    const fs::path file_path = guard.path / "time.txt";
    write_text_file(file_path, "time");

    const xer::path target(filesystem_path_to_u8string(file_path));

    const auto status = xer::lstat(target);
    xer_assert(status.has_value());

    const auto atime = xer::fileatime(target);
    const auto mtime = xer::filemtime(target);
    const auto ctime = xer::filectime(target);

    xer_assert(atime.has_value());
    xer_assert(mtime.has_value());
    xer_assert(ctime.has_value());

    xer_assert_eq(*atime, static_cast<xer::time_t>(status->atime));
    xer_assert_eq(*mtime, static_cast<xer::time_t>(status->mtime));
    xer_assert_eq(*ctime, static_cast<xer::time_t>(status->ctime));
}

void test_file_time_helpers_missing_file() {
    const test_directory_guard guard(make_unique_test_root());
    const fs::path file_path = guard.path / "missing_time.txt";

    const xer::path target(filesystem_path_to_u8string(file_path));

    const auto atime = xer::fileatime(target);
    const auto mtime = xer::filemtime(target);
    const auto ctime = xer::filectime(target);

    xer_assert_not(atime.has_value());
    xer_assert_not(mtime.has_value());
    xer_assert_not(ctime.has_value());
    xer_assert_eq(atime.error().code, xer::error_t::noent);
    xer_assert_eq(mtime.error().code, xer::error_t::noent);
    xer_assert_eq(ctime.error().code, xer::error_t::noent);
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
    test_file_time_helpers_match_lstat();
    test_file_time_helpers_missing_file();
    test_filesize_invalid_path_encoding();

    return 0;
}
