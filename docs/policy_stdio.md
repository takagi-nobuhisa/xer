# Policy for Input and Output

## Basic Policy

XER input and output is designed on top of C standard-library `FILE`-based functions.

However, the public API is redesigned as a C++ library and does not aim for full source-level compatibility with C.

- the low-level foundation uses input/output based on the `FILE` type
- `iostream` is not used
- high-level input/output functions are built on top of this low-level foundation

The main low-level functions to be used are as follows:

- `fopen`
- `fclose`
- `fseek`
- `ftell`
- `setvbuf`
- `feof`
- `ferror`
- `fread`
- `fwrite`

## Public Types

The public API does not expose `FILE` directly.
Instead, the following types are used according to purpose:

- `binary_stream`
- `text_stream`

These two are clearly separated, and are not designed as a single type with mode switching.

These stream types are intended to handle at least the following kinds of targets in a unified way:

- files
- in-memory data
- strings
- standard input
- standard output
- standard error

## Design Policy for `binary_stream` / `text_stream`

### Move-Only

`binary_stream` and `text_stream` are non-copyable and movable only.

### RAII

These stream types manage open / close by RAII.

Here, RAII includes lifetime management of the internal state owned by the stream.
Accordingly, internal state allocated by open functions is released by `fclose` or by the destructor.

### Destructor

The destructor performs automatic close.

- internal state created by open functions is destroyed after performing processing equivalent to `close` in the destructor
- even if `close` fails, that failure is not reported from the destructor
- reusing the same stream object after destruction is not considered

### Explicit Close

`fclose` is also provided as an explicit operation.

- after `fclose`, the stream is treated as closed regardless of success or failure
- double close is a no-op
- reopening the same stream object is not allowed
- if reuse is needed, a newly opened stream should be moved into place

### Closed State

Operations on a closed stream result in failure.

At present, such failures may primarily be normalized to `error_t::io_error`.

## Internal Implementation Policy

Each stream type is a lightweight handle type that internally stores the following:

- an opaque handle value
- function pointers such as `close`, `read`, `write`, `seek`, and `tell`

The opaque handle value uses `std::uintptr_t`.

This handle is intended to store an internal state object, a `FILE*`, or a similar entity by means of `reinterpret_cast`.

### `binary_stream`

`binary_stream` stores at least the following internal function pointers:

- `close`
- `read`
- `write`
- `seek`
- `tell`

### `text_stream`

`text_stream` stores at least the following internal function pointers:

- `close`
- `read`
- `write`
- `getpos`
- `setpos`
- `seek_end`

Here, `read` and `write` mean entry points for text character input/output.
At present, `fopen` itself is not required to complete those higher-level operations; the actual work may be performed by higher-level functions corresponding to `fgetc` / `fputc` and similar facilities.

### Default Error Functions

For empty streams or unsupported operations, `nullptr` should not be used.
Instead, an error function prepared for each signature should be installed.

This avoids requiring the caller to check for `nullptr` before calling through the operation entry points.

## Open Functions

### Files

Functions that open files are separated into binary and text variants.

- binary open functions return `binary_stream`
- text open functions return `text_stream`

The signatures are as follows:

- `auto fopen(const path& filename, const char* mode) noexcept -> xer::result<binary_stream>;`
- `auto fopen(const path& filename, const char* mode, encoding_t encoding) noexcept -> xer::result<text_stream>;`

### Memory

Functions that open memory are also separated by purpose.

- `memopen` is for binary use and returns `binary_stream`
- `stropen` is for text use and returns `text_stream`

At present, these are treated as borrowing externally owned storage.

- `memopen(std::span<std::byte>, const char* mode)`
- `stropen(std::u8string_view, const char* mode)`
- `stropen(std::u8string&, const char* mode)`

To preserve symmetry with file opening, open functions do not take ownership of containers.

## Mode Specification

Mode strings use `char` strings, as in C.

Examples:

- `"r"`
- `"w"`
- `"a"`
- `"rb"`
- `"wb"`
- `"r+"`

Mode strings are control tokens rather than text data themselves, so `char8_t`-based types are not used for them.

Exclusive-creation modes such as `"x"` are unsupported at present.

### Handling of `b` / `t`

- in binary open functions, `b` is optional and `t` is invalid
- in text open functions, `t` is optional and `b` is invalid
- variation in the position of `b` / `t` is allowed
- the kind of open function is determined by overload resolution itself, and `b` / `t` is treated only as an auxiliary validation token

### Internal Normalization

Mode strings are normalized internally into the following enumeration values:

- `r`
- `rp`
- `w`
- `wp`
- `a`
- `ap`
- `error`

Here, `p` is the internal representation of `+`.

This normalized result is temporary information used at open time and is not stored inside the stream.
Behavior after opening is determined by the combination of function pointers and internal state.

## Policy for Binary Streams

### Basic Policy

Binary data is handled as `std::byte`.

### Functions

The following functions are reimplemented under the same names:

- `fread`
- `fwrite`

For one-byte input/output, the following are used:

- `fgetb`
- `fputb`

`fgetc` and `fputc` are names for character-oriented operations and are therefore not used for one-byte binary I/O.

### Handling of EOF

When `fgetb` reaches EOF, that one-byte input operation is treated as failure and handled as an error.

### Position Operations

`binary_stream` provides ordinary seek / tell operations in byte units.

### Internal State

The internal state of `binary_stream` should be able to store at least one of the following:

- a file source holding a `FILE*`
- a memory source holding a byte-sequence region and a current position

Whether reading is allowed, whether writing is allowed, and append behavior may also be stored in the internal state.

## Policy for Text Streams

### Basic Policy

In text mode, UTF-8 and CP932 are handled.
There is no dependence on locale.

The external encodings for text input/output are limited to the following:

- UTF-8
- CP932

The values handled internally are normalized as follows:

- single-character input uses `char32_t`
- string input uses UTF-8 `char8_t` strings

Whether the actual source encoding is UTF-8 or CP932, the representation seen from higher-level APIs is unified.

### Encoding Specification for Text Input

The encoding specification for text input is as follows:

- `utf8`
- `cp932`
- `auto_detect`

Automatic detection is performed only when `auto_detect` is specified.

### Encoding Specification for Text Output

At present, the text open functions also accept the following three encoding designators:

- `utf8`
- `cp932`
- `auto_detect`

However, `auto_detect` is input-only and is invalid in write-oriented modes.

Whether BOM is present, and distinctions such as UTF-8N, are not yet fixed as part of the public specification.
Accordingly, they are not included in the current `encoding_t`.

### `text_stream_state`

A `text_stream_state` object is used behind the handle of `text_stream`.

It stores at least the following:

- the underlying I/O target
  - `FILE*`
  - a read-only string view
  - a writable string
- a lookahead buffer
  - 1024 bytes
- the encoding state, whether resolved or unresolved
- an intermediate state corresponding to `mbstate_t`

### Lookahead Buffer

For automatic detection and multibyte character processing, `text_stream_state` contains a 1024-byte lookahead buffer.

### `mbstate_t`-Like Intermediate State

To handle partial UTF-8 or CP932 state, `text_stream_state` contains an internal state corresponding to `mbstate_t`.

At present, it is sufficient if it can store incomplete UTF-8 sequences and partial CP932 state.

## Policy for Automatic Detection of Text Input Encoding

### Basic Policy

When `auto_detect` is specified, automatic detection between UTF-8 and CP932 is performed.

The possibility of misdetection cannot be reduced to zero, so some nonzero probability of misdetection is acceptable.

### Detection Rules

Detection is performed in the following priority order:

1. if a UTF-8 BOM exists, treat the input as UTF-8
2. if there is no BOM and a valid CP932 multibyte sequence is found, treat the input as CP932
3. otherwise, treat the input as UTF-8

However, if only ASCII has been observed so far, the encoding is not yet considered definitively resolved to UTF-8.

### How Detection Proceeds

Automatic detection is not a one-shot process used only for lookahead.
Instead, it proceeds as input advances.

Internally, buffering in units of about 1 KiB is permitted.

- if 1 KiB is read and there is enough information to determine the encoding, it is resolved
- if only ASCII is found, the state remains unresolved and input proceeds
- detection may continue on the next 1 KiB block as well
- if the encoding is still unresolved at EOF, it is treated as UTF-8

### Handling of the Unresolved State

In the unresolved state, ASCII should be readable without problems.

This is because ASCII is the common subset of UTF-8 and CP932.

Only when a non-ASCII byte appears does the implementation proceed with deciding whether the input should be interpreted as UTF-8 or as CP932.

For the purpose of this detection, the number of bytes that may be held back while unresolved is at most one byte.

Here, “held back” means holding bytes only for deciding whether the stream should be interpreted as UTF-8 or as CP932, not for validating the completeness of a UTF-8 character.

Whether a sequence is valid as a 3-byte or 4-byte UTF-8 character is handled later by the ordinary decoding process after the encoding has been resolved as UTF-8.

### Handling of Invalid Input

If, not merely because of buffering, the input truly contains only one half of a multibyte character, it is an error.

Likewise, once the encoding has been resolved as UTF-8, any sequence invalid as UTF-8 is an error.

The same applies after resolution as CP932: any sequence invalid as CP932 is also an error.

## Return Value Policy

Ordinary failure in input/output functions is represented, as a rule, by `xer::result`.

For example, open functions, close functions, read / write functions, and position functions should generally return `xer::result<T>` or `xer::result<void>` rather than special values.

`xer::result<T>` is implemented as `std::expected<T, error<void>>`, and the design may be extended to use `error<Detail>` when needed.

For I/O-originated failures that are not mapped in detail to a specific `errno`, `error_t::io_error` may be used.

---

## Supplement

Although XER input/output is built on C `FILE`, its public design is reconstructed as a C++ library.

Accordingly, the following priorities are emphasized:

- do not expose `FILE*` directly
- use type-safe stream types
- manage lifetime by RAII
- represent ordinary failure with `xer::result` rather than with special values
- handle character encoding according to XER's own policy rather than locale
