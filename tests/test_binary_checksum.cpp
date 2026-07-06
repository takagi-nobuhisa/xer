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


void test_checksum_span()
{
    constexpr std::array<std::byte, 5> bytes{
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0x04},
        std::byte{0x05},
    };

    XER_TEST_BINARY_STATIC_ASSERT(xer::checksum_add8(std::span<const std::byte>(bytes)) == 0x0fu);
    XER_TEST_BINARY_STATIC_ASSERT(xer::checksum_xor8(std::span<const std::byte>(bytes)) == 0x01u);

    XER_TEST_BINARY_STATIC_ASSERT(xer::checksum_add16(
        std::span<const std::byte>(bytes),
        xer::byte_order::big_endian) == 0x0906u);
    XER_TEST_BINARY_STATIC_ASSERT(xer::checksum_add16(
        std::span<const std::byte>(bytes),
        xer::byte_order::little_endian) == 0x0609u);

    XER_TEST_BINARY_STATIC_ASSERT(xer::checksum_xor16(
        std::span<const std::byte>(bytes),
        xer::byte_order::big_endian) == 0x0706u);
    XER_TEST_BINARY_STATIC_ASSERT(xer::checksum_xor16(
        std::span<const std::byte>(bytes),
        xer::byte_order::little_endian) == 0x0607u);

    XER_TEST_BINARY_STATIC_ASSERT(xer::checksum_add32(
        std::span<const std::byte>(bytes),
        xer::byte_order::big_endian) == 0x06020304u);
    XER_TEST_BINARY_STATIC_ASSERT(xer::checksum_add32(
        std::span<const std::byte>(bytes),
        xer::byte_order::little_endian) == 0x04030206u);

    XER_TEST_BINARY_STATIC_ASSERT(xer::checksum_xor32(
        std::span<const std::byte>(bytes),
        xer::byte_order::big_endian) == 0x04020304u);
    XER_TEST_BINARY_STATIC_ASSERT(xer::checksum_xor32(
        std::span<const std::byte>(bytes),
        xer::byte_order::little_endian) == 0x04030204u);

    xer_assert_eq(xer::checksum_add8(std::span<const std::byte>(bytes)), 0x0fu);
    xer_assert_eq(xer::checksum_xor8(std::span<const std::byte>(bytes)), 0x01u);
    xer_assert_eq(xer::checksum_add16(
        std::span<const std::byte>(bytes),
        xer::byte_order::big_endian), 0x0906u);
    xer_assert_eq(xer::checksum_add16(
        std::span<const std::byte>(bytes),
        xer::byte_order::little_endian), 0x0609u);
    xer_assert_eq(xer::checksum_add32(
        std::span<const std::byte>(bytes),
        xer::byte_order::big_endian), 0x06020304u);
    xer_assert_eq(xer::checksum_add32(
        std::span<const std::byte>(bytes),
        xer::byte_order::little_endian), 0x04030206u);
}

void test_checksum_pointer()
{
    const std::array<std::byte, 4> bytes{
        std::byte{0x10},
        std::byte{0x20},
        std::byte{0x30},
        std::byte{0x40},
    };

    const auto add8 = xer::checksum_add8(bytes.data(), bytes.size());
    xer_assert(add8.has_value());
    xer_assert_eq(*add8, 0xa0u);

    const auto xor8 = xer::checksum_xor8(bytes.data(), bytes.size());
    xer_assert(xor8.has_value());
    xer_assert_eq(*xor8, 0x40u);

    const auto add16 = xer::checksum_add16(
        bytes.data(),
        bytes.size(),
        xer::byte_order::big_endian);
    xer_assert(add16.has_value());
    xer_assert_eq(*add16, 0x4060u);

    const auto add32 = xer::checksum_add32(
        bytes.data(),
        bytes.size(),
        xer::byte_order::big_endian);
    xer_assert(add32.has_value());
    xer_assert_eq(*add32, 0x10203040u);

    const auto invalid = xer::checksum_add8(nullptr, 1);
    xer_assert_not(invalid.has_value());
}

void test_checksum_iterators()
{
    const std::vector<unsigned char> bytes{0x01u, 0x02u, 0x03u, 0x04u};

    xer_assert_eq(xer::checksum_add8(bytes.begin(), bytes.end()), 0x0au);
    xer_assert_eq(xer::checksum_xor8(bytes.begin(), bytes.end()), 0x04u);
    xer_assert_eq(xer::checksum_add16(
        bytes.begin(),
        bytes.end(),
        xer::byte_order::big_endian), 0x0406u);
    xer_assert_eq(xer::checksum_xor16(
        bytes.begin(),
        bytes.end(),
        xer::byte_order::big_endian), 0x0206u);
    xer_assert_eq(xer::checksum_add32(
        bytes.begin(),
        bytes.end(),
        xer::byte_order::big_endian), 0x01020304u);
    xer_assert_eq(xer::checksum_xor32(
        bytes.begin(),
        bytes.end(),
        xer::byte_order::big_endian), 0x01020304u);
}

void test_checksum_file()
{
    const std::array<std::byte, 4> bytes{
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0x04},
    };

    const xer::path filename(u8"test_binary_checksum.tmp");
    const auto write_result = xer::file_put_contents(filename, std::span<const std::byte>(bytes));
    xer_assert(write_result.has_value());

    const auto add8 = xer::checksum_add8(filename);
    xer_assert(add8.has_value());
    xer_assert_eq(*add8, 0x0au);

    const auto xor8 = xer::checksum_xor8(filename);
    xer_assert(xor8.has_value());
    xer_assert_eq(*xor8, 0x04u);

    const auto add16 = xer::checksum_add16(filename, xer::byte_order::big_endian);
    xer_assert(add16.has_value());
    xer_assert_eq(*add16, 0x0406u);

    const auto xor32 = xer::checksum_xor32(filename, xer::byte_order::big_endian);
    xer_assert(xor32.has_value());
    xer_assert_eq(*xor32, 0x01020304u);

    const auto remove_result = xer::remove(filename);
    xer_assert(remove_result.has_value());
}

void test_checksum_aliases()
{
    constexpr std::array<std::byte, 5> bytes{
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0x04},
        std::byte{0x05},
    };

    XER_TEST_BINARY_STATIC_ASSERT(xer::checksum8(std::span<const std::byte>(bytes))
        == xer::checksum_add8(std::span<const std::byte>(bytes)));
    XER_TEST_BINARY_STATIC_ASSERT(xer::checksum16(
        std::span<const std::byte>(bytes),
        xer::byte_order::big_endian)
        == xer::checksum_add16(
            std::span<const std::byte>(bytes),
            xer::byte_order::big_endian));
    XER_TEST_BINARY_STATIC_ASSERT(xer::checksum32(
        std::span<const std::byte>(bytes),
        xer::byte_order::little_endian)
        == xer::checksum_add32(
            std::span<const std::byte>(bytes),
            xer::byte_order::little_endian));

    xer_assert_eq(
        xer::checksum8(std::span<const std::byte>(bytes)),
        xer::checksum_add8(std::span<const std::byte>(bytes)));
    xer_assert_eq(
        xer::checksum16(std::span<const std::byte>(bytes), xer::byte_order::big_endian),
        xer::checksum_add16(std::span<const std::byte>(bytes), xer::byte_order::big_endian));
    xer_assert_eq(
        xer::checksum32(std::span<const std::byte>(bytes), xer::byte_order::little_endian),
        xer::checksum_add32(std::span<const std::byte>(bytes), xer::byte_order::little_endian));

    const auto pointer8 = xer::checksum8(bytes.data(), bytes.size());
    const auto pointer_add8 = xer::checksum_add8(bytes.data(), bytes.size());
    xer_assert(pointer8.has_value());
    xer_assert(pointer_add8.has_value());
    xer_assert_eq(*pointer8, *pointer_add8);

    const auto pointer16 = xer::checksum16(
        bytes.data(),
        bytes.size(),
        xer::byte_order::big_endian);
    const auto pointer_add16 = xer::checksum_add16(
        bytes.data(),
        bytes.size(),
        xer::byte_order::big_endian);
    xer_assert(pointer16.has_value());
    xer_assert(pointer_add16.has_value());
    xer_assert_eq(*pointer16, *pointer_add16);

    xer_assert_eq(xer::checksum8(bytes.begin(), bytes.end()), xer::checksum_add8(bytes.begin(), bytes.end()));
    xer_assert_eq(
        xer::checksum16(bytes.begin(), bytes.end(), xer::byte_order::little_endian),
        xer::checksum_add16(bytes.begin(), bytes.end(), xer::byte_order::little_endian));
    xer_assert_eq(
        xer::checksum32(bytes.begin(), bytes.end(), xer::byte_order::big_endian),
        xer::checksum_add32(bytes.begin(), bytes.end(), xer::byte_order::big_endian));

    const xer::path filename(u8"test_binary_checksum_alias.tmp");
    const auto write_result = xer::file_put_contents(filename, std::span<const std::byte>(bytes));
    xer_assert(write_result.has_value());

    const auto file8 = xer::checksum8(filename);
    const auto file_add8 = xer::checksum_add8(filename);
    xer_assert(file8.has_value());
    xer_assert(file_add8.has_value());
    xer_assert_eq(*file8, *file_add8);

    const auto file32 = xer::checksum32(filename, xer::byte_order::big_endian);
    const auto file_add32 = xer::checksum_add32(filename, xer::byte_order::big_endian);
    xer_assert(file32.has_value());
    xer_assert(file_add32.has_value());
    xer_assert_eq(*file32, *file_add32);

    const auto remove_result = xer::remove(filename);
    xer_assert(remove_result.has_value());
}

#undef XER_TEST_BINARY_STATIC_ASSERT

} // namespace

auto main() -> int
{
    test_checksum_span();
    test_checksum_pointer();
    test_checksum_iterators();
    test_checksum_file();
    test_checksum_aliases();

    return 0;
}
