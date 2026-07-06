#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <vector>

#include <xer/assert.h>
#include <xer/binary.h>
#include <xer/stdint.h>
#include <xer/stdio.h>

namespace {

#if defined(_MSC_VER)
#define XER_TEST_BINARY_STATIC_ASSERT(...) ((void)0)
#else
#define XER_TEST_BINARY_STATIC_ASSERT(...) static_assert(__VA_ARGS__)
#endif


void test_crc_span()
{
    constexpr std::array<std::byte, 9> bytes{
        std::byte{'1'},
        std::byte{'2'},
        std::byte{'3'},
        std::byte{'4'},
        std::byte{'5'},
        std::byte{'6'},
        std::byte{'7'},
        std::byte{'8'},
        std::byte{'9'},
    };

    XER_TEST_BINARY_STATIC_ASSERT(xer::crc16(std::span<const std::byte>(bytes)) == 0xbb3du);
    XER_TEST_BINARY_STATIC_ASSERT(xer::crc32(std::span<const std::byte>(bytes)) == 0xcbf43926u);

    xer_assert_eq(xer::crc16(std::span<const std::byte>(bytes)), 0xbb3du);
    xer_assert_eq(xer::crc32(std::span<const std::byte>(bytes)), 0xcbf43926u);
}

void test_crc_pointer()
{
    const std::array<std::byte, 9> bytes{
        std::byte{'1'},
        std::byte{'2'},
        std::byte{'3'},
        std::byte{'4'},
        std::byte{'5'},
        std::byte{'6'},
        std::byte{'7'},
        std::byte{'8'},
        std::byte{'9'},
    };

    const auto crc16 = xer::crc16(bytes.data(), bytes.size());
    xer_assert(crc16.has_value());
    xer_assert_eq(*crc16, 0xbb3du);

    const auto crc32 = xer::crc32(bytes.data(), bytes.size());
    xer_assert(crc32.has_value());
    xer_assert_eq(*crc32, 0xcbf43926u);

    const auto invalid = xer::crc32(nullptr, 1);
    xer_assert_not(invalid.has_value());
}

void test_crc_iterators()
{
    const std::vector<unsigned char> bytes{
        '1', '2', '3', '4', '5', '6', '7', '8', '9',
    };

    xer_assert_eq(xer::crc16(bytes.begin(), bytes.end()), 0xbb3du);
    xer_assert_eq(xer::crc32(bytes.begin(), bytes.end()), 0xcbf43926u);
}

void test_crc_file()
{
    const std::array<std::byte, 9> bytes{
        std::byte{'1'},
        std::byte{'2'},
        std::byte{'3'},
        std::byte{'4'},
        std::byte{'5'},
        std::byte{'6'},
        std::byte{'7'},
        std::byte{'8'},
        std::byte{'9'},
    };

    const xer::path filename(u8"test_binary_crc.tmp");
    const auto write_result = xer::file_put_contents(filename, std::span<const std::byte>(bytes));
    xer_assert(write_result.has_value());

    const auto crc16 = xer::crc16(filename);
    xer_assert(crc16.has_value());
    xer_assert_eq(*crc16, 0xbb3du);

    const auto crc32 = xer::crc32(filename);
    xer_assert(crc32.has_value());
    xer_assert_eq(*crc32, 0xcbf43926u);

    const auto remove_result = xer::remove(filename);
    xer_assert(remove_result.has_value());
}

#undef XER_TEST_BINARY_STATIC_ASSERT

} // namespace

auto main() -> int
{
    test_crc_span();
    test_crc_pointer();
    test_crc_iterators();
    test_crc_file();

    return 0;
}
