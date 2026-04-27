/**
 * @file examples/example_file_contents.cpp
 * @brief Example for xer::file_get_contents and xer::file_put_contents.
 */

// XER_EXAMPLE_BEGIN: file_contents_basic
//
// This example writes and reads both binary and text file contents.
//
// Expected output:
// binary size: 4
// text:
// hello
// XER

#include <array>
#include <cstddef>
#include <span>
#include <string_view>

#include <xer/stdio.h>

auto main() -> int
{
    const xer::path binary_file(u8"example_file_contents.bin");
    const xer::path text_file(u8"example_file_contents.txt");

    const std::array<std::byte, 4> bytes{
        std::byte{0x58},
        std::byte{0x45},
        std::byte{0x52},
        std::byte{0x00},
    };

    const auto binary_written =
        xer::file_put_contents(binary_file, std::span<const std::byte>(bytes));
    if (!binary_written.has_value()) {
        return 1;
    }

    const auto binary_read = xer::file_get_contents(binary_file);
    if (!binary_read.has_value()) {
        static_cast<void>(xer::remove(binary_file));
        return 1;
    }

    if (!xer::printf(u8"binary size: %@\n", binary_read->size()).has_value()) {
        static_cast<void>(xer::remove(binary_file));
        return 1;
    }

    const auto text_written = xer::file_put_contents(
        text_file,
        std::u8string_view(u8"hello\nXER\n"),
        xer::encoding_t::utf8);
    if (!text_written.has_value()) {
        static_cast<void>(xer::remove(binary_file));
        return 1;
    }

    const auto text_read =
        xer::file_get_contents(text_file, xer::encoding_t::utf8);
    if (!text_read.has_value()) {
        static_cast<void>(xer::remove(binary_file));
        static_cast<void>(xer::remove(text_file));
        return 1;
    }

    if (!xer::puts(u8"text:").has_value()) {
        static_cast<void>(xer::remove(binary_file));
        static_cast<void>(xer::remove(text_file));
        return 1;
    }

    if (!xer::fputs(*text_read, xer_stdout).has_value()) {
        static_cast<void>(xer::remove(binary_file));
        static_cast<void>(xer::remove(text_file));
        return 1;
    }

    if (!xer::remove(binary_file).has_value()) {
        return 1;
    }

    if (!xer::remove(text_file).has_value()) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: file_contents_basic
