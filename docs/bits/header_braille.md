# `<xer/braille.h>`

## Purpose

`<xer/braille.h>` provides low-level braille-related building blocks for XER.

At the current stage, this header provides:

- common braille sign constants as UTF-8 string views
- one-character conversion helpers for English letters, digits, English braille punctuation, Japanese kana, and Japanese punctuation
- kana-text conversion that handles ordinary kana, yoon, extended foreign-sound kana sequences, Japanese punctuation, and ASCII spaces

It does not perform complete Japanese braille translation. In particular, it does not decide readings from kanji, does not perform full braille wakachi-gaki by itself, and does not automatically switch numeric, alphabetic, or information-processing braille modes.

This keeps the first braille layer small and reusable: callers can combine the provided constants and conversion helpers while higher-level conversion APIs handle analysis and mode control.

---

## Main Role

The main role of `<xer/braille.h>` is to expose reusable low-level parts for constructing braille strings.

The current implementation provides:

- Japanese braille numeric and alphabetic indicators
- Japanese braille capital indicators
- information-processing braille indicators
- one-character English alphabet conversion
- one-character digit conversion under an already active numeric mode
- one-character alphanumeric dispatch
- one-character English braille punctuation conversion
- one-character Japanese kana conversion
- one-character Japanese punctuation conversion
- kana-text conversion for kana strings, yoon sequences, extended foreign-sound sequences, punctuation, and spaces

The constants are represented as:

```cpp
std::u8string_view
```

The one-character conversion functions return:

```cpp
xer::result<std::u8string_view>
```

This avoids allocation for the returned braille fragments and allows unsupported input characters to be reported explicitly.

The kana-text conversion function returns:

```cpp
xer::result<std::u8string>
```

It decodes UTF-8 input, combines supported multi-kana sequences, and returns an owned braille string.

---

## Main Entities

`<xer/braille.h>` currently provides the following constants and functions:

```cpp
namespace xer::braille {

inline constexpr std::u8string_view numeric_indicator;
inline constexpr std::u8string_view alphabetic_indicator;
inline constexpr std::u8string_view capital_indicator;
inline constexpr std::u8string_view double_capital_indicator;

[[nodiscard]] constexpr auto alpha_to_braille(char32_t c)
    -> result<std::u8string_view>;

[[nodiscard]] constexpr auto digit_to_braille(char32_t c)
    -> result<std::u8string_view>;

[[nodiscard]] constexpr auto alnum_to_braille(char32_t c)
    -> result<std::u8string_view>;

[[nodiscard]] constexpr auto punct_to_braille(char32_t c)
    -> result<std::u8string_view>;

[[nodiscard]] constexpr auto japanese_punct_to_braille(char32_t c)
    -> result<std::u8string_view>;

[[nodiscard]] constexpr auto kana_to_braille(char32_t c)
    -> result<std::u8string_view>;

[[nodiscard]] auto kana_text_to_braille(std::u8string_view text)
    -> result<std::u8string>;

namespace information_processing {

inline constexpr std::u8string_view lowercase_indicator;
inline constexpr std::u8string_view uppercase_indicator;
inline constexpr std::u8string_view single_uppercase_indicator;
inline constexpr std::u8string_view double_uppercase_indicator;
inline constexpr std::u8string_view numeric_indicator;

} // namespace information_processing

} // namespace xer::braille
```

---

## Japanese Braille Indicators

### `numeric_indicator`

```cpp
inline constexpr std::u8string_view numeric_indicator = u8"⠼";
```

`numeric_indicator` is the Japanese braille numeric indicator.

It is the Unicode braille pattern for dots 3-4-5-6.

Example:

```cpp
std::u8string text;
text += xer::braille::numeric_indicator;
text += u8"⠁⠃⠉";
```

---

### `alphabetic_indicator`

```cpp
inline constexpr std::u8string_view alphabetic_indicator = u8"⠰";
```

`alphabetic_indicator` is the Japanese braille alphabetic indicator.

It is the Unicode braille pattern for dots 5-6.

Example:

```cpp
std::u8string text;
text += xer::braille::alphabetic_indicator;
text += u8"⠁⠃⠉";
```

---

### `capital_indicator`

```cpp
inline constexpr std::u8string_view capital_indicator = u8"⠠";
```

`capital_indicator` is the Japanese braille capital indicator.

It is the Unicode braille pattern for dot 6.

Example:

```cpp
std::u8string text;
text += xer::braille::capital_indicator;
text += u8"⠁";
```

---

### `double_capital_indicator`

```cpp
inline constexpr std::u8string_view double_capital_indicator = u8"⠠⠠";
```

`double_capital_indicator` represents two consecutive capital indicators.

It is useful when constructing a braille string that needs a double-capital prefix.

Example:

```cpp
std::u8string text;
text += xer::braille::double_capital_indicator;
text += u8"⠁⠃⠉";
```

---

## Information-Processing Braille Indicators

Information-processing braille indicators are placed under:

```cpp
namespace xer::braille::information_processing
```

This keeps them separate from the ordinary Japanese braille indicators while still making them available through the same public header.

### `lowercase_indicator`

```cpp
inline constexpr std::u8string_view lowercase_indicator = u8"⠰";
```

`lowercase_indicator` is the information-processing braille lowercase indicator.

It is the Unicode braille pattern for dots 5-6.

---

### `uppercase_indicator`

```cpp
inline constexpr std::u8string_view uppercase_indicator = u8"⠠";
```

`uppercase_indicator` is the information-processing braille uppercase indicator.

It is the Unicode braille pattern for dot 6.

---

### `single_uppercase_indicator`

```cpp
inline constexpr std::u8string_view single_uppercase_indicator = u8"⠠";
```

`single_uppercase_indicator` represents the single-uppercase indicator.

At the current stage, it has the same cell as `uppercase_indicator`.

---

### `double_uppercase_indicator`

```cpp
inline constexpr std::u8string_view double_uppercase_indicator = u8"⠠⠠";
```

`double_uppercase_indicator` represents two consecutive uppercase indicators.

---

### `numeric_indicator`

```cpp
inline constexpr std::u8string_view numeric_indicator = u8"⠼";
```

`numeric_indicator` is the information-processing braille numeric indicator.

It is the Unicode braille pattern for dots 3-4-5-6.

---

## One-Character Conversion Helpers

The one-character conversion helpers convert a single source character to the corresponding braille cell or short braille fragment.

They do not emit mode indicators. Callers are responsible for adding `numeric_indicator`, `alphabetic_indicator`, `capital_indicator`, or other indicators where required.

All conversion helpers return `xer::result<std::u8string_view>`.

If the input character is not supported by the selected helper, the function returns `error_t::invalid_argument`.

---

## `alpha_to_braille`

```cpp
[[nodiscard]] constexpr auto alpha_to_braille(char32_t c)
    -> result<std::u8string_view>;
```

`alpha_to_braille` converts one ASCII alphabetic character to an English braille alphabet cell.

The accepted input range is:

- `A` to `Z`
- `a` to `z`

Uppercase and lowercase letters map to the same braille cell. The function does not emit a capital indicator.

| Input | Output |
|---|---|
| `a` / `A` | `⠁` |
| `b` / `B` | `⠃` |
| `c` / `C` | `⠉` |
| `d` / `D` | `⠙` |
| `e` / `E` | `⠑` |
| `f` / `F` | `⠋` |
| `g` / `G` | `⠛` |
| `h` / `H` | `⠓` |
| `i` / `I` | `⠊` |
| `j` / `J` | `⠚` |
| `k` / `K` | `⠅` |
| `l` / `L` | `⠇` |
| `m` / `M` | `⠍` |
| `n` / `N` | `⠝` |
| `o` / `O` | `⠕` |
| `p` / `P` | `⠏` |
| `q` / `Q` | `⠟` |
| `r` / `R` | `⠗` |
| `s` / `S` | `⠎` |
| `t` / `T` | `⠞` |
| `u` / `U` | `⠥` |
| `v` / `V` | `⠧` |
| `w` / `W` | `⠺` |
| `x` / `X` | `⠭` |
| `y` / `Y` | `⠽` |
| `z` / `Z` | `⠵` |

Example:

```cpp
std::u8string text{xer::braille::alphabetic_indicator};
text += *xer::braille::alpha_to_braille(U'x');
text += *xer::braille::alpha_to_braille(U'e');
text += *xer::braille::alpha_to_braille(U'r');
```

---

## `digit_to_braille`

```cpp
[[nodiscard]] constexpr auto digit_to_braille(char32_t c)
    -> result<std::u8string_view>;
```

`digit_to_braille` converts one ASCII digit to the corresponding numeric braille cell.

The accepted input range is:

- `0` to `9`

This function assumes that the caller has already emitted a numeric indicator when required.

Digits `1` to `9` map to the same cells as `a` to `i`, and digit `0` maps to the same cell as `j`.

| Input | Output |
|---|---|
| `1` | `⠁` |
| `2` | `⠃` |
| `3` | `⠉` |
| `4` | `⠙` |
| `5` | `⠑` |
| `6` | `⠋` |
| `7` | `⠛` |
| `8` | `⠓` |
| `9` | `⠊` |
| `0` | `⠚` |

Example:

```cpp
std::u8string text{xer::braille::numeric_indicator};
text += *xer::braille::digit_to_braille(U'1');
text += *xer::braille::digit_to_braille(U'2');
text += *xer::braille::digit_to_braille(U'3');
```

---

## `alnum_to_braille`

```cpp
[[nodiscard]] constexpr auto alnum_to_braille(char32_t c)
    -> result<std::u8string_view>;
```

`alnum_to_braille` converts one ASCII alphanumeric character to a braille cell.

It is a small dispatcher for `alpha_to_braille` and `digit_to_braille`.

The accepted input ranges are:

- `A` to `Z`
- `a` to `z`
- `0` to `9`

This function does not emit alphabetic, capital, numeric, or other mode indicators.

Example:

```cpp
std::u8string text;
text += *xer::braille::alnum_to_braille(U'x');
text += *xer::braille::alnum_to_braille(U'e');
text += *xer::braille::alnum_to_braille(U'r');
text += *xer::braille::alnum_to_braille(U'1');
text += *xer::braille::alnum_to_braille(U'2');
text += *xer::braille::alnum_to_braille(U'3');
```

---

## `punct_to_braille`

```cpp
[[nodiscard]] constexpr auto punct_to_braille(char32_t c)
    -> result<std::u8string_view>;
```

`punct_to_braille` converts one English braille punctuation character to a braille cell.

The current implementation targets basic Grade 1 English braille punctuation marks.
It does not implement information-processing braille punctuation, Japanese punctuation, or context-sensitive punctuation rules.

Supported characters are:

| Input | Output | Note |
|---|---|---|
| `,` | `⠂` | comma |
| `;` | `⠆` | semicolon |
| `:` | `⠒` | colon |
| `.` | `⠲` | period |
| `!` | `⠖` | exclamation mark |
| `(` | `⠶` | parenthesis |
| `)` | `⠶` | parenthesis |
| `?` | `⠦` | question mark |
| `“` | `⠦` | opening quotation mark |
| `*` | `⠔` | asterisk |
| `”` | `⠴` | closing quotation mark |
| `'` | `⠄` | apostrophe |
| `-` | `⠤` | hyphen |
| `‐` | `⠤` | hyphen |

ASCII double quotation mark `"` is not supported because a one-character conversion function cannot determine whether it is an opening or closing quotation mark.

---

## `japanese_punct_to_braille`

```cpp
[[nodiscard]] constexpr auto japanese_punct_to_braille(char32_t c)
    -> result<std::u8string_view>;
```

`japanese_punct_to_braille` converts one Japanese punctuation mark to Japanese braille cells.

This function is separate from `punct_to_braille`. `punct_to_braille` targets basic English braille punctuation, while `japanese_punct_to_braille` targets Japanese kana-braille punctuation used in Japanese text.

Supported characters are:

| Input | Output | Note |
|---|---|---|
| `。` | `⠲` | Japanese full stop |
| `、` | `⠰` | Japanese comma |
| `？` | `⠢` | Japanese question mark |
| `?` | `⠢` | ASCII question mark treated as Japanese punctuation |
| `！` | `⠖` | Japanese exclamation mark |
| `!` | `⠖` | ASCII exclamation mark treated as Japanese punctuation |
| `・` | `⠂` | middle dot |
| `「` | `⠤` | corner bracket |
| `」` | `⠤` | corner bracket |
| `『` | `⠰⠤` | double corner bracket |
| `』` | `⠰⠤` | double corner bracket |
| `（` | `⠶` | parenthesis |
| `）` | `⠶` | parenthesis |
| `(` | `⠶` | ASCII parenthesis treated as Japanese punctuation |
| `)` | `⠶` | ASCII parenthesis treated as Japanese punctuation |
| `…` | `⠄⠄⠄` | ellipsis |
| `‥` | `⠄⠄` | two-dot leader |

`japanese_punct_to_braille` does not insert spaces around punctuation. Spacing is handled by higher-level text conversion functions such as `mecab_braille_wakati`.

---

## `kana_to_braille`

```cpp
[[nodiscard]] constexpr auto kana_to_braille(char32_t c)
    -> result<std::u8string_view>;
```

`kana_to_braille` converts one Japanese kana character to Japanese braille cells.

It accepts both hiragana and katakana for the same syllable.

The function handles:

- basic kana
- `ゐ` / `ヰ`
- `ゑ` / `ヱ`
- `ん` / `ン`
- prolonged sound mark `ー`
- sokuon `っ` / `ッ`
- voiced kana
- semi-voiced kana
- `ゔ` / `ヴ`

Some kana characters map to multiple braille cells. For example, voiced and semi-voiced kana are represented by a sign followed by the base kana cell.

This function converts only one input character. It does not combine multiple input characters, so sequences such as `きゃ`, `シェ`, or `ティ` are handled by `kana_text_to_braille`, not by `kana_to_braille`.

---

## `kana_text_to_braille`

```cpp
[[nodiscard]] auto kana_text_to_braille(std::u8string_view text)
    -> result<std::u8string>;
```

`kana_text_to_braille` converts UTF-8 kana text to Japanese braille text.

It performs the following low-level text conversion:

- decodes UTF-8 input
- preserves ASCII spaces as wakachi-gaki separators
- converts Japanese punctuation through `japanese_punct_to_braille`
- converts ordinary kana through `kana_to_braille`
- combines supported small-kana sequences before conversion

The function returns `error_t::encoding_error` for invalid UTF-8 input and `error_t::invalid_argument` for unsupported characters or unsupported small-kana combinations.

### Supported multi-kana sequences

`kana_text_to_braille` supports ordinary yoon sequences and several extended foreign-sound kana sequences.

Supported base-plus-small-kana groups include:

- `きゃ` / `きゅ` / `きょ`
- `しゃ` / `しゅ` / `しょ` / `しぇ`
- `ちゃ` / `ちゅ` / `ちょ` / `ちぇ`
- `にゃ` / `にゅ` / `にょ`
- `ひゃ` / `ひゅ` / `ひょ` / `ひぇ`
- `みゃ` / `みゅ` / `みょ`
- `りゃ` / `りゅ` / `りょ`
- `ぎゃ` / `ぎゅ` / `ぎょ`
- `じゃ` / `じゅ` / `じょ` / `じぇ`
- `ぢゃ` / `ぢゅ` / `ぢょ`
- `びゃ` / `びゅ` / `びょ`
- `ぴゃ` / `ぴゅ` / `ぴょ`
- `いぇ`
- `うぃ` / `うぇ` / `うぉ`
- `くぁ` / `くぃ` / `くぇ` / `くぉ`
- `ぐぁ` / `ぐぃ` / `ぐぇ` / `ぐぉ`
- `つぁ` / `つぃ` / `つぇ` / `つぉ`
- `てぃ` / `てゅ`
- `でぃ` / `でゅ`
- `とぅ`
- `どぅ`
- `ふぁ` / `ふぃ` / `ふぇ` / `ふぉ` / `ふゅ` / `ふょ`
- `ゔぁ` / `ゔぃ` / `ゔぇ` / `ゔぉ` / `ゔゅ` / `ゔょ`

The same combinations are accepted in katakana form, such as `キャ`, `シェ`, `ティ`, `ファ`, and `ヴォ`.

Unsupported small-kana combinations are rejected rather than guessed.

### Scope

`kana_text_to_braille` is still a low-level kana conversion function.

It does not:

- determine readings from kanji
- correct particles such as `は`, `へ`, or `を`
- perform full Japanese braille wakachi-gaki
- automatically emit numeric or alphabetic indicators
- automatically switch between ordinary Japanese braille and information-processing braille

Use `mecab_braille_wakati` when MeCab-derived readings and approximate braille-oriented wakachi-gaki are needed.

---

## Error Handling

The braille conversion helpers use `xer::result` to report unsupported input explicitly.

Common errors are:

| Error | Meaning |
|---|---|
| `error_t::invalid_argument` | The input character or character sequence is not supported by the selected conversion helper. |
| `error_t::encoding_error` | `kana_text_to_braille` received invalid UTF-8 input. |

One-character helpers do not allocate. `kana_text_to_braille` returns an owned `std::u8string` because it may combine input characters and append multiple output fragments.

---

## Header Dependencies

`<xer/braille.h>` is the public header for braille-related APIs.

Implementation details are split under `xer/bits/`, including:

```cpp
#include <xer/bits/braille_symbols.h>
#include <xer/bits/braille_chars.h>
```

User code should include:

```cpp
#include <xer/braille.h>
```

rather than including the `xer/bits/` headers directly.
