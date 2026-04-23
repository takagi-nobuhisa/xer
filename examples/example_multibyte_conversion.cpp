// XER_EXAMPLE_BEGIN: multibyte_conversion_basic
//
// This example shows basic usage of xer::mbtotc,
// xer::tctomb, xer::mbstotcs, and xer::tcstombs.
//
// Expected output:
// mbtotc   = 3 U+3042
// tctomb   = 3 あ
// mbstotcs = 2 U+3042 U+3044
// tcstombs = 6 あい

#include <array>

#include <xer/stdio.h>
#include <xer/stdlib.h>

auto main() -> int
{
    constexpr char8_t one_char_text[] = u8"あ";
    constexpr char8_t two_chars_text[] = u8"あい";
    constexpr char32_t two_chars_tc[] = U"あい";

    char32_t first = U'\0';
    const auto mbtotc_result =
        xer::mbtotc(&first, one_char_text, sizeof(one_char_text));
    if (!mbtotc_result) {
        return 1;
    }

    if (!xer::printf(
            u8"mbtotc   = %llu U+%04X\n",
            static_cast<unsigned long long>(*mbtotc_result),
            static_cast<unsigned int>(first))) {
        return 1;
    }

    std::array<char8_t, 8> encoded {};
    const auto tctomb_result =
        xer::tctomb(encoded.data(), encoded.size(), U'あ');
    if (!tctomb_result) {
        return 1;
    }

    encoded[*tctomb_result] = u8'\0';

    if (!xer::printf(
            u8"tctomb   = %llu ",
            static_cast<unsigned long long>(*tctomb_result))) {
        return 1;
    }
    if (!xer::puts(encoded.data())) {
        return 1;
    }

    std::array<char32_t, 8> decoded {};
    const auto mbstotcs_result =
        xer::mbstotcs(decoded.data(), two_chars_text, decoded.size());
    if (!mbstotcs_result) {
        return 1;
    }

    if (!xer::printf(
            u8"mbstotcs = %llu U+%04X U+%04X\n",
            static_cast<unsigned long long>(*mbstotcs_result),
            static_cast<unsigned int>(decoded[0]),
            static_cast<unsigned int>(decoded[1]))) {
        return 1;
    }

    std::array<char8_t, 16> rebuilt {};
    const auto tcstombs_result =
        xer::tcstombs(rebuilt.data(), rebuilt.size(), two_chars_tc);
    if (!tcstombs_result) {
        return 1;
    }

    rebuilt[*tcstombs_result] = u8'\0';

    if (!xer::printf(
            u8"tcstombs = %llu ",
            static_cast<unsigned long long>(*tcstombs_result))) {
        return 1;
    }
    if (!xer::puts(rebuilt.data())) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: multibyte_conversion_basic
