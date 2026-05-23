// XER_EXAMPLE_BEGIN: braille_japanese_text_basic
//
// This example converts kana text that contains Japanese punctuation and
// special kana sequences to Japanese braille text.
//
// Expected output:
// sentence: ⠈⠡⠑⠩⠕⠒⠰⠐⠕⠜⠲
// foreign sounds: ⠲⠮ ⠢⠥ ⠢⠧ ⠢⠯ ⠢⠮
// punctuation: ⠰⠤⠇⠨⠴⠐⠡⠒⠰⠴⠢⠖

#include <string>
#include <string_view>

#include <xer/braille.h>
#include <xer/stdio.h>

namespace {

auto print_result(
    std::u8string_view label,
    const xer::result<std::u8string>& value) -> bool
{
    if (!value.has_value()) {
        return false;
    }

    return xer::printf(u8"%@%@\n", label, *value).has_value();
}

} // namespace

auto main() -> int
{
    const auto sentence = xer::braille::kana_text_to_braille(u8"キャラクター、だよ。");
    const auto foreign_sounds = xer::braille::kana_text_to_braille(u8"ヴォ ファ フィ フェ フォ");
    const auto punctuation = xer::braille::kana_text_to_braille(u8"「にほんごてんじ」？！");

    if (!print_result(u8"sentence: ", sentence)) {
        return 1;
    }
    if (!print_result(u8"foreign sounds: ", foreign_sounds)) {
        return 1;
    }
    if (!print_result(u8"punctuation: ", punctuation)) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: braille_japanese_text_basic
