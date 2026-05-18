# `<xer/mecab.h>`

## Purpose

`<xer/mecab.h>` provides XER's initial MeCab-based Japanese text analysis API.

The current implementation focuses on the lowest-level public foundation:

- invoking MeCab as a child process
- sending UTF-8 source text to MeCab
- receiving UTF-8 analysis output
- returning raw morphological token results

Higher-level Japanese text processing such as bunsetsu grouping, human-readable spacing, readings, ruby, romanization, and braille-oriented conversion is planned to build on top of this raw analysis layer.

---

## Main Role

The main role of `<xer/mecab.h>` at the current stage is to expose MeCab's morphological analysis result in a form that XER users can inspect and reuse directly.

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

struct mecab_token {
    std::u8string surface;
    std::u8string feature;
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

## `mecab_token`

```cpp
struct mecab_token {
    std::u8string surface;
    std::u8string feature;
};
```

`mecab_token` represents one raw token returned by MeCab.

### `surface`

`surface` is the surface text of the token.

### `feature`

`feature` is the raw MeCab feature string emitted by MeCab's `%H` formatter.

Its exact contents depend on the installed MeCab dictionary.
XER intentionally preserves it as raw text rather than imposing a dictionary-specific public interpretation at this layer.

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
}
```

The exact tokenization and feature strings depend on the installed dictionary.

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

`mecab_token::feature` is dictionary-dependent.

Different MeCab dictionaries may:

- split text differently
- report different feature-column layouts
- produce different readings or base-form fields

The current raw layer intentionally exposes that data without attempting to normalize it.

Higher-level XER Japanese text processing facilities may later define their own supported interpretation strategy where needed.

---

## Current Scope

At the current stage, `<xer/mecab.h>` provides only raw morphological analysis.

Implemented:

- UTF-8 MeCab child-process invocation
- executable-path resolution
- raw token collection
- surface text preservation
- raw feature text preservation

Not yet implemented in this header:

- bunsetsu grouping
- bunsetsu-based spacing
- reading extraction
- hiragana conversion
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
