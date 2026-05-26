# `<xer/ja.h>`

## Purpose

`<xer/ja.h>` is a convenience umbrella header for Japanese-specific XER facilities.

It includes:

```cpp
#include <xer/furigana.h>
#include <xer/kansuji.h>
#include <xer/mecab.h>
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
xer::ja::to_hiragana(u8"カタカナ");
xer::ja::to_katakana(u8"ひらがな");
xer::ja::mecab_parse(u8"私は猫です。");
```

---

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
