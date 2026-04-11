/**
 * @file tests/test_fclose.cpp
 * @brief Tests for xer/bits/fclose.h.
 */

#include <expected>
#include <string_view>

#include <xer/assert.h>
#include <xer/bits/fclose.h>
#include <xer/bits/fopen.h>

namespace {

int failing_binary_close(xer::binary_stream_handle_t) noexcept {
    return -1;
}

int failing_text_close(xer::text_stream_handle_t) noexcept {
    return -1;
}

void test_fclose_empty_binary_stream() {
    xer::binary_stream stream;
    const auto result = xer::fclose(stream);
    xer_assert(result.has_value());
    xer_assert(!stream.has_value());
}

void test_fclose_empty_text_stream() {
    xer::text_stream stream;
    const auto result = xer::fclose(stream);
    xer_assert(result.has_value());
    xer_assert(!stream.has_value());
}

void test_fclose_binary_stream_success() {
    std::byte buffer[4] = {};

    auto stream_result = xer::memopen(std::span<std::byte>(buffer), "r+");
    xer_assert(stream_result.has_value());

    xer::binary_stream stream = std::move(stream_result.value());
    xer_assert(stream.has_value());

    const auto result = xer::fclose(stream);
    xer_assert(result.has_value());
    xer_assert(!stream.has_value());
}

void test_fclose_text_stream_success() {
    std::u8string text = u8"abc";

    auto stream_result = xer::stropen(text, "r");
    xer_assert(stream_result.has_value());

    xer::text_stream stream = std::move(stream_result.value());
    xer_assert(stream.has_value());

    const auto result = xer::fclose(stream);
    xer_assert(result.has_value());
    xer_assert(!stream.has_value());
}

void test_fclose_binary_stream_failure_resets_stream() {
    xer::binary_stream stream(
        static_cast<xer::binary_stream_handle_t>(1),
        failing_binary_close,
        xer::detail::binary_stream_read_error,
        xer::detail::binary_stream_write_error,
        xer::detail::binary_stream_seek_error,
        xer::detail::binary_stream_tell_error);

    xer_assert(stream.has_value());

    const auto result = xer::fclose(stream);
    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::runtime_error);
    xer_assert(!stream.has_value());
}

void test_fclose_text_stream_failure_resets_stream() {
    xer::text_stream stream(
        static_cast<xer::text_stream_handle_t>(1),
        failing_text_close,
        xer::detail::text_stream_read_error,
        xer::detail::text_stream_write_error,
        xer::detail::text_stream_getpos_error,
        xer::detail::text_stream_setpos_error,
        xer::detail::text_stream_seek_end_error);

    xer_assert(stream.has_value());

    const auto result = xer::fclose(stream);
    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::runtime_error);
    xer_assert(!stream.has_value());
}

} // namespace

int main() {
    test_fclose_empty_binary_stream();
    test_fclose_empty_text_stream();
    test_fclose_binary_stream_success();
    test_fclose_text_stream_success();
    test_fclose_binary_stream_failure_resets_stream();
    test_fclose_text_stream_failure_resets_stream();
    return 0;
}
