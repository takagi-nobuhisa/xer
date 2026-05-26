# `<xer/ja.h>`

## Purpose

`<xer/ja.h>` is a convenience umbrella header for Japanese-specific XER facilities.

It includes:

```cpp
#include <xer/braille.h>
#include <xer/furigana.h>
#include <xer/kansuji.h>
#include <xer/mecab.h>
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
xer::ja::mecab_parse(u8"私は猫です。");
xer::ja::kana_text_to_braille(u8"てんじ");
```

---

## Notes

`<xer/ja.h>` does not define a separate implementation layer. It is an include-only convenience header.

Individual headers such as `<xer/braille.h>`, `<xer/kansuji.h>`, `<xer/furigana.h>`, and `<xer/mecab.h>` remain available when a caller wants to include only a smaller component.
