/**
 * @file tests/test_dirent.cpp
 * @brief Tests for xer/dirent.h.
 */

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

#ifdef _WIN32
#    include <windows.h>
#else
#    include <unistd.h>
#endif

#include <xer/assert.h>
#include <xer/dirent.h>
#include <xer/error.h>
#include <xer/path.h>
#include "test_helpers.h"

namespace {

using xer_test::filesystem_path_to_u8string;
using xer_test::make_unique_test_root;
using xer_test::test_directory_guard;
using xer_test::write_text_file;

namespace fs = std::filesystem;

[[nodiscard]] auto read_all_entries(xer::dir& directory) -> std::vector<std::u8string> {
    std::vector<std::u8string> result;

    for (;;) {
        auto entry = xer::readdir(directory);
        if (!entry.has_value()) {
            xer_assert_eq(entry.error().code, xer::error_t::end_of_file);
            break;
        }

        result.push_back(*entry);
    }

    std::ranges::sort(result);
    return result;
}

[[nodiscard]] auto contains(
    const std::vector<std::u8string>& entries,
    std::u8string_view name) -> bool {
    return std::ranges::find(entries, name) != entries.end();
}

void test_opendir_and_readdir_basic() {
    const test_directory_guard guard(make_unique_test_root());
    write_text_file(guard.path / "alpha.txt", "alpha");
    write_text_file(guard.path / "beta.txt", "beta");
    xer_assert(fs::create_directory(guard.path / "child"));

    const xer::path target(filesystem_path_to_u8string(guard.path));
    auto directory = xer::opendir(target);

    xer_assert(directory.has_value());
    xer_assert(directory->is_open());

    const auto entries = read_all_entries(*directory);

    xer_assert(contains(entries, u8"."));
    xer_assert(contains(entries, u8".."));
    xer_assert(contains(entries, u8"alpha.txt"));
    xer_assert(contains(entries, u8"beta.txt"));
    xer_assert(contains(entries, u8"child"));
}

void test_readdir_end_is_end_of_file() {
    const test_directory_guard guard(make_unique_test_root());

    const xer::path target(filesystem_path_to_u8string(guard.path));
    auto directory = xer::opendir(target);
    xer_assert(directory.has_value());

    for (;;) {
        const auto entry = xer::readdir(*directory);
        if (!entry.has_value()) {
            xer_assert_eq(entry.error().code, xer::error_t::end_of_file);
            break;
        }
    }
}

void test_rewinddir_reads_again_from_beginning() {
    const test_directory_guard guard(make_unique_test_root());
    write_text_file(guard.path / "again.txt", "again");

    const xer::path target(filesystem_path_to_u8string(guard.path));
    auto directory = xer::opendir(target);
    xer_assert(directory.has_value());

    const auto first_entries = read_all_entries(*directory);
    xer_assert(contains(first_entries, u8"again.txt"));

    const auto rewind_result = xer::rewinddir(*directory);
    xer_assert(rewind_result.has_value());

    const auto second_entries = read_all_entries(*directory);
    xer_assert_eq(first_entries.size(), second_entries.size());
    xer_assert(contains(second_entries, u8"again.txt"));
}

void test_closedir_closes_directory() {
    const test_directory_guard guard(make_unique_test_root());

    const xer::path target(filesystem_path_to_u8string(guard.path));
    auto directory = xer::opendir(target);
    xer_assert(directory.has_value());
    xer_assert(directory->is_open());

    const auto close_result = xer::closedir(*directory);
    xer_assert(close_result.has_value());
    xer_assert_not(directory->is_open());

    const auto entry = xer::readdir(*directory);
    xer_assert_not(entry.has_value());
    xer_assert_eq(entry.error().code, xer::error_t::badf);
}

void test_opendir_missing_directory_fails() {
    const test_directory_guard guard(make_unique_test_root());
    const fs::path missing = guard.path / "missing";

    const xer::path target(filesystem_path_to_u8string(missing));
    const auto directory = xer::opendir(target);

    xer_assert_not(directory.has_value());
    xer_assert_eq(directory.error().code, xer::error_t::noent);
}

void test_opendir_file_fails() {
    const test_directory_guard guard(make_unique_test_root());
    const fs::path file_path = guard.path / "not_directory.txt";
    write_text_file(file_path, "file");

    const xer::path target(filesystem_path_to_u8string(file_path));
    const auto directory = xer::opendir(target);

    xer_assert_not(directory.has_value());
}

void test_opendir_invalid_path_encoding() {
    std::u8string invalid;
    invalid.push_back(static_cast<char8_t>(0x80));

    const xer::path target{std::u8string_view(invalid)};
    const auto directory = xer::opendir(target);

    xer_assert_not(directory.has_value());
}

} // namespace

int main() {
    test_opendir_and_readdir_basic();
    test_readdir_end_is_end_of_file();
    test_rewinddir_reads_again_from_beginning();
    test_closedir_closes_directory();
    test_opendir_missing_directory_fails();
    test_opendir_file_fails();
    test_opendir_invalid_path_encoding();

    return 0;
}
