/**
 * @file tests/test_stream_position_io.cpp
 * @brief Tests for xer/bits/stream_position_io.h.
 */

#include <cstdint>
#include <limits>

#include <xer/assert.h>
#include <xer/bits/stream_position_io.h>

namespace {

struct binary_stream_mock {
    std::int64_t position = 0;
    int seek_return = 0;
    std::int64_t tell_return = 0;
    int close_count = 0;
    std::int64_t last_seek_offset = 0;
    int last_seek_origin = -1;
};

int binary_close_noop(xer::binary_stream_handle_t handle) noexcept {
    auto* mock = reinterpret_cast<binary_stream_mock*>(handle);
    ++mock->close_count;
    return 0;
}

int binary_read_unused(xer::binary_stream_handle_t, void*, int) noexcept {
    return -1;
}

int binary_write_unused(xer::binary_stream_handle_t, const void*, int) noexcept {
    return -1;
}

int binary_seek_stub(xer::binary_stream_handle_t handle, std::int64_t pos, int whence) noexcept {
    auto* mock = reinterpret_cast<binary_stream_mock*>(handle);
    mock->last_seek_offset = pos;
    mock->last_seek_origin = whence;

    if (mock->seek_return < 0) {
        return -1;
    }

    switch (whence) {
        case xer::seek_set:
            mock->position = pos;
            break;
        case xer::seek_cur:
            mock->position += pos;
            break;
        case xer::seek_end:
            mock->position = 100 + pos;
            break;
        default:
            return -1;
    }

    mock->tell_return = mock->position;
    return 0;
}

std::int64_t binary_tell_stub(xer::binary_stream_handle_t handle) noexcept {
    auto* mock = reinterpret_cast<binary_stream_mock*>(handle);
    return mock->tell_return;
}

xer::binary_stream make_binary_stream(binary_stream_mock& mock) {
    return xer::binary_stream(
        reinterpret_cast<xer::binary_stream_handle_t>(&mock),
        binary_close_noop,
        binary_read_unused,
        binary_write_unused,
        binary_seek_stub,
        binary_tell_stub);
}

struct text_stream_mock {
    xer::text_stream_pos_t position = 0;
    int seek_end_return = 0;
    xer::text_stream_pos_t getpos_return = 0;
    int setpos_return = 0;
    int close_count = 0;
    int seek_end_count = 0;
    xer::text_stream_pos_t last_setpos = -1;
};

int text_close_noop(xer::text_stream_handle_t handle) noexcept {
    auto* mock = reinterpret_cast<text_stream_mock*>(handle);
    ++mock->close_count;
    return 0;
}

int text_read_unused(xer::text_stream_handle_t, char32_t*, int) noexcept {
    return -1;
}

int text_write_unused(xer::text_stream_handle_t, const char32_t*, int) noexcept {
    return -1;
}

xer::text_stream_pos_t text_getpos_stub(xer::text_stream_handle_t handle) noexcept {
    auto* mock = reinterpret_cast<text_stream_mock*>(handle);
    return mock->getpos_return;
}

int text_setpos_stub(xer::text_stream_handle_t handle, xer::text_stream_pos_t pos) noexcept {
    auto* mock = reinterpret_cast<text_stream_mock*>(handle);
    mock->last_setpos = pos;

    if (mock->setpos_return < 0) {
        return -1;
    }

    mock->position = pos;
    mock->getpos_return = pos;
    return 0;
}

int text_seek_end_stub(xer::text_stream_handle_t handle) noexcept {
    auto* mock = reinterpret_cast<text_stream_mock*>(handle);
    ++mock->seek_end_count;

    if (mock->seek_end_return < 0) {
        return -1;
    }

    mock->position = 999;
    mock->getpos_return = 999;
    return 0;
}

xer::text_stream make_text_stream(text_stream_mock& mock) {
    return xer::text_stream(
        reinterpret_cast<xer::text_stream_handle_t>(&mock),
        text_close_noop,
        text_read_unused,
        text_write_unused,
        text_getpos_stub,
        text_setpos_stub,
        text_seek_end_stub);
}

void test_binary_fseek_seek_set_success() {
    binary_stream_mock mock{};
    auto stream = make_binary_stream(mock);

    const auto result = xer::fseek(stream, 12, xer::seek_set);

    xer_assert(result.has_value());
    xer_assert_eq(mock.last_seek_offset, 12);
    xer_assert_eq(mock.last_seek_origin, static_cast<int>(xer::seek_set));
    xer_assert_eq(mock.position, 12);
}

void test_binary_fseek_seek_cur_success() {
    binary_stream_mock mock{};
    mock.position = 10;
    mock.tell_return = 10;

    auto stream = make_binary_stream(mock);

    const auto result = xer::fseek(stream, 7, xer::seek_cur);

    xer_assert(result.has_value());
    xer_assert_eq(mock.last_seek_offset, 7);
    xer_assert_eq(mock.last_seek_origin, static_cast<int>(xer::seek_cur));
    xer_assert_eq(mock.position, 17);
}

void test_binary_fseek_invalid_origin() {
    binary_stream_mock mock{};
    auto stream = make_binary_stream(mock);

    const auto result = xer::fseek(
        stream,
        0,
        static_cast<xer::seek_origin_t>(12345));

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_binary_fseek_failure() {
    binary_stream_mock mock{};
    mock.seek_return = -1;

    auto stream = make_binary_stream(mock);

    const auto result = xer::fseek(stream, 3, xer::seek_set);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::io_error);
}

void test_binary_ftell_success() {
    binary_stream_mock mock{};
    mock.tell_return = 42;

    auto stream = make_binary_stream(mock);

    const auto result = xer::ftell(stream);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::uint64_t>(42));
}

void test_binary_ftell_failure() {
    binary_stream_mock mock{};
    mock.tell_return = -1;

    auto stream = make_binary_stream(mock);

    const auto result = xer::ftell(stream);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::io_error);
}

void test_binary_fgetpos_success() {
    binary_stream_mock mock{};
    mock.tell_return = 88;

    auto stream = make_binary_stream(mock);

    const auto result = xer::fgetpos(stream);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<xer::fpos_t>(88));
}

void test_binary_fsetpos_success() {
    binary_stream_mock mock{};
    auto stream = make_binary_stream(mock);

    const auto result = xer::fsetpos(stream, static_cast<xer::fpos_t>(55));

    xer_assert(result.has_value());
    xer_assert_eq(mock.last_seek_offset, 55);
    xer_assert_eq(mock.last_seek_origin, static_cast<int>(xer::seek_set));
    xer_assert_eq(mock.position, 55);
}

void test_binary_fsetpos_out_of_range() {
    binary_stream_mock mock{};
    auto stream = make_binary_stream(mock);

    const xer::fpos_t too_large =
        static_cast<xer::fpos_t>(std::numeric_limits<std::int64_t>::max()) + 1u;

    const auto result = xer::fsetpos(stream, too_large);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::out_of_range);
}

void test_text_fseek_seek_end_zero_success() {
    text_stream_mock mock{};
    auto stream = make_text_stream(mock);

    const auto result = xer::fseek(stream, 0, xer::seek_end);

    xer_assert(result.has_value());
    xer_assert_eq(mock.seek_end_count, 1);
    xer_assert_eq(mock.position, static_cast<xer::text_stream_pos_t>(999));
}

void test_text_fseek_nonzero_offset_rejected() {
    text_stream_mock mock{};
    auto stream = make_text_stream(mock);

    const auto result = xer::fseek(stream, 1, xer::seek_end);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
    xer_assert_eq(mock.seek_end_count, 0);
}

void test_text_fseek_non_end_rejected() {
    text_stream_mock mock{};
    auto stream = make_text_stream(mock);

    const auto result = xer::fseek(stream, 0, xer::seek_set);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
    xer_assert_eq(mock.seek_end_count, 0);
}

void test_text_fseek_invalid_origin() {
    text_stream_mock mock{};
    auto stream = make_text_stream(mock);

    const auto result = xer::fseek(
        stream,
        0,
        static_cast<xer::seek_origin_t>(-99));

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_text_fseek_failure() {
    text_stream_mock mock{};
    mock.seek_end_return = -1;

    auto stream = make_text_stream(mock);

    const auto result = xer::fseek(stream, 0, xer::seek_end);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::io_error);
    xer_assert_eq(mock.seek_end_count, 1);
}

void test_text_ftell_success() {
    text_stream_mock mock{};
    mock.getpos_return = 123;

    auto stream = make_text_stream(mock);

    const auto result = xer::ftell(stream);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::uint64_t>(123));
}

void test_text_ftell_failure() {
    text_stream_mock mock{};
    mock.getpos_return = -1;

    auto stream = make_text_stream(mock);

    const auto result = xer::ftell(stream);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::io_error);
}

void test_text_fgetpos_success() {
    text_stream_mock mock{};
    mock.getpos_return = 321;

    auto stream = make_text_stream(mock);

    const auto result = xer::fgetpos(stream);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<xer::fpos_t>(321));
}

void test_text_fgetpos_failure() {
    text_stream_mock mock{};
    mock.getpos_return = -1;

    auto stream = make_text_stream(mock);

    const auto result = xer::fgetpos(stream);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::io_error);
}

void test_text_fsetpos_success() {
    text_stream_mock mock{};
    auto stream = make_text_stream(mock);

    const auto result = xer::fsetpos(stream, static_cast<xer::fpos_t>(456));

    xer_assert(result.has_value());
    xer_assert_eq(mock.last_setpos, static_cast<xer::text_stream_pos_t>(456));
    xer_assert_eq(mock.position, static_cast<xer::text_stream_pos_t>(456));
}

void test_text_fsetpos_failure() {
    text_stream_mock mock{};
    mock.setpos_return = -1;

    auto stream = make_text_stream(mock);

    const auto result = xer::fsetpos(stream, static_cast<xer::fpos_t>(222));

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::io_error);
    xer_assert_eq(mock.last_setpos, static_cast<xer::text_stream_pos_t>(222));
}

void test_text_fsetpos_out_of_range() {
    text_stream_mock mock{};
    auto stream = make_text_stream(mock);

    const xer::fpos_t too_large =
        static_cast<xer::fpos_t>(std::numeric_limits<xer::text_stream_pos_t>::max()) + 1u;

    const auto result = xer::fsetpos(stream, too_large);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::out_of_range);
}

} // namespace

int main() {
    test_binary_fseek_seek_set_success();
    test_binary_fseek_seek_cur_success();
    test_binary_fseek_invalid_origin();
    test_binary_fseek_failure();
    test_binary_ftell_success();
    test_binary_ftell_failure();
    test_binary_fgetpos_success();
    test_binary_fsetpos_success();
    test_binary_fsetpos_out_of_range();

    test_text_fseek_seek_end_zero_success();
    test_text_fseek_nonzero_offset_rejected();
    test_text_fseek_non_end_rejected();
    test_text_fseek_invalid_origin();
    test_text_fseek_failure();
    test_text_ftell_success();
    test_text_ftell_failure();
    test_text_fgetpos_success();
    test_text_fgetpos_failure();
    test_text_fsetpos_success();
    test_text_fsetpos_failure();
    test_text_fsetpos_out_of_range();

    return 0;
}
