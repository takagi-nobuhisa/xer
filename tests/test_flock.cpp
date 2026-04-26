/**
 * @file tests/test_flock.cpp
 * @brief Tests for xer/bits/flock.h.
 */

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

    return base / ("xer_test_flock_" + std::to_string(pid) + "_" +
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

void test_lock_constants_can_be_combined() {
    const auto operation = xer::lock_ex | xer::lock_nb;

    xer_assert_eq(operation & xer::lock_ex, xer::lock_ex);
    xer_assert_eq(operation & xer::lock_nb, xer::lock_nb);
}

void test_binary_exclusive_lock_blocks_second_exclusive_lock() {
    const test_directory_guard guard(make_unique_test_root());
    const fs::path file_path = guard.path / "binary_lock.dat";
    write_text_file(file_path, "lock");

    const xer::path target(filesystem_path_to_u8string(file_path));

    auto first = xer::fopen(target, "r+");
    xer_assert(first.has_value());

    auto second = xer::fopen(target, "r+");
    xer_assert(second.has_value());

    const auto first_lock = xer::flock(*first, xer::lock_ex);
    xer_assert(first_lock.has_value());

    bool would_block = false;
    const auto second_lock = xer::flock(
        *second,
        xer::lock_ex | xer::lock_nb,
        &would_block);

    xer_assert_not(second_lock.has_value());
    xer_assert_eq(second_lock.error().code, xer::error_t::busy);
    xer_assert(would_block);

    const auto first_unlock = xer::flock(*first, xer::lock_un);
    xer_assert(first_unlock.has_value());

    would_block = false;
    const auto second_retry = xer::flock(
        *second,
        xer::lock_ex | xer::lock_nb,
        &would_block);

    xer_assert(second_retry.has_value());
    xer_assert_not(would_block);

    const auto second_unlock = xer::flock(*second, xer::lock_un);
    xer_assert(second_unlock.has_value());
}

void test_binary_shared_locks_can_coexist() {
    const test_directory_guard guard(make_unique_test_root());
    const fs::path file_path = guard.path / "shared_lock.dat";
    write_text_file(file_path, "lock");

    const xer::path target(filesystem_path_to_u8string(file_path));

    auto first = xer::fopen(target, "r");
    xer_assert(first.has_value());

    auto second = xer::fopen(target, "r");
    xer_assert(second.has_value());

    bool would_block = false;

    const auto first_lock = xer::flock(*first, xer::lock_sh | xer::lock_nb, &would_block);
    xer_assert(first_lock.has_value());
    xer_assert_not(would_block);

    const auto second_lock = xer::flock(*second, xer::lock_sh | xer::lock_nb, &would_block);
    xer_assert(second_lock.has_value());
    xer_assert_not(would_block);

    xer_assert(xer::flock(*second, xer::lock_un).has_value());
    xer_assert(xer::flock(*first, xer::lock_un).has_value());
}

void test_text_exclusive_lock_blocks_second_exclusive_lock() {
    const test_directory_guard guard(make_unique_test_root());
    const fs::path file_path = guard.path / "text_lock.txt";
    write_text_file(file_path, "lock");

    const xer::path target(filesystem_path_to_u8string(file_path));

    auto first = xer::fopen(target, "r+", xer::encoding_t::utf8);
    xer_assert(first.has_value());

    auto second = xer::fopen(target, "r+", xer::encoding_t::utf8);
    xer_assert(second.has_value());

    const auto first_lock = xer::flock(*first, xer::lock_ex);
    xer_assert(first_lock.has_value());

    bool would_block = false;
    const auto second_lock = xer::flock(
        *second,
        xer::lock_ex | xer::lock_nb,
        &would_block);

    xer_assert_not(second_lock.has_value());
    xer_assert_eq(second_lock.error().code, xer::error_t::busy);
    xer_assert(would_block);

    xer_assert(xer::flock(*first, xer::lock_un).has_value());

    would_block = false;
    const auto second_retry = xer::flock(
        *second,
        xer::lock_ex | xer::lock_nb,
        &would_block);

    xer_assert(second_retry.has_value());
    xer_assert_not(would_block);

    xer_assert(xer::flock(*second, xer::lock_un).has_value());
}

void test_invalid_operation_fails() {
    const test_directory_guard guard(make_unique_test_root());
    const fs::path file_path = guard.path / "invalid_lock.dat";
    write_text_file(file_path, "lock");

    const xer::path target(filesystem_path_to_u8string(file_path));

    auto stream = xer::fopen(target, "r+");
    xer_assert(stream.has_value());

    const auto result = xer::flock(
        *stream,
        static_cast<xer::lock_t>(0));

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_memory_stream_flock_fails() {
    std::byte buffer[4] {};
    auto stream = xer::memopen(buffer, "r+");
    xer_assert(stream.has_value());

    bool would_block = true;
    const auto result = xer::flock(*stream, xer::lock_ex | xer::lock_nb, &would_block);

    xer_assert_not(result.has_value());
    xer_assert_not(would_block);
}

} // namespace

int main() {
    test_lock_constants_can_be_combined();
    test_binary_exclusive_lock_blocks_second_exclusive_lock();
    test_binary_shared_locks_can_coexist();
    test_text_exclusive_lock_blocks_second_exclusive_lock();
    test_invalid_operation_fails();
    test_memory_stream_flock_fails();

    return 0;
}
