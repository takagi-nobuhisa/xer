/**
 * @file tests/test_text_stream_io.cpp
 * @brief Tests for xer/bits/text_stream_io.h.
 */

#include <cstdint>
#include <expected>
#include <string>
#include <string_view>
#include <utility>

#include <xer/assert.h>
#include <xer/bits/stream_position_io.h>
#include <xer/bits/text_stream.h>
#include <xer/bits/text_stream_io.h>
#include <xer/error.h>

namespace {

struct mock_text_stream_state {
    std::u32string storage;
    std::size_t pos = 0;
};

[[nodiscard]] mock_text_stream_state* to_state(xer::text_stream_handle_t handle) noexcept {
    return reinterpret_cast<mock_text_stream_state*>(handle);
}

int mock_close(xer::text_stream_handle_t handle) noexcept {
    delete to_state(handle);
    return 0;
}

int mock_read(xer::text_stream_handle_t handle, char32_t* s, int n) noexcept {
    if (s == nullptr || n < 0) {
        return -1;
    }

    mock_text_stream_state* const state = to_state(handle);
    if (state == nullptr) {
        return -1;
    }

    int count = 0;
    while (count < n && state->pos < state->storage.size()) {
        s[count] = state->storage[state->pos];
        ++count;
        ++state->pos;
    }

    return count;
}

int mock_write(xer::text_stream_handle_t handle, const char32_t* s, int n) noexcept {
    if (s == nullptr || n < 0) {
        return -1;
    }

    mock_text_stream_state* const state = to_state(handle);
    if (state == nullptr) {
        return -1;
    }

    for (int i = 0; i < n; ++i) {
        if (state->pos < state->storage.size()) {
            state->storage[state->pos] = s[i];
        } else {
            state->storage.push_back(s[i]);
        }

        ++state->pos;
    }

    return n;
}

xer::text_stream_pos_t mock_getpos(xer::text_stream_handle_t handle) noexcept {
    mock_text_stream_state* const state = to_state(handle);
    if (state == nullptr) {
        return -1;
    }

    return static_cast<xer::text_stream_pos_t>(state->pos);
}

int mock_setpos(xer::text_stream_handle_t handle, xer::text_stream_pos_t pos) noexcept {
    mock_text_stream_state* const state = to_state(handle);
    if (state == nullptr || pos < 0) {
        return -1;
    }

    if (static_cast<std::size_t>(pos) > state->storage.size()) {
        return -1;
    }

    state->pos = static_cast<std::size_t>(pos);
    return 0;
}

int mock_seek_end(xer::text_stream_handle_t handle) noexcept {
    mock_text_stream_state* const state = to_state(handle);
    if (state == nullptr) {
        return -1;
    }

    state->pos = state->storage.size();
    return 0;
}

[[nodiscard]] xer::text_stream make_mock_text_stream(std::u32string initial = U"") {
    auto* const state = new mock_text_stream_state{
        .storage = std::move(initial),
        .pos = 0,
    };

    return xer::text_stream(
        reinterpret_cast<xer::text_stream_handle_t>(state),
        mock_close,
        mock_read,
        mock_write,
        mock_getpos,
        mock_setpos,
        mock_seek_end);
}

[[nodiscard]] std::u32string& storage_of(xer::text_stream& stream) {
    return to_state(stream.handle())->storage;
}

void test_fgetc_reads_ascii_and_unicode() {
    xer::text_stream stream = make_mock_text_stream(U"Aあ🙂");

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

    {
        const auto ch = xer::fgetc(stream);
        xer_assert(!ch.has_value());
        xer_assert_eq(ch.error().code, xer::error_t::not_found);
    }
}

void test_fgetc_empty_stream_is_invalid_argument() {
    xer::text_stream stream;
    const auto result = xer::fgetc(stream);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_fgets_keep_newline_true() {
    xer::text_stream stream = make_mock_text_stream(U"abc\nあいう\nlast");

    {
        const auto line = xer::fgets(stream, true);
        xer_assert(line.has_value());
        xer_assert_eq(*line, std::u8string(u8"abc\n"));
    }

    {
        const auto line = xer::fgets(stream, true);
        xer_assert(line.has_value());
        xer_assert_eq(*line, std::u8string(u8"あいう\n"));
    }

    {
        const auto line = xer::fgets(stream, true);
        xer_assert(line.has_value());
        xer_assert_eq(*line, std::u8string(u8"last"));
    }

    {
        const auto line = xer::fgets(stream, true);
        xer_assert(!line.has_value());
        xer_assert_eq(line.error().code, xer::error_t::not_found);
    }
}

void test_fgets_keep_newline_false() {
    xer::text_stream stream = make_mock_text_stream(U"abc\nあいう\n");

    {
        const auto line = xer::fgets(stream, false);
        xer_assert(line.has_value());
        xer_assert_eq(*line, std::u8string(u8"abc"));
    }

    {
        const auto line = xer::fgets(stream, false);
        xer_assert(line.has_value());
        xer_assert_eq(*line, std::u8string(u8"あいう"));
    }

    {
        const auto line = xer::fgets(stream, false);
        xer_assert(!line.has_value());
        xer_assert_eq(line.error().code, xer::error_t::not_found);
    }
}

void test_fputc_writes_ascii_and_unicode() {
    xer::text_stream stream = make_mock_text_stream();

    {
        const auto result = xer::fputc(U'A', stream);
        xer_assert(result.has_value());
        xer_assert_eq(*result, U'A');
    }

    {
        const auto result = xer::fputc(U'あ', stream);
        xer_assert(result.has_value());
        xer_assert_eq(*result, U'あ');
    }

    {
        const auto result = xer::fputc(U'🙂', stream);
        xer_assert(result.has_value());
        xer_assert_eq(*result, U'🙂');
    }

    xer_assert_eq(storage_of(stream), std::u32string(U"Aあ🙂"));
}

void test_fputc_empty_stream_is_invalid_argument() {
    xer::text_stream stream;
    const auto result = xer::fputc(U'X', stream);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_fputs_without_newline() {
    xer::text_stream stream = make_mock_text_stream();

    const auto written = xer::fputs(u8"abcあ🙂", stream, false);
    xer_assert(written.has_value());
    xer_assert_eq(*written, std::u8string_view(u8"abcあ🙂").size());

    xer_assert_eq(storage_of(stream), std::u32string(U"abcあ🙂"));
}

void test_fputs_with_newline() {
    xer::text_stream stream = make_mock_text_stream();

    const auto written = xer::fputs(u8"line", stream, true);
    xer_assert(written.has_value());
    xer_assert_eq(*written, std::u8string_view(u8"line").size() + 1u);

    xer_assert_eq(storage_of(stream), std::u32string(U"line\n"));
}

void test_fgets_after_fputc_round_trip() {
    xer::text_stream stream = make_mock_text_stream();

    xer_assert(xer::fputc(U'x', stream).has_value());
    xer_assert(xer::fputc(U'y', stream).has_value());
    xer_assert(xer::fputc(U'\n', stream).has_value());
    xer_assert(xer::fputc(U'z', stream).has_value());

    xer_assert_eq(mock_setpos(stream.handle(), 0), 0);

    const auto first = xer::fgets(stream, true);
    xer_assert(first.has_value());
    xer_assert_eq(*first, std::u8string(u8"xy\n"));

    const auto second = xer::fgets(stream, true);
    xer_assert(second.has_value());
    xer_assert_eq(*second, std::u8string(u8"z"));
}

void test_ungetc_returns_pushed_back_character() {
    xer::text_stream stream = make_mock_text_stream(U"ab");

    const auto first = xer::fgetc(stream);
    xer_assert(first.has_value());
    xer_assert_eq(*first, U'a');

    const auto pushed = xer::ungetc(U'X', stream);
    xer_assert(pushed.has_value());
    xer_assert_eq(*pushed, U'X');

    const auto next = xer::fgetc(stream);
    xer_assert(next.has_value());
    xer_assert_eq(*next, U'X');

    const auto after = xer::fgetc(stream);
    xer_assert(after.has_value());
    xer_assert_eq(*after, U'b');
}

void test_ungetc_only_one_character_is_allowed() {
    xer::text_stream stream = make_mock_text_stream(U"abc");

    const auto first = xer::ungetc(U'X', stream);
    xer_assert(first.has_value());
    xer_assert_eq(*first, U'X');

    const auto second = xer::ungetc(U'Y', stream);
    xer_assert(!second.has_value());
    xer_assert_eq(second.error().code, xer::error_t::runtime_error);
}

void test_ungetc_clears_eof_state() {
    xer::text_stream stream = make_mock_text_stream(U"a");

    {
        const auto first = xer::fgetc(stream);
        xer_assert(first.has_value());
        xer_assert_eq(*first, U'a');
    }

    {
        const auto eof_result = xer::fgetc(stream);
        xer_assert(!eof_result.has_value());
        xer_assert_eq(eof_result.error().code, xer::error_t::not_found);
        xer_assert(stream.eof());
    }

    {
        const auto pushed = xer::ungetc(U'Z', stream);
        xer_assert(pushed.has_value());
        xer_assert_eq(*pushed, U'Z');
        xer_assert_not(stream.eof());
    }

    {
        const auto again = xer::fgetc(stream);
        xer_assert(again.has_value());
        xer_assert_eq(*again, U'Z');
    }

    {
        const auto eof_result = xer::fgetc(stream);
        xer_assert(!eof_result.has_value());
        xer_assert_eq(eof_result.error().code, xer::error_t::not_found);
    }
}

void test_ungetc_invalid_argument_for_empty_stream() {
    xer::text_stream stream;
    const auto result = xer::ungetc(U'X', stream);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_fsetpos_clears_unget_character() {
    xer::text_stream stream = make_mock_text_stream(U"abc");

    const auto pushed = xer::ungetc(U'X', stream);
    xer_assert(pushed.has_value());

    const auto setpos_result = xer::fsetpos(stream, static_cast<xer::fpos_t>(1));
    xer_assert(setpos_result.has_value());

    const auto ch = xer::fgetc(stream);
    xer_assert(ch.has_value());
    xer_assert_eq(*ch, U'b');
}

} // namespace

int main() {
    test_fgetc_reads_ascii_and_unicode();
    test_fgetc_empty_stream_is_invalid_argument();
    test_fgets_keep_newline_true();
    test_fgets_keep_newline_false();
    test_fputc_writes_ascii_and_unicode();
    test_fputc_empty_stream_is_invalid_argument();
    test_fputs_without_newline();
    test_fputs_with_newline();
    test_fgets_after_fputc_round_trip();
    test_ungetc_returns_pushed_back_character();
    test_ungetc_only_one_character_is_allowed();
    test_ungetc_clears_eof_state();
    test_ungetc_invalid_argument_for_empty_stream();
    test_fsetpos_clears_unget_character();
    return 0;
}
