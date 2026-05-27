# Policy for `is` Functions and `to` Functions

## Basic Policy

In xer, `is` functions and `to` functions are locale-independent, and the basic individual functions operate only on the ASCII range.

The criteria for classification and conversion correspond to the `"C"` locale.

The character type of the argument for individual functions is unified to `char32_t`.

## Individual `is` Functions

Individual `is` functions return `bool`.

These functions perform classification only for characters in the ASCII range, and always return `false` when a non-ASCII character is passed.

At least the following functions are provided:

- `isalpha`
- `isdigit`
- `isalnum`
- `islower`
- `isupper`
- `isspace`
- `isblank`
- `iscntrl`
- `isprint`
- `isgraph`
- `ispunct`
- `isxdigit`
- `isascii`
- `isoctal`
- `isbinary`

## Individual `to` Functions

Individual `to` functions return `xer::result<char32_t>`.

These functions perform conversion only for characters in the ASCII range.

- if the input is a conversion target, they return the converted character
- if the input is not a conversion target, they return the original character as a successful result
- if a non-ASCII character is passed, they return failure

At least the following functions are provided:

- `tolower`
- `toupper`

`toascii` is often understood in C-family libraries as a function that extracts the lower 7 bits.
Because that does not fit well with xer's character handling policy, it is not adopted at this time.

## Dynamic Character Classification and Dynamic Character Conversion

In addition to individual `is` functions and `to` functions, xer provides a dynamic character classification function `isctype` and a dynamic character conversion function `toctrans`.

These functions take a character class specifier `ctype_id` and a conversion kind specifier `ctrans_id`, respectively.

`ctype_id` and `ctrans_id` contain both ASCII-limited categories and extended categories that also cover non-ASCII characters.
This allows classification and conversion kinds to be handled dynamically through a single variable.

## `isctype`

`isctype` classifies a character according to the specified character class and returns `bool`.

When an ASCII-limited category is specified, it behaves like the individual `is` functions:
it targets only the ASCII range and returns `false` for non-ASCII characters.

When an extended category is specified, it may perform more capable classification that also covers non-ASCII characters.

This makes it possible, for example, for individual functions such as `isdigit` not to include fullwidth digits, kanji numerals, or Roman numerals, while allowing such classification explicitly through `isctype` only when needed.

## `toctrans`

`toctrans` converts a character according to the specified conversion kind and returns `xer::result<char32_t>`.

When an ASCII-limited category is specified, it behaves like the individual `to` functions:
it targets only the ASCII range and returns failure for non-ASCII characters.

When an extended category is specified, it may perform more capable conversion that also covers non-ASCII characters.

Extended categories may cover at least the following kinds of conversion:

- fullwidth/halfwidth conversion for Japanese use
- conversion between Hiragana and Katakana
- uppercase/lowercase conversion for Greek and Cyrillic letters
- kana-to-romaji conversion at the string level

Some conversion identifiers are meaningful only for string-level conversion.
A conversion that depends on neighboring characters or produces multi-code-point output should be provided through `strtoctrans`, even if its category is represented by `ctrans_id`.

## Fullwidth/Halfwidth Conversion

The fullwidth/halfwidth conversion in `toctrans` is intended for Japanese use.

The target characters include at least the following:

- ASCII-compatible letters, digits, symbols, and space
- halfwidth Katakana
- fullwidth Katakana
- punctuation marks related to Kana, dakuten, and handakuten

Hiragana is excluded from fullwidth/halfwidth conversion.

When converting fullwidth Katakana with dakuten or handakuten into halfwidth Katakana, the dakuten or handakuten may be dropped, because the return value is limited to a single character.

## Kana Conversion

`toctrans` supports conversion between fullwidth Hiragana and fullwidth Katakana.

- `katakana`: converts fullwidth Hiragana to the corresponding fullwidth Katakana
- `hiragana`: converts fullwidth Katakana to the corresponding fullwidth Hiragana

Halfwidth Katakana is excluded from Kana conversion.



## Kana-to-Romaji Conversion

`ctrans_id` includes the following string-level romanization categories:

- `romaji`
- `romaji_alt`

These identifiers are used with `strtoctrans`.

They are not supported by single-character `toctrans`, because romanization requires string context and may change the number of output code points.
For these identifiers, `toctrans` reports `error_t::invalid_argument`.

The romanization policy follows the 2025 Cabinet Notification **“Romanization Spelling”**.

- `romaji` uses the ordinary long-vowel notation with macrons
- `romaji_alt` uses the separately allowed no-macron spelling that writes vowel letters according to modern kana spelling

Examples:

| Source | `romaji` | `romaji_alt` |
|---|---|---|
| `とうきょう` | `tōkyō` | `toukyou` |
| `おおさか` | `ōsaka` | `oosaka` |
| `えいが` | `ēga` | `eiga` |
| `コーヒー` | `kōhī` | `koohii` |

The initial implementation supports Hiragana, fullwidth Katakana, and the Katakana prolonged sound mark `ー`.
Inputs outside the supported conversion domain are reported as `error_t::invalid_argument`.

The conversion should correctly handle at least:

- yōon combinations such as `きゃ`, `しゃ`, and `ちゃ`
- sokuon
- syllabic `ん`, including separator apostrophes when needed
- long vowels expressed through kana spelling or Katakana long sound marks


## Unicode Scalar and BMP Classification

xer provides code-point validity checks as part of `isctype` and as individual helper functions.

- `is_unicode_scalar_value` checks whether a `char32_t` value is a valid Unicode scalar value
- `is_unicode_bmp_scalar_value` checks whether it is both a valid Unicode scalar value and in the Basic Multilingual Plane
- `ctype_id::unicode` selects Unicode scalar value classification
- `ctype_id::unicode_bmp` selects BMP Unicode scalar value classification

Surrogate code points are not Unicode scalar values and therefore return `false`.

These functions are deliberately limited to Unicode code-point validity.
They do not attempt to determine whether a character is valid for Unicode identifiers or C++ universal character names.
That area requires table-based rules and is deferred.


## Braille Pattern Classification

`ctype_id` includes `braille` for Unicode Braille Patterns classification.

`isctype(c, ctype_id::braille)` returns `true` when `c` is in the Unicode Braille Patterns block, `U+2800..U+28FF`.
`U+2800 BRAILLE PATTERN BLANK` is included.

This classification is intentionally limited to the Unicode block.
It does not attempt to validate language-specific braille notation, contractions, word spacing, Japanese kana-based braille rules, or other higher-level braille text rules.

An individual `isbraille` function is not provided at this stage.
Braille pattern checks should use `isctype` with `ctype_id::braille`.

## Identifier-Related Functions

As for `iscsym` and `iscsymf`, their adoption and specification should be considered separately, because their relationship to international character names and Unicode identifiers needs to be examined.
