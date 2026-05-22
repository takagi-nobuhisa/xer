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

enum class mecab_phrase_kind {
    bunsetsu,
    symbol,
};

struct mecab_phrase {
    mecab_phrase_kind kind = mecab_phrase_kind::bunsetsu;
    std::size_t index = 0;
    std::size_t count = 0;
};

enum class mecab_kana_kind {
    mixed,
    hiragana,
    katakana,
};

struct mecab_kana_options {
    mecab_kana_kind kind = mecab_kana_kind::mixed;
    bool particle_reading = true;
};

[[nodiscard]]
auto mecab_split_phrases(
    std::span<const mecab_token> tokens)
    -> std::vector<mecab_phrase>;

[[nodiscard]]
auto mecab_to_kana(
    std::span<const mecab_token> tokens,
    const mecab_kana_options& options = {})
    -> std::u8string;

[[nodiscard]]
auto mecab_kana_wakati(
    std::span<const mecab_token> tokens,
    const mecab_kana_options& options = {})
    -> std::u8string;

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

`mecab_split_phrases` derives practical bunsetsu-like ranges and separate symbol ranges from an existing token sequence. It does not invoke MeCab and does not perform kana conversion by itself.

`mecab_to_kana` converts token readings to kana without inserting spaces. `mecab_kana_wakati` combines reading-based kana conversion with `mecab_split_phrases` and inserts spaces between the derived phrase ranges.

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

The raw-data portion is implemented through `mecab_parse`.
It preserves both the raw MeCab feature string and an owned split representation in `mecab_features`.

The first bunsetsu-oriented primitive is implemented through `mecab_split_phrases`.
It returns bunsetsu-like token ranges and symbol ranges, and is the common foundation for kana spacing, romanization, braille-oriented conversion, and bunsetsu counts.

The first reading-based output helpers are implemented through `mecab_to_kana` and `mecab_kana_wakati`.
They provide practical kana conversion and kana wakachi-gaki based on MeCab-derived readings.

---

## Bunsetsu as a First-Class Processing Unit

Bunsetsu segmentation is essential, not optional.

XER derives bunsetsu-like groups from MeCab morphological analysis results and uses them as the main unit for human-readable Japanese processing.

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


## Current Bunsetsu Segmentation API

The current public bunsetsu-oriented primitive is:

```cpp
enum class mecab_phrase_kind {
    bunsetsu,
    symbol,
};

struct mecab_phrase {
    mecab_phrase_kind kind = mecab_phrase_kind::bunsetsu;
    std::size_t index = 0;
    std::size_t count = 0;
};

[[nodiscard]]
auto mecab_split_phrases(
    std::span<const mecab_token> tokens)
    -> std::vector<mecab_phrase>;
```

The function returns ranges into the original token sequence. It intentionally does not copy token text, readings, or feature data.

The result contains two kinds of ranges:

- `mecab_phrase_kind::bunsetsu` for bunsetsu-like token groups
- `mecab_phrase_kind::symbol` for symbols and consecutive symbol groups

Symbols are kept separate from bunsetsu because later stages often need to treat punctuation and brackets differently from words. This is especially useful for kana-based spacing, romanization, and braille-oriented conversion.

`mecab_split_phrases` is a pure token-sequence operation. It does not launch MeCab, does not return `xer::result`, and does not attempt notation conversion.

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


## Initial Bunsetsu Rule Set

The initial implementation uses a deliberately simple rule set.

| Condition | Treatment |
|---|---|
| `features.品詞 == u8"記号"` | emit as `symbol` |
| consecutive symbols | merge into one `symbol` range |
| `助詞` or `助動詞` | attach to preceding `bunsetsu` |
| suffix-like token or non-independent token | attach to preceding `bunsetsu` |
| `接頭詞` followed by another token | keep in the same `bunsetsu` as the following token |
| `名詞 + 名詞` | keep in the same `bunsetsu` as a compound-like sequence |
| `動詞` or `形容詞` in `連用*` form followed by an independent word | keep in the same `bunsetsu` |
| other independent word | usually starts a new `bunsetsu` |

Example target grouping:

```text
私は明日、学校へ行きます。
```

```text
bunsetsu: 私 は
bunsetsu: 明日
symbol: 、
bunsetsu: 学校 へ
bunsetsu: 行き ます
symbol: 。
```

This rule set is a starting point for practical processing. It is expected to evolve as kana spacing, romanization, and braille-oriented conversion expose additional edge cases.

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

The current low-level kana wakachi-gaki helper is `mecab_kana_wakati`.
It uses `mecab_split_phrases` and inserts one ASCII space between phrase ranges. Symbols are kept as independent ranges at this layer.

---

## Kana Conversion Policy

Reading-based kana conversion is implemented as a practical helper layer above MeCab tokens.

The current API is:

```cpp
enum class mecab_kana_kind {
    mixed,
    hiragana,
    katakana,
};

struct mecab_kana_options {
    mecab_kana_kind kind = mecab_kana_kind::mixed;
    bool particle_reading = true;
};

[[nodiscard]]
auto mecab_to_kana(
    std::span<const mecab_token> tokens,
    const mecab_kana_options& options = {})
    -> std::u8string;

[[nodiscard]]
auto mecab_kana_wakati(
    std::span<const mecab_token> tokens,
    const mecab_kana_options& options = {})
    -> std::u8string;
```

`mecab_to_kana` converts a token sequence to kana without adding spaces.
It uses `mecab_features::読み` when available and falls back to `mecab_token::surface` when the reading is empty or `*`.

`mecab_kana_wakati` first derives phrase ranges with `mecab_split_phrases`, then converts each phrase to kana and inserts one ASCII space between phrases.

The kana kind policy is:

| Kind | Treatment |
|---|---|
| `mixed` | Use hiragana by default, while preserving katakana-like source tokens as katakana |
| `hiragana` | Convert readings to hiragana |
| `katakana` | Convert readings to katakana |

`mixed` is the default because it is the most natural starting point for ordinary Japanese text containing katakana loanwords.

### Particle Reading Policy

When `mecab_kana_options::particle_reading` is `true`, XER uses pronunciation-oriented readings for common particles:

| Surface | Condition | Hiragana reading | Katakana reading |
|---|---|---|---|
| `は` | token is a particle | `わ` | `ワ` |
| `へ` | token is a particle | `え` | `エ` |
| `を` | always | `お` | `オ` |

The `は` and `へ` rules require particle detection so that unrelated words are not rewritten merely because their surface text contains those characters.
The `を` rule is treated as a simple pronunciation rule for this practical conversion layer.

`particle_reading` defaults to `true` because kana wakachi-gaki, romanization, and braille-oriented conversion usually need pronunciation rather than orthographic particle spelling.

### Scope and Accuracy

Kana conversion is based on MeCab readings and is therefore dictionary-dependent.
It is not a perfect reading resolver for all Japanese text.

The helper is intended to be useful for ordinary processing, including:

- kana wakachi-gaki
- preparation for romanization
- preparation for braille-oriented conversion
- simple reading display

Display-oriented punctuation spacing is intentionally not handled here. At this layer, symbols remain independent phrase ranges and are separated by spaces.

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

Reading-related helpers are built on top of MeCab-derived readings.
The low-level `mecab_features::読み` member provides the initial access point for this data when the installed dictionary supplies it.

The current reading-based helpers are:

- `mecab_to_kana`
- `mecab_kana_wakati`

Later helpers may build on these for:

- ruby
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

- dictionary-dependent feature interpretation strategy beyond the current IPADIC-style named members for higher-level helpers
- detailed ruby output format
- display-oriented punctuation spacing rules
- detailed romanization rules
- detailed braille conversion rules
- additional bunsetsu segmentation refinements based on real examples
- error models for later higher-level transformations where additional failures arise

---

## Summary

XER's MeCab-based Japanese text processing should:

- invoke MeCab as a UTF-8 child process
- expose raw morphological analysis data and split feature fields through `mecab_parse`
- derive practical bunsetsu-like segmentation through `mecab_split_phrases`
- provide reading-based kana conversion through `mecab_to_kana`
- provide kana wakachi-gaki through `mecab_kana_wakati`
- use bunsetsu, not tokens, as the default unit for human-readable spacing
- support the future implementation of ruby, romanization, braille-oriented conversion, and counts
- accept that natural language processing cannot be perfect
- aim for behavior that is **not perfect, but sufficiently good**
