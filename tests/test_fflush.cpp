/**
 * @file tests/test_fflush.cpp
 * @brief Tests for xer/bits/fflush.h.
 */

#include <array>
#include <cstddef>
#include <cstdio>
#include <span>
#include <string>
#include <string_view>

#include <xer/assert.h>
#include <xer/bits/binary_stream_io.h>
#include <xer/bits/fclose.h>
#include <xer/bits/fflush.h>
#include <xer/bits/fopen.h>
#include <xer/path.h>
#include <xer/bits/text_stream_io.h>
#include <xer/bits/tmpfile.h>

namespace {

void test_fflush_binary_tmpfile_success() {
    auto stream_result = xer::tmpfile();
    xer_assert(stream_result.has_value());

    xer::binary_stream stream = std::move(*stream_result);
    xer_assert(stream.has_value());

    constexpr std::array<std::byte, 4> expected = {
        std::byte{0x11},
        std::byte{0x22},
        std::byte{0x33},
        std::byte{0x44},
    };

    const auto write_result = xer::fwrite(std::span<const std::byte>(expected), stream);
    xer_assert(write_result.has_value());
    xer_assert_eq(*write_result, expected.size());

    const auto flush_result = xer::fflush(stream);
    xer_assert(flush_result.has_value());
    xer_assert_not(stream.error());

    xer_assert_eq(stream.seek_fn()(stream.handle(), 0, SEEK_SET), 0);

    std::array<std::byte, 4> actual = {};
    const auto read_result = xer::fread(std::span<std::byte>(actual), stream);
    xer_assert(read_result.has_value());
    xer_assert_eq(*read_result, actual.size());

    xer_assert_eq(actual[0], expected[0]);
    xer_assert_eq(actual[1], expected[1]);
    xer_assert_eq(actual[2], expected[2]);
    xer_assert_eq(actual[3], expected[3]);

    const auto close_result = xer::fclose(stream);
    xer_assert(close_result.has_value());
}

void test_fflush_text_tmpfile_success() {
    auto stream_result = xer::tmpfile(xer::encoding_t::utf8);
    xer_assert(stream_result.has_value());

    xer::text_stream stream = std::move(*stream_result);
    xer_assert(stream.has_value());

    {
        const auto result = xer::fputc(U'A', stream);
        xer_assert(result.has_value());
    }

    {
        const auto result = xer::fputc(U'あ', stream);
        xer_assert(result.has_value());
    }

    {
        const auto result = xer::fputc(U'🙂', stream);
        xer_assert(result.has_value());
    }

    const auto flush_result = xer::fflush(stream);
    xer_assert(flush_result.has_value());
    xer_assert_not(stream.error());

    xer_assert_eq(stream.setpos_fn()(stream.handle(), 0), 0);

    {
        const auto ch = xer::fgetc(stream);
        xer_assert(ch.has_value());
        xer_assert_eq(*ch, U'A');
    }

    {
        const auto ch = xer::fgetc(stream);
        xer_assert(ch.has_value());
        xer_assert_eq(*ch, U'あ');
    }

    {
        const auto ch = xer::fgetc(stream);
        xer_assert(ch.has_value());
        xer_assert_eq(*ch, U'🙂');
    }

    const auto close_result = xer::fclose(stream);
    xer_assert(close_result.has_value());
}

void test_fflush_binary_closed_stream_fails() {
    auto stream_result = xer::tmpfile();
    xer_assert(stream_result.has_value());

    xer::binary_stream stream = std::move(*stream_result);
    xer_assert(stream.has_value());

    {
        const auto close_result = xer::fclose(stream);
        xer_assert(close_result.has_value());
    }

    const auto flush_result = xer::fflush(stream);
    xer_assert(!flush_result.has_value());
    xer_assert_eq(flush_result.error().code, xer::error_t::runtime_error);
}

void test_fflush_text_closed_stream_fails() {
    auto stream_result = xer::tmpfile(xer::encoding_t::utf8);
    xer_assert(stream_result.has_value());

    xer::text_stream stream = std::move(*stream_result);
    xer_assert(stream.has_value());

    {
        const auto close_result = xer::fclose(stream);
        xer_assert(close_result.has_value());
    }

    const auto flush_result = xer::fflush(stream);
    xer_assert(!flush_result.has_value());
    xer_assert_eq(flush_result.error().code, xer::error_t::runtime_error);
}

} // namespace

int main() {
    test_fflush_binary_tmpfile_success();
    test_fflush_text_tmpfile_success();
    test_fflush_binary_closed_stream_fails();
    test_fflush_text_closed_stream_fails();
    return 0;
}
