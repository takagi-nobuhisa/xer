#include <string>
#include <string_view>

#include <xer/assert.h>
#include <xer/stdio.h>

namespace {

auto test_stropen_u8string_view_read_simple() -> void {
    constexpr std::u8string_view input = u8"abc";

    auto stream_result = xer::stropen(input, "r");
    xer_assert(stream_result.has_value());

    auto c1 = xer::fgetc(*stream_result);
    xer_assert(c1.has_value());
    xer_assert_eq(*c1, U'a');

    auto c2 = xer::fgetc(*stream_result);
    xer_assert(c2.has_value());
    xer_assert_eq(*c2, U'b');

    auto c3 = xer::fgetc(*stream_result);
    xer_assert(c3.has_value());
    xer_assert_eq(*c3, U'c');

    auto c4 = xer::fgetc(*stream_result);
    xer_assert(!c4.has_value());
    xer_assert_eq(c4.error().code, xer::error_t::not_found);
}

auto test_stropen_u8string_view_read_newlines() -> void {
    constexpr std::u8string_view input = u8"a\nb\r\nc\r";

    auto stream_result = xer::stropen(input, "r");
    xer_assert(stream_result.has_value());

    auto c1 = xer::fgetc(*stream_result);
    xer_assert(c1.has_value());
    xer_assert_eq(*c1, U'a');

    auto c2 = xer::fgetc(*stream_result);
    xer_assert(c2.has_value());
    xer_assert_eq(*c2, U'\n');

    auto c3 = xer::fgetc(*stream_result);
    xer_assert(c3.has_value());
    xer_assert_eq(*c3, U'b');

    auto c4 = xer::fgetc(*stream_result);
    xer_assert(c4.has_value());
    xer_assert_eq(*c4, U'\r');

    auto c5 = xer::fgetc(*stream_result);
    xer_assert(c5.has_value());
    xer_assert_eq(*c5, U'\n');

    auto c6 = xer::fgetc(*stream_result);
    xer_assert(c6.has_value());
    xer_assert_eq(*c6, U'c');

    auto c7 = xer::fgetc(*stream_result);
    xer_assert(c7.has_value());
    xer_assert_eq(*c7, U'\r');

    auto c8 = xer::fgetc(*stream_result);
    xer_assert(!c8.has_value());
    xer_assert_eq(c8.error().code, xer::error_t::not_found);
}

auto test_stropen_u8string_view_read_utf8() -> void {
    constexpr std::u8string_view input = u8"あい";

    auto stream_result = xer::stropen(input, "r");
    xer_assert(stream_result.has_value());

    auto c1 = xer::fgetc(*stream_result);
    xer_assert(c1.has_value());
    xer_assert_eq(*c1, U'あ');

    auto c2 = xer::fgetc(*stream_result);
    xer_assert(c2.has_value());
    xer_assert_eq(*c2, U'い');

    auto c3 = xer::fgetc(*stream_result);
    xer_assert(!c3.has_value());
    xer_assert_eq(c3.error().code, xer::error_t::not_found);
}

auto test_stropen_u8string_view_ungetc() -> void {
    constexpr std::u8string_view input = u8"xyz";

    auto stream_result = xer::stropen(input, "r");
    xer_assert(stream_result.has_value());

    auto c1 = xer::fgetc(*stream_result);
    xer_assert(c1.has_value());
    xer_assert_eq(*c1, U'x');

    auto unget_result = xer::ungetc(U'x', *stream_result);
    xer_assert(unget_result.has_value());

    auto c2 = xer::fgetc(*stream_result);
    xer_assert(c2.has_value());
    xer_assert_eq(*c2, U'x');

    auto c3 = xer::fgetc(*stream_result);
    xer_assert(c3.has_value());
    xer_assert_eq(*c3, U'y');
}

auto test_stropen_u8string_view_getpos_setpos() -> void {
    constexpr std::u8string_view input = u8"abc";

    auto stream_result = xer::stropen(input, "r");
    xer_assert(stream_result.has_value());

    auto pos0 = xer::fgetpos(*stream_result);
    xer_assert(pos0.has_value());

    auto c1 = xer::fgetc(*stream_result);
    xer_assert(c1.has_value());
    xer_assert_eq(*c1, U'a');

    auto pos1 = xer::fgetpos(*stream_result);
    xer_assert(pos1.has_value());

    auto c2 = xer::fgetc(*stream_result);
    xer_assert(c2.has_value());
    xer_assert_eq(*c2, U'b');

    auto setpos_result = xer::fsetpos(*stream_result, *pos1);
    xer_assert(setpos_result.has_value());

    auto c3 = xer::fgetc(*stream_result);
    xer_assert(c3.has_value());
    xer_assert_eq(*c3, U'b');

    auto setpos_result2 = xer::fsetpos(*stream_result, *pos0);
    xer_assert(setpos_result2.has_value());

    auto c4 = xer::fgetc(*stream_result);
    xer_assert(c4.has_value());
    xer_assert_eq(*c4, U'a');
}

auto test_stropen_u8string_read_simple() -> void {
    std::u8string input = u8"abc";

    auto stream_result = xer::stropen(input, "r");
    xer_assert(stream_result.has_value());

    auto c1 = xer::fgetc(*stream_result);
    xer_assert(c1.has_value());
    xer_assert_eq(*c1, U'a');

    auto c2 = xer::fgetc(*stream_result);
    xer_assert(c2.has_value());
    xer_assert_eq(*c2, U'b');

    auto c3 = xer::fgetc(*stream_result);
    xer_assert(c3.has_value());
    xer_assert_eq(*c3, U'c');

    auto c4 = xer::fgetc(*stream_result);
    xer_assert(!c4.has_value());
    xer_assert_eq(c4.error().code, xer::error_t::not_found);
}

auto test_stropen_u8string_empty_read() -> void {
    std::u8string input;

    auto stream_result = xer::stropen(input, "r");
    xer_assert(stream_result.has_value());

    auto c1 = xer::fgetc(*stream_result);
    xer_assert(!c1.has_value());
    xer_assert_eq(c1.error().code, xer::error_t::not_found);
}

auto test_stropen_u8string_invalid_mode_is_error() -> void {
    std::u8string input = u8"abc";

    auto stream_result = xer::stropen(input, "w");
    xer_assert(stream_result.has_value());

    auto c1 = xer::fgetc(*stream_result);
    xer_assert(!c1.has_value());
}

} // namespace

auto main() -> int {
    test_stropen_u8string_view_read_simple();
    test_stropen_u8string_view_read_newlines();
    test_stropen_u8string_view_read_utf8();
    test_stropen_u8string_view_ungetc();
    test_stropen_u8string_view_getpos_setpos();
    test_stropen_u8string_read_simple();
    test_stropen_u8string_empty_read();
    test_stropen_u8string_invalid_mode_is_error();
    return 0;
}
