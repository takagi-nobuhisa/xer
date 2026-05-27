# Policy for Kansuji Handling

## Overview

xer provides Kansuji handling as part of its practical Japanese text-processing facilities.

The initial goal is to support non-negative integer conversion between ordinary numeric values and commonly used Japanese numeric notations.
This facility should prioritize:

- practical notations seen in real Japanese text
- clear and memorable format selection
- permissive parsing where natural variations are common
- canonical generation with stable output rules

Negative numbers and fractional numbers are outside the initial scope.

---

## Basic Scope

The initial Kansuji facility targets:

- conversion from non-negative integer values to Japanese numeric text
- parsing of Japanese numeric text into non-negative integer values
- support for several representative notation styles
- limited acceptance of practical Daiji variants during parsing

The initial facility does **not** target:

- negative values
- fractional values
- arbitrary free-form Japanese number expressions
- full historical or legal orthography coverage
- exhaustive support for all rare Daiji numerals

---

## Naming Policy

The generation function should be named:

```cpp
xer::ja::to_kansuji(...)
```

The parser function should be named:

```cpp
xer::ja::from_kansuji(...)
```

The word `kansuji` is used directly rather than expanding it into a longer English phrase such as `kanji_number`.

The format selector should use short, immediately recognizable identifiers.

```cpp
xer::ja::k10
xer::ja::k十
xer::ja::k一〇
xer::ja::k拾
```

These selectors are intentionally based on how the number `10` is written in each style.

---

## Public Function Shapes

```cpp
auto to_kansuji(std::uint64_t value, kansuji_style style)
    -> std::u8string;

auto from_kansuji(std::u8string_view text)
    -> xer::result<std::uint64_t>;
```

`from_kansuji` reports ordinary parse failure through `xer::result`.

- malformed text: `error_t::invalid_argument`
- values exceeding `std::uint64_t`: `error_t::overflow_error`

---

## Output Styles

### `xer::ja::k10`

This style uses Arabic numerals inside four-digit Japanese large-number groups.

Example:

```text
1234億5678万9012
```

For the value `123456789012`, generation with `xer::ja::k10` should produce:

```text
1234億5678万9012
```

---

### `xer::ja::k十`

This style uses ordinary positional Kansuji with small units such as `十`, `百`, and `千`.

Example:

```text
千二百三十四億五千六百七十八万九千十二
```

For the value `123456789012`, generation with `xer::ja::k十` should produce:

```text
千二百三十四億五千六百七十八万九千十二
```

Generation omits the leading `一` before `十`, `百`, and `千`.

Examples:

| Value | Generated Text |
|---:|---|
| 10 | `十` |
| 100 | `百` |
| 1000 | `千` |
| 110 | `百十` |

However, parsing may accept forms such as `一十`, `一百`, and `一千`.

---

### `xer::ja::k一〇`

This style writes each decimal digit independently with Kansuji digits and uses `〇` for zero.

Example:

```text
一二三四億五六七八万九〇一二
```

For the value `123456789012`, generation with `xer::ja::k一〇` should produce:

```text
一二三四億五六七八万九〇一二
```

This style is useful for vertical writing, years, and cases where digits are conventionally read one by one.

Examples:

| Value | Generated Text |
|---:|---|
| 10 | `一〇` |
| 2026 | `二〇二六` |
| 9012 | `九〇一二` |

---

### `xer::ja::k拾`

This style generates a practical Daiji positional notation.

Example:

```text
壱千弐百参拾四億五千六百七拾八万九千壱拾弐
```

The intended generated Daiji set is practical rather than exhaustive.
The initial output policy uses:

- `壱`
- `弐`
- `参`
- `拾`

while ordinary forms remain for the other digits and for larger units unless a later policy revision says otherwise.

Unlike `xer::ja::k十`, generation with `xer::ja::k拾` does **not** omit `壱` before small units.

Examples:

| Value | Generated Text |
|---:|---|
| 10 | `壱拾` |
| 100 | `壱百` |
| 110 | `壱百壱拾` |
| 1000 | `壱千` |
| 10000 | `壱万` |

This non-omission rule reflects common Daiji usage for clarity and tamper resistance.

---

## Zero Output

Zero should be generated according to the selected output style.

| Style | Generated Text |
|---|---|
| `xer::ja::k10` | `0` |
| `xer::ja::k十` | `零` |
| `xer::ja::k一〇` | `〇` |
| `xer::ja::k拾` | `零` |

Parsing accepts:

```text
0
零
〇
```

as whole-value zero forms.

---

## Large-Unit Structure

Large units divide the number into four-digit groups.

The initial implementation supports:

```text
万
億
兆
京
```

The notation should naturally support shortened groups.

The following examples are valid parsing targets and represent the same structural idea across the supported styles:

```text
12億34万5
一二億三四万五
十二億三十四万五
```

These should be interpreted as:

```text
12億0034万0005
```

Conceptually, omitted leading zeros within a large-unit group are allowed.

---

## Rejected Zero-Padded Group Forms

Explicit zero-padded groups are not part of the initial accepted notation.

The following forms are unnecessary and should not be treated as supported examples:

```text
1億0001万0001
一億〇〇〇一万〇〇〇一
```

---

## Small-Unit Rules

### Parsing

For ordinary positional Kansuji and Daiji-normalized inputs:

- `十` and `一十` are both accepted
- `百` and `一百` are both accepted
- `千` and `一千` are both accepted

This permissiveness is for parsing only.

### Generation

For `xer::ja::k十`:

- `十`, `百`, and `千` omit leading `一`

For `xer::ja::k拾`:

- `壱拾`, `壱百`, and `壱千` retain leading `壱`

---

## Large-Unit Prefix Requirement

Large units such as `万`, `億`, `兆`, and `京` must not appear without a preceding numeric group.

Examples:

| Text | Accepted |
|---|:---:|
| `一万` | yes |
| `万` | no |
| `一億` | yes |
| `億` | no |
| `一京` | yes |
| `京` | no |

This rule applies consistently during parsing.

---

## Daiji Parsing Policy

Parsing should accept a practical subset of Daiji and normalize it before ordinary positional Kansuji parsing.

The initial normalization table is:

| Input | Normalized Form |
|---|---|
| `壱` | `一` |
| `弐` | `二` |
| `参` | `三` |
| `拾` | `十` |
| `佰` | `百` |
| `阡` | `千` |
| `萬` | `万` |

The following rare Daiji digit forms are outside the initial accepted set:

- `肆`
- `伍`
- `陸`
- `漆`
- `捌`
- `玖`

Daiji parsing should be intentionally permissive about omission of `壱`.

Examples such as the following should be accepted after normalization:

```text
拾
壱拾
百拾
壱百壱拾
阡佰拾
壱阡壱佰壱拾
```

---

## Parsing Philosophy

Generation is canonical.
Parsing is practical and somewhat permissive.

That means:

- output should follow a single stable form for each style
- input should accept common omitted forms
- Daiji should be normalized when supported
- unnecessarily obscure or rare variants need not be accepted initially

---

## Deferred Items

The following are deferred:

- negative number support
- fractional number support
- complete Daiji coverage
- support for rare digit Daiji such as `肆`, `伍`, `陸`, `漆`, `捌`, and `玖`
- broader historical orthography
- any richer natural-language number interpretation beyond the supported notation families

---

## Summary

The initial Kansuji facility should:

- provide `xer::ja::to_kansuji(...)`
- provide `xer::ja::from_kansuji(...)`
- use memorable style selectors:
  - `xer::ja::k10`
  - `xer::ja::k十`
  - `xer::ja::k一〇`
  - `xer::ja::k拾`
- support practical Japanese integer notation families
- generate canonical notation
- parse common abbreviated forms
- accept a limited practical subset of Daiji during parsing
- defer negative numbers, fractions, and rare historical variants
