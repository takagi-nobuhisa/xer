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

    return base / ("xer_test_dirent_" + std::to_string(pid) + "_" +
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

[[nodiscard]] auto read_all_entries(xer::dir& directory) -> std::vector<std::u8string> {
    std::vector<std::u8string> result;

    for (;;) {
        auto entry = xer::readdir(directory);
        if (!entry.has_value()) {
            xer_assert_eq(entry.error().code, xer::error_t::not_found);
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

void test_readdir_end_is_not_found() {
    const test_directory_guard guard(make_unique_test_root());

    const xer::path target(filesystem_path_to_u8string(guard.path));
    auto directory = xer::opendir(target);
    xer_assert(directory.has_value());

    for (;;) {
        const auto entry = xer::readdir(*directory);
        if (!entry.has_value()) {
            xer_assert_eq(entry.error().code, xer::error_t::not_found);
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
    test_readdir_end_is_not_found();
    test_rewinddir_reads_again_from_beginning();
    test_closedir_closes_directory();
    test_opendir_missing_directory_fails();
    test_opendir_file_fails();
    test_opendir_invalid_path_encoding();

    return 0;
}
