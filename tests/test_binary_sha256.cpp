#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include <xer/assert.h>
#include <xer/binary.h>
#include <xer/stdio.h>

namespace {

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
    test_sha256_span();
    test_sha256_pointer();
    test_sha256_iterators();
    test_sha256_file();

    return 0;
}
