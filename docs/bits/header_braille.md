# `<xer/braille.h>`

## Purpose

`<xer/braille.h>` provides low-level braille-related building blocks for XER.

At the current stage, this header provides:

- common braille sign constants as UTF-8 string views
- simple one-character conversion helpers for English letters, digits, and English braille punctuation

It does not yet perform full text-to-braille conversion, Japanese braille translation, English contraction handling, automatic mode switching, wakachi-gaki, or context-sensitive punctuation analysis.

This keeps the first braille layer small and stable: callers can combine the provided constants and conversion helpers to build braille strings while higher-level conversion APIs are added later.

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

The constants are represented as:

```cpp
std::u8string_view
```

The one-character conversion functions return:

```cpp
xer::result<std::u8string_view>
```

This avoids allocation for the returned braille fragments and allows unsupported input characters to be reported explicitly.

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
| `-` | `⠤` | hyphen-minus |
| `‐` | `⠤` | hyphen |

ASCII double quotation mark `"` is not supported because a single character is not enough to distinguish an opening quotation mark from a closing quotation mark.
Use `U+201C` or `U+201D` when that distinction is required.

Example:

```cpp
std::u8string text;
text += *xer::braille::punct_to_braille(U',');
text += *xer::braille::punct_to_braille(U';');
text += *xer::braille::punct_to_braille(U':');
text += *xer::braille::punct_to_braille(U'.');
```

---

## Error Handling

All one-character conversion helpers return `xer::result<std::u8string_view>`.

Unsupported input characters produce:

```cpp
error_t::invalid_argument
```

Examples of unsupported input include:

- non-ASCII letters such as `é`
- kana and kanji
- spaces
- unsupported symbols such as ASCII double quotation mark `"`
- characters outside the selected helper's category

For example, `alpha_to_braille(U'1')` fails because `1` is not an alphabetic character.
Use `digit_to_braille` or `alnum_to_braille` for digits.

---

## Design Notes

### Conversion Helpers Do Not Manage Modes

The conversion helpers are intentionally low-level.
They only map one input character to one braille cell or short braille fragment.

They do not add:

- numeric indicators
- alphabetic indicators
- capital indicators
- information-processing indicators
- spaces
- word boundaries
- contraction marks

This keeps the functions predictable and usable as building blocks for later high-level conversion APIs.

### Constants Are UTF-8 String Views

The constants are `std::u8string_view`, not `char32_t` or a dedicated cell type.

This is intentional for the first layer because these constants represent output text fragments rather than abstract braille cells.
They can be appended directly to `std::u8string` without conversion.

### Conversion Results Are UTF-8 String Views

The conversion helpers also return `std::u8string_view` through `xer::result`.

At the current stage, each supported character maps to one braille cell. Returning a string view instead of a scalar value leaves room for later helpers that may need short fixed braille fragments without changing the basic style of the API.

### Relation to `isctype`

`<xer/ctype.h>` provides `isctype(c, ctype_id::braille)` for checking whether a Unicode scalar value belongs to the Unicode Braille Patterns block.

`<xer/braille.h>` is different: it provides actual UTF-8 braille sign constants and conversion helpers that can be used when constructing braille strings.

---

## Example

```cpp
#include <array>
#include <string>
#include <string_view>

#include <xer/braille.h>

std::u8string text{xer::braille::alphabetic_indicator};

for (const auto c : std::array{U'x', U'e', U'r'}) {
    auto cell = xer::braille::alpha_to_braille(c);
    if (!cell.has_value()) {
        return;
    }
    text += *cell;
}
```
