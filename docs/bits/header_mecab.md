# `<xer/mecab.h>`

## Purpose

`<xer/mecab.h>` provides XER's initial MeCab-based Japanese text analysis API.

The current implementation focuses on the lowest-level public foundation:

- invoking MeCab as a child process
- sending UTF-8 source text to MeCab
- receiving UTF-8 analysis output
- returning morphological token results
- preserving raw feature text
- providing split feature fields for common MeCab/IPADIC-style columns
- deriving practical bunsetsu-like phrase ranges and symbol ranges
- converting MeCab-derived readings to kana text
- producing kana wakachi-gaki text using the phrase ranges
- producing romaji wakachi-gaki text by combining kana conversion and `strtoctrans`
- producing Japanese braille wakachi-gaki text by combining phrase ranges, MeCab-derived kana readings, Japanese punctuation handling, ASCII fragment conversion, and `<xer/braille.h>`
- producing information-processing braille variants for ASCII fragments
- parsing source text and converting it directly to braille through convenience wrappers

Higher-level Japanese text processing such as ruby generation is planned to build on top of this analysis layer. Braille-oriented wakachi-gaki conversion now has an initial helper built on the kana layer.

---

## Main Role

The main role of `<xer/mecab.h>` at the current stage is to expose MeCab's morphological analysis result in a form that XER users can inspect and reuse directly.

The raw feature string is preserved, and XER also splits it into `mecab_features` so that common items such as part of speech and reading can be accessed without reparsing the comma-separated feature string in user code.

On top of the token layer, XER provides `mecab_split_phrases` to derive practical bunsetsu-like ranges and separate symbol ranges. MeCab itself does not return bunsetsu boundaries, so this layer is an XER rule-based approximation.

The kana layer uses `mecab_features::読み` where available and provides `mecab_to_kana` and `mecab_kana_wakati` as practical reading-based conversion helpers.

The braille layer builds on the token, phrase, kana, punctuation, and ASCII-fragment conversion layers. It provides `mecab_braille_wakati` as a practical Japanese braille wakachi-gaki helper for MeCab token sequences. Unlike `mecab_kana_wakati`, it handles symbol ranges directly so that Japanese punctuation can be attached more naturally in braille output. It also converts ASCII alphanumeric-and-punctuation fragments from the original surface text rather than from MeCab readings.

The information-processing braille variant is `mecab_ip_braille_wakati`. It uses the same Japanese reading and spacing rules, but converts ASCII fragments through information-processing braille.

For callers that want to pass source text directly, `mecab_braille_translate` and `mecab_ip_braille_translate` combine `mecab_parse` with the corresponding braille wakachi-gaki helper.

The romaji layer builds on the kana layer and `strtoctrans`. It provides `mecab_romaji_wakati` as a practical romaji wakachi-gaki helper. Particle reading correction is performed before romanization, so particles such as `は`, `へ`, and `を` can become `wa`, `e`, and `o` in the final output.

XER does not link against the MeCab library.
Instead, it executes the `mecab` command as a child process using XER's process facilities.

This keeps the integration compatible with XER's header-only model while making MeCab-derived analysis data available through ordinary `xer::result` APIs.

---

## Environment Assumption

The current implementation assumes UTF-8 MeCab I/O.

The project has checked the ordinary target environments used for this feature:

- Ubuntu with MeCab installed through the usual package flow
- MSYS2 UCRT64 with the ordinary MeCab packages

In both cases, `mecab -D` reported UTF-8 dictionary encoding during design verification.

XER therefore:

- sends UTF-8 input text to MeCab
- validates MeCab output as UTF-8
- does not perform character-set conversion around the MeCab process

---

## Main Entities

`<xer/mecab.h>` currently provides:

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

struct mecab_romaji_options {
    mecab_kana_options kana;
    ctrans_id romaji = ctrans_id::romaji;
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
auto mecab_braille_wakati(
    std::span<const mecab_token> tokens,
    const mecab_kana_options& options = {})
    -> xer::result<std::u8string>;

[[nodiscard]]
auto mecab_ip_braille_wakati(
    std::span<const mecab_token> tokens,
    const mecab_kana_options& options = {})
    -> xer::result<std::u8string>;

[[nodiscard]]
auto mecab_braille_translate(
    std::u8string_view text,
    const mecab_options& parse_options = {},
    const mecab_kana_options& kana_options = {})
    -> xer::result<std::u8string>;

[[nodiscard]]
auto mecab_ip_braille_translate(
    std::u8string_view text,
    const mecab_options& parse_options = {},
    const mecab_kana_options& kana_options = {})
    -> xer::result<std::u8string>;

[[nodiscard]]
auto mecab_romaji_wakati(
    std::span<const mecab_token> tokens,
    const mecab_romaji_options& options = {})
    -> xer::result<std::u8string>;

[[nodiscard]]
auto mecab_parse(
    std::u8string_view text,
    const mecab_options& options = {})
    -> xer::result<std::vector<mecab_token>>;
```

---

## `mecab_options`

```cpp
struct mecab_options {
    xer::path program;
};
```

`mecab_options` controls how XER locates the MeCab executable.

### `program`

`program` specifies the MeCab executable path explicitly.

If `program` is empty, XER searches the `PATH` environment variable for the platform's ordinary executable name:

- Windows: `mecab.exe`
- POSIX-like environments: `mecab`

Example:

```cpp
xer::mecab_options options {
    .program = xer::path(u8"/usr/bin/mecab"),
};
```

In ordinary Ubuntu and MSYS2 UCRT64 installations, callers will usually leave this empty.

---

## `mecab_features`

```cpp
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
```

`mecab_features` stores the split form of MeCab's raw feature string.

The named members follow the ordinary MeCab/IPADIC-style feature order:

| Member | Source field | Meaning |
|---|---:|---|
| `品詞` | 0 | part of speech |
| `品詞細分類1` | 1 | part-of-speech subclass 1 |
| `品詞細分類2` | 2 | part-of-speech subclass 2 |
| `品詞細分類3` | 3 | part-of-speech subclass 3 |
| `活用型` | 4 | conjugation type |
| `活用形` | 5 | conjugation form |
| `原形` | 6 | base form |
| `読み` | 7 | reading |
| `発音` | 8 | pronunciation |

`項目` stores all comma-separated fields in order, including dictionary-specific fields that do not have named members.
If a field is missing, the corresponding named member is an empty string.

For practical compatibility with dictionaries whose feature layout differs from IPADIC-style order, XER may supplement `読み` and `発音` by scanning the split `項目` fields for kana-only values when the direct IPADIC-style field is missing, `*`, or not kana-like. This is a convenience for higher-level helpers such as kana and romaji wakachi-gaki, not a complete dictionary-normalization layer.

The member names intentionally use Japanese identifiers because they correspond directly to MeCab feature terminology.
XER does not restrict identifiers to ASCII.
Users are responsible for using a source-code environment that can handle these identifiers when they access the members directly.

`mecab_features` owns its strings.
It does not store `std::u8string_view` into `mecab_token::feature`, so `mecab_token` remains safely copyable as a `std::vector` element.

---

## `mecab_token`

```cpp
struct mecab_token {
    std::u8string surface;
    std::u8string feature;
    mecab_features features;
};
```

`mecab_token` represents one token returned by MeCab.

### `surface`

`surface` is the surface text of the token.

### `feature`

`feature` is the raw MeCab feature string emitted by MeCab's `%H` formatter.

Its exact contents depend on the installed MeCab dictionary.
XER preserves it as raw text for debugging and for users that need dictionary-specific data.

### `features`

`features` is the parsed feature data derived from `feature`.

It is intended for common Japanese-processing operations such as inspecting part of speech, obtaining readings, or checking conjugation forms without reparsing the raw feature string.


---

## `mecab_phrase_kind`

```cpp
enum class mecab_phrase_kind {
    bunsetsu,
    symbol,
};
```

`mecab_phrase_kind` identifies the kind of range returned by `mecab_split_phrases`.

| Value | Meaning |
|---|---|
| `bunsetsu` | A bunsetsu-like phrase derived from MeCab tokens |
| `symbol` | A symbol token or consecutive symbol-token range |

`symbol` is intentionally separated from `bunsetsu`. This makes later processing easier for kana spacing, romanization, and braille-oriented conversion, where punctuation and other symbols often need their own handling.

---

## `mecab_phrase`

```cpp
struct mecab_phrase {
    mecab_phrase_kind kind = mecab_phrase_kind::bunsetsu;
    std::size_t index = 0;
    std::size_t count = 0;
};
```

`mecab_phrase` represents a subrange of the original `mecab_token` sequence.

The range does not own or copy token text. `index` is the first token index in the original token sequence, and `count` is the number of tokens in the range.

This representation is intentionally simple so that callers can reuse the original token objects, inspect their features, and join surfaces or readings in the way required by the next processing step.

---

## `mecab_split_phrases`

```cpp
[[nodiscard]]
auto mecab_split_phrases(
    std::span<const mecab_token> tokens)
    -> std::vector<mecab_phrase>;
```

### Purpose

`mecab_split_phrases` splits a MeCab token sequence into practical bunsetsu-like phrase ranges and symbol ranges.

MeCab itself does not provide bunsetsu segmentation. XER therefore derives approximate phrase boundaries from the split feature fields in `mecab_token::features`.

### Basic Rules

The current rule set is practical rather than linguistically complete.

- Tokens whose `features.品詞` is `記号` are emitted as `mecab_phrase_kind::symbol`
- Consecutive symbols are grouped into one `symbol` range
- `助詞`, `助動詞`, suffix-like tokens, and non-independent tokens attach to the preceding `bunsetsu`
- `接頭詞` attaches to the following token by keeping the next token in the same `bunsetsu`
- Consecutive `名詞` tokens remain in the same `bunsetsu` as a practical compound-word rule
- A `動詞` or `形容詞` whose `活用形` starts with `連用` remains with the following independent word
- Other independent words usually begin a new `bunsetsu`

### Example

For a token sequence corresponding to:

```text
私は明日、学校へ行きます。
```

`mecab_split_phrases` is intended to produce ranges equivalent to:

```text
bunsetsu: 私 は
bunsetsu: 明日
symbol: 、
bunsetsu: 学校 へ
bunsetsu: 行き ます
symbol: 。
```

Actual token surfaces and feature values still depend on the installed MeCab dictionary.

### Empty Input

An empty token span returns an empty phrase vector.

### Error Model

`mecab_split_phrases` does not invoke MeCab and does not allocate external resources. It returns a plain `std::vector<mecab_phrase>` instead of `xer::result`.

The function assumes that `tokens` was produced by `mecab_parse` or otherwise contains compatible feature data. If feature data is missing or dictionary-specific, the result may be less natural but still follows the documented rules.

---

## `mecab_kana_kind`

```cpp
enum class mecab_kana_kind {
    mixed,
    hiragana,
    katakana,
};
```

`mecab_kana_kind` controls how MeCab-derived readings are written as kana.

| Value | Meaning |
|---|---|
| `mixed` | Use hiragana by default, while preserving katakana-like source tokens as katakana |
| `hiragana` | Convert readings to hiragana |
| `katakana` | Convert readings to katakana |

`mixed` is the default because it keeps ordinary Japanese readings readable while preserving common katakana words such as `コンピューター`.

---

## `mecab_kana_options`

```cpp
struct mecab_kana_options {
    mecab_kana_kind kind = mecab_kana_kind::mixed;
    bool particle_reading = true;
};
```

`mecab_kana_options` controls reading-based kana conversion.

### `kind`

`kind` selects the kana output style.

The default is `mecab_kana_kind::mixed`.

### `particle_reading`

When `particle_reading` is `true`, XER uses pronunciation-oriented readings for common particles:

| Surface | Condition | Hiragana output | Katakana output |
|---|---|---|---|
| `は` | `features.品詞 == u8"助詞"` | `わ` | `ワ` |
| `へ` | `features.品詞 == u8"助詞"` | `え` | `エ` |
| `を` | always | `お` | `オ` |

The default is `true` because kana wakachi-gaki, romanization, and braille-oriented processing usually need pronunciation-oriented particle readings.

When `particle_reading` is `false`, the function uses MeCab's reading field, or the token surface when the reading is unavailable.

---

## `mecab_to_kana`

```cpp
[[nodiscard]]
auto mecab_to_kana(
    std::span<const mecab_token> tokens,
    const mecab_kana_options& options = {})
    -> std::u8string;
```

### Purpose

`mecab_to_kana` converts a MeCab token sequence to kana text.

Each token is converted independently. The function uses `mecab_features::読み` when it is available and not `*`; otherwise, it falls back to `mecab_token::surface`.

This function does not insert spaces between tokens. It is useful when the caller already has the desired token or phrase range.

### Example

For tokens corresponding to:

```text
私はコンピューターを使います。
```

The default `mixed` mode is intended to produce kana text close to:

```text
わたしわコンピューターおつかいます。
```

Actual readings depend on the installed MeCab dictionary.

---

## `mecab_kana_wakati`

```cpp
[[nodiscard]]
auto mecab_kana_wakati(
    std::span<const mecab_token> tokens,
    const mecab_kana_options& options = {})
    -> std::u8string;
```

### Purpose

`mecab_kana_wakati` converts a MeCab token sequence to kana wakachi-gaki text.

The function first calls `mecab_split_phrases`, then inserts one ASCII space between the returned phrase ranges. Symbols are kept as independent ranges, so punctuation and other symbols are also separated by spaces in this low-level helper.

For example, a token sequence corresponding to:

```text
私はコンピューターを使います。
```

is intended to produce output close to:

```text
わたしわ コンピューターお つかいます 。
```

With `mecab_kana_kind::hiragana`, katakana-like source words are also converted to hiragana:

```text
わたしわ こんぴゅーたーお つかいます 。
```

With `mecab_kana_kind::katakana`, the output is katakana-oriented:

```text
ワタシワ コンピューターオ ツカイマス 。
```

### Error Model

`mecab_kana_wakati` does not invoke MeCab and does not return `xer::result`.
It assumes that the input token sequence was already produced by `mecab_parse` or by an equivalent compatible source.

Display-oriented spacing that attaches punctuation to the previous phrase can be layered on top later. The current helper intentionally keeps symbols independent because later romanization and braille-oriented processing often need that separation.

---

## `mecab_braille_wakati`

```cpp
[[nodiscard]]
auto mecab_braille_wakati(
    std::span<const mecab_token> tokens,
    const mecab_kana_options& options = {})
    -> xer::result<std::u8string>;
```

### Purpose

`mecab_braille_wakati` converts a MeCab token sequence to Japanese braille wakachi-gaki text.

The function uses `mecab_split_phrases` to process bunsetsu-like ranges and symbol ranges separately.

For ordinary bunsetsu-like ranges, it normally calls `mecab_to_kana` with the same kana options and then converts the resulting kana text through `xer::braille::kana_text_to_braille`.

When a token surface is an ASCII alphanumeric-and-punctuation fragment, the function converts that fragment from the original surface text through `xer::braille::alnum_punct_text_to_braille` instead of using MeCab readings. This allows fragments such as `ABC123` or `UTF-8` to keep their visible ASCII form in braille output.

For symbol ranges, it converts each symbol directly through the Japanese punctuation conversion layer used by `<xer/braille.h>`. This avoids inserting unnecessary spaces before punctuation such as `。`, `、`, `」`, or `）`.

Spacing is controlled as follows:

- ordinary bunsetsu-like ranges are separated by ASCII spaces
- opening symbols such as `「`, `『`, `（`, and `(` are attached to the following phrase after any required preceding space
- closing symbols, sentence-ending symbols, pause symbols, and leaders request a following space
- symbol marks themselves are emitted as braille punctuation, not as surface text

For example, a token sequence corresponding to:

```text
私は猫です。
```

is intended to produce braille wakachi-gaki close to the braille representation of:

```text
わたしわ ねこです。
```

without an extra space before the Japanese full stop.

The exact reading and phrase boundaries depend on the installed MeCab dictionary.

### Error Model

`mecab_braille_wakati` returns `xer::result<std::u8string>` because the braille conversion layer can fail.

Errors from `xer::braille::kana_text_to_braille`, `xer::braille::alnum_punct_text_to_braille`, and the Japanese punctuation conversion layer are propagated. For example, if the token sequence contains a symbol that is not supported as Japanese braille punctuation, or an ASCII fragment contains punctuation that is not supported by ordinary English braille punctuation conversion, the function returns `error_t::invalid_argument`.

`mecab_braille_wakati` does not invoke MeCab. It assumes that the input token sequence was already produced by `mecab_parse` or by an equivalent compatible source.

---

## `mecab_ip_braille_wakati`

```cpp
[[nodiscard]]
auto mecab_ip_braille_wakati(
    std::span<const mecab_token> tokens,
    const mecab_kana_options& options = {})
    -> xer::result<std::u8string>;
```

### Purpose

`mecab_ip_braille_wakati` is the information-processing braille variant of `mecab_braille_wakati`.

Japanese tokens are converted with the same MeCab reading, kana conversion, punctuation, and spacing rules as `mecab_braille_wakati`.
ASCII alphanumeric-and-punctuation fragments are converted from the original surface text through `xer::braille::ip_alnum_punct_text_to_braille`.

This variant is intended for mixed Japanese text that contains programming-language-like ASCII fragments, such as `C++23`, `UTF-8`, `x>=10`, or similar text where information-processing braille punctuation is more appropriate than ordinary English braille punctuation.

### Error Model

`mecab_ip_braille_wakati` returns `xer::result<std::u8string>`.

Errors from kana conversion, Japanese punctuation conversion, and information-processing ASCII conversion are propagated.
It does not invoke MeCab and assumes that the input token sequence was already produced by `mecab_parse` or by an equivalent compatible source.

---

## `mecab_romaji_options`

```cpp
struct mecab_romaji_options {
    mecab_kana_options kana;
    ctrans_id romaji = ctrans_id::romaji;
};
```

`mecab_romaji_options` controls romaji wakachi-gaki conversion.

### `kana`

`kana` controls the kana conversion performed before romanization.

The default keeps `mecab_kana_options::particle_reading` enabled, so common particles are converted by pronunciation before they are romanized.

For example, particles are intended to romanize as follows:

| Surface | Kana after particle correction | Romaji |
|---|---|---|
| `は` | `わ` | `wa` |
| `へ` | `え` | `e` |
| `を` | `お` | `o` |

### `romaji`

`romaji` selects the `strtoctrans` romanization mode.

Supported values are:

| Value | Meaning |
|---|---|
| `ctrans_id::romaji` | Macron-based long-vowel form |
| `ctrans_id::romaji_alt` | Kana-spelling-based alternate form |

Other `ctrans_id` values are rejected with `error_t::invalid_argument`.

---

## `mecab_romaji_wakati`

```cpp
[[nodiscard]]
auto mecab_romaji_wakati(
    std::span<const mecab_token> tokens,
    const mecab_romaji_options& options = {})
    -> xer::result<std::u8string>;
```

### Purpose

`mecab_romaji_wakati` converts a MeCab token sequence to romaji wakachi-gaki text.

The function first calls `mecab_split_phrases`. Each `bunsetsu` range is converted to kana, then romanized through `strtoctrans`. Each `symbol` range is preserved as surface text and is not passed to `strtoctrans`. One ASCII space is inserted between phrase ranges.

For example, a token sequence corresponding to:

```text
私は猫です。
```

is intended to produce output close to:

```text
watashiwa nekodesu 。
```

With `ctrans_id::romaji_alt`, long vowels use the alternate kana-spelling-based form.

The exact result depends on the installed MeCab dictionary, because readings and token boundaries are dictionary-dependent.

### Error Model

Unlike `mecab_to_kana` and `mecab_kana_wakati`, `mecab_romaji_wakati` returns `xer::result<std::u8string>`.

It reports `error_t::invalid_argument` when `options.romaji` is not `ctrans_id::romaji` or `ctrans_id::romaji_alt`.

If `strtoctrans` cannot romanize a kana sequence, the error from `strtoctrans` is propagated.

`mecab_romaji_wakati` does not invoke MeCab. It assumes that the input token sequence was already produced by `mecab_parse` or by an equivalent compatible source.

---

## `mecab_parse`

```cpp
[[nodiscard]]
auto mecab_parse(
    std::u8string_view text,
    const mecab_options& options = {})
    -> xer::result<std::vector<mecab_token>>;
```

### Purpose

`mecab_parse` invokes MeCab and returns raw morphological analysis results.

### Output Format Used Internally

XER explicitly asks MeCab to emit one token per line in this format:

```text
surface<TAB>feature
```

The `EOS` marker is consumed internally and is not returned as a token.

Conceptually, XER configures MeCab so that normal and unknown tokens use equivalent raw output structure:

```text
%m<TAB>%H
```

This keeps the parser independent from human-readable MeCab default formatting.

### Empty Input

An empty input string is accepted.

```cpp
const auto tokens = xer::mecab_parse(u8"");
```

On success, the result is an empty token vector.

### Basic Example

```cpp
const auto tokens = xer::mecab_parse(u8"私は猫です。");
if (!tokens) {
    return;
}

for (const auto& token : *tokens) {
    // token.surface
    // token.feature
    // token.features.品詞
    // token.features.読み
}
```

The exact tokenization, feature strings, and split feature fields depend on the installed dictionary.

---

## `mecab_braille_translate`

```cpp
[[nodiscard]]
auto mecab_braille_translate(
    std::u8string_view text,
    const mecab_options& parse_options = {},
    const mecab_kana_options& kana_options = {})
    -> xer::result<std::u8string>;
```

### Purpose

`mecab_braille_translate` parses UTF-8 source text with MeCab and converts the resulting token sequence with `mecab_braille_wakati`.

It is a convenience wrapper for callers that do not need to inspect the intermediate token sequence.
MeCab determines readings for Japanese text. ASCII alphanumeric-and-punctuation fragments are converted from the original surface text by the ordinary braille ASCII-fragment conversion layer.

### Error Model

`mecab_braille_translate` propagates errors from both stages:

- `mecab_parse`
- `mecab_braille_wakati`

This means the function can report MeCab execution errors, UTF-8 errors, and braille conversion errors.

---

## `mecab_ip_braille_translate`

```cpp
[[nodiscard]]
auto mecab_ip_braille_translate(
    std::u8string_view text,
    const mecab_options& parse_options = {},
    const mecab_kana_options& kana_options = {})
    -> xer::result<std::u8string>;
```

### Purpose

`mecab_ip_braille_translate` parses UTF-8 source text with MeCab and converts the resulting token sequence with `mecab_ip_braille_wakati`.

Japanese text is handled in the same way as `mecab_braille_translate`.
ASCII alphanumeric-and-punctuation fragments are converted through the information-processing braille ASCII-fragment conversion layer.

This function is the convenient entry point for Japanese text that may contain code-like or technical ASCII fragments.

### Error Model

`mecab_ip_braille_translate` propagates errors from both stages:

- `mecab_parse`
- `mecab_ip_braille_wakati`

---

## Executable Resolution

If `mecab_options::program` is empty, XER:

1. reads `PATH`
2. searches each path entry
3. checks for the platform's ordinary MeCab executable name
4. executes the first matching file

If no executable is found, `mecab_parse` returns:

```cpp
error_t::not_found
```

---

## Error Model

`mecab_parse` returns `xer::result<std::vector<mecab_token>>`.

The current implementation uses these errors:

| Condition | Error |
|---|---|
| input text is not valid UTF-8 | `error_t::encoding_error` |
| MeCab output is not valid UTF-8 | `error_t::encoding_error` |
| automatic executable search cannot find MeCab | `error_t::not_found` |
| MeCab cannot be executed, exits unsuccessfully, or emits unexpected output | `error_t::process_error` |

Some lower-level process or stream failures may preserve their own XER error code when they arise before the final MeCab-level validation step.

---

## Dictionary Dependence

`mecab_token::feature` and `mecab_token::features` are dictionary-dependent.

Different MeCab dictionaries may:

- split text differently
- report different feature-column layouts
- produce different readings or base-form fields

XER splits the feature string according to the comma-separated structure emitted by `%H`, and fills named members using the ordinary MeCab/IPADIC-style field positions.
This is a practical convenience, not a complete normalization layer for all possible dictionaries.

Higher-level XER Japanese text processing facilities may later define their own supported interpretation strategy where needed.

---

## Current Scope

At the current stage, `<xer/mecab.h>` provides the low-level MeCab morphological analysis foundation.

Implemented:

- UTF-8 MeCab child-process invocation
- executable-path resolution
- token collection
- surface text preservation
- raw feature text preservation
- split feature field preservation
- common MeCab/IPADIC-style named feature members
- practical bunsetsu-like phrase and symbol segmentation through `mecab_split_phrases`
- kana conversion based on MeCab-derived readings through `mecab_to_kana`
- kana wakachi-gaki through `mecab_kana_wakati`
- braille wakachi-gaki through `mecab_braille_wakati` and `mecab_ip_braille_wakati`
- direct braille translation through `mecab_braille_translate` and `mecab_ip_braille_translate`
- romaji wakachi-gaki through `mecab_romaji_wakati`

Not yet implemented in this header:

- ruby-oriented structures
- word or bunsetsu counting helpers

These are planned on top of the raw layer and are described at the policy level in `policy_mecab.md`.

---

## Relationship to Other Headers

`<xer/mecab.h>` is related to:

- `<xer/process.h>`
- `<xer/path.h>`
- `<xer/error.h>`
- `policy_mecab.md`
- `policy_project_outline.md`
- `<xer/toctrans.h>`
- `<xer/braille.h>`
