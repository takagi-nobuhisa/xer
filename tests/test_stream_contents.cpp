/**
 * @file tests/test_stream_contents.cpp
 * @brief Tests for stream_get_contents and stream_put_contents.
 */

#include <array>
#include <cstddef>
#include <span>
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

void test_stream_put_get_contents_binary_all()
{
    const xer::path filename(u8"test_stream_contents_binary_all.tmp");
    cleanup(filename);

    {
        auto stream = xer::fopen(filename, "w");
        xer_assert(stream.has_value());

        const std::array<std::byte, 4> bytes{
            std::byte{0x41},
            std::byte{0x42},
            std::byte{0x00},
            std::byte{0x43},
        };

        const auto written =
            xer::stream_put_contents(*stream, std::span<const std::byte>(bytes));
        xer_assert(written.has_value());
    }

    {
        auto stream = xer::fopen(filename, "r");
        xer_assert(stream.has_value());

        const auto read = xer::stream_get_contents(*stream);
        xer_assert(read.has_value());
        xer_assert_eq(read->size(), static_cast<std::size_t>(4));
        xer_assert_eq((*read)[0], std::byte{0x41});
        xer_assert_eq((*read)[1], std::byte{0x42});
        xer_assert_eq((*read)[2], std::byte{0x00});
        xer_assert_eq((*read)[3], std::byte{0x43});
    }

    cleanup(filename);
}

void test_stream_get_contents_binary_length()
{
    const xer::path filename(u8"test_stream_contents_binary_length.tmp");
    cleanup(filename);

    const std::array<std::byte, 5> bytes{
        std::byte{0x10},
        std::byte{0x11},
        std::byte{0x12},
        std::byte{0x13},
        std::byte{0x14},
    };

    const auto file_written =
        xer::file_put_contents(filename, std::span<const std::byte>(bytes));
    xer_assert(file_written.has_value());

    auto stream = xer::fopen(filename, "r");
    xer_assert(stream.has_value());

    const auto read = xer::stream_get_contents(*stream, 3);
    xer_assert(read.has_value());
    xer_assert_eq(read->size(), static_cast<std::size_t>(3));
    xer_assert_eq((*read)[0], std::byte{0x10});
    xer_assert_eq((*read)[1], std::byte{0x11});
    xer_assert_eq((*read)[2], std::byte{0x12});

    cleanup(filename);
}

void test_stream_get_contents_binary_current_position()
{
    const xer::path filename(u8"test_stream_contents_binary_position.tmp");
    cleanup(filename);

    const std::array<std::byte, 5> bytes{
        std::byte{0x20},
        std::byte{0x21},
        std::byte{0x22},
        std::byte{0x23},
        std::byte{0x24},
    };

    const auto file_written =
        xer::file_put_contents(filename, std::span<const std::byte>(bytes));
    xer_assert(file_written.has_value());

    auto stream = xer::fopen(filename, "r");
    xer_assert(stream.has_value());

    const auto seeked = xer::fseek(*stream, 2, xer::seek_set);
    xer_assert(seeked.has_value());

    const auto read = xer::stream_get_contents(*stream, 2);
    xer_assert(read.has_value());
    xer_assert_eq(read->size(), static_cast<std::size_t>(2));
    xer_assert_eq((*read)[0], std::byte{0x22});
    xer_assert_eq((*read)[1], std::byte{0x23});

    cleanup(filename);
}

void test_stream_get_contents_binary_zero_length()
{
    const xer::path filename(u8"test_stream_contents_binary_zero.tmp");
    cleanup(filename);

    const std::array<std::byte, 2> bytes{
        std::byte{0x01},
        std::byte{0x02},
    };

    const auto file_written =
        xer::file_put_contents(filename, std::span<const std::byte>(bytes));
    xer_assert(file_written.has_value());

    auto stream = xer::fopen(filename, "r");
    xer_assert(stream.has_value());

    const auto read = xer::stream_get_contents(*stream, 0);
    xer_assert(read.has_value());
    xer_assert(read->empty());

    cleanup(filename);
}

void test_stream_put_get_contents_text()
{
    const xer::path filename(u8"test_stream_contents_text.tmp");
    cleanup(filename);

    {
        auto stream = xer::fopen(filename, "w", xer::encoding_t::utf8);
        xer_assert(stream.has_value());

        const auto written =
            xer::stream_put_contents(*stream, std::u8string_view(u8"hello\n猫\n"));
        xer_assert(written.has_value());
    }

    {
        auto stream = xer::fopen(filename, "r", xer::encoding_t::utf8);
        xer_assert(stream.has_value());

        const auto read = xer::stream_get_contents(*stream);
        xer_assert(read.has_value());
        xer_assert_eq(*read, std::u8string(u8"hello\n猫\n"));
    }

    cleanup(filename);
}

void test_stream_get_contents_text_current_position()
{
    const xer::path filename(u8"test_stream_contents_text_position.tmp");
    cleanup(filename);

    const auto file_written = xer::file_put_contents(
        filename,
        std::u8string_view(u8"abc"),
        xer::encoding_t::utf8);
    xer_assert(file_written.has_value());

    auto stream = xer::fopen(filename, "r", xer::encoding_t::utf8);
    xer_assert(stream.has_value());

    const auto skipped = xer::fgetc(*stream);
    xer_assert(skipped.has_value());
    xer_assert_eq(*skipped, U'a');

    const auto read = xer::stream_get_contents(*stream);
    xer_assert(read.has_value());
    xer_assert_eq(*read, std::u8string(u8"bc"));

    cleanup(filename);
}

} // namespace

auto main() -> int
{
    test_stream_put_get_contents_binary_all();
    test_stream_get_contents_binary_length();
    test_stream_get_contents_binary_current_position();
    test_stream_get_contents_binary_zero_length();
    test_stream_put_get_contents_text();
    test_stream_get_contents_text_current_position();

    return 0;
}
