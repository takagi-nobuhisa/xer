#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include <xer/assert.h>
#include <xer/binary.h>
#include <xer/stdio.h>

namespace {

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

} // namespace

auto main() -> int
{
    test_md5_span();
    test_md5_pointer();
    test_md5_iterators();
    test_md5_file();

    return 0;
}
