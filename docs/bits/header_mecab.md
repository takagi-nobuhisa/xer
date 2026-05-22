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

Higher-level Japanese text processing such as bunsetsu grouping, human-readable spacing, kana conversion, ruby, romanization, and braille-oriented conversion is planned to build on top of this analysis layer.

---

## Main Role

The main role of `<xer/mecab.h>` at the current stage is to expose MeCab's morphological analysis result in a form that XER users can inspect and reuse directly.

The raw feature string is preserved, and XER also splits it into `mecab_features` so that common items such as part of speech and reading can be accessed without reparsing the comma-separated feature string in user code.

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

Not yet implemented in this header:

- bunsetsu grouping
- bunsetsu-based spacing
- kana conversion based on readings
- ruby-oriented structures
- romanization
- braille-oriented conversion
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
