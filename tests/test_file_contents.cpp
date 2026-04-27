/**
 * @file tests/test_file_contents.cpp
 * @brief Tests for file_get_contents and file_put_contents.
 */

#include <array>
#include <cstddef>
#include <string>

#include <xer/assert.h>
#include <xer/stdio.h>

namespace {

void cleanup(const xer::path& filename)
{
    if (xer::is_file(filename)) {
        static_cast<void>(xer::remove(filename));
    }
}

void test_file_put_get_contents_binary_all()
{
    const xer::path filename(u8"test_file_contents_binary_all.tmp");
    cleanup(filename);

    const std::array<std::byte, 5> bytes{
        std::byte{0x41},
        std::byte{0x00},
        std::byte{0x42},
        std::byte{0xff},
        std::byte{0x43},
    };

    const auto written = xer::file_put_contents(filename, std::span<const std::byte>(bytes));
    xer_assert(written.has_value());

    const auto read = xer::file_get_contents(filename);
    xer_assert(read.has_value());
    xer_assert_eq(read->size(), bytes.size());

    for (std::size_t i = 0; i < bytes.size(); ++i) {
        xer_assert_eq((*read)[i], bytes[i]);
    }

    cleanup(filename);
}

void test_file_get_contents_binary_range()
{
    const xer::path filename(u8"test_file_contents_binary_range.tmp");
    cleanup(filename);

    const std::array<std::byte, 6> bytes{
        std::byte{0x10},
        std::byte{0x11},
        std::byte{0x12},
        std::byte{0x13},
        std::byte{0x14},
        std::byte{0x15},
    };

    const auto written = xer::file_put_contents(filename, std::span<const std::byte>(bytes));
    xer_assert(written.has_value());

    const auto read = xer::file_get_contents(filename, 2, 3);
    xer_assert(read.has_value());
    xer_assert_eq(read->size(), static_cast<std::size_t>(3));
    xer_assert_eq((*read)[0], std::byte{0x12});
    xer_assert_eq((*read)[1], std::byte{0x13});
    xer_assert_eq((*read)[2], std::byte{0x14});

    cleanup(filename);
}

void test_file_get_contents_binary_offset_at_end()
{
    const xer::path filename(u8"test_file_contents_binary_end.tmp");
    cleanup(filename);

    const std::array<std::byte, 2> bytes{std::byte{0x01}, std::byte{0x02}};

    const auto written = xer::file_put_contents(filename, std::span<const std::byte>(bytes));
    xer_assert(written.has_value());

    const auto read = xer::file_get_contents(filename, 2, 10);
    xer_assert(read.has_value());
    xer_assert(read->empty());

    cleanup(filename);
}

void test_file_get_contents_binary_offset_past_end_fails()
{
    const xer::path filename(u8"test_file_contents_binary_past_end.tmp");
    cleanup(filename);

    const std::array<std::byte, 2> bytes{std::byte{0x01}, std::byte{0x02}};

    const auto written = xer::file_put_contents(filename, std::span<const std::byte>(bytes));
    xer_assert(written.has_value());

    const auto read = xer::file_get_contents(filename, 3, 1);
    xer_assert_not(read.has_value());
    xer_assert_eq(read.error().code, xer::error_t::invalid_argument);

    cleanup(filename);
}

void test_file_get_contents_binary_zero_length()
{
    const xer::path filename(u8"test_file_contents_binary_zero.tmp");
    cleanup(filename);

    const std::array<std::byte, 2> bytes{std::byte{0x01}, std::byte{0x02}};

    const auto written = xer::file_put_contents(filename, std::span<const std::byte>(bytes));
    xer_assert(written.has_value());

    const auto read = xer::file_get_contents(filename, 1, 0);
    xer_assert(read.has_value());
    xer_assert(read->empty());

    cleanup(filename);
}

void test_file_put_get_contents_text_utf8()
{
    const xer::path filename(u8"test_file_contents_text_utf8.tmp");
    cleanup(filename);

    const auto written = xer::file_put_contents(
        filename,
        std::u8string_view(u8"hello\n猫\n"),
        xer::encoding_t::utf8);
    xer_assert(written.has_value());

    const auto read = xer::file_get_contents(filename, xer::encoding_t::utf8);
    xer_assert(read.has_value());
    xer_assert_eq(*read, std::u8string(u8"hello\n猫\n"));

    cleanup(filename);
}

void test_file_get_contents_text_auto_detect()
{
    const xer::path filename(u8"test_file_contents_text_auto.tmp");
    cleanup(filename);

    const auto written = xer::file_put_contents(
        filename,
        std::u8string_view(u8"auto detect\n"),
        xer::encoding_t::utf8);
    xer_assert(written.has_value());

    const auto read = xer::file_get_contents(filename, xer::encoding_t::auto_detect);
    xer_assert(read.has_value());
    xer_assert_eq(*read, std::u8string(u8"auto detect\n"));

    cleanup(filename);
}

void test_file_put_contents_text_auto_detect_fails()
{
    const xer::path filename(u8"test_file_contents_text_invalid.tmp");
    cleanup(filename);

    const auto written = xer::file_put_contents(
        filename,
        std::u8string_view(u8"text"),
        xer::encoding_t::auto_detect);
    xer_assert_not(written.has_value());
    xer_assert_eq(written.error().code, xer::error_t::invalid_argument);

    cleanup(filename);
}

} // namespace

auto main() -> int
{
    test_file_put_get_contents_binary_all();
    test_file_get_contents_binary_range();
    test_file_get_contents_binary_offset_at_end();
    test_file_get_contents_binary_offset_past_end_fails();
    test_file_get_contents_binary_zero_length();
    test_file_put_get_contents_text_utf8();
    test_file_get_contents_text_auto_detect();
    test_file_put_contents_text_auto_detect_fails();

    return 0;
}
