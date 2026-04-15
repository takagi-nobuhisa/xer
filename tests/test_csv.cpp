/**
 * @file tests/test_csv.cpp
 * @brief Tests for xer/bits/csv.h.
 */

#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <xer/assert.h>
#include <xer/stdio.h>

namespace {

struct mock_text_stream_state {
    std::u32string storage;
    std::size_t pos = 0;
};

[[nodiscard]] auto to_state(xer::text_stream_handle_t handle) noexcept -> mock_text_stream_state* {
    return reinterpret_cast<mock_text_stream_state*>(handle);
}

auto mock_close(xer::text_stream_handle_t handle) noexcept -> int {
    delete to_state(handle);
    return 0;
}

auto mock_read(xer::text_stream_handle_t handle, char32_t* s, int n) noexcept -> int {
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

auto mock_write(xer::text_stream_handle_t handle, const char32_t* s, int n) noexcept -> int {
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

auto mock_getpos(xer::text_stream_handle_t handle) noexcept -> xer::text_stream_pos_t {
    mock_text_stream_state* const state = to_state(handle);
    if (state == nullptr) {
        return -1;
    }

    return static_cast<xer::text_stream_pos_t>(state->pos);
}

auto mock_setpos(xer::text_stream_handle_t handle, xer::text_stream_pos_t pos) noexcept -> int {
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

auto mock_seek_end(xer::text_stream_handle_t handle) noexcept -> int {
    mock_text_stream_state* const state = to_state(handle);
    if (state == nullptr) {
        return -1;
    }

    state->pos = state->storage.size();
    return 0;
}

[[nodiscard]] auto make_mock_text_stream(std::u32string initial = U"") -> xer::text_stream {
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

[[nodiscard]] auto storage_of(xer::text_stream& stream) -> std::u32string& {
    return to_state(stream.handle())->storage;
}

auto test_fgetcsv_simple_lf() -> void {
    xer::text_stream stream = make_mock_text_stream(U"a,b,c\n");

    const auto result = xer::fgetcsv(stream);
    xer_assert(result.has_value());
    xer_assert_eq(result->size(), static_cast<std::size_t>(3));
    xer_assert_eq((*result)[0], std::u8string(u8"a"));
    xer_assert_eq((*result)[1], std::u8string(u8"b"));
    xer_assert_eq((*result)[2], std::u8string(u8"c"));
}

auto test_fgetcsv_simple_crlf() -> void {
    xer::text_stream stream = make_mock_text_stream(U"a,b,c\r\n");

    const auto result = xer::fgetcsv(stream);
    xer_assert(result.has_value());
    xer_assert_eq(result->size(), static_cast<std::size_t>(3));
    xer_assert_eq((*result)[0], std::u8string(u8"a"));
    xer_assert_eq((*result)[1], std::u8string(u8"b"));
    xer_assert_eq((*result)[2], std::u8string(u8"c"));
}

auto test_fgetcsv_simple_cr() -> void {
    xer::text_stream stream = make_mock_text_stream(U"a,b,c\r");

    const auto result = xer::fgetcsv(stream);
    xer_assert(result.has_value());
    xer_assert_eq(result->size(), static_cast<std::size_t>(3));
    xer_assert_eq((*result)[0], std::u8string(u8"a"));
    xer_assert_eq((*result)[1], std::u8string(u8"b"));
    xer_assert_eq((*result)[2], std::u8string(u8"c"));
}

auto test_fgetcsv_empty_fields() -> void {
    xer::text_stream stream = make_mock_text_stream(U",,\n");

    const auto result = xer::fgetcsv(stream);
    xer_assert(result.has_value());
    xer_assert_eq(result->size(), static_cast<std::size_t>(3));
    xer_assert_eq((*result)[0], std::u8string());
    xer_assert_eq((*result)[1], std::u8string());
    xer_assert_eq((*result)[2], std::u8string());
}

auto test_fgetcsv_trailing_empty_field() -> void {
    xer::text_stream stream = make_mock_text_stream(U"a,b,\n");

    const auto result = xer::fgetcsv(stream);
    xer_assert(result.has_value());
    xer_assert_eq(result->size(), static_cast<std::size_t>(3));
    xer_assert_eq((*result)[0], std::u8string(u8"a"));
    xer_assert_eq((*result)[1], std::u8string(u8"b"));
    xer_assert_eq((*result)[2], std::u8string());
}

auto test_fgetcsv_quoted_separator() -> void {
    xer::text_stream stream = make_mock_text_stream(U"\"a,b\",c\n");

    const auto result = xer::fgetcsv(stream);
    xer_assert(result.has_value());
    xer_assert_eq(result->size(), static_cast<std::size_t>(2));
    xer_assert_eq((*result)[0], std::u8string(u8"a,b"));
    xer_assert_eq((*result)[1], std::u8string(u8"c"));
}

auto test_fgetcsv_quoted_newline() -> void {
    xer::text_stream stream = make_mock_text_stream(U"\"a\nb\",c\n");

    const auto result = xer::fgetcsv(stream);
    xer_assert(result.has_value());
    xer_assert_eq(result->size(), static_cast<std::size_t>(2));
    xer_assert_eq((*result)[0], std::u8string(u8"a\nb"));
    xer_assert_eq((*result)[1], std::u8string(u8"c"));
}

auto test_fgetcsv_doubled_enclosure() -> void {
    xer::text_stream stream = make_mock_text_stream(U"\"a\"\"b\",c\n");

    const auto result = xer::fgetcsv(stream);
    xer_assert(result.has_value());
    xer_assert_eq(result->size(), static_cast<std::size_t>(2));
    xer_assert_eq((*result)[0], std::u8string(u8"a\"b"));
    xer_assert_eq((*result)[1], std::u8string(u8"c"));
}

auto test_fgetcsv_space_after_quoted_field() -> void {
    xer::text_stream stream = make_mock_text_stream(U"\"a\" \t,bc\n");

    const auto result = xer::fgetcsv(stream);
    xer_assert(result.has_value());
    xer_assert_eq(result->size(), static_cast<std::size_t>(2));
    xer_assert_eq((*result)[0], std::u8string(u8"a"));
    xer_assert_eq((*result)[1], std::u8string(u8"bc"));
}

auto test_fgetcsv_unclosed_quote_is_error() -> void {
    xer::text_stream stream = make_mock_text_stream(U"\"abc\n");

    const auto result = xer::fgetcsv(stream);
    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

auto test_fgetcsv_garbage_after_quoted_field_is_error() -> void {
    xer::text_stream stream = make_mock_text_stream(U"\"abc\"x,def\n");

    const auto result = xer::fgetcsv(stream);
    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

auto test_fgetcsv_multiple_records() -> void {
    xer::text_stream stream = make_mock_text_stream(U"a,b\nc,d\n");

    const auto first = xer::fgetcsv(stream);
    xer_assert(first.has_value());
    xer_assert_eq(first->size(), static_cast<std::size_t>(2));
    xer_assert_eq((*first)[0], std::u8string(u8"a"));
    xer_assert_eq((*first)[1], std::u8string(u8"b"));

    const auto second = xer::fgetcsv(stream);
    xer_assert(second.has_value());
    xer_assert_eq(second->size(), static_cast<std::size_t>(2));
    xer_assert_eq((*second)[0], std::u8string(u8"c"));
    xer_assert_eq((*second)[1], std::u8string(u8"d"));
}

auto test_fputcsv_round_trip_vector() -> void {
    xer::text_stream stream = make_mock_text_stream();

    const std::vector<std::u8string> fields = {
        std::u8string(u8"a"),
        std::u8string(u8"b,c"),
        std::u8string(u8""),
        std::u8string(u8" x "),
        std::u8string(u8"q\"r"),
    };

    const auto write_result = xer::fputcsv(fields, stream);
    xer_assert(write_result.has_value());

    xer_assert_eq(mock_setpos(stream.handle(), 0), 0);

    const auto read_result = xer::fgetcsv(stream);
    xer_assert(read_result.has_value());
    xer_assert_eq(read_result->size(), fields.size());

    for (std::size_t i = 0; i < fields.size(); ++i) {
        xer_assert_eq((*read_result)[i], fields[i]);
    }
}

auto test_fputcsv_round_trip_span() -> void {
    xer::text_stream stream = make_mock_text_stream();

    const std::u8string field1 = u8"alpha";
    const std::u8string field2 = u8"beta,gamma";
    const std::u8string field3 = u8"delta\"epsilon";
    const std::u8string field4 = u8"line1\nline2";

    const std::u8string_view fields[] = {
        field1,
        field2,
        field3,
        field4,
    };

    const auto write_result = xer::fputcsv(std::span<const std::u8string_view>(fields), stream);
    xer_assert(write_result.has_value());

    xer_assert_eq(mock_setpos(stream.handle(), 0), 0);

    const auto read_result = xer::fgetcsv(stream);
    xer_assert(read_result.has_value());
    xer_assert_eq(read_result->size(), static_cast<std::size_t>(4));
    xer_assert_eq((*read_result)[0], std::u8string(field1));
    xer_assert_eq((*read_result)[1], std::u8string(field2));
    xer_assert_eq((*read_result)[2], std::u8string(field3));
    xer_assert_eq((*read_result)[3], std::u8string(field4));
}

auto test_fputcsv_writes_logical_newline() -> void {
    xer::text_stream stream = make_mock_text_stream();

    const std::vector<std::u8string> fields = {
        std::u8string(u8"a"),
        std::u8string(u8"b"),
    };

    const auto result = xer::fputcsv(fields, stream);
    xer_assert(result.has_value());

    xer_assert_eq(storage_of(stream), std::u32string(U"a,b\n"));
}

auto test_fputcsv_invalid_separator_is_error() -> void {
    xer::text_stream stream = make_mock_text_stream();

    const std::vector<std::u8string> fields = {
        std::u8string(u8"a"),
        std::u8string(u8"b"),
    };

    const auto result = xer::fputcsv(fields, stream, U'\n', U'"');
    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

auto test_fgetcsv_invalid_separator_is_error() -> void {
    xer::text_stream stream = make_mock_text_stream(U"a,b\n");

    const auto result = xer::fgetcsv(stream, U'\n', U'"');
    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

} // namespace

auto main() -> int {
    test_fgetcsv_simple_lf();
    test_fgetcsv_simple_crlf();
    test_fgetcsv_simple_cr();
    test_fgetcsv_empty_fields();
    test_fgetcsv_trailing_empty_field();
    test_fgetcsv_quoted_separator();
    test_fgetcsv_quoted_newline();
    test_fgetcsv_doubled_enclosure();
    test_fgetcsv_space_after_quoted_field();
    test_fgetcsv_unclosed_quote_is_error();
    test_fgetcsv_garbage_after_quoted_field_is_error();
    test_fgetcsv_multiple_records();
    test_fputcsv_round_trip_vector();
    test_fputcsv_round_trip_span();
    test_fputcsv_writes_logical_newline();
    test_fputcsv_invalid_separator_is_error();
    test_fgetcsv_invalid_separator_is_error();
    return 0;
}
