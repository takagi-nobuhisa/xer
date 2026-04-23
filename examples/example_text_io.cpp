// XER_EXAMPLE_BEGIN: text_io_basic
//
// This example compares xer::fputc / xer::fgetc and
// xer::fputs / xer::fgets.
//
// - xer::fputc and xer::fgetc handle one char32_t character.
// - xer::fputs and xer::fgets handle UTF-8 char8_t strings.
//
// Expected output:
// written = AあHello
// World
// fgetc = U+0041 U+3042
// fgets = Hello
// fgets = World

#include <xer/stdio.h>

auto main() -> int
{
    std::u8string written_text;

    auto write_stream = xer::stropen(written_text, "w");
    if (!write_stream) {
        return 1;
    }

    // Write one character at a time as char32_t.
    if (!xer::fputc(U'A', *write_stream)) {
        return 1;
    }

    if (!xer::fputc(U'あ', *write_stream)) {
        return 1;
    }

    // Write UTF-8 strings as char8_t string data.
    if (!xer::fputs(u8"Hello\n", *write_stream)) {
        return 1;
    }

    if (!xer::fputs(u8"World", *write_stream)) {
        return 1;
    }

    if (!xer::fputs(u8"written = ", xer_stdout)) {
        return 1;
    }

    if (!xer::puts(written_text)) {
        return 1;
    }

    auto read_stream = xer::stropen(std::u8string_view(u8"AあHello\nWorld"), "r");
    if (!read_stream) {
        return 1;
    }

    // Read one character at a time as char32_t.
    const auto c1 = xer::fgetc(*read_stream);
    if (!c1) {
        return 1;
    }

    const auto c2 = xer::fgetc(*read_stream);
    if (!c2) {
        return 1;
    }

    if (!xer::printf(
            u8"fgetc = U+%04X U+%04X\n",
            static_cast<unsigned int>(*c1),
            static_cast<unsigned int>(*c2))) {
        return 1;
    }

    // Read lines as UTF-8 char8_t strings.
    const auto line1 = xer::fgets(*read_stream, false);
    if (!line1) {
        return 1;
    }

    const auto line2 = xer::fgets(*read_stream, false);
    if (!line2) {
        return 1;
    }

    if (!xer::fputs(u8"fgets = ", xer_stdout)) {
        return 1;
    }

    if (!xer::puts(*line1)) {
        return 1;
    }

    if (!xer::fputs(u8"fgets = ", xer_stdout)) {
        return 1;
    }

    if (!xer::puts(*line2)) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: text_io_basic
