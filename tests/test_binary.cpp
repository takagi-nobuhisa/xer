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

void test_split_make_u16()
{
    constexpr std::uint16_t value = 0x1234u;

    static_assert(xer::high_u8(value) == 0x12u);
    static_assert(xer::low_u8(value) == 0x34u);
    static_assert(xer::make_u16(0x12u, 0x34u) == value);

    xer_assert_eq(xer::high_u8(value), 0x12u);
    xer_assert_eq(xer::low_u8(value), 0x34u);
    xer_assert_eq(xer::make_u16(0x12u, 0x34u), value);
}

void test_split_make_u32()
{
    constexpr std::uint32_t value = 0x12345678u;

    static_assert(xer::high_u16(value) == 0x1234u);
    static_assert(xer::low_u16(value) == 0x5678u);
    static_assert(xer::make_u32(0x1234u, 0x5678u) == value);

    xer_assert_eq(xer::high_u16(value), 0x1234u);
    xer_assert_eq(xer::low_u16(value), 0x5678u);
    xer_assert_eq(xer::make_u32(0x1234u, 0x5678u), value);
}

void test_split_make_u64()
{
    constexpr std::uint64_t value = 0x0123456789abcdefull;

    static_assert(xer::high_u32(value) == 0x01234567u);
    static_assert(xer::low_u32(value) == 0x89abcdefu);
    static_assert(xer::make_u64(0x01234567u, 0x89abcdefu) == value);

    xer_assert_eq(xer::high_u32(value), 0x01234567u);
    xer_assert_eq(xer::low_u32(value), 0x89abcdefu);
    xer_assert_eq(xer::make_u64(0x01234567u, 0x89abcdefu), value);
}

#if defined(__SIZEOF_INT128__)

void test_split_make_u128()
{
    constexpr xer::uint128_t value = xer::make_u128(
        0x0123456789abcdefull,
        0xfedcba9876543210ull);

    static_assert(xer::high_u64(value) == 0x0123456789abcdefull);
    static_assert(xer::low_u64(value) == 0xfedcba9876543210ull);
    static_assert(xer::make_u128(
        0x0123456789abcdefull,
        0xfedcba9876543210ull) == value);

    xer_assert_eq(xer::high_u64(value), 0x0123456789abcdefull);
    xer_assert_eq(xer::low_u64(value), 0xfedcba9876543210ull);
    xer_assert(xer::make_u128(
        0x0123456789abcdefull,
        0xfedcba9876543210ull) == value);
}

void test_byteswap_u128()
{
    constexpr xer::uint128_t value = xer::make_u128(
        0x0011223344556677ull,
        0x8899aabbccddeeffull);
    constexpr xer::uint128_t expected = xer::make_u128(
        0xffeeddccbbaa9988ull,
        0x7766554433221100ull);

    static_assert(xer::byteswap(value) == expected);
    xer_assert(xer::byteswap(value) == expected);
}

#endif

void test_byteswap_imported_from_std()
{
    constexpr std::uint32_t value = 0x12345678u;

    static_assert(xer::byteswap(value) == std::byteswap(value));
    static_assert(xer::byteswap(value) == 0x78563412u);

    xer_assert_eq(xer::byteswap(value), 0x78563412u);
}

void test_reverse_bits_u8()
{
    constexpr std::uint8_t value = 0b00010010u;
    constexpr std::uint8_t expected = 0b01001000u;

    static_assert(xer::reverse_bits(value) == expected);
    xer_assert_eq(xer::reverse_bits(value), expected);
}

void test_reverse_bits_u16()
{
    constexpr std::uint16_t value = 0x1234u;
    constexpr std::uint16_t expected = 0x2c48u;

    static_assert(xer::reverse_bits(value) == expected);
    xer_assert_eq(xer::reverse_bits(value), expected);
}

void test_reverse_bits_u32()
{
    constexpr std::uint32_t value = 0x12345678u;
    constexpr std::uint32_t expected = 0x1e6a2c48u;

    static_assert(xer::reverse_bits(value) == expected);
    xer_assert_eq(xer::reverse_bits(value), expected);
}

void test_reverse_bits_u64()
{
    constexpr std::uint64_t value = 0x0123456789abcdefull;
    constexpr std::uint64_t expected = 0xf7b3d591e6a2c480ull;

    static_assert(xer::reverse_bits(value) == expected);
    xer_assert_eq(xer::reverse_bits(value), expected);
}

#if defined(__SIZEOF_INT128__)

void test_reverse_bits_u128()
{
    constexpr xer::uint128_t value = xer::make_u128(
        0x0123456789abcdefull,
        0xfedcba9876543210ull);
    constexpr xer::uint128_t expected = xer::make_u128(
        0x084c2a6e195d3b7full,
        0xf7b3d591e6a2c480ull);

    static_assert(xer::reverse_bits(value) == expected);
    xer_assert(xer::reverse_bits(value) == expected);
}

#endif

void test_reverse_bits_round_trip()
{
    constexpr std::uint64_t value = 0x13579bdf2468ace0ull;

    static_assert(xer::reverse_bits(xer::reverse_bits(value)) == value);
    xer_assert_eq(xer::reverse_bits(xer::reverse_bits(value)), value);
}


void test_checksum_span()
{
    constexpr std::array<std::byte, 5> bytes{
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0x04},
        std::byte{0x05},
    };

    static_assert(xer::checksum_add8(std::span<const std::byte>(bytes)) == 0x0fu);
    static_assert(xer::checksum_xor8(std::span<const std::byte>(bytes)) == 0x01u);

    static_assert(xer::checksum_add16(
        std::span<const std::byte>(bytes),
        xer::byte_order::big_endian) == 0x0906u);
    static_assert(xer::checksum_add16(
        std::span<const std::byte>(bytes),
        xer::byte_order::little_endian) == 0x0609u);

    static_assert(xer::checksum_xor16(
        std::span<const std::byte>(bytes),
        xer::byte_order::big_endian) == 0x0706u);
    static_assert(xer::checksum_xor16(
        std::span<const std::byte>(bytes),
        xer::byte_order::little_endian) == 0x0607u);

    static_assert(xer::checksum_add32(
        std::span<const std::byte>(bytes),
        xer::byte_order::big_endian) == 0x06020304u);
    static_assert(xer::checksum_add32(
        std::span<const std::byte>(bytes),
        xer::byte_order::little_endian) == 0x04030206u);

    static_assert(xer::checksum_xor32(
        std::span<const std::byte>(bytes),
        xer::byte_order::big_endian) == 0x04020304u);
    static_assert(xer::checksum_xor32(
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

    static_assert(xer::checksum8(std::span<const std::byte>(bytes))
        == xer::checksum_add8(std::span<const std::byte>(bytes)));
    static_assert(xer::checksum16(
        std::span<const std::byte>(bytes),
        xer::byte_order::big_endian)
        == xer::checksum_add16(
            std::span<const std::byte>(bytes),
            xer::byte_order::big_endian));
    static_assert(xer::checksum32(
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

    static_assert(xer::crc16(std::span<const std::byte>(bytes)) == 0xbb3du);
    static_assert(xer::crc32(std::span<const std::byte>(bytes)) == 0xcbf43926u);

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

void test_md5_span()
{
    constexpr std::array<std::byte, 0> empty{};
    const auto empty_digest = xer::md5(std::span<const std::byte>(empty));
    xer_assert_eq(xer::bin2hex(empty_digest.begin(), empty_digest.end()), std::u8string(u8"d41d8cd98f00b204e9800998ecf8427e"));

    constexpr std::array<std::byte, 3> abc{
        std::byte{'a'},
        std::byte{'b'},
        std::byte{'c'},
    };
    const auto abc_digest = xer::md5(std::span<const std::byte>(abc));
    xer_assert_eq(xer::bin2hex(abc_digest.begin(), abc_digest.end()), std::u8string(u8"900150983cd24fb0d6963f7d28e17f72"));

    constexpr std::array<std::byte, 56> long_text{
        std::byte{'a'}, std::byte{'b'}, std::byte{'c'}, std::byte{'d'},
        std::byte{'e'}, std::byte{'f'}, std::byte{'g'}, std::byte{'h'},
        std::byte{'i'}, std::byte{'j'}, std::byte{'k'}, std::byte{'l'},
        std::byte{'m'}, std::byte{'n'}, std::byte{'o'}, std::byte{'p'},
        std::byte{'q'}, std::byte{'r'}, std::byte{'s'}, std::byte{'t'},
        std::byte{'u'}, std::byte{'v'}, std::byte{'w'}, std::byte{'x'},
        std::byte{'y'}, std::byte{'z'}, std::byte{'A'}, std::byte{'B'},
        std::byte{'C'}, std::byte{'D'}, std::byte{'E'}, std::byte{'F'},
        std::byte{'G'}, std::byte{'H'}, std::byte{'I'}, std::byte{'J'},
        std::byte{'K'}, std::byte{'L'}, std::byte{'M'}, std::byte{'N'},
        std::byte{'O'}, std::byte{'P'}, std::byte{'Q'}, std::byte{'R'},
        std::byte{'S'}, std::byte{'T'}, std::byte{'U'}, std::byte{'V'},
        std::byte{'W'}, std::byte{'X'}, std::byte{'Y'}, std::byte{'Z'},
        std::byte{'0'}, std::byte{'1'}, std::byte{'2'}, std::byte{'3'},
    };
    const auto long_digest = xer::md5(std::span<const std::byte>(long_text));
    xer_assert_eq(xer::bin2hex(long_digest.begin(), long_digest.end()), std::u8string(u8"d43e61e9b5f8c9d22c4dc5db6e6df775"));
}

void test_md5_pointer()
{
    const std::array<std::byte, 1> bytes{std::byte{'a'}};

    const auto digest = xer::md5(bytes.data(), bytes.size());
    xer_assert(digest.has_value());
    xer_assert_eq(xer::bin2hex(digest->begin(), digest->end()), std::u8string(u8"0cc175b9c0f1b6a831c399e269772661"));

    const auto invalid = xer::md5(nullptr, 1);
    xer_assert_not(invalid.has_value());
}

void test_md5_iterators()
{
    const std::vector<unsigned char> bytes{
        'm', 'e', 's', 's', 'a', 'g', 'e', ' ',
        'd', 'i', 'g', 'e', 's', 't',
    };

    const auto digest = xer::md5(bytes.begin(), bytes.end());
    xer_assert_eq(xer::bin2hex(digest.begin(), digest.end()), std::u8string(u8"f96b697d7cb7938d525a2f31aaf161d0"));
}

void test_md5_file()
{
    const std::array<std::byte, 26> bytes{
        std::byte{'a'}, std::byte{'b'}, std::byte{'c'}, std::byte{'d'},
        std::byte{'e'}, std::byte{'f'}, std::byte{'g'}, std::byte{'h'},
        std::byte{'i'}, std::byte{'j'}, std::byte{'k'}, std::byte{'l'},
        std::byte{'m'}, std::byte{'n'}, std::byte{'o'}, std::byte{'p'},
        std::byte{'q'}, std::byte{'r'}, std::byte{'s'}, std::byte{'t'},
        std::byte{'u'}, std::byte{'v'}, std::byte{'w'}, std::byte{'x'},
        std::byte{'y'}, std::byte{'z'},
    };

    const xer::path filename(u8"test_binary_md5.tmp");
    const auto write_result = xer::file_put_contents(filename, std::span<const std::byte>(bytes));
    xer_assert(write_result.has_value());

    const auto digest = xer::md5(filename);
    xer_assert(digest.has_value());
    xer_assert_eq(xer::bin2hex(digest->begin(), digest->end()), std::u8string(u8"c3fcd3d76192e4007dfb496cca67e13b"));

    const auto remove_result = xer::remove(filename);
    xer_assert(remove_result.has_value());
}



void test_sha1_span()
{
    constexpr std::array<std::byte, 0> empty{};
    const auto empty_digest = xer::sha1(std::span<const std::byte>(empty));
    xer_assert_eq(xer::bin2hex(empty_digest.begin(), empty_digest.end()), std::u8string(u8"da39a3ee5e6b4b0d3255bfef95601890afd80709"));

    constexpr std::array<std::byte, 3> abc{
        std::byte{'a'},
        std::byte{'b'},
        std::byte{'c'},
    };
    const auto abc_digest = xer::sha1(std::span<const std::byte>(abc));
    xer_assert_eq(xer::bin2hex(abc_digest.begin(), abc_digest.end()), std::u8string(u8"a9993e364706816aba3e25717850c26c9cd0d89d"));

    constexpr std::array<std::byte, 56> long_text{
        std::byte{'a'}, std::byte{'b'}, std::byte{'c'}, std::byte{'d'},
        std::byte{'e'}, std::byte{'f'}, std::byte{'g'}, std::byte{'h'},
        std::byte{'i'}, std::byte{'j'}, std::byte{'k'}, std::byte{'l'},
        std::byte{'m'}, std::byte{'n'}, std::byte{'o'}, std::byte{'p'},
        std::byte{'q'}, std::byte{'r'}, std::byte{'s'}, std::byte{'t'},
        std::byte{'u'}, std::byte{'v'}, std::byte{'w'}, std::byte{'x'},
        std::byte{'y'}, std::byte{'z'}, std::byte{'A'}, std::byte{'B'},
        std::byte{'C'}, std::byte{'D'}, std::byte{'E'}, std::byte{'F'},
        std::byte{'G'}, std::byte{'H'}, std::byte{'I'}, std::byte{'J'},
        std::byte{'K'}, std::byte{'L'}, std::byte{'M'}, std::byte{'N'},
        std::byte{'O'}, std::byte{'P'}, std::byte{'Q'}, std::byte{'R'},
        std::byte{'S'}, std::byte{'T'}, std::byte{'U'}, std::byte{'V'},
        std::byte{'W'}, std::byte{'X'}, std::byte{'Y'}, std::byte{'Z'},
        std::byte{'0'}, std::byte{'1'}, std::byte{'2'}, std::byte{'3'},
    };
    const auto long_digest = xer::sha1(std::span<const std::byte>(long_text));
    xer_assert_eq(xer::bin2hex(long_digest.begin(), long_digest.end()), std::u8string(u8"329c9c4de87d10dec65d56fdfb41da089cbf2a62"));
}

void test_sha1_pointer()
{
    const std::array<std::byte, 1> bytes{std::byte{'a'}};

    const auto digest = xer::sha1(bytes.data(), bytes.size());
    xer_assert(digest.has_value());
    xer_assert_eq(xer::bin2hex(digest->begin(), digest->end()), std::u8string(u8"86f7e437faa5a7fce15d1ddcb9eaeaea377667b8"));

    const auto invalid = xer::sha1(nullptr, 1);
    xer_assert_not(invalid.has_value());
}

void test_sha1_iterators()
{
    const std::vector<unsigned char> bytes{
        'm', 'e', 's', 's', 'a', 'g', 'e', ' ',
        'd', 'i', 'g', 'e', 's', 't',
    };

    const auto digest = xer::sha1(bytes.begin(), bytes.end());
    xer_assert_eq(xer::bin2hex(digest.begin(), digest.end()), std::u8string(u8"c12252ceda8be8994d5fa0290a47231c1d16aae3"));
}

void test_sha1_file()
{
    const std::array<std::byte, 26> bytes{
        std::byte{'a'}, std::byte{'b'}, std::byte{'c'}, std::byte{'d'},
        std::byte{'e'}, std::byte{'f'}, std::byte{'g'}, std::byte{'h'},
        std::byte{'i'}, std::byte{'j'}, std::byte{'k'}, std::byte{'l'},
        std::byte{'m'}, std::byte{'n'}, std::byte{'o'}, std::byte{'p'},
        std::byte{'q'}, std::byte{'r'}, std::byte{'s'}, std::byte{'t'},
        std::byte{'u'}, std::byte{'v'}, std::byte{'w'}, std::byte{'x'},
        std::byte{'y'}, std::byte{'z'},
    };

    const xer::path filename(u8"test_binary_sha1.tmp");
    const auto write_result = xer::file_put_contents(filename, std::span<const std::byte>(bytes));
    xer_assert(write_result.has_value());

    const auto digest = xer::sha1(filename);
    xer_assert(digest.has_value());
    xer_assert_eq(xer::bin2hex(digest->begin(), digest->end()), std::u8string(u8"32d10c7b8cf96570ca04ce37f2a19d84240d3a89"));

    const auto remove_result = xer::remove(filename);
    xer_assert(remove_result.has_value());
}

void test_sha256_span()
{
    constexpr std::array<std::byte, 0> empty{};
    const auto empty_digest = xer::sha256(std::span<const std::byte>(empty));
    xer_assert_eq(xer::bin2hex(empty_digest.begin(), empty_digest.end()), std::u8string(u8"e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"));

    constexpr std::array<std::byte, 3> abc{
        std::byte{'a'},
        std::byte{'b'},
        std::byte{'c'},
    };
    const auto abc_digest = xer::sha256(std::span<const std::byte>(abc));
    xer_assert_eq(xer::bin2hex(abc_digest.begin(), abc_digest.end()), std::u8string(u8"ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"));

    constexpr std::array<std::byte, 56> long_text{
        std::byte{'a'}, std::byte{'b'}, std::byte{'c'}, std::byte{'d'},
        std::byte{'e'}, std::byte{'f'}, std::byte{'g'}, std::byte{'h'},
        std::byte{'i'}, std::byte{'j'}, std::byte{'k'}, std::byte{'l'},
        std::byte{'m'}, std::byte{'n'}, std::byte{'o'}, std::byte{'p'},
        std::byte{'q'}, std::byte{'r'}, std::byte{'s'}, std::byte{'t'},
        std::byte{'u'}, std::byte{'v'}, std::byte{'w'}, std::byte{'x'},
        std::byte{'y'}, std::byte{'z'}, std::byte{'A'}, std::byte{'B'},
        std::byte{'C'}, std::byte{'D'}, std::byte{'E'}, std::byte{'F'},
        std::byte{'G'}, std::byte{'H'}, std::byte{'I'}, std::byte{'J'},
        std::byte{'K'}, std::byte{'L'}, std::byte{'M'}, std::byte{'N'},
        std::byte{'O'}, std::byte{'P'}, std::byte{'Q'}, std::byte{'R'},
        std::byte{'S'}, std::byte{'T'}, std::byte{'U'}, std::byte{'V'},
        std::byte{'W'}, std::byte{'X'}, std::byte{'Y'}, std::byte{'Z'},
        std::byte{'0'}, std::byte{'1'}, std::byte{'2'}, std::byte{'3'},
    };
    const auto long_digest = xer::sha256(std::span<const std::byte>(long_text));
    xer_assert_eq(xer::bin2hex(long_digest.begin(), long_digest.end()), std::u8string(u8"8fb605eab2efae3d1fcc881fa5c5dd6219a17ca3663e46642ff566847c24c272"));
}

void test_sha256_pointer()
{
    const std::array<std::byte, 1> bytes{std::byte{'a'}};

    const auto digest = xer::sha256(bytes.data(), bytes.size());
    xer_assert(digest.has_value());
    xer_assert_eq(xer::bin2hex(digest->begin(), digest->end()), std::u8string(u8"ca978112ca1bbdcafac231b39a23dc4da786eff8147c4e72b9807785afee48bb"));

    const auto invalid = xer::sha256(nullptr, 1);
    xer_assert_not(invalid.has_value());
}

void test_sha256_iterators()
{
    const std::vector<unsigned char> bytes{
        'm', 'e', 's', 's', 'a', 'g', 'e', ' ',
        'd', 'i', 'g', 'e', 's', 't',
    };

    const auto digest = xer::sha256(bytes.begin(), bytes.end());
    xer_assert_eq(xer::bin2hex(digest.begin(), digest.end()), std::u8string(u8"f7846f55cf23e14eebeab5b4e1550cad5b509e3348fbc4efa3a1413d393cb650"));
}

void test_sha256_file()
{
    const std::array<std::byte, 26> bytes{
        std::byte{'a'}, std::byte{'b'}, std::byte{'c'}, std::byte{'d'},
        std::byte{'e'}, std::byte{'f'}, std::byte{'g'}, std::byte{'h'},
        std::byte{'i'}, std::byte{'j'}, std::byte{'k'}, std::byte{'l'},
        std::byte{'m'}, std::byte{'n'}, std::byte{'o'}, std::byte{'p'},
        std::byte{'q'}, std::byte{'r'}, std::byte{'s'}, std::byte{'t'},
        std::byte{'u'}, std::byte{'v'}, std::byte{'w'}, std::byte{'x'},
        std::byte{'y'}, std::byte{'z'},
    };

    const xer::path filename(u8"test_binary_sha256.tmp");
    const auto write_result = xer::file_put_contents(filename, std::span<const std::byte>(bytes));
    xer_assert(write_result.has_value());

    const auto digest = xer::sha256(filename);
    xer_assert(digest.has_value());
    xer_assert_eq(xer::bin2hex(digest->begin(), digest->end()), std::u8string(u8"71c480df93d6ae2f1efad1447c66c9525e316218cf51fc8d9ed832f2daf18b73"));

    const auto remove_result = xer::remove(filename);
    xer_assert(remove_result.has_value());
}

} // namespace

auto main() -> int
{
    test_split_make_u16();
    test_split_make_u32();
    test_split_make_u64();
#if defined(__SIZEOF_INT128__)
    test_split_make_u128();
    test_byteswap_u128();
#endif
    test_byteswap_imported_from_std();
    test_reverse_bits_u8();
    test_reverse_bits_u16();
    test_reverse_bits_u32();
    test_reverse_bits_u64();
#if defined(__SIZEOF_INT128__)
    test_reverse_bits_u128();
#endif
    test_reverse_bits_round_trip();
    test_checksum_span();
    test_checksum_pointer();
    test_checksum_iterators();
    test_checksum_file();
    test_checksum_aliases();
    test_crc_span();
    test_crc_pointer();
    test_crc_iterators();
    test_crc_file();
    test_bin2hex_span();
    test_bin2hex_pointer();
    test_bin2hex_iterators();
    test_hex2bin();
    test_md5_span();
    test_md5_pointer();
    test_md5_iterators();
    test_md5_file();
    test_sha1_span();
    test_sha1_pointer();
    test_sha1_iterators();
    test_sha1_file();
    test_sha256_span();
    test_sha256_pointer();
    test_sha256_iterators();
    test_sha256_file();

    return 0;
}
