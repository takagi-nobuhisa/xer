#include <array>
#include <cstddef>

#include <xer/base64.h>
#include <xer/stdio.h>

// XER_EXAMPLE_BEGIN: base64_basic
//
// This example encodes binary data into Base64 text and decodes it back.
//
// Expected output:
// Encoded: aGVsbG8=
// Decoded: hello

auto main() -> int
{
    const std::array<std::byte, 5> data = {
        std::byte{'h'},
        std::byte{'e'},
        std::byte{'l'},
        std::byte{'l'},
        std::byte{'o'},
    };

    const auto encoded = xer::base64_encode(data);
    if (!encoded.has_value()) {
        return 1;
    }

    if (!xer::printf(u8"Encoded: %@\n", *encoded).has_value()) {
        return 1;
    }

    const auto decoded = xer::base64_decode(*encoded);
    if (!decoded.has_value()) {
        return 1;
    }

    std::u8string decoded_text;
    decoded_text.reserve(decoded->size());

    for (const auto b : *decoded) {
        decoded_text.push_back(static_cast<char8_t>(b));
    }

    if (!xer::printf(u8"Decoded: %@\n", decoded_text).has_value()) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: base64_basic
