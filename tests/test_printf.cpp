/**
 * @file tests/test_printf.cpp
 * @brief Tests for xer/bits/printf.h.
 */

#include <cstdint>
#include <cstdio>
#include <string>
#include <string_view>
#include <vector>

#include <xer/assert.h>
#include <xer/bits/printf.h>

namespace {

struct mock_text_sink {
    std::u8string data;
};

int mock_text_close(xer::text_stream_handle_t handle) noexcept {
    auto* sink = reinterpret_cast<mock_text_sink*>(handle);
    delete sink;
    return 0;
}

int mock_text_read(xer::text_stream_handle_t, char32_t*, int) noexcept {
    return -1;
}

int mock_text_write(xer::text_stream_handle_t handle, const char32_t* s, int n) noexcept {
    if (s == nullptr || n < 0) {
        return -1;
    }

    auto* sink = reinterpret_cast<mock_text_sink*>(handle);
    if (sink == nullptr) {
        return -1;
    }

    for (int i = 0; i < n; ++i) {
        const char32_t ch = s[i];

        if (ch <= 0x7f) {
            sink->data.push_back(static_cast<char8_t>(ch));
        } else if (ch <= 0x7ff) {
            sink->data.push_back(static_cast<char8_t>(0xc0u | ((ch >> 6) & 0x1fu)));
            sink->data.push_back(static_cast<char8_t>(0x80u | (ch & 0x3fu)));
        } else if (ch <= 0xffff) {
            if (ch >= 0xd800 && ch <= 0xdfff) {
                return -1;
            }

            sink->data.push_back(static_cast<char8_t>(0xe0u | ((ch >> 12) & 0x0fu)));
            sink->data.push_back(static_cast<char8_t>(0x80u | ((ch >> 6) & 0x3fu)));
            sink->data.push_back(static_cast<char8_t>(0x80u | (ch & 0x3fu)));
        } else if (ch <= 0x10ffff) {
            sink->data.push_back(static_cast<char8_t>(0xf0u | ((ch >> 18) & 0x07u)));
            sink->data.push_back(static_cast<char8_t>(0x80u | ((ch >> 12) & 0x3fu)));
            sink->data.push_back(static_cast<char8_t>(0x80u | ((ch >> 6) & 0x3fu)));
            sink->data.push_back(static_cast<char8_t>(0x80u | (ch & 0x3fu)));
        } else {
            return -1;
        }
    }

    return n;
}

xer::text_stream_pos_t mock_text_getpos(xer::text_stream_handle_t handle) noexcept {
    auto* sink = reinterpret_cast<mock_text_sink*>(handle);
    if (sink == nullptr) {
        return -1;
    }

    return static_cast<xer::text_stream_pos_t>(sink->data.size());
}

int mock_text_setpos(xer::text_stream_handle_t, xer::text_stream_pos_t) noexcept {
    return -1;
}

int mock_text_seek_end(xer::text_stream_handle_t) noexcept {
    return 0;
}

[[nodiscard]] xer::text_stream make_mock_text_stream(mock_text_sink*& sink_out) {
    sink_out = new mock_text_sink();

    return xer::text_stream(
        reinterpret_cast<xer::text_stream_handle_t>(sink_out),
        mock_text_close,
        mock_text_read,
        mock_text_write,
        mock_text_getpos,
        mock_text_setpos,
        mock_text_seek_end);
}

void test_sprintf_writes_literal_only() {
    std::u8string out = u8"old";
    const auto result = xer::sprintf(out, u8"hello world");

    xer_assert(result.has_value());
    xer_assert_eq(*result, std::size_t{11});
    xer_assert_eq(out, std::u8string(u8"hello world"));
}

void test_sprintf_overwrites_existing_string() {
    std::u8string out = u8"previous contents";
    const auto result = xer::sprintf(out, u8"%s", u8"new");

    xer_assert(result.has_value());
    xer_assert_eq(*result, std::size_t{3});
    xer_assert_eq(out, std::u8string(u8"new"));
}

void test_sprintf_string_and_char() {
    std::u8string out;
    const auto result = xer::sprintf(out, u8"%s %c", u8"abc", U'猫');

    xer_assert(result.has_value());
    xer_assert_eq(out, std::u8string(u8"abc 猫"));
    xer_assert_eq(*result, out.size());
}

void test_sprintf_signed_integer_decimal() {
    std::u8string out;
    const std::int16_t value = -123;

    const auto result = xer::sprintf(out, u8"%d", value);

    xer_assert(result.has_value());
    xer_assert_eq(out, std::u8string(u8"-123"));
    xer_assert_eq(*result, std::size_t{4});
}

void test_sprintf_unsigned_integer_formats() {
    std::u8string out1;
    std::u8string out2;
    std::u8string out3;

    const auto r1 = xer::sprintf(out1, u8"%u", static_cast<unsigned int>(255));
    const auto r2 = xer::sprintf(out2, u8"%x", static_cast<unsigned int>(255));
    const auto r3 = xer::sprintf(out3, u8"%X", static_cast<unsigned int>(255));

    xer_assert(r1.has_value());
    xer_assert(r2.has_value());
    xer_assert(r3.has_value());

    xer_assert_eq(out1, std::u8string(u8"255"));
    xer_assert_eq(out2, std::u8string(u8"ff"));
    xer_assert_eq(out3, std::u8string(u8"FF"));
}

void test_sprintf_octal_and_alternate_form() {
    std::u8string out1;
    std::u8string out2;

    const auto r1 = xer::sprintf(out1, u8"%o", static_cast<unsigned int>(8));
    const auto r2 = xer::sprintf(out2, u8"%#x", static_cast<unsigned int>(255));

    xer_assert(r1.has_value());
    xer_assert(r2.has_value());

    xer_assert_eq(out1, std::u8string(u8"10"));
    xer_assert_eq(out2, std::u8string(u8"0xff"));
}

void test_sprintf_default_specifier_percent_at() {
    std::u8string out1;
    std::u8string out2;
    std::u8string out3;
    std::u8string out4;

    const auto r1 = xer::sprintf(out1, u8"%@", 42);
    const auto r2 = xer::sprintf(out2, u8"%@", true);
    const auto r3 = xer::sprintf(out3, u8"%@", u8"xyz");
    const auto r4 = xer::sprintf(out4, u8"%@", nullptr);

    xer_assert(r1.has_value());
    xer_assert(r2.has_value());
    xer_assert(r3.has_value());
    xer_assert(r4.has_value());

    xer_assert_eq(out1, std::u8string(u8"42"));
    xer_assert_eq(out2, std::u8string(u8"true"));
    xer_assert_eq(out3, std::u8string(u8"xyz"));
    xer_assert_eq(out4, std::u8string(u8"null"));
}

void test_sprintf_percent_escape() {
    std::u8string out;
    const auto result = xer::sprintf(out, u8"100%%");

    xer_assert(result.has_value());
    xer_assert_eq(out, std::u8string(u8"100%"));
    xer_assert_eq(*result, std::size_t{4});
}

void test_sprintf_min_width_and_left_align() {
    std::u8string out1;
    std::u8string out2;

    const auto r1 = xer::sprintf(out1, u8"%5d", 12);
    const auto r2 = xer::sprintf(out2, u8"%-5d", 12);

    xer_assert(r1.has_value());
    xer_assert(r2.has_value());

    xer_assert_eq(out1, std::u8string(u8"   12"));
    xer_assert_eq(out2, std::u8string(u8"12   "));
}

void test_sprintf_zero_fill_and_sign() {
    std::u8string out1;
    std::u8string out2;
    std::u8string out3;

    const auto r1 = xer::sprintf(out1, u8"%05d", 12);
    const auto r2 = xer::sprintf(out2, u8"%+d", 12);
    const auto r3 = xer::sprintf(out3, u8"% d", 12);

    xer_assert(r1.has_value());
    xer_assert(r2.has_value());
    xer_assert(r3.has_value());

    xer_assert_eq(out1, std::u8string(u8"00012"));
    xer_assert_eq(out2, std::u8string(u8"+12"));
    xer_assert_eq(out3, std::u8string(u8" 12"));
}

void test_sprintf_precision_for_string_and_integer() {
    std::u8string out1;
    std::u8string out2;

    const auto r1 = xer::sprintf(out1, u8"%.3s", u8"abcdef");
    const auto r2 = xer::sprintf(out2, u8"%.5d", 42);

    xer_assert(r1.has_value());
    xer_assert(r2.has_value());

    xer_assert_eq(out1, std::u8string(u8"abc"));
    xer_assert_eq(out2, std::u8string(u8"00042"));
}

void test_sprintf_positional_arguments() {
    std::u8string out;
    const auto result = xer::sprintf(out, u8"%2$s %1$d %2$s", 7, u8"xy");

    xer_assert(result.has_value());
    xer_assert_eq(out, std::u8string(u8"xy 7 xy"));
}

void test_sprintf_positional_arguments_with_default_specifier() {
    std::u8string out;
    const auto result = xer::sprintf(out, u8"%2$@ %1$@", 10, false);

    xer_assert(result.has_value());
    xer_assert_eq(out, std::u8string(u8"false 10"));
}

void test_sprintf_rejects_mixed_positional_and_sequential() {
    std::u8string out;
    const auto result = xer::sprintf(out, u8"%2$d %d", 11, 22);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_sprintf_rejects_out_of_range_positional_argument() {
    std::u8string out;
    const auto result = xer::sprintf(out, u8"%2$d", 11);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_sprintf_rejects_invalid_string_pointer() {
    std::u8string out;
    const auto result = xer::sprintf(out, u8"%s", static_cast<const char8_t*>(nullptr));

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_sprintf_rejects_invalid_specifier_for_type() {
    std::u8string out;
    const auto result = xer::sprintf(out, u8"%s", 123);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_sprintf_pointer_format() {
    std::u8string out;
    int value = 0;

    const auto result = xer::sprintf(out, u8"%p", static_cast<void*>(&value));

    xer_assert(result.has_value());
    xer_assert(out.size() >= 3);
    xer_assert_eq(out[0], u8'0');
    xer_assert(out[1] == u8'x' || out[1] == u8'X');
}

void test_fprintf_writes_text_stream() {
    mock_text_sink* sink = nullptr;
    xer::text_stream stream = make_mock_text_stream(sink);

    const auto write_result = xer::fprintf(stream, u8"%s %d %c", u8"abc", 123, U'猫');
    xer_assert(write_result.has_value());
    xer_assert_eq(sink->data, std::u8string(u8"abc 123 猫"));
    xer_assert_eq(*write_result, sink->data.size());

    const auto close_result = stream.close();
    xer_assert_eq(close_result, 0);
}

} // namespace

int main() {
    test_sprintf_writes_literal_only();
    test_sprintf_overwrites_existing_string();
    test_sprintf_string_and_char();
    test_sprintf_signed_integer_decimal();
    test_sprintf_unsigned_integer_formats();
    test_sprintf_octal_and_alternate_form();
    test_sprintf_default_specifier_percent_at();
    test_sprintf_percent_escape();
    test_sprintf_min_width_and_left_align();
    test_sprintf_zero_fill_and_sign();
    test_sprintf_precision_for_string_and_integer();
    test_sprintf_positional_arguments();
    test_sprintf_positional_arguments_with_default_specifier();
    test_sprintf_rejects_mixed_positional_and_sequential();
    test_sprintf_rejects_out_of_range_positional_argument();
    test_sprintf_rejects_invalid_string_pointer();
    test_sprintf_rejects_invalid_specifier_for_type();
    test_sprintf_pointer_format();
    test_fprintf_writes_text_stream();
    return 0;
}
