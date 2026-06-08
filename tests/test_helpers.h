/**
 * @file tests/test_helpers.h
 * @brief Shared helpers for tests.
 */

#ifndef XER_TEST_HELPERS_H
#define XER_TEST_HELPERS_H

#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#ifdef _WIN32
#    include <windows.h>
#else
#    include <unistd.h>
#endif

#include <xer/assert.h>
#include <xer/path.h>

namespace xer_test {

namespace fs = std::filesystem;

inline auto filesystem_path_to_u8string(const fs::path& value) -> std::u8string
{
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

inline auto make_unique_test_root(std::string_view prefix = "xer_test") -> fs::path
{
    const fs::path base = fs::temp_directory_path();
    const auto now =
        std::chrono::high_resolution_clock::now().time_since_epoch().count();

#ifdef _WIN32
    const unsigned long pid = ::GetCurrentProcessId();
#else
    const auto pid = static_cast<unsigned long>(::getpid());
#endif

    return base / (std::string(prefix) + "_" + std::to_string(pid) + "_" +
                   std::to_string(static_cast<long long>(now)));
}

struct test_directory_guard {
    fs::path path;

    explicit test_directory_guard(fs::path value)
        : path(std::move(value))
    {
        fs::create_directories(path);
    }

    ~test_directory_guard()
    {
        std::error_code ec;
        fs::remove_all(path, ec);
    }
};

inline auto write_text_file(const fs::path& path, std::string_view contents) -> void
{
    std::ofstream stream(path, std::ios::binary);
    xer_assert(stream.good());
    stream.write(contents.data(), static_cast<std::streamsize>(contents.size()));
    xer_assert(stream.good());
}

inline auto write_binary_file(
    const fs::path& path,
    std::span<const std::byte> contents) -> void
{
    std::ofstream stream(path, std::ios::binary);
    xer_assert(stream.good());

    if (!contents.empty()) {
        stream.write(
            reinterpret_cast<const char*>(contents.data()),
            static_cast<std::streamsize>(contents.size()));
    }

    xer_assert(stream.good());
}

inline auto write_binary_file(
    const fs::path& path,
    std::span<const unsigned char> contents) -> void
{
    std::ofstream stream(path, std::ios::binary);
    xer_assert(stream.good());

    if (!contents.empty()) {
        stream.write(
            reinterpret_cast<const char*>(contents.data()),
            static_cast<std::streamsize>(contents.size()));
    }

    xer_assert(stream.good());
}

inline auto write_binary_file(const fs::path& path, std::u8string_view contents) -> void
{
    std::ofstream stream(path, std::ios::binary);
    xer_assert(stream.good());
    stream.write(
        reinterpret_cast<const char*>(contents.data()),
        static_cast<std::streamsize>(contents.size()));
    xer_assert(stream.good());
}

inline auto read_binary_file(const fs::path& path) -> std::string
{
    std::ifstream stream(path, std::ios::binary);
    xer_assert(stream.good());

    return std::string(
        std::istreambuf_iterator<char>(stream),
        std::istreambuf_iterator<char>());
}

inline auto bytes(std::string_view text) -> std::vector<std::byte>
{
    std::vector<std::byte> result;
    result.reserve(text.size());

    for (const char c : text) {
        result.push_back(static_cast<std::byte>(static_cast<unsigned char>(c)));
    }

    return result;
}

inline auto assert_bytes_eq(
    std::span<const std::byte> lhs,
    std::span<const std::byte> rhs) -> void
{
    xer_assert_eq(lhs.size(), rhs.size());

    for (std::size_t i = 0; i < lhs.size(); ++i) {
        xer_assert(lhs[i] == rhs[i]);
    }
}

} // namespace xer_test

#endif
