/**
 * @file tests/test_binary_stream_io.cpp
 * @brief Tests for xer/bits/binary_stream_io.h.
 */

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <vector>

#include <xer/assert.h>
#include <xer/bits/binary_stream_io.h>

namespace {

struct binary_stream_mock {
    std::vector<std::byte> read_source;
    std::vector<std::byte> write_sink;

    int read_result = 0;
    int write_result = 0;

    int last_read_count = -1;
    int last_write_count = -1;

    int close_count = 0;
};

int close_stub(xer::binary_stream_handle_t handle) noexcept {
    auto* mock = reinterpret_cast<binary_stream_mock*>(handle);
    ++mock->close_count;
    return 0;
}

int read_stub(xer::binary_stream_handle_t handle, void* s, int n) noexcept {
    auto* mock = reinterpret_cast<binary_stream_mock*>(handle);
    mock->last_read_count = n;

    if (mock->read_result < 0) {
        return -1;
    }

    const int actual =
        (mock->read_result <= n) ? mock->read_result : n;

    if (actual > 0) {
        std::memcpy(s, mock->read_source.data(), static_cast<std::size_t>(actual));
    }

    return actual;
}

int write_stub(xer::binary_stream_handle_t handle, const void* s, int n) noexcept {
    auto* mock = reinterpret_cast<binary_stream_mock*>(handle);
    mock->last_write_count = n;

    if (mock->write_result < 0) {
        return -1;
    }

    const int actual =
        (mock->write_result <= n) ? mock->write_result : n;

    if (actual > 0) {
        const auto* bytes = static_cast<const std::byte*>(s);
        mock->write_sink.assign(bytes, bytes + actual);
    } else {
        mock->write_sink.clear();
    }

    return actual;
}

int seek_unused(xer::binary_stream_handle_t, std::int64_t, int) noexcept {
    return -1;
}

std::int64_t tell_unused(xer::binary_stream_handle_t) noexcept {
    return -1;
}

xer::binary_stream make_stream(binary_stream_mock& mock) {
    return xer::binary_stream(
        reinterpret_cast<xer::binary_stream_handle_t>(&mock),
        close_stub,
        read_stub,
        write_stub,
        seek_unused,
        tell_unused);
}

void test_fread_reads_all_bytes() {
    binary_stream_mock mock{};
    mock.read_source = {
        std::byte{0x11},
        std::byte{0x22},
        std::byte{0x33},
    };
    mock.read_result = 3;

    auto stream = make_stream(mock);
    std::array<std::byte, 3> buffer{};

    const auto result = xer::fread(std::span<std::byte>(buffer), stream);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(3));
    xer_assert_eq(mock.last_read_count, 3);
    xer_assert_eq(buffer[0], std::byte{0x11});
    xer_assert_eq(buffer[1], std::byte{0x22});
    xer_assert_eq(buffer[2], std::byte{0x33});
}

void test_fread_partial_read_is_success() {
    binary_stream_mock mock{};
    mock.read_source = {
        std::byte{0xaa},
        std::byte{0xbb},
        std::byte{0xcc},
    };
    mock.read_result = 2;

    auto stream = make_stream(mock);
    std::array<std::byte, 3> buffer{
        std::byte{0x00},
        std::byte{0x00},
        std::byte{0x00},
    };

    const auto result = xer::fread(std::span<std::byte>(buffer), stream);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(2));
    xer_assert_eq(buffer[0], std::byte{0xaa});
    xer_assert_eq(buffer[1], std::byte{0xbb});
    xer_assert_eq(buffer[2], std::byte{0x00});
}

void test_fread_zero_length_buffer() {
    binary_stream_mock mock{};
    mock.read_result = 5;

    auto stream = make_stream(mock);
    std::span<std::byte> buffer{};

    const auto result = xer::fread(buffer, stream);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(0));
    xer_assert_eq(mock.last_read_count, -1);
}

void test_fread_failure() {
    binary_stream_mock mock{};
    mock.read_result = -1;

    auto stream = make_stream(mock);
    std::array<std::byte, 4> buffer{};

    const auto result = xer::fread(std::span<std::byte>(buffer), stream);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::runtime_error);
    xer_assert_eq(mock.last_read_count, 4);
}

void test_fwrite_writes_all_bytes() {
    binary_stream_mock mock{};
    mock.write_result = 3;

    auto stream = make_stream(mock);
    const std::array<std::byte, 3> buffer{
        std::byte{0x41},
        std::byte{0x42},
        std::byte{0x43},
    };

    const auto result = xer::fwrite(std::span<const std::byte>(buffer), stream);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(3));
    xer_assert_eq(mock.last_write_count, 3);
    xer_assert_eq(mock.write_sink.size(), static_cast<std::size_t>(3));
    xer_assert_eq(mock.write_sink[0], std::byte{0x41});
    xer_assert_eq(mock.write_sink[1], std::byte{0x42});
    xer_assert_eq(mock.write_sink[2], std::byte{0x43});
}

void test_fwrite_partial_write_is_success() {
    binary_stream_mock mock{};
    mock.write_result = 2;

    auto stream = make_stream(mock);
    const std::array<std::byte, 3> buffer{
        std::byte{0x90},
        std::byte{0x91},
        std::byte{0x92},
    };

    const auto result = xer::fwrite(std::span<const std::byte>(buffer), stream);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(2));
    xer_assert_eq(mock.write_sink.size(), static_cast<std::size_t>(2));
    xer_assert_eq(mock.write_sink[0], std::byte{0x90});
    xer_assert_eq(mock.write_sink[1], std::byte{0x91});
}

void test_fwrite_zero_length_buffer() {
    binary_stream_mock mock{};
    mock.write_result = 5;

    auto stream = make_stream(mock);
    std::span<const std::byte> buffer{};

    const auto result = xer::fwrite(buffer, stream);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(0));
    xer_assert_eq(mock.last_write_count, -1);
}

void test_fwrite_failure() {
    binary_stream_mock mock{};
    mock.write_result = -1;

    auto stream = make_stream(mock);
    const std::array<std::byte, 2> buffer{
        std::byte{0x01},
        std::byte{0x02},
    };

    const auto result = xer::fwrite(std::span<const std::byte>(buffer), stream);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::runtime_error);
    xer_assert_eq(mock.last_write_count, 2);
}

void test_fgetb_success() {
    binary_stream_mock mock{};
    mock.read_source = {std::byte{0x7f}};
    mock.read_result = 1;

    auto stream = make_stream(mock);

    const auto result = xer::fgetb(stream);

    xer_assert(result.has_value());
    xer_assert_eq(*result, std::byte{0x7f});
    xer_assert_eq(mock.last_read_count, 1);
}

void test_fgetb_eof_is_failure() {
    binary_stream_mock mock{};
    mock.read_result = 0;

    auto stream = make_stream(mock);

    const auto result = xer::fgetb(stream);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::runtime_error);
    xer_assert_eq(mock.last_read_count, 1);
}

void test_fgetb_error_is_failure() {
    binary_stream_mock mock{};
    mock.read_result = -1;

    auto stream = make_stream(mock);

    const auto result = xer::fgetb(stream);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::runtime_error);
    xer_assert_eq(mock.last_read_count, 1);
}

void test_fputb_success() {
    binary_stream_mock mock{};
    mock.write_result = 1;

    auto stream = make_stream(mock);

    const auto result = xer::fputb(std::byte{0xa5}, stream);

    xer_assert(result.has_value());
    xer_assert_eq(mock.last_write_count, 1);
    xer_assert_eq(mock.write_sink.size(), static_cast<std::size_t>(1));
    xer_assert_eq(mock.write_sink[0], std::byte{0xa5});
}

void test_fputb_zero_write_is_failure() {
    binary_stream_mock mock{};
    mock.write_result = 0;

    auto stream = make_stream(mock);

    const auto result = xer::fputb(std::byte{0x5a}, stream);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::runtime_error);
    xer_assert_eq(mock.last_write_count, 1);
}

void test_fputb_error_is_failure() {
    binary_stream_mock mock{};
    mock.write_result = -1;

    auto stream = make_stream(mock);

    const auto result = xer::fputb(std::byte{0x5a}, stream);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::runtime_error);
    xer_assert_eq(mock.last_write_count, 1);
}

void test_binary_stream_io_size_to_int_out_of_range() {
    const auto result = xer::detail::binary_stream_io_size_to_int(
        static_cast<std::size_t>(std::numeric_limits<int>::max()) + 1u);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::out_of_range);
}

} // namespace

int main() {
    test_fread_reads_all_bytes();
    test_fread_partial_read_is_success();
    test_fread_zero_length_buffer();
    test_fread_failure();

    test_fwrite_writes_all_bytes();
    test_fwrite_partial_write_is_success();
    test_fwrite_zero_length_buffer();
    test_fwrite_failure();

    test_fgetb_success();
    test_fgetb_eof_is_failure();
    test_fgetb_error_is_failure();

    test_fputb_success();
    test_fputb_zero_write_is_failure();
    test_fputb_error_is_failure();

    test_binary_stream_io_size_to_int_out_of_range();

    return 0;
}
