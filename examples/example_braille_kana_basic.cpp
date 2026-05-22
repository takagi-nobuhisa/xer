// XER_EXAMPLE_BEGIN: braille_kana_basic
//
// This example converts single Japanese kana characters to braille cells.
// Some kana, such as voiced and semi-voiced kana, are converted to multiple
// braille cells.
//
// Expected output:
// basic kana: ⠡⠣⠩⠫⠪
// voiced kana: ⠐⠡⠐⠣⠐⠩⠐⠫⠐⠪
// semi-voiced kana: ⠠⠥⠠⠧⠠⠭⠠⠯⠠⠮
// marks: ⠂⠒⠴
// katakana: ⠡⠕⠡⠅

#include <array>
#include <string>
#include <string_view>

#include <xer/braille.h>
#include <xer/stdio.h>

namespace {

auto append_braille(
    std::u8string& output,
    const xer::result<std::u8string_view>& value) -> bool
{
    if (!value.has_value()) {
        return false;
    }

    output += *value;
    return true;
}

auto append_kana(std::u8string& output, char32_t c) -> bool
{
    return append_braille(output, xer::braille::kana_to_braille(c));
}

template <std::size_t N>
auto convert_kana(const std::array<char32_t, N>& input) -> xer::result<std::u8string>
{
    std::u8string output;

    for (const auto c : input) {
        const auto value = xer::braille::kana_to_braille(c);
        if (!value.has_value()) {
            return std::unexpected(value.error());
        }
        output += *value;
    }

    return output;
}

auto print_label_and_text(std::u8string_view label, std::u8string_view text) -> bool
{
    return xer::printf(u8"%@%@\n", label, text).has_value();
}

} // namespace

auto main() -> int
{
    const auto basic_kana = convert_kana(std::array{U'か', U'き', U'く', U'け', U'こ'});
    if (!basic_kana.has_value()) {
        return 1;
    }

    const auto voiced_kana = convert_kana(std::array{U'が', U'ぎ', U'ぐ', U'げ', U'ご'});
    if (!voiced_kana.has_value()) {
        return 1;
    }

    const auto semi_voiced_kana = convert_kana(std::array{U'ぱ', U'ぴ', U'ぷ', U'ぺ', U'ぽ'});
    if (!semi_voiced_kana.has_value()) {
        return 1;
    }

    std::u8string marks;
    for (const auto c : std::array{U'っ', U'ー', U'ん'}) {
        if (!append_kana(marks, c)) {
            return 1;
        }
    }

    const auto katakana = convert_kana(std::array{U'カ', U'タ', U'カ', U'ナ'});
    if (!katakana.has_value()) {
        return 1;
    }

    if (!print_label_and_text(u8"basic kana: ", *basic_kana)) {
        return 1;
    }
    if (!print_label_and_text(u8"voiced kana: ", *voiced_kana)) {
        return 1;
    }
    if (!print_label_and_text(u8"semi-voiced kana: ", *semi_voiced_kana)) {
        return 1;
    }
    if (!print_label_and_text(u8"marks: ", marks)) {
        return 1;
    }
    if (!print_label_and_text(u8"katakana: ", *katakana)) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: braille_kana_basic
