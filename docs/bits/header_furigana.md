# `<xer/furigana.h>`

## Purpose

`<xer/furigana.h>` provides lightweight furigana formatting helpers.

The current facility is intentionally independent from morphological analysis.
It receives:

- base text
- a reading
- an output style

and produces a formatted UTF-8 string.

This makes it useful on its own, and also makes it a reusable building block for later MeCab-based automatic furigana generation.

---

## Main Role

The main role of `<xer/furigana.h>` is to turn an already known reading into a display-oriented text representation.

The current implementation supports:

- HTML ruby markup
- parenthesized furigana text

It does **not** attempt to determine the reading by itself.
Reading extraction belongs to higher-level Japanese text processing built on top of MeCab or other caller-provided logic.

---

## Main Entities

`<xer/furigana.h>` provides:

```cpp
enum class furigana_style : std::uint8_t {
    html,
    paren,
};

inline constexpr furigana_style ruby_html;
inline constexpr furigana_style ruby_paren;

[[nodiscard]]
auto to_furigana(
    std::u8string_view text,
    std::u8string_view reading,
    furigana_style style)
    -> std::u8string;
```

---

## `furigana_style`

```cpp
enum class furigana_style : std::uint8_t {
    html,
    paren,
};
```

`furigana_style` selects the output representation used by `to_furigana`.

Callers normally use the public selector constants:

```cpp
xer::ruby_html
xer::ruby_paren
```

---

## `ruby_html`

`ruby_html` produces HTML ruby markup.

```cpp
const auto result =
    xer::to_furigana(u8"学校", u8"がっこう", xer::ruby_html);
```

Result:

```html
<ruby>学校<rt>がっこう</rt></ruby>
```

The generated shape is:

```html
<ruby>BASE_TEXT<rt>READING</rt></ruby>
```

### HTML Escaping

For `ruby_html`, both the base text and the reading are HTML-escaped.

At the current stage, the internal escape helper handles:

| Character | Output |
|---|---|
| `&` | `&amp;` |
| `<` | `&lt;` |
| `>` | `&gt;` |
| `"` | `&quot;` |
| `'` | `&#39;` |

Example:

```cpp
const auto result =
    xer::to_furigana(u8"A&B", u8"えー&びー", xer::ruby_html);
```

Result:

```html
<ruby>A&amp;B<rt>えー&amp;びー</rt></ruby>
```

---

## `ruby_paren`

`ruby_paren` produces a simple parenthesized furigana representation.

```cpp
const auto result =
    xer::to_furigana(u8"学校", u8"がっこう", xer::ruby_paren);
```

Result:

```text
学校(がっこう)
```

The generated shape is:

```text
BASE_TEXT(READING)
```

No escaping or special transformation is applied in this style.

---

## `to_furigana`

```cpp
[[nodiscard]]
auto to_furigana(
    std::u8string_view text,
    std::u8string_view reading,
    furigana_style style)
    -> std::u8string;
```

### Purpose

`to_furigana` formats a base text and its reading according to the selected furigana style.

### Examples

```cpp
xer::to_furigana(u8"漢字", u8"かんじ", xer::ruby_html);
// <ruby>漢字<rt>かんじ</rt></ruby>
```

```cpp
xer::to_furigana(u8"漢字", u8"かんじ", xer::ruby_paren);
// 漢字(かんじ)
```

### Return Type

`to_furigana` returns `std::u8string`.

It is a formatting helper and does not perform reading analysis or external-process execution.
It therefore does not return `xer::result`.

---

## Empty Input

Empty base text and empty reading are accepted.

```cpp
xer::to_furigana(u8"", u8"", xer::ruby_html);
// <ruby><rt></rt></ruby>
```

```cpp
xer::to_furigana(u8"", u8"", xer::ruby_paren);
// ()
```

---

## Internal HTML Escape Helper

The HTML escaping used by `ruby_html` is implemented through the internal helper header:

```text
xer/bits/escape_html.h
```

This helper is intentionally kept small and internal.
It is expected to be reused later by higher-level HTML-oriented facilities such as `htmlspecialchars`-style APIs, without forcing `furigana.h` to depend on a heavier public HTML header.

---

## Relationship to Future MeCab-Based Furigana

`to_furigana` is intentionally independent from MeCab.

Later MeCab-based helpers can:

1. analyze text
2. determine candidate readings
3. call `to_furigana` to produce HTML ruby or parenthesized output

This separation keeps formatting logic reusable and keeps automatic reading logic out of the low-level formatter.

---

## Relationship to Other Headers

`<xer/furigana.h>` is related to:

- `<xer/mecab.h>` as a future higher-level reading source
- `<xer/string.h>` in the broader Japanese text-processing area
- `policy_mecab.md`
