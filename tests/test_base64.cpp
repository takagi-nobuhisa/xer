#include <array>
#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <xer/assert.h>
#include <xer/base64.h>
#include <xer/error.h>

namespace {

[[nodiscard]] auto bytes(std::string_view text) -> std::vector<std::byte>
{
    std::vector<std::byte> result;
    result.reserve(text.size());

    for (const char c : text) {
        result.push_back(static_cast<std::byte>(static_cast<unsigned char>(c)));
    }

    return result;
}

void assert_bytes_eq(std::span<const std::byte> lhs, std::span<const std::byte> rhs)
{
    xer_assert_eq(lhs.size(), rhs.size());

    for (std::size_t i = 0; i < lhs.size(); ++i) {
        xer_assert(lhs[i] == rhs[i]);
    }
}

void test_base64_encode_empty()
{
    const std::vector<std::byte> data;
    const auto result = xer::base64_encode(data);

    xer_assert(result.has_value());
    xer_assert_eq(*result, u8"");
}

void test_base64_encode_rfc_vectors()
{
    const auto r1 = xer::base64_encode(bytes("f"));
    const auto r2 = xer::base64_encode(bytes("fo"));
    const auto r3 = xer::base64_encode(bytes("foo"));
    const auto r4 = xer::base64_encode(bytes("foob"));
    const auto r5 = xer::base64_encode(bytes("fooba"));
    const auto r6 = xer::base64_encode(bytes("foobar"));

    xer_assert(r1.has_value());
    xer_assert(r2.has_value());
    xer_assert(r3.has_value());
    xer_assert(r4.has_value());
    xer_assert(r5.has_value());
    xer_assert(r6.has_value());

    xer_assert_eq(*r1, u8"Zg==");
    xer_assert_eq(*r2, u8"Zm8=");
    xer_assert_eq(*r3, u8"Zm9v");
    xer_assert_eq(*r4, u8"Zm9vYg==");
    xer_assert_eq(*r5, u8"Zm9vYmE=");
    xer_assert_eq(*r6, u8"Zm9vYmFy");
}

void test_base64_encode_binary_bytes()
{
    const std::array<std::byte, 6> data = {
        std::byte{0x00},
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0xFD},
        std::byte{0xFE},
        std::byte{0xFF},
    };

    const auto result = xer::base64_encode(data);

    xer_assert(result.has_value());
    xer_assert_eq(*result, u8"AAEC/f7/");
}

void test_base64_decode_empty()
{
    const auto result = xer::base64_decode(u8"");

    xer_assert(result.has_value());
    xer_assert(result->empty());
}

void test_base64_decode_rfc_vectors()
{
    const auto r1 = xer::base64_decode(u8"Zg==");
    const auto r2 = xer::base64_decode(u8"Zm8=");
    const auto r3 = xer::base64_decode(u8"Zm9v");
    const auto r4 = xer::base64_decode(u8"Zm9vYg==");
    const auto r5 = xer::base64_decode(u8"Zm9vYmE=");
    const auto r6 = xer::base64_decode(u8"Zm9vYmFy");

    xer_assert(r1.has_value());
    xer_assert(r2.has_value());
    xer_assert(r3.has_value());
    xer_assert(r4.has_value());
    xer_assert(r5.has_value());
    xer_assert(r6.has_value());

    assert_bytes_eq(*r1, bytes("f"));
    assert_bytes_eq(*r2, bytes("fo"));
    assert_bytes_eq(*r3, bytes("foo"));
    assert_bytes_eq(*r4, bytes("foob"));
    assert_bytes_eq(*r5, bytes("fooba"));
    assert_bytes_eq(*r6, bytes("foobar"));
}

void test_base64_decode_ignores_ascii_space()
{
    const auto result = xer::base64_decode(u8" Zm9v\r\nYmFy\t ");

    xer_assert(result.has_value());
    assert_bytes_eq(*result, bytes("foobar"));
}

void test_base64_decode_binary_bytes()
{
    const auto result = xer::base64_decode(u8"AAEC/f7/");
    const std::vector<std::byte> expected = {
        std::byte{0x00},
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0xFD},
        std::byte{0xFE},
        std::byte{0xFF},
    };

    xer_assert(result.has_value());
    assert_bytes_eq(*result, expected);
}

void test_base64_round_trip()
{
    const auto source = bytes("Base64 round trip for xer.");

    const auto encoded = xer::base64_encode(source);
    xer_assert(encoded.has_value());

    const auto decoded = xer::base64_decode(*encoded);
    xer_assert(decoded.has_value());
    assert_bytes_eq(*decoded, source);
}

void test_base64_decode_rejects_invalid_character()
{
    const auto result = xer::base64_decode(u8"Zm9v*");

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_base64_decode_rejects_invalid_length()
{
    const auto result = xer::base64_decode(u8"Zg=");

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_base64_decode_rejects_padding_in_middle()
{
    const auto result = xer::base64_decode(u8"Zg==Zm8=");

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_base64_decode_rejects_malformed_padding()
{
    const auto result1 = xer::base64_decode(u8"Z=g=");
    const auto result2 = xer::base64_decode(u8"Zg=Z");

    xer_assert_not(result1.has_value());
    xer_assert_eq(result1.error().code, xer::error_t::invalid_argument);

    xer_assert_not(result2.has_value());
    xer_assert_eq(result2.error().code, xer::error_t::invalid_argument);
}

void test_base64_decode_rejects_non_canonical_padding_bits()
{
    const auto result1 = xer::base64_decode(u8"TR==");
    const auto result2 = xer::base64_decode(u8"TWF=");

    xer_assert_not(result1.has_value());
    xer_assert_eq(result1.error().code, xer::error_t::invalid_argument);

    xer_assert_not(result2.has_value());
    xer_assert_eq(result2.error().code, xer::error_t::invalid_argument);
}

} // namespace

auto main() -> int
{
    test_base64_encode_empty();
    test_base64_encode_rfc_vectors();
    test_base64_encode_binary_bytes();
    test_base64_decode_empty();
    test_base64_decode_rfc_vectors();
    test_base64_decode_ignores_ascii_space();
    test_base64_decode_binary_bytes();
    test_base64_round_trip();
    test_base64_decode_rejects_invalid_character();
    test_base64_decode_rejects_invalid_length();
    test_base64_decode_rejects_padding_in_middle();
    test_base64_decode_rejects_malformed_padding();
    test_base64_decode_rejects_non_canonical_padding_bits();

    return 0;
}
