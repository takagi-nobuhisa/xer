#include <string_view>

#include <xer/base64.h>
#include <xer/bytes.h>
#include <xer/stdio.h>

// XER_EXAMPLE_BEGIN: bytes_basic
//
// This example views UTF-8 text as bytes without copying and also creates an
// owning byte vector.
//
// Expected output:
// View size: 5
// Copy size: 5
// Base64: aGVsbG8=

auto main() -> int
{
    constexpr std::u8string_view text = u8"hello";

    const auto view = xer::to_bytes_view(text);
    const auto copied = xer::to_bytes(text);

    const auto encoded = xer::base64_encode(view);
    if (!encoded.has_value()) {
        return 1;
    }

    if (!xer::printf(u8"View size: %zu\n", view.size()).has_value()) {
        return 1;
    }

    if (!xer::printf(u8"Copy size: %zu\n", copied.size()).has_value()) {
        return 1;
    }

    if (!xer::printf(u8"Base64: %@\n", *encoded).has_value()) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: bytes_basic
