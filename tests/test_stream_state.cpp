/**
 * @file tests/test_stream_state.cpp
 * @brief Tests for feof, ferror, and clearerr.
 */

#include <cstdint>

#include <xer/assert.h>
#include <xer/bits/binary_stream.h>
#include <xer/bits/clearerr.h>
#include <xer/bits/feof.h>
#include <xer/bits/ferror.h>
#include <xer/bits/text_stream.h>

namespace {

int dummy_binary_close(xer::binary_stream_handle_t) noexcept {
    return 0;
}

int dummy_binary_read(xer::binary_stream_handle_t, void*, int) noexcept {
    return 0;
}

int dummy_binary_write(xer::binary_stream_handle_t, const void*, int) noexcept {
    return 0;
}

int dummy_binary_seek(xer::binary_stream_handle_t, std::int64_t, int) noexcept {
    return 0;
}

std::int64_t dummy_binary_tell(xer::binary_stream_handle_t) noexcept {
    return 0;
}

int dummy_text_close(xer::text_stream_handle_t) noexcept {
    return 0;
}

int dummy_text_read(xer::text_stream_handle_t, char32_t*, int) noexcept {
    return 0;
}

int dummy_text_write(xer::text_stream_handle_t, const char32_t*, int) noexcept {
    return 0;
}

xer::text_stream_pos_t dummy_text_getpos(xer::text_stream_handle_t) noexcept {
    return 0;
}

int dummy_text_setpos(xer::text_stream_handle_t, xer::text_stream_pos_t) noexcept {
    return 0;
}

int dummy_text_seek_end(xer::text_stream_handle_t) noexcept {
    return 0;
}

[[nodiscard]] xer::binary_stream make_binary_stream() {
    return xer::binary_stream(
        static_cast<xer::binary_stream_handle_t>(1),
        dummy_binary_close,
        dummy_binary_read,
        dummy_binary_write,
        dummy_binary_seek,
        dummy_binary_tell);
}

[[nodiscard]] xer::text_stream make_text_stream() {
    return xer::text_stream(
        static_cast<xer::text_stream_handle_t>(1),
        dummy_text_close,
        dummy_text_read,
        dummy_text_write,
        dummy_text_getpos,
        dummy_text_setpos,
        dummy_text_seek_end);
}

void test_binary_stream_initial_state() {
    const xer::binary_stream stream = make_binary_stream();

    xer_assert(stream.has_value());
    xer_assert(!xer::feof(stream));
    xer_assert(!xer::ferror(stream));
}

void test_text_stream_initial_state() {
    const xer::text_stream stream = make_text_stream();

    xer_assert(stream.has_value());
    xer_assert(!xer::feof(stream));
    xer_assert(!xer::ferror(stream));
}

void test_binary_stream_feof_reflects_indicator() {
    xer::binary_stream stream = make_binary_stream();

    stream.set_eof(true);
    xer_assert(xer::feof(stream));

    stream.set_eof(false);
    xer_assert(!xer::feof(stream));
}

void test_text_stream_feof_reflects_indicator() {
    xer::text_stream stream = make_text_stream();

    stream.set_eof(true);
    xer_assert(xer::feof(stream));

    stream.set_eof(false);
    xer_assert(!xer::feof(stream));
}

void test_binary_stream_ferror_reflects_indicator() {
    xer::binary_stream stream = make_binary_stream();

    stream.set_error(true);
    xer_assert(xer::ferror(stream));

    stream.set_error(false);
    xer_assert(!xer::ferror(stream));
}

void test_text_stream_ferror_reflects_indicator() {
    xer::text_stream stream = make_text_stream();

    stream.set_error(true);
    xer_assert(xer::ferror(stream));

    stream.set_error(false);
    xer_assert(!xer::ferror(stream));
}

void test_binary_stream_clearerr_clears_both_indicators() {
    xer::binary_stream stream = make_binary_stream();

    stream.set_eof(true);
    stream.set_error(true);

    xer_assert(xer::feof(stream));
    xer_assert(xer::ferror(stream));

    xer::clearerr(stream);

    xer_assert(!xer::feof(stream));
    xer_assert(!xer::ferror(stream));
}

void test_text_stream_clearerr_clears_both_indicators() {
    xer::text_stream stream = make_text_stream();

    stream.set_eof(true);
    stream.set_error(true);

    xer_assert(xer::feof(stream));
    xer_assert(xer::ferror(stream));

    xer::clearerr(stream);

    xer_assert(!xer::feof(stream));
    xer_assert(!xer::ferror(stream));
}

void test_binary_stream_clearerr_on_empty_stream_is_noop() {
    xer::binary_stream stream;

    xer_assert(!stream.has_value());
    xer_assert(!xer::feof(stream));
    xer_assert(!xer::ferror(stream));

    xer::clearerr(stream);

    xer_assert(!xer::feof(stream));
    xer_assert(!xer::ferror(stream));
}

void test_text_stream_clearerr_on_empty_stream_is_noop() {
    xer::text_stream stream;

    xer_assert(!stream.has_value());
    xer_assert(!xer::feof(stream));
    xer_assert(!xer::ferror(stream));

    xer::clearerr(stream);

    xer_assert(!xer::feof(stream));
    xer_assert(!xer::ferror(stream));
}

void test_binary_stream_move_preserves_indicators() {
    xer::binary_stream source = make_binary_stream();
    source.set_eof(true);
    source.set_error(true);

    xer::binary_stream moved(std::move(source));

    xer_assert(!source.has_value());
    xer_assert(moved.has_value());
    xer_assert(xer::feof(moved));
    xer_assert(xer::ferror(moved));
}

void test_text_stream_move_preserves_indicators() {
    xer::text_stream source = make_text_stream();
    source.set_eof(true);
    source.set_error(true);

    xer::text_stream moved(std::move(source));

    xer_assert(!source.has_value());
    xer_assert(moved.has_value());
    xer_assert(xer::feof(moved));
    xer_assert(xer::ferror(moved));
}

void test_binary_stream_close_resets_indicators() {
    xer::binary_stream stream = make_binary_stream();
    stream.set_eof(true);
    stream.set_error(true);

    const int result = stream.close();

    xer_assert_eq(result, 0);
    xer_assert(!stream.has_value());
    xer_assert(!xer::feof(stream));
    xer_assert(!xer::ferror(stream));
}

void test_text_stream_close_resets_indicators() {
    xer::text_stream stream = make_text_stream();
    stream.set_eof(true);
    stream.set_error(true);

    const int result = stream.close();

    xer_assert_eq(result, 0);
    xer_assert(!stream.has_value());
    xer_assert(!xer::feof(stream));
    xer_assert(!xer::ferror(stream));
}

} // namespace

int main() {
    test_binary_stream_initial_state();
    test_text_stream_initial_state();
    test_binary_stream_feof_reflects_indicator();
    test_text_stream_feof_reflects_indicator();
    test_binary_stream_ferror_reflects_indicator();
    test_text_stream_ferror_reflects_indicator();
    test_binary_stream_clearerr_clears_both_indicators();
    test_text_stream_clearerr_clears_both_indicators();
    test_binary_stream_clearerr_on_empty_stream_is_noop();
    test_text_stream_clearerr_on_empty_stream_is_noop();
    test_binary_stream_move_preserves_indicators();
    test_text_stream_move_preserves_indicators();
    test_binary_stream_close_resets_indicators();
    test_text_stream_close_resets_indicators();
    return 0;
}
