# Public Headers

## Basic Policy

- Public headers are placed directly under `xer/`.
- Headers under `xer/bits/` are for internal implementation and are not treated as public headers.
- The goal is not to mirror the names of C standard library headers exactly, but to reorganize them into units that are meaningful for xer.
- `wchar.h` and `wctype.h` are not provided as independent headers; their related functionality is absorbed into other headers.
- `math.h`, `statistics.h`, and `complex.h` provide lightweight real-number, statistical, and complex-number mathematical utilities.

---

## Public Headers to Be Provided

### Foundation

- `xer/error.h`
- `xer/assert.h`
- `xer/typeinfo.h`
- `xer/diagnostics.h`
- `xer/scope.h`
- `xer/convert.h`

### Character and String Processing

- `xer/string.h`
- `xer/ctype.h`
- `xer/braille.h`
- `xer/stdlib.h`
- `xer/kansuji.h`
- `xer/mecab.h`
- `xer/furigana.h`
- `xer/ja.h`
- `xer/unicode.h`

### Data Encoding and Format Processing

- `xer/bytes.h`
- `xer/binary.h`
- `xer/base64.h`
- `xer/zip.h`
- `xer/serialize.h`
- `xer/parse.h`
- `xer/json.h`
- `xer/ini.h`
- `xer/toml.h`

### Input/Output and File-Related Facilities

- `xer/stdio.h`
- `xer/iostream.h`
- `xer/path.h`
- `xer/dirent.h`
- `xer/socket.h`
- `xer/tk.h`

### Numeric and Arithmetic Facilities

- `xer/stdint.h`
- `xer/stdfloat.h`
- `xer/arithmetic.h`
- `xer/math.h`
- `xer/statistics.h`
- `xer/complex.h`
- `xer/cyclic.h`
- `xer/interval.h`
- `xer/color.h`
- `xer/quantity.h`
- `xer/matrix.h`
- `xer/image.h`

### Process

- `xer/process.h`
- `xer/cmdline.h`

### Time

- `xer/time.h`

### Version Information

- `xer/version.h`

---

## Headers Not Provided as Independent Public Headers

The following are not provided as separate public headers.
Their necessary functionality is absorbed into existing headers.

- `wchar.h`
  - mainly absorbed into `stdlib.h`, `stdio.h`, and `time.h`
- `wctype.h`
  - mainly absorbed into `ctype.h`

---

## Headers Currently Out of Scope for Reimplementation

At least at the current stage, xer does not reimplement the following headers:

- `fenv.h`
- `float.h`
- `limits.h`
- `locale.h`
- `signal.h`

---

## Mathematical Header Scope

`xer/math.h`, `xer/statistics.h`, and `xer/complex.h` provide lightweight mathematical helpers whose behavior fits xer's explicit error model.

At this stage, the scope is intentionally practical and limited. The real-number helpers are provided through `xer/math.h`, statistical helpers are provided through `xer/statistics.h`, and complex-number helpers are provided through `xer/complex.h` so that users who need only real-number calculations do not have to include complex-number facilities.

---

## Supplement

### Why `locale.h` Is Not Provided

xer minimizes locale dependence as much as possible and handles character classification, character conversion, and character encoding conversion according to its own library policy.
For that reason, `locale.h` is not placed at the core of the public API.

### Why `wchar.h` and `wctype.h` Are Not Separated

xer does not attempt to mimic the header structure of the C standard library as it is.
Instead, it reorganizes APIs by use case.

Accordingly, functionality related to `wchar_t` and wide-character classification is integrated into existing headers as follows:

- character conversion: `stdlib.h`
- text input/output: `stdio.h`
- character classification and character conversion: `ctype.h`
- time-string formatting related facilities: `time.h`

### Why `bytes.h` Is Independent

Byte-sequence conversion helpers are used across several areas, including Base64, binary streams, sockets, and process pipes.

`to_bytes_view` creates a non-owning `std::span<const std::byte>` view from byte-like or text storage without copying. `to_bytes` creates an owning `std::vector<std::byte>` copy.

These helpers are not ordinary string processing because their purpose is to cross the boundary from character storage or byte-like storage into explicit binary bytes. For that reason, they are provided through the independent public header `xer/bytes.h` rather than being absorbed into `xer/string.h`.

### Why `base64.h` Is Independent

Base64 encode/decode is a binary-to-text conversion facility.
It is useful together with text-based data formats, but it is not itself a structured data format like JSON, INI, or TOML.
It is also not ordinary string processing, because its input and output cross the boundary between binary bytes and textual representation.

For that reason, Base64 functionality is not absorbed into `xer/string.h`, `xer/stdlib.h`, or `xer/stdio.h`, but is provided as the independent public header `xer/base64.h`.

This keeps the API boundary clear:

- `base64_encode` converts bytes into UTF-8 text
- `base64_decode` converts UTF-8 Base64 text back into bytes


### Why `zip.h` Is Independent

ZIP archive reading is treated as a compression and archive utility rather than as ordinary file I/O or a structured text format.
It is useful together with serialization and binary data handling, but it has its own container format, compression methods, entry metadata, and sequential read model.

For that reason, ZIP functionality is not absorbed into `xer/stdio.h`, `xer/path.h`, or `xer/base64.h`, but is provided as the independent public header `xer/zip.h`.

At the current stage, this keeps the responsibility clear:

- `zip_open` opens a ZIP archive for reading
- `zip_create` opens a ZIP archive for writing
- `zip_read` reads entry metadata sequentially from the central directory
- `zip_locate_name` and `zip_entry_read_by_name` provide exact-name lookup helpers
- `zip_entry_*` functions obtain metadata, expanded entry data, and extraction behavior through `xer::result`
- `zip_add_from_bytes` and `zip_add_file` add deflated entries to a writer
- `zip_commit` finalizes a writer so finalization errors can be reported
- reaching the end of the entry stream is reported as `error_t::end_of_file`

The initial implementation focuses on ordinary non-ZIP64 single-disk archives. ZIP64, encrypted entries, archive comments, and streaming large-entry I/O are deferred.

### Why `serialize.h` Is Independent

Binary serialization is related to byte sequences and ZIP archives, but it has a different responsibility.
It defines a fixed-schema binary transfer format for scalar values and selected standard containers.

For that reason, serialization is not absorbed into `xer/binary.h`, `xer/bytes.h`, or `xer/zip.h`, but is provided as the independent public header `xer/serialize.h`.

At the current stage, this keeps the responsibility clear:

- `binary_output_archive` appends fixed-schema binary data to an owned byte buffer
- `binary_input_archive` reads fixed-schema binary data from a byte span
- archive `operator()` overloads transfer supported scalar and container types
- user-defined structures are handled by generated `xfer` functions rather than by reflection or runtime schemas

The format stores only data. Type names, field names, schema information, object identifiers, and version metadata are intentionally not written by the low-level archive layer.

### Why `braille.h` Is Independent

Braille handling is Japanese and general text processing, but it has its own notation-specific building blocks such as numeric indicators, alphabetic indicators, capital indicators, and information-processing braille indicators.

For that reason, braille-related facilities are not absorbed into `xer/string.h`, `xer/ctype.h`, or `xer/mecab.h`, but are provided through the independent public header `xer/braille.h`.

At the current stage, this keeps the responsibility clear:

- `xer/ctype.h` classifies Unicode braille pattern characters through `isctype(c, ctype_id::braille)`
- `xer/braille.h` provides reusable braille sign constants as UTF-8 string views
- `xer/braille.h` provides reusable braille sign constants and language-neutral / English low-level helpers under `xer::braille`
- `xer/braille.h` also declares Japanese kana-braille helpers, but those Japanese-specific APIs are placed under `xer::ja`
- MeCab-based higher-level Japanese text conversion builds on these low-level pieces through `xer/mecab.h`


### Why `ja.h` Is Provided

`xer/ja.h` is a convenience umbrella header for Japanese-specific facilities.

It includes the public headers whose primary purpose is Japanese text processing:

- `xer/braille.h`
- `xer/furigana.h`
- `xer/kansuji.h`
- `xer/mecab.h`

The APIs provided by these headers are placed under `xer::ja`. This keeps the
main `xer` namespace focused on language-neutral utilities while allowing xer
to deepen Japanese support before v1.0.0.

---

### Why `kansuji.h` Is Independent

Kansuji handling is Japanese text processing, but it is not an ordinary low-level string algorithm such as search, trim, or replacement.
It defines its own notation families, output styles, normalization rules, and parse failure conditions.

For that reason, Kansuji conversion is not absorbed into `xer/string.h` or `xer/stdlib.h`, but is provided through the independent public header `xer/kansuji.h`.

This keeps the responsibility clear:

- `to_kansuji` converts unsigned integer values into Japanese numeric text
- `from_kansuji` parses practical Japanese numeric text back into `std::uint64_t`

### Why `mecab.h` Is Independent

MeCab-based Japanese text processing is not an ordinary low-level string algorithm.
It involves invoking an external morphological analyzer, validating UTF-8 process I/O, preserving dictionary-dependent feature data, and serving as the foundation for higher-level Japanese text processing.

For that reason, MeCab integration is not absorbed into `xer/string.h`, `xer/process.h`, or `xer/stdlib.h`, but is provided through the independent public header `xer/mecab.h`.

At the current stage, this keeps the responsibility clear:

- `mecab_parse` invokes MeCab and returns raw morphological token results
- `mecab_options` controls the MeCab executable path when automatic `PATH` lookup is not sufficient
- `mecab_token` preserves the surface text, raw MeCab feature text, and split feature fields through `mecab_features`
- `mecab_split_phrases` derives practical bunsetsu-like ranges and symbol ranges from a token sequence
- `mecab_to_kana` converts MeCab-derived readings to kana text
- `mecab_kana_wakati` produces kana wakachi-gaki text using the derived phrase ranges
- `mecab_romaji_wakati` produces romaji wakachi-gaki text by combining kana conversion and `strtoctrans`
- `mecab_braille_wakati` and `mecab_ip_braille_wakati` produce practical braille-oriented wakachi-gaki text
- `mecab_braille_translate` and `mecab_ip_braille_translate` parse source text and directly produce braille-oriented output

Future higher-level facilities such as ruby formatting, counting helpers, and more precise braille refinements can build on this foundation.

### Why `furigana.h` Is Independent

Furigana formatting is a small but distinct Japanese text presentation facility.
It does not perform morphological analysis by itself, and it is not merely a generic string operation such as trimming or case conversion.

For that reason, furigana formatting is not absorbed into `xer/string.h` or `xer/mecab.h`, but is provided through the independent public header `xer/furigana.h`.

This keeps the responsibility clear:

- `to_furigana` formats already-known base text and reading pairs
- `ruby_html` selects HTML ruby markup
- `ruby_paren` selects a simple parenthesized representation

Future MeCab-based automatic furigana helpers can reuse this formatter without making the low-level formatting API depend on MeCab process execution.

### Why `convert.h` Is Independent

Generic value conversion is broader than ordinary numeric parsing and broader than text formatting.
It connects numeric values, explicitly encoded character strings, character values, and selected xer value types through one `xer::to<T>` entry point.

For that reason, generic conversion is not absorbed into `xer/stdlib.h`, `xer/string.h`, or `xer/stdio.h`, but is provided through the independent public header `xer/convert.h`.

This keeps the responsibility clear:

- `xer::to<T>` returns `xer::result<T>` because parsing, range validation, and formatting may fail
- arithmetic-to-arithmetic conversion is range-checked by `xer::in_range`
- `char` strings are not interpreted as text because their encoding is ambiguous
- `char8_t`, `char16_t`, `char32_t`, and `wchar_t` strings are treated as explicitly encoded text
- conversion to `xer::path` accepts explicitly encoded text and normalizes path separators through `xer::path`

Future locale-dependent or native-code-page conversion helpers can be added separately without weakening this header's encoding rule.

### Why `unicode.h` Is Independent

Unicode normalization is a text-processing facility, but it depends on the external ICU C API and is heavier than ordinary low-level string operations.

For that reason, Unicode normalization is not absorbed into `xer/string.h` or `xer/ctype.h`, but is provided through the independent public header `xer/unicode.h`.

At the current stage, this keeps the responsibility clear:

- `normalize_nfc` converts valid UTF-8 text to Unicode Normalization Form C
- `is_normalized_nfc` checks whether valid UTF-8 text is already NFC
- including `xer/unicode.h` requires ICU development headers and ICU libraries at link time

Future normalization forms or ICU-based Unicode helpers can build on this header without making the ordinary string headers depend on ICU.

### Why `json.h` Is Independent

JSON encode/decode is not merely string manipulation.
It is an independent data-format facility involving arrays, objects, booleans, numbers, and `null`.

For that reason, it is not absorbed into `string.h` or `stdlib.h`, but instead provided as the independent public header `xer/json.h`.

### Why `ini.h` Is Independent

INI encode/decode is a small but distinct data-format facility.

Although INI values are represented as strings, the format still has its own file-level structure, including global entries, sections, key-value entries, comments, and serialization rules.
For that reason, it is not absorbed into `xer/string.h` or `xer/stdio.h`, but instead provided as the independent public header `xer/ini.h`.

This also keeps it parallel with `xer/json.h` and leaves room for `xer/toml.h` to be added later as another data-format header.

### Why `toml.h` Is Independent

TOML decode/encode is an independent data-format facility for configuration data.

Unlike INI, TOML has typed values such as booleans, integers, floating-point numbers, strings, arrays, and tables.
It is therefore not merely string processing.
For that reason, TOML functionality is not absorbed into `xer/string.h`, `xer/stdlib.h`, or `xer/stdio.h`, but is provided as the independent public header `xer/toml.h`.

This keeps TOML parallel with `xer/json.h` and `xer/ini.h` as a data-format header.

### Why `dirent.h` Is Independent

Directory stream operations such as `opendir`, `closedir`, `readdir`, and `rewinddir` form a small but distinct group of filesystem traversal facilities.

Although they are related to file handling, they manage a directory stream state rather than an ordinary file stream.
For that reason, they are provided through the independent public header `xer/dirent.h` rather than being absorbed into `xer/stdio.h`.

### Why `iostream.h` Is Independent

`xer/iostream.h` provides opt-in iostream insertion and extraction operators for selected xer value types.

xer's ordinary input/output model remains based on `xer/stdio.h`, `binary_stream`, and `text_stream`. However, iostream operators are useful as a bridge for diagnostics, tests, examples, and generic `%@` formatting and scanning support.

For that reason, iostream support is provided through the independent public header `xer/iostream.h` rather than being included automatically from each value-type header.

### Why `interval.h` Is Independent

Interval values are bounded scalar value types distinct from ordinary arithmetic helpers.
They are especially useful for normalized values such as color components, alpha values, gain values, and ratios.

Although interval values are numeric, their main role is to preserve the invariant that the stored value remains inside a fixed closed interval.
For that reason, interval functionality is not absorbed into `xer/arithmetic.h`, but is provided as the independent public header `xer/interval.h`.

### Why `quantity.h` Is Independent

Physical quantity and unit facilities are a kind of type-system-oriented feature distinct from string handling, input/output, and arithmetic helpers.

Also, concepts such as `dimension`, `unit`, `quantity`, and `xer::units` are easier for users to understand when treated as one coherent group.

For that reason, physical quantity and unit functionality is not absorbed into `xer/arithmetic.h`, but is provided as the independent public header `xer/quantity.h`.

### Why `matrix.h` Is Independent

Matrix and affine transform facilities form a small but coherent numeric feature group.
They are related to arithmetic helpers, but their main concepts are fixed-size matrices, column vectors, and transform construction rather than scalar arithmetic.

For that reason, matrix functionality is not absorbed into `xer/arithmetic.h`, but is provided as the independent public header `xer/matrix.h`.

### Why `image.h` Is Independent

Image and framebuffer facilities form a small but coherent feature group.
They are related to color handling and Tk GUI integration, but their main concepts are logical pixels, framebuffer storage policies, fixed-size images, dynamic-size images, and drawing helpers.

For that reason, image functionality is not absorbed into `xer/color.h` or `xer/tk.h`, but is provided as the independent public header `xer/image.h`.
`xer/tk.h` may provide bridge functions for Tk photo images, but pure image storage, drawing, and image processing belong to `xer/image.h`.
