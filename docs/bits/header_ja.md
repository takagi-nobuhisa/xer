# `<xer/ja.h>`

## Purpose

`<xer/ja.h>` is a convenience umbrella header for Japanese-specific XER facilities.

It includes:

```cpp
#include <xer/furigana.h>
#include <xer/kansuji.h>
#include <xer/mecab.h>
#include <xer/bits/ja_is.h>
#include <xer/bits/ja_to.h>
```

The APIs provided by these headers are placed under `xer::ja`.

---

## Namespace Policy

Japanese-specific APIs are collected under `xer::ja`.

This keeps the main `xer` namespace focused on language-neutral utilities, while allowing XER to provide deeper Japanese support before v1.0.0.

Examples:

```cpp
xer::ja::to_kansuji(2026, xer::ja::k十);
xer::ja::from_kansuji(u8"二千二十六");
xer::ja::to_furigana(u8"学校", u8"がっこう", xer::ja::ruby_html);
xer::ja::is_hiragana(U'あ');
xer::ja::is_katakana(U'ア');
xer::ja::is_kanji(U'漢');
xer::ja::to_hiragana(u8"カタカナ");
xer::ja::to_katakana(u8"ひらがな");
xer::ja::mecab_parse(u8"私は猫です。");
```

---

## Character Classification

`xer::ja::is_hiragana` checks whether a code point belongs to the Unicode Hiragana block.

`xer::ja::is_katakana` checks whether a code point is katakana, including fullwidth katakana, halfwidth katakana, Katakana Phonetic Extensions, and Kana Supplement/Extended blocks.

`xer::ja::is_kana` checks whether a code point is hiragana or katakana.

`xer::ja::is_kanji` checks whether a code point belongs to the common CJK unified ideograph or CJK compatibility ideograph ranges. Unicode shares many kanji/hanzi/hanja code points, so this function does not attempt to identify language-specific usage.

`xer::ja::is_japanese_punctuation` checks whether a code point is punctuation commonly used in Japanese text.

`xer::ja::is_japanese` checks whether a code point is kana, kanji, or Japanese punctuation.

```cpp
xer::ja::is_hiragana(U'あ'); // true
xer::ja::is_katakana(U'ア'); // true
xer::ja::is_kana(U'ｱ'); // true
xer::ja::is_kanji(U'漢'); // true
xer::ja::is_japanese_punctuation(U'。'); // true
xer::ja::is_japanese(U'日'); // true
```

These functions are defined in the internal implementation header `<xer/bits/ja_is.h>` and are available through `<xer/ja.h>`.

## Kana Conversion

`xer::ja::to_hiragana` converts fullwidth katakana in UTF-8 text to hiragana.

```cpp
const auto text = xer::ja::to_hiragana(u8"カタカナとヴ");
// text == u8"かたかなとゔ"
```

`xer::ja::to_katakana` converts hiragana in UTF-8 text to fullwidth katakana.

```cpp
const auto text = xer::ja::to_katakana(u8"ひらがなとゔ");
// text == u8"ヒラガナトヴ"
```

Both functions keep unrelated code points unchanged and return `xer::result<std::u8string>` so invalid UTF-8 input can be reported as `encoding_error`.

These functions are defined in the internal implementation header `<xer/bits/ja_to.h>` and are available through `<xer/ja.h>`.

## Notes

`<xer/ja.h>` does not define a separate implementation layer. It is an include-only convenience header.

Individual headers such as `<xer/kansuji.h>`, `<xer/furigana.h>`, and `<xer/mecab.h>` remain available when a caller wants to include only a smaller component. Kana conversion is currently available through `<xer/ja.h>` only.
