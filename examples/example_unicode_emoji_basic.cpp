#include <string_view>

#include <xer/stdio.h>
#include <xer/unicode.h>

// XER_TEST_FEATURES: icu

// XER_EXAMPLE_BEGIN: unicode_emoji_basic
//
// This example checks whether a UTF-8 string is one emoji grapheme cluster.
//
// Expected output:
// emoji
// text
// emoji

auto main() -> int
{
    constexpr std::u8string_view samples[] = {
        u8"👩‍💻",
        u8"A",
        u8"🇯🇵",
    };

    for (const auto sample : samples) {
        const auto result = xer::is_emoji(sample);
        if (!result.has_value()) {
            return 1;
        }

        if (!xer::printf(*result ? u8"emoji\n" : u8"text\n").has_value()) {
            return 1;
        }
    }

    return 0;
}

// XER_EXAMPLE_END: unicode_emoji_basic
