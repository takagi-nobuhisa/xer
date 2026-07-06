#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include <xer/assert.h>
#include <xer/binary.h>
#include <xer/stdio.h>

namespace {

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

} // namespace

auto main() -> int
{
    test_sha1_span();
    test_sha1_pointer();
    test_sha1_iterators();
    test_sha1_file();

    return 0;
}
