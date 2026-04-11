/**
 * @file tests/test_file_entry.cpp
 * @brief Tests for xer/bits/file_entry.h.
 */

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

#ifdef _WIN32
#    include <windows.h>
#else
#    include <sys/stat.h>
#    include <unistd.h>
#endif

#include <xer/assert.h>
#include <xer/bits/file_entry.h>
#include <xer/error.h>
#include <xer/path.h>

namespace {

namespace fs = std::filesystem;

[[nodiscard]] std::u8string filesystem_path_to_u8string(const fs::path& value) {
#ifdef _WIN32
    const std::wstring native = value.native();
    const auto converted = xer::from_native_path(std::wstring_view(native));
    xer_assert(converted.has_value());
    return std::u8string(converted->str());
#else
    const std::string native = value.native();
    const auto converted = xer::from_native_path(std::string_view(native));
    xer_assert(converted.has_value());
    return std::u8string(converted->str());
#endif
}

[[nodiscard]] fs::path make_unique_test_root() {
    const fs::path base = fs::temp_directory_path();
    const auto now =
        std::chrono::high_resolution_clock::now().time_since_epoch().count();

#ifdef _WIN32
    const unsigned long pid = ::GetCurrentProcessId();
#else
    const auto pid = static_cast<unsigned long>(::getpid());
#endif

    return base / ("xer_test_file_entry_" + std::to_string(pid) + "_" +
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

void write_binary_file(const fs::path& path, const std::vector<unsigned char>& data) {
    std::ofstream stream(path, std::ios::binary);
    xer_assert(stream.good());

    if (!data.empty()) {
        stream.write(
            reinterpret_cast<const char*>(data.data()),
            static_cast<std::streamsize>(data.size()));
    }

    xer_assert(stream.good());
}

void write_text_file(const fs::path& path, const std::string& contents) {
    std::ofstream stream(path, std::ios::binary);
    xer_assert(stream.good());
    stream.write(contents.data(), static_cast<std::streamsize>(contents.size()));
    xer_assert(stream.good());
}

[[nodiscard]] std::string read_binary_file(const fs::path& path) {
    std::ifstream stream(path, std::ios::binary);
    xer_assert(stream.good());

    return std::string(
        std::istreambuf_iterator<char>(stream),
        std::istreambuf_iterator<char>());
}

void test_remove_regular_file() {
    const test_directory_guard guard(make_unique_test_root());
    const fs::path file_path = guard.path / "remove_me.txt";

    write_text_file(file_path, "hello");

    const xer::path target(filesystem_path_to_u8string(file_path));
    const auto result = xer::remove(target);

    xer_assert(result.has_value());
    xer_assert(!fs::exists(file_path));
}

void test_remove_missing_file() {
    const test_directory_guard guard(make_unique_test_root());
    const fs::path file_path = guard.path / "missing.txt";

    const xer::path target(filesystem_path_to_u8string(file_path));
    const auto result = xer::remove(target);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::noent);
}

void test_remove_directory_fails() {
    const test_directory_guard guard(make_unique_test_root());
    const fs::path dir_path = guard.path / "dir_target";

    xer_assert(fs::create_directory(dir_path));

    const xer::path target(filesystem_path_to_u8string(dir_path));
    const auto result = xer::remove(target);

    xer_assert(!result.has_value());
    xer_assert(fs::exists(dir_path));
    xer_assert(fs::is_directory(dir_path));
}

void test_rename_regular_file() {
    const test_directory_guard guard(make_unique_test_root());
    const fs::path from_path = guard.path / "before.txt";
    const fs::path to_path = guard.path / "after.txt";

    write_text_file(from_path, "rename");

    const xer::path from(filesystem_path_to_u8string(from_path));
    const xer::path to(filesystem_path_to_u8string(to_path));

    const auto result = xer::rename(from, to);

    xer_assert(result.has_value());
    xer_assert(!fs::exists(from_path));
    xer_assert(fs::exists(to_path));
    xer_assert_eq(read_binary_file(to_path), std::string("rename"));
}

void test_rename_missing_source() {
    const test_directory_guard guard(make_unique_test_root());
    const fs::path from_path = guard.path / "missing.txt";
    const fs::path to_path = guard.path / "dest.txt";

    const xer::path from(filesystem_path_to_u8string(from_path));
    const xer::path to(filesystem_path_to_u8string(to_path));

    const auto result = xer::rename(from, to);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::noent);
    xer_assert(!fs::exists(to_path));
}

void test_rename_directory_same_volume() {
    const test_directory_guard guard(make_unique_test_root());
    const fs::path from_path = guard.path / "dir_before";
    const fs::path to_path = guard.path / "dir_after";

    xer_assert(fs::create_directory(from_path));
    write_text_file(from_path / "child.txt", "child");

    const xer::path from(filesystem_path_to_u8string(from_path));
    const xer::path to(filesystem_path_to_u8string(to_path));

    const auto result = xer::rename(from, to);

    xer_assert(result.has_value());
    xer_assert(!fs::exists(from_path));
    xer_assert(fs::exists(to_path));
    xer_assert(fs::is_directory(to_path));
    xer_assert(fs::exists(to_path / "child.txt"));
}

void test_mkdir_creates_single_directory() {
    const test_directory_guard guard(make_unique_test_root());
    const fs::path dir_path = guard.path / "created_dir";

    const xer::path target(filesystem_path_to_u8string(dir_path));
    const auto result = xer::mkdir(target);

    xer_assert(result.has_value());
    xer_assert(fs::exists(dir_path));
    xer_assert(fs::is_directory(dir_path));
}

void test_mkdir_existing_directory_fails() {
    const test_directory_guard guard(make_unique_test_root());
    const fs::path dir_path = guard.path / "existing_dir";

    xer_assert(fs::create_directory(dir_path));

    const xer::path target(filesystem_path_to_u8string(dir_path));
    const auto result = xer::mkdir(target);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::exist);
}

void test_mkdir_missing_parent_fails() {
    const test_directory_guard guard(make_unique_test_root());
    const fs::path dir_path = guard.path / "missing_parent" / "child";

    const xer::path target(filesystem_path_to_u8string(dir_path));
    const auto result = xer::mkdir(target);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::noent);
    xer_assert(!fs::exists(dir_path));
}

void test_rmdir_removes_empty_directory() {
    const test_directory_guard guard(make_unique_test_root());
    const fs::path dir_path = guard.path / "empty_dir";

    xer_assert(fs::create_directory(dir_path));

    const xer::path target(filesystem_path_to_u8string(dir_path));
    const auto result = xer::rmdir(target);

    xer_assert(result.has_value());
    xer_assert(!fs::exists(dir_path));
}

void test_rmdir_nonempty_directory_fails() {
    const test_directory_guard guard(make_unique_test_root());
    const fs::path dir_path = guard.path / "nonempty_dir";

    xer_assert(fs::create_directory(dir_path));
    write_text_file(dir_path / "child.txt", "child");

    const xer::path target(filesystem_path_to_u8string(dir_path));
    const auto result = xer::rmdir(target);

    xer_assert(!result.has_value());
    xer_assert(fs::exists(dir_path));
    xer_assert(fs::is_directory(dir_path));
}

void test_rmdir_file_fails() {
    const test_directory_guard guard(make_unique_test_root());
    const fs::path file_path = guard.path / "not_a_dir.txt";

    write_text_file(file_path, "x");

    const xer::path target(filesystem_path_to_u8string(file_path));
    const auto result = xer::rmdir(target);

    xer_assert(!result.has_value());
    xer_assert(fs::exists(file_path));
    xer_assert(fs::is_regular_file(file_path));
}

void test_copy_regular_file() {
    const test_directory_guard guard(make_unique_test_root());
    const fs::path from_path = guard.path / "source.bin";
    const fs::path to_path = guard.path / "dest.bin";

    const std::vector<unsigned char> data = {
        0x00, 0x01, 0x7f, 0x80, 0xff, 0x41, 0x42, 0x43
    };
    write_binary_file(from_path, data);

    const xer::path from(filesystem_path_to_u8string(from_path));
    const xer::path to(filesystem_path_to_u8string(to_path));

    const auto result = xer::copy(from, to);

    xer_assert(result.has_value());
    xer_assert(fs::exists(from_path));
    xer_assert(fs::exists(to_path));
    xer_assert_eq(read_binary_file(from_path), read_binary_file(to_path));
}

void test_copy_missing_source_fails() {
    const test_directory_guard guard(make_unique_test_root());
    const fs::path from_path = guard.path / "missing.bin";
    const fs::path to_path = guard.path / "dest.bin";

    const xer::path from(filesystem_path_to_u8string(from_path));
    const xer::path to(filesystem_path_to_u8string(to_path));

    const auto result = xer::copy(from, to);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::noent);
    xer_assert(!fs::exists(to_path));
}

void test_copy_existing_destination_fails() {
    const test_directory_guard guard(make_unique_test_root());
    const fs::path from_path = guard.path / "source.bin";
    const fs::path to_path = guard.path / "dest.bin";

    write_text_file(from_path, "source");
    write_text_file(to_path, "dest");

    const std::string before = read_binary_file(to_path);

    const xer::path from(filesystem_path_to_u8string(from_path));
    const xer::path to(filesystem_path_to_u8string(to_path));

    const auto result = xer::copy(from, to);

    xer_assert(!result.has_value());
    xer_assert_eq(read_binary_file(to_path), before);
}

void test_copy_directory_fails() {
    const test_directory_guard guard(make_unique_test_root());
    const fs::path from_path = guard.path / "source_dir";
    const fs::path to_path = guard.path / "dest_dir";

    xer_assert(fs::create_directory(from_path));

    const xer::path from(filesystem_path_to_u8string(from_path));
    const xer::path to(filesystem_path_to_u8string(to_path));

    const auto result = xer::copy(from, to);

    xer_assert(!result.has_value());
    xer_assert(!fs::exists(to_path));
}

#ifndef _WIN32
void test_copy_preserves_mode_bits() {
    const test_directory_guard guard(make_unique_test_root());
    const fs::path from_path = guard.path / "mode_source.bin";
    const fs::path to_path = guard.path / "mode_dest.bin";

    write_text_file(from_path, "mode");

    xer_assert(::chmod(from_path.c_str(), 0640) == 0);

    const xer::path from(filesystem_path_to_u8string(from_path));
    const xer::path to(filesystem_path_to_u8string(to_path));

    const auto result = xer::copy(from, to);

    xer_assert(result.has_value());

    struct stat from_stat {};
    struct stat to_stat {};

    xer_assert(::stat(from_path.c_str(), &from_stat) == 0);
    xer_assert(::stat(to_path.c_str(), &to_stat) == 0);

    xer_assert_eq(static_cast<unsigned>(from_stat.st_mode & 07777u),
                  static_cast<unsigned>(to_stat.st_mode & 07777u));
}
#endif

} // namespace

int main() {
    test_remove_regular_file();
    test_remove_missing_file();
    test_remove_directory_fails();

    test_rename_regular_file();
    test_rename_missing_source();
    test_rename_directory_same_volume();

    test_mkdir_creates_single_directory();
    test_mkdir_existing_directory_fails();
    test_mkdir_missing_parent_fails();

    test_rmdir_removes_empty_directory();
    test_rmdir_nonempty_directory_fails();
    test_rmdir_file_fails();

    test_copy_regular_file();
    test_copy_missing_source_fails();
    test_copy_existing_destination_fails();
    test_copy_directory_fails();

#ifndef _WIN32
    test_copy_preserves_mode_bits();
#endif

    return 0;
}
