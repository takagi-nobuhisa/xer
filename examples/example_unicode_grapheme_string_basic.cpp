#include <string_view>

#include <xer/stdio.h>
#include <xer/unicode.h>

// XER_TEST_FEATURES: icu

// XER_EXAMPLE_BEGIN: unicode_grapheme_string_basic
//
// This example extracts text by extended grapheme cluster count.
//
// Expected output:
// length=4
// left=ÁB
// right=👩‍💻C

auto main() -> int
{
    constexpr std::u8string_view text = u8"A\u0301B👩‍💻C";

    const auto length = xer::grapheme_length(text);
    const auto left = xer::grapheme_left(text, 2);
    const auto right = xer::grapheme_right(text, 2);

    if (!length.has_value() || !left.has_value() || !right.has_value()) {
        return 1;
    }

    if (!xer::printf(
        u8"length=%llu\n",
        static_cast<unsigned long long>(*length)).has_value()) {
        return 1;
    }
    if (!xer::printf(u8"left=%@\n", *left).has_value()) {
        return 1;
    }
    if (!xer::printf(u8"right=%@\n", *right).has_value()) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: unicode_grapheme_string_basic
