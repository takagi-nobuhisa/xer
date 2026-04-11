/**
 * @file tests/test_tmpfile.cpp
 * @brief Tests for xer/bits/tmpfile.h.
 */

#include <array>
#include <cstddef>
#include <string>

#include <xer/assert.h>
#include <xer/bits/binary_stream_io.h>
#include <xer/bits/fclose.h>
#include <xer/bits/text_stream_io.h>
#include <xer/bits/tmpfile.h>

namespace {

void test_tmpfile_binary_round_trip() {
    auto stream_result = xer::tmpfile();
    xer_assert(stream_result.has_value());

    xer::binary_stream stream = std::move(stream_result.value());
    xer_assert(stream.has_value());

    constexpr std::array<std::byte, 4> expected = {
        std::byte{0x10},
        std::byte{0x20},
        std::byte{0x30},
        std::byte{0x40},
    };

    const auto written = xer::fwrite(std::span<const std::byte>(expected), stream);
    xer_assert(written.has_value());
    xer_assert_eq(*written, expected.size());

    xer_assert_eq(stream.tell_fn()(stream.handle()), static_cast<std::int64_t>(expected.size()));
    xer_assert_eq(stream.seek_fn()(stream.handle(), 0, SEEK_SET), 0);
    xer_assert_eq(stream.tell_fn()(stream.handle()), 0);

    std::array<std::byte, 4> actual = {};
    const auto read = xer::fread(std::span<std::byte>(actual), stream);
    xer_assert(read.has_value());
    xer_assert_eq(*read, actual.size());

    xer_assert_eq(actual[0], expected[0]);
    xer_assert_eq(actual[1], expected[1]);
    xer_assert_eq(actual[2], expected[2]);
    xer_assert_eq(actual[3], expected[3]);

    const auto close_result = xer::fclose(stream);
    xer_assert(close_result.has_value());
    xer_assert(!stream.has_value());
}

void test_tmpfile_text_utf8_position_ops() {
    auto stream_result = xer::tmpfile(xer::encoding_t::utf8);
    xer_assert(stream_result.has_value());

    xer::text_stream stream = std::move(stream_result.value());
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

    const auto end_pos = stream.getpos_fn()(stream.handle());
    xer_assert(end_pos >= 0);

    xer_assert_eq(stream.setpos_fn()(stream.handle(), 0), 0);

    const auto ch1 = xer::fgetc(stream);
    xer_assert(ch1.has_value());
    xer_assert_eq(*ch1, U'A');

    const auto ch2 = xer::fgetc(stream);
    xer_assert(ch2.has_value());
    xer_assert_eq(*ch2, U'あ');

    const auto pos_after_two = stream.getpos_fn()(stream.handle());
    xer_assert(pos_after_two >= 0);

    xer_assert_eq(stream.setpos_fn()(stream.handle(), end_pos), 0);
    xer_assert_eq(stream.seek_end_fn()(stream.handle()), 0);
    xer_assert_eq(stream.getpos_fn()(stream.handle()), end_pos);

    const auto close_result = xer::fclose(stream);
    xer_assert(close_result.has_value());
}

void test_tmpfile_text_cp932_round_trip() {
    auto stream_result = xer::tmpfile(xer::encoding_t::cp932);
    xer_assert(stream_result.has_value());

    xer::text_stream stream = std::move(stream_result.value());
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
        const auto result = xer::fputc(U'漢', stream);
        xer_assert(result.has_value());
    }

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
        xer_assert_eq(*ch, U'漢');
    }

    {
        const auto ch = xer::fgetc(stream);
        xer_assert(!ch.has_value());
        xer_assert_eq(ch.error().code, xer::error_t::not_found);
    }

    const auto close_result = xer::fclose(stream);
    xer_assert(close_result.has_value());
}

void test_tmpfile_text_auto_detect_is_invalid() {
    const auto result = xer::tmpfile(xer::encoding_t::auto_detect);
    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

} // namespace

int main() {
    test_tmpfile_binary_round_trip();
    test_tmpfile_text_utf8_position_ops();
    test_tmpfile_text_cp932_round_trip();
    test_tmpfile_text_auto_detect_is_invalid();
    return 0;
}
