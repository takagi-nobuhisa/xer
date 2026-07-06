#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include <xer/assert.h>
#include <xer/binary.h>
#include <xer/stdio.h>

namespace {

void test_bin2hex_span()
{
    const std::array<std::byte, 4> bytes{
        std::byte{0x00},
        std::byte{0x12},
        std::byte{0xab},
        std::byte{0xff},
    };

    xer_assert_eq(xer::bin2hex(std::span<const std::byte>(bytes)), std::u8string(u8"0012abff"));

    const std::array<std::byte, 0> empty{};
    xer_assert_eq(xer::bin2hex(std::span<const std::byte>(empty)), std::u8string());
}

void test_bin2hex_pointer()
{
    const std::array<std::byte, 3> bytes{
        std::byte{0x01},
        std::byte{0x23},
        std::byte{0x45},
    };

    const auto result = xer::bin2hex(bytes.data(), bytes.size());
    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string(u8"012345"));

    const auto empty = xer::bin2hex(nullptr, 0);
    xer_assert(empty.has_value());
    xer_assert_eq(*empty, std::u8string());

    const auto invalid = xer::bin2hex(nullptr, 1);
    xer_assert_not(invalid.has_value());
}

void test_bin2hex_iterators()
{
    const std::vector<unsigned char> bytes{0xdeu, 0xadu, 0xbeu, 0xefu};

    xer_assert_eq(xer::bin2hex(bytes.begin(), bytes.end()), std::u8string(u8"deadbeef"));
}

void test_hex2bin()
{
    const auto result = xer::hex2bin(u8"0012ABff");
    xer_assert(result.has_value());
    xer_assert_eq(result->size(), 4u);
    xer_assert_eq((*result)[0], std::byte{0x00});
    xer_assert_eq((*result)[1], std::byte{0x12});
    xer_assert_eq((*result)[2], std::byte{0xab});
    xer_assert_eq((*result)[3], std::byte{0xff});

    const auto empty = xer::hex2bin(u8"");
    xer_assert(empty.has_value());
    xer_assert_eq(empty->size(), 0u);

    xer_assert_not(xer::hex2bin(u8"0").has_value());
    xer_assert_not(xer::hex2bin(u8"xx").has_value());
}

} // namespace

auto main() -> int
{
    test_bin2hex_span();
    test_bin2hex_pointer();
    test_bin2hex_iterators();
    test_hex2bin();

    return 0;
}
