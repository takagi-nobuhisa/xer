# `<xer/ja.h>`

## Purpose

`<xer/ja.h>` is a convenience umbrella header for Japanese-specific XER facilities.

It includes:

```cpp
#include <xer/furigana.h>
#include <xer/kansuji.h>
#include <xer/mecab.h>
#include <xer/bits/ja_is.h>
#include <xer/bits/ja_kanji.h>
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
xer::ja::is_name_kanji(U'凜');
xer::ja::jis_kanji_level_of(U'亜');
xer::ja::to_hiragana(u8"カタカナ");
xer::ja::to_katakana(u8"ひらがな");
xer::ja::mecab_parse(u8"私は猫です。");
```

---

## Character Classification

`xer::ja::is_hiragana` checks whether a code point is practical hiragana. It accepts the Unicode Hiragana block and the prolonged sound mark `U+30FC`.

`xer::ja::is_katakana` checks whether a code point is practical katakana, including fullwidth katakana, halfwidth katakana, Katakana Phonetic Extensions, Kana Supplement/Extended blocks, and prolonged sound marks.

`xer::ja::is_kana` checks whether a code point is practical hiragana or practical katakana.

`xer::ja::is_kanji` checks whether a code point belongs to the common CJK unified ideograph or CJK compatibility ideograph ranges. Unicode shares many kanji/hanzi/hanja code points, so this function does not attempt to identify language-specific usage.

`xer::ja::is_japanese_punctuation` checks whether a code point is punctuation commonly used in Japanese text.

`xer::ja::is_japanese` checks whether a code point is kana, kanji, or Japanese punctuation.

`xer::ja::contains_hiragana`, `xer::ja::contains_katakana`, `xer::ja::contains_kana`, `xer::ja::contains_kanji`, and `xer::ja::contains_japanese` check whether a UTF-8 string contains at least one matching code point. Empty input returns `false`. Invalid UTF-8 input returns `encoding_error`.

`xer::ja::is_all_hiragana`, `xer::ja::is_all_katakana`, and `xer::ja::is_all_kana` check whether all code points in a UTF-8 string belong to practical hiragana, katakana, or kana text. Empty input returns `false`. Invalid UTF-8 input returns `encoding_error`.

The `contains_*` and `is_all_*` predicates use the same practical character sets as the corresponding code point predicates. The `is_all_*` kana predicates do not accept spaces, punctuation, kanji, or Latin letters.

```cpp
xer::ja::is_hiragana(U'あ'); // true
xer::ja::is_katakana(U'ア'); // true
xer::ja::is_kana(U'ｱ'); // true
xer::ja::is_kanji(U'漢'); // true
xer::ja::is_japanese_punctuation(U'。'); // true
xer::ja::is_japanese(U'日'); // true

xer::ja::contains_hiragana(u8"abcあ"); // true
xer::ja::contains_katakana(u8"abcア"); // true
xer::ja::contains_kana(u8"abcｱ"); // true
xer::ja::contains_kanji(u8"abc漢"); // true
xer::ja::contains_japanese(u8"hello日本語"); // true
xer::ja::contains_japanese(u8""); // false

xer::ja::is_all_hiragana(u8"こんにちはー"); // true
xer::ja::is_all_katakana(u8"コンニチハー"); // true
xer::ja::is_all_kana(u8"こんにちはコンニチハー"); // true
xer::ja::is_all_kana(u8""); // false
xer::ja::is_all_kana(u8"こんにちは。"); // false
```

These functions are defined in the internal implementation header `<xer/bits/ja_is.h>` and are available through `<xer/ja.h>`.


## Kanji Classification Tables

`xer::ja::name_kanji_class_of` returns the practical Japanese name-use class of a code point.

```cpp
namespace xer::ja {

enum class name_kanji_class {
    none = 0,
    name = 1,
    jouyou = 2,
    kyouiku = 3,
};

}
```

The class is ordered by containment: educational kanji are treated as a subset of common-use kanji, and common-use kanji are treated as usable in Japanese given names.

```cpp
xer::ja::is_name_kanji(U'凜'); // true
xer::ja::is_jouyou_kanji(U'鬱'); // true
xer::ja::is_kyouiku_kanji(U'日'); // true
```

`xer::ja::jis_kanji_level_of` returns the JIS kanji level.

```cpp
namespace xer::ja {

enum class jis_kanji_level {
    none = 0,
    level_1 = 1,
    level_2 = 2,
    level_3 = 3,
    level_4 = 4,
};

}
```

Convenience predicates are also provided.

```cpp
xer::ja::is_jis_level_1_kanji(U'亜'); // true
xer::ja::is_jis_level_2_kanji(U'弌'); // true
xer::ja::is_jis_level_3_kanji(U'俱'); // true
xer::ja::is_jis_level_4_kanji(U'㐆'); // true
```

The implementation stores compact one-byte flags for the basic CJK unified ideograph range and uses a sparse fallback table for supported code points outside that range. The low two bits store `name_kanji_class`, and bits 2..4 store `jis_kanji_level`.

These functions are defined in the internal implementation header `<xer/bits/ja_kanji.h>` and are available through `<xer/ja.h>`.

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
