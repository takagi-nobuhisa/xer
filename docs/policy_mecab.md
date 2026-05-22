# Policy for MeCab-Based Japanese Text Processing

## Overview

XER provides practical Japanese text-processing facilities based on MeCab.

The goal is not to expose MeCab merely as a thin command wrapper.
Instead, XER should use MeCab as the analysis foundation for higher-level Japanese text processing that is often needed in applications but is difficult for ordinary programmers to implement correctly from scratch.

The intended direction is:

- obtain raw morphological analysis data
- derive practical bunsetsu segmentation
- provide human-readable Japanese transformations and counts on top of that foundation

The core design principle is:

> XER should aim for results that are **not perfect, but sufficiently good** for practical use.

---

## Why This Facility Exists

Many applications benefit from Japanese text processing such as:

- adding ruby or obtaining readings
- converting text to hiragana
- inserting readable spaces
- romanizing Japanese text
- converting Japanese text to braille-oriented representations
- counting words or bunsetsu

These needs appear frequently, but reliable implementation requires knowledge of Japanese morphology, readings, segmentation, and notation conventions.
A typical application programmer should not need to rebuild all of that logic independently.

XER should provide a practical foundation that makes such processing accessible.

---

## MeCab Integration Policy

### Execution Model

XER invokes MeCab as a child process rather than linking against the MeCab library.

This choice keeps XER consistent with its header-only design and avoids introducing a direct binary-library dependency into ordinary builds.

The process is launched directly through XER's process facilities, not through a shell.

### Current Public Foundation

The first implemented public layer is raw morphological analysis:

```cpp
struct mecab_options {
    xer::path program;
};

struct mecab_features {
    std::u8string 品詞;
    std::u8string 品詞細分類1;
    std::u8string 品詞細分類2;
    std::u8string 品詞細分類3;
    std::u8string 活用型;
    std::u8string 活用形;
    std::u8string 原形;
    std::u8string 読み;
    std::u8string 発音;
    std::vector<std::u8string> 項目;
};

struct mecab_token {
    std::u8string surface;
    std::u8string feature;
    mecab_features features;
};

[[nodiscard]]
auto mecab_parse(
    std::u8string_view text,
    const mecab_options& options = {})
    -> xer::result<std::vector<mecab_token>>;
```

`mecab_parse` returns one `mecab_token` for each token emitted by MeCab.

- `surface` preserves the token surface text
- `feature` preserves MeCab's raw `%H` feature text
- `features` provides split feature fields and common named MeCab/IPADIC-style members

The raw `feature` text and parsed `features` data are intentionally dictionary-dependent at this layer.
XER preserves raw feature text and also provides practical parsed access because higher-level Japanese processing needs part-of-speech information, conjugation form, and readings.

---

## Encoding Policy

XER treats MeCab standard input, standard output, and standard error as UTF-8.

This assumption is based on the target standard environments checked for the project:

- Ubuntu with MeCab installed through the ordinary package flow
- MSYS2 UCRT64 with the ordinary MeCab packages

In both checked environments, `mecab -D` reported UTF-8 dictionary encoding.

The initial XER implementation therefore proceeds on a UTF-8 basis and does not attempt character-set conversion around MeCab I/O.

Invalid UTF-8 input or invalid UTF-8 output is reported as:

```cpp
error_t::encoding_error
```

---

## Executable Resolution

If the caller leaves `mecab_options::program` empty, XER searches the `PATH` environment variable for the ordinary platform-specific MeCab executable name.

- Windows: `mecab.exe`
- POSIX-like environments: `mecab`

If no executable is found automatically, `mecab_parse` reports:

```cpp
error_t::not_found
```

Callers may specify an explicit executable path through `mecab_options::program` when necessary.

---

## Raw Output Contract

XER requests a stable process-output structure from MeCab rather than parsing MeCab's ordinary human-readable default display.

The implementation asks MeCab to emit token lines equivalent to:

```text
surface<TAB>feature
```

with `feature` provided by MeCab's `%H` formatter.

The `EOS` marker is consumed internally and is not exposed as a token.

Malformed output or unsuccessful process execution is reported as:

```cpp
error_t::process_error
```

---


## Feature Field Policy

`mecab_token::feature` remains the authoritative raw feature text emitted by MeCab's `%H` formatter.
It is preserved because dictionaries may expose additional fields or layouts that XER cannot fully normalize at this layer.

`mecab_token::features` is a practical parsed representation derived from that raw text.
The named members follow the ordinary MeCab/IPADIC-style field order: `品詞`, `品詞細分類1`, `品詞細分類2`, `品詞細分類3`, `活用型`, `活用形`, `原形`, `読み`, and `発音`.
All comma-separated fields are also stored in `項目` so that dictionary-specific data remains available.

The `mecab_features` members intentionally use Japanese identifiers.
These names correspond directly to MeCab feature terminology and avoid unclear English approximations.
XER already permits non-ASCII identifiers where they improve clarity; user source environments that cannot handle such identifiers are outside the scope of this API decision.

`mecab_features` owns its strings.
It must not store `std::u8string_view` into `mecab_token::feature`, because `mecab_token` is returned in `std::vector` and must remain safely copyable and movable.

---

## Intended Feature Set

The MeCab-based Japanese processing facility should support, or provide the foundation for, the following features:

- ruby generation or reading extraction
- hiragana conversion
- bunsetsu-based spacing
- romanization
- braille-oriented conversion
- word counting
- bunsetsu counting
- access to raw morphological analysis data

Raw morphological data is important because it lets users build their own higher-level processing on top of XER when the built-in helpers do not match their needs.

The raw-data portion is now implemented through `mecab_parse`.
It preserves both the raw MeCab feature string and an owned split representation in `mecab_features`.

---

## Bunsetsu as a First-Class Processing Unit

Bunsetsu segmentation is essential, not optional.

XER should derive bunsetsu-like groups from MeCab morphological analysis results and use them as the main unit for human-readable Japanese processing.

At minimum, bunsetsu are needed for:

- readable spacing
- braille-oriented conversion
- bunsetsu counting
- downstream Japanese rendering helpers

---

## Bunsetsu Segmentation Philosophy

The basic model is:

```text
independent word + following dependent words
```

However, practical segmentation must also account for compound words and similar structures.
A simplistic implementation that starts a new bunsetsu at every independent word is not sufficient.

The bunsetsu algorithm should therefore be rule-based and practical rather than purely mechanical.

---

## Compound Handling

The initial bunsetsu rules should aim to avoid unnatural splitting of common compounds.

### Consecutive Nouns

Consecutive nouns may be kept in the same bunsetsu when that yields the natural compound interpretation.

### Ren'yōkei Verb Form Followed by a Noun

A verb in continuative form followed by a noun should normally be treated as a compound and kept in the same bunsetsu.

Example:

```text
返り点
```

Treating this as:

```text
返り / 点
```

would be unnatural in ordinary interpretation.
For practical bunsetsu segmentation, it should be grouped as one unit:

```text
返り点
```

The same principle is useful for forms such as:

```text
読み方
帰り道
売り場
```

This rule is not intended to be a perfect linguistic theorem.
It is a practical segmentation rule that improves ordinary output quality.

---

## Accuracy Policy

Japanese text processing cannot be made perfectly correct without broader context and implicit knowledge.

Examples include:

- words with multiple possible readings
- proper nouns and rare names
- domain-specific vocabulary
- ambiguous compounds
- dictionary-dependent tokenization
- cases where the intended bunsetsu boundary depends on context

XER should not claim to solve these perfectly.

Instead, XER should:

- use MeCab analysis as a strong baseline
- apply practical, documented grouping rules
- produce outputs that are generally useful and natural
- accept that some readings and segmentations will be wrong

This is intentional.
The target is **not perfect, but sufficiently good** behavior.

---

## Wakati Policy

XER's high-level Japanese spacing facility should use bunsetsu boundaries, not morphological token boundaries.

Token-level spacing is useful for machine analysis, but it is not the right default for human readability.

The intended spacing resembles old kana-heavy game text or other kana-only text layouts where spaces are inserted to preserve readability.

For example, the goal is closer to:

```text
わたしは あした がっこうへ いきます
```

than to a token-by-token split that separates particles and auxiliary verbs.

This is especially important for predicates, where multiple auxiliaries commonly attach to a lexical core.
Splitting them one by one makes the text harder, not easier, to read.

---

## Counts

XER should distinguish between:

- word count
- bunsetsu count

Word count may be based on morphological tokens.
Bunsetsu count should be based on XER's derived bunsetsu groups.

Both are useful, but they serve different purposes.

---

## Readings and Derived Conversions

Reading-related helpers should be built on top of MeCab-derived readings.
The low-level `mecab_features::読み` member provides the initial access point for this data when the installed dictionary supplies it.

These helpers may later support:

- ruby
- hiragana conversion
- romanization
- braille-oriented conversion

The accuracy of these derived outputs is limited by the correctness of the underlying reading selection.
A word with multiple possible readings may be converted using a reading that is plausible but not intended in the original context.

This limitation is acceptable within the overall design goal.

---

## Braille-Oriented Processing

Braille-oriented conversion depends on bunsetsu-aware segmentation rather than naïve token spacing.

The details of braille conversion rules are to be defined separately, but the MeCab facility should provide the segmentation and readings needed to support it.

---

## Current Error Model

The raw-analysis layer currently uses:

| Condition | Error |
|---|---|
| invalid UTF-8 input or output | `error_t::encoding_error` |
| automatic MeCab executable lookup fails | `error_t::not_found` |
| process execution fails, MeCab exits unsuccessfully, or output is malformed | `error_t::process_error` |

Underlying process or stream failures may retain their own XER error code when they occur before the final MeCab-specific validation step.

---

## Deferred or Later-Stage Design Items

The following items require later API or algorithm design:

- exact public types for bunsetsu results
- dictionary-dependent feature interpretation strategy beyond the current IPADIC-style named members for higher-level helpers
- detailed ruby output format
- detailed romanization rules
- detailed braille conversion rules
- concrete bunsetsu segmentation rule table
- error models for later higher-level transformations where additional failures arise

---

## Summary

XER's MeCab-based Japanese text processing should:

- invoke MeCab as a UTF-8 child process
- expose raw morphological analysis data and split feature fields through `mecab_parse`
- derive practical bunsetsu segmentation in later layers
- use bunsetsu, not tokens, as the default unit for human-readable spacing
- support the future implementation of ruby, hiragana conversion, romanization, braille-oriented conversion, and counts
- accept that natural language processing cannot be perfect
- aim for behavior that is **not perfect, but sufficiently good**
