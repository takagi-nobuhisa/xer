/**
 * @file tests/test_fopen.cpp
 * @brief Tests for xer/bits/fopen.h.
 */

#include <array>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <span>
#include <string>
#include <vector>

#include <xer/assert.h>
#include <xer/bits/fclose.h>
#include <xer/bits/fopen.h>
#include <xer/path.h>

namespace {

[[nodiscard]] xer::path make_temp_path(const char8_t* suffix) {
    const auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();

    std::filesystem::path native_path =
        std::filesystem::temp_directory_path() /
        ("xer_test_" + std::to_string(static_cast<long long>(now)) +
         reinterpret_cast<const char*>(suffix));

    return xer::path(native_path.u8string());
}

void remove_if_exists(const xer::path& path_value) {
    const auto native = xer::to_native_path(path_value);
    if (!native.has_value()) {
        return;
    }

    std::filesystem::remove(std::filesystem::path(*native));
}

void test_fopen_binary_file_round_trip() {
    const xer::path file_path = make_temp_path(u8"_binary_round_trip.bin");
    remove_if_exists(file_path);

    {
        auto stream_result = xer::fopen(file_path, "wb");
        xer_assert(stream_result.has_value());

        xer::binary_stream stream = std::move(stream_result.value());

        constexpr std::array<std::byte, 4> expected = {
            std::byte{0x11},
            std::byte{0x22},
            std::byte{0x33},
            std::byte{0x44},
        };

        const int written =
            stream.write_fn()(stream.handle(), expected.data(), static_cast<int>(expected.size()));
        xer_assert_eq(written, static_cast<int>(expected.size()));

        const auto close_result = xer::fclose(stream);
        xer_assert(close_result.has_value());
        xer_assert(!stream.has_value());
    }

    {
        auto stream_result = xer::fopen(file_path, "rb");
        xer_assert(stream_result.has_value());

        xer::binary_stream stream = std::move(stream_result.value());

        std::array<std::byte, 4> actual = {};
        const int read =
            stream.read_fn()(stream.handle(), actual.data(), static_cast<int>(actual.size()));
        xer_assert_eq(read, static_cast<int>(actual.size()));
        xer_assert_eq(actual[0], std::byte{0x11});
        xer_assert_eq(actual[1], std::byte{0x22});
        xer_assert_eq(actual[2], std::byte{0x33});
        xer_assert_eq(actual[3], std::byte{0x44});

        const auto close_result = xer::fclose(stream);
        xer_assert(close_result.has_value());
    }

    remove_if_exists(file_path);
}

void test_fopen_binary_invalid_mode() {
    const xer::path file_path = make_temp_path(u8"_binary_invalid_mode.bin");
    const auto result = xer::fopen(file_path, "rt");
    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_fopen_text_file_position_ops() {
    const xer::path file_path = make_temp_path(u8"_text_position_ops.txt");
    remove_if_exists(file_path);

    {
        const auto native = xer::to_native_path(file_path);
        xer_assert(native.has_value());

#ifdef _WIN32
        FILE* fp = _wfopen(native->c_str(), L"wb");
#else
        FILE* fp = std::fopen(native->c_str(), "wb");
#endif
        xer_assert(fp != nullptr);

        const char bytes[] = "abcdef";
        const std::size_t written = std::fwrite(bytes, 1, sizeof(bytes) - 1, fp);
        xer_assert_eq(written, sizeof(bytes) - 1);
        xer_assert_eq(std::fclose(fp), 0);
    }

    auto stream_result = xer::fopen(file_path, "rt", xer::encoding_t::utf8);
    xer_assert(stream_result.has_value());

    xer::text_stream stream = std::move(stream_result.value());

    const auto pos0 = stream.getpos_fn()(stream.handle());
    xer_assert_eq(pos0, 0);

    const int seek_end_result = stream.seek_end_fn()(stream.handle());
    xer_assert_eq(seek_end_result, 0);

    const auto end_pos = stream.getpos_fn()(stream.handle());
    xer_assert_eq(end_pos, 6);

    const int setpos_result = stream.setpos_fn()(stream.handle(), 2);
    xer_assert_eq(setpos_result, 0);

    const auto pos2 = stream.getpos_fn()(stream.handle());
    xer_assert_eq(pos2, 2);

    const auto close_result = xer::fclose(stream);
    xer_assert(close_result.has_value());
    xer_assert(!stream.has_value());

    remove_if_exists(file_path);
}

void test_fopen_text_auto_detect_write_invalid() {
    const xer::path file_path = make_temp_path(u8"_text_auto_detect_invalid.txt");
    const auto result = xer::fopen(file_path, "w", xer::encoding_t::auto_detect);
    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_memopen_round_trip() {
    std::array<std::byte, 8> buffer = {};

    auto stream_result = xer::memopen(std::span<std::byte>(buffer), "r+");
    xer_assert(stream_result.has_value());

    xer::binary_stream stream = std::move(stream_result.value());

    constexpr std::array<std::byte, 3> expected = {
        std::byte{0xaa},
        std::byte{0xbb},
        std::byte{0xcc},
    };

    const int written =
        stream.write_fn()(stream.handle(), expected.data(), static_cast<int>(expected.size()));
    xer_assert_eq(written, static_cast<int>(expected.size()));

    xer_assert_eq(stream.seek_fn()(stream.handle(), 0, SEEK_SET), 0);
    xer_assert_eq(stream.tell_fn()(stream.handle()), 0);

    std::array<std::byte, 3> actual = {};
    const int read =
        stream.read_fn()(stream.handle(), actual.data(), static_cast<int>(actual.size()));
    xer_assert_eq(read, static_cast<int>(actual.size()));

    xer_assert_eq(actual[0], std::byte{0xaa});
    xer_assert_eq(actual[1], std::byte{0xbb});
    xer_assert_eq(actual[2], std::byte{0xcc});

    const auto close_result = xer::fclose(stream);
    xer_assert(close_result.has_value());
}

void test_stropen_view_read_only() {
    const std::u8string_view text = u8"hello";

    auto stream_result = xer::stropen(text, "r");
    xer_assert(stream_result.has_value());

    xer::text_stream stream = std::move(stream_result.value());

    xer_assert_eq(stream.getpos_fn()(stream.handle()), 0);
    xer_assert_eq(stream.seek_end_fn()(stream.handle()), 0);
    xer_assert_eq(stream.getpos_fn()(stream.handle()), 5);
    xer_assert_eq(stream.setpos_fn()(stream.handle(), 1), 0);
    xer_assert_eq(stream.getpos_fn()(stream.handle()), 1);

    const auto close_result = xer::fclose(stream);
    xer_assert(close_result.has_value());
}

void test_stropen_view_invalid_mode() {
    const std::u8string_view text = u8"hello";
    const auto result = xer::stropen(text, "w");
    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_stropen_string_truncate_and_append_open_position() {
    {
        std::u8string text = u8"abcdef";
        auto stream_result = xer::stropen(text, "w");
        xer_assert(stream_result.has_value());
        xer_assert_eq(text, std::u8string());

        xer::text_stream stream = std::move(stream_result.value());
        xer_assert_eq(stream.getpos_fn()(stream.handle()), 0);

        const auto close_result = xer::fclose(stream);
        xer_assert(close_result.has_value());
    }

    {
        std::u8string text = u8"abcdef";
        auto stream_result = xer::stropen(text, "a");
        xer_assert(stream_result.has_value());

        xer::text_stream stream = std::move(stream_result.value());
        xer_assert_eq(stream.getpos_fn()(stream.handle()), 6);

        const auto close_result = xer::fclose(stream);
        xer_assert(close_result.has_value());
    }
}

} // namespace

int main() {
    test_fopen_binary_file_round_trip();
    test_fopen_binary_invalid_mode();
    test_fopen_text_file_position_ops();
    test_fopen_text_auto_detect_write_invalid();
    test_memopen_round_trip();
    test_stropen_view_read_only();
    test_stropen_view_invalid_mode();
    test_stropen_string_truncate_and_append_open_position();
    return 0;
}
