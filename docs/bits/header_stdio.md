# `<xer/stdio.h>`

## Purpose

`<xer/stdio.h>` provides stream-based input and output facilities in XER.

Its role is similar in spirit to the C standard library `<stdio.h>`, but it is not intended to be a literal reproduction.
Instead, it reconstructs practical I/O around explicit stream types, explicit encodings, and XER's ordinary failure model.

This header is one of the most important public headers in XER because it provides the main user-facing path for:

- binary stream I/O
- text stream I/O
- formatted input/output
- file-entry operations
- CSV input/output
- stream state and positioning
- stream rewinding
- stream content convenience operations
- whole-file content convenience operations

---

## Main Role

The main role of `<xer/stdio.h>` is to provide a coherent I/O model built on the following principles:

- do not expose `FILE*` directly as the public abstraction
- distinguish binary I/O from text I/O explicitly
- use RAII for stream lifetime management
- handle text encodings explicitly rather than through locale
- report ordinary failure through `xer::result`

This makes the header easier to use safely from modern C++ code while preserving the familiarity of many classic C-style function names.

---

## Core Stream Types

At minimum, `<xer/stdio.h>` provides the following public stream types:

```cpp
class binary_stream;
class text_stream;
```

These are the central abstractions of the header.

### `binary_stream`

`binary_stream` represents binary input/output.

It is used for:

* files opened in binary mode
* memory-backed binary streams
* binary temporary streams
* other byte-oriented stream targets

### `text_stream`

`text_stream` represents text input/output.

It is used for:

* files opened with explicit text encoding
* UTF-8 or CP932 text sources
* string-backed text streams
* text temporary streams
* standard text-oriented input/output targets

### Design Direction

These two stream types are intentionally separate.

XER does not model them as one stream class with a mode switch.
Instead, the distinction between binary and text I/O is made explicit at the type level.

---

## Move-Only RAII Objects

`binary_stream` and `text_stream` are move-only RAII objects.

### Meaning

This implies at least the following:

* they are not copyable
* they are movable
* they acquire and release stream resources through object lifetime
* the destructor performs automatic cleanup

### Why This Matters

This makes stream ownership explicit and avoids many ambiguities associated with raw handle sharing.

It also fits XER's broader design preference for explicit ownership and explicit failure handling.

---

## Stream Opening

`<xer/stdio.h>` provides functions for opening streams from files, memory, and strings.

### File Opening

At minimum, the public file-opening forms are:

```cpp
auto fopen(const path& filename, const char* mode) noexcept -> xer::result<binary_stream>;
auto fopen(const path& filename, const char* mode, encoding_t encoding) noexcept -> xer::result<text_stream>;
```

These two overloads separate binary opening from text opening.

### Memory Opening

For memory-backed streams, the header may provide forms such as:

```cpp
auto memopen(std::span<std::byte> memory, const char* mode) noexcept -> xer::result<binary_stream>;
auto stropen(std::u8string_view text, const char* mode) noexcept -> xer::result<text_stream>;
auto stropen(std::u8string& text, const char* mode) noexcept -> xer::result<text_stream>;
```

### Design Direction

These open functions are designed so that:

* binary streams and text streams are distinguished at open time
* ordinary ownership of the backing container is not silently transferred
* borrowed storage remains explicit in the API shape

---

## Text Encoding Selection

For text streams, `<xer/stdio.h>` provides explicit encoding selection.

At minimum, the public encoding enumeration is:

```cpp
enum class encoding_t {
    utf8,
    cp932,
    auto_detect,
};
```

### Meaning

* `utf8` means UTF-8 text
* `cp932` means CP932 text
* `auto_detect` means automatic input-side detection between supported encodings

### Important Notes

* text I/O in XER is not locale-centered
* encoding is part of the stream-opening model
* `auto_detect` is intended for input, not for general write-side behavior

This is one of the clearest differences from traditional locale-driven C text I/O.

---

## Binary I/O

For binary streams, the header provides byte-oriented operations.

At minimum, this includes functions such as:

```cpp
fread
fwrite
fgetb
fputb
```

### Role of This Group

These functions provide:

* block input/output
* single-byte input/output
* binary-stream operations in byte units

### Design Direction

Binary data is handled as raw byte-oriented data rather than as text.

`fgetc` and `fputc` are therefore not the single-byte binary I/O interface.
Instead, XER uses `fgetb` and `fputb` for that role.

---

## Text I/O

For text streams, the header provides character- and string-oriented operations.

At minimum, this includes functions such as:

```cpp
fgetc
getchar
ungetc

fputc
putchar

fgets
gets
fputs
puts
```

### Role of This Group

These functions provide:

* single-character text input/output
* string-oriented text input/output
* standard-stream convenience operations
* limited push-back support through `ungetc`

### Design Direction

Text streams are normalized internally around XER's text model.

In particular:

* single-character input is centered on `char32_t`
* string-oriented text handling is centered on UTF-8 `char8_t` strings

The exact visible overload set depends on the implementation, but the conceptual model remains the same.

---

## Formatted Input and Output

`<xer/stdio.h>` also provides formatted I/O facilities.

At minimum, this includes the following families:

```cpp
fprintf
sprintf
snprintf
printf
fscanf
sscanf
scanf
```

### Role of This Group

These functions provide familiar formatted I/O in a style approachable to users familiar with C.

### Design Direction

Although the naming resembles the standard library, the surrounding design is XER's own:

* stream types are explicit
* text model is UTF-8-oriented
* ordinary failure is reported through XER-style result handling where applicable
* integration with XER stream abstractions takes priority over strict source-level emulation of C

### printf Format Details

<!-- XER_INCLUDE: stdio_printf_format.md -->

### scanf Format Details

<!-- XER_INCLUDE: stdio_scanf_format.md -->

---

## CSV Support

`<xer/stdio.h>` also includes CSV-oriented helpers:

```cpp
fgetcsv
fputcsv
```

### Role of This Group

These functions provide convenient CSV input and output on top of XER streams.

They are particularly useful because CSV is a text-oriented format that benefits from integration with:

* explicit text encodings
* explicit stream ownership
* UTF-8-oriented strings

### Design Direction

These functions are not merely string helpers.
They belong naturally in the I/O layer because they operate on streams and formatted text records.

---

## Position and Stream State Helpers

At minimum, `<xer/stdio.h>` provides helpers such as:

```cpp
fseek
ftell
fgetpos
fsetpos
feof
ferror
clearerr
```

and the related public types:

```cpp
enum seek_origin_t { seek_set, seek_cur, seek_end };
using fpos_t = std::uint64_t;
```

### Role of This Group

These facilities provide:

* byte- or position-oriented stream movement
* stream status inspection
* stream error-state control
* explicit text-stream position handling

### Binary vs Text Positioning

The basic intended distinction is:

* `fseek` / `ftell` are the ordinary position helpers for `binary_stream`
* `fgetpos` / `fsetpos` are the primary position helpers for `text_stream`

This reflects the fact that text streams may not always map cleanly to simple byte offsets after decoding and buffering.

---

## Rewinding

`<xer/stdio.h>` provides `rewind` for both stream kinds:

```cpp
auto rewind(binary_stream& stream) noexcept -> xer::result<void>;
auto rewind(text_stream& stream) noexcept -> xer::result<void>;
```

Unlike the C standard-library function, XER's `rewind` returns `xer::result<void>` so that invalid streams and seek failures can be reported explicitly.

For text streams, rewinding also clears pushed-back characters, lookahead bytes, and partial decoding state. If the stream was opened with `encoding_t::auto_detect`, the concrete encoding is returned to the undecided state.

---

## Whole-Stream Convenience Operations

`<xer/stdio.h>` provides whole-stream convenience operations:

```cpp
auto stream_get_contents(
    binary_stream& stream,
    std::uint64_t length = std::numeric_limits<std::uint64_t>::max())
    -> xer::result<std::vector<std::byte>>;

auto stream_get_contents(text_stream& stream)
    -> xer::result<std::u8string>;

auto stream_put_contents(
    binary_stream& stream,
    std::span<const std::byte> contents)
    -> xer::result<void>;

auto stream_put_contents(
    text_stream& stream,
    std::u8string_view contents)
    -> xer::result<void>;
```

### Purpose

`stream_get_contents` and `stream_put_contents` provide compact helpers for reading from and writing to an already-open XER stream.

They are the stream-level counterparts of `file_get_contents` and `file_put_contents`.

Because they operate on streams rather than file names, they can be used with any stream source or destination supported by XER, including files, temporary files, memory streams, string streams, process pipes, and socket-derived streams where applicable.

### Binary `stream_get_contents`

```cpp
auto stream_get_contents(
    binary_stream& stream,
    std::uint64_t length = std::numeric_limits<std::uint64_t>::max())
    -> xer::result<std::vector<std::byte>>;
```

This overload reads binary data from the current position of `stream`.

It reads at most `length` bytes, or stops earlier if EOF is reached.

If `length` is zero, the function succeeds and returns an empty byte vector.

### No Offset Argument

XER intentionally does not provide an offset parameter for `stream_get_contents`.

A stream already has a current position. If the caller needs to choose the starting position, the caller should use `fseek`, `fsetpos`, or another appropriate positioning function explicitly before calling `stream_get_contents`.

This also avoids the confusing argument-order difference found in PHP, where `file_get_contents` and `stream_get_contents` place offset and length differently.

In XER, the rule is simple:

* `file_get_contents` may take an offset because it opens the file internally
* `stream_get_contents` reads from the stream's current position

### Text `stream_get_contents`

```cpp
auto stream_get_contents(text_stream& stream)
    -> xer::result<std::u8string>;
```

This overload reads text from the current position of `stream` until EOF.

The returned string is UTF-8 text.

The stream's own encoding state controls how external bytes are decoded.

Text-mode `stream_get_contents` does not provide `offset` or `length` arguments because byte offsets, decoded characters, line ending behavior, and encoding state can otherwise become ambiguous.

### Binary `stream_put_contents`

```cpp
auto stream_put_contents(
    binary_stream& stream,
    std::span<const std::byte> contents)
    -> xer::result<void>;
```

This overload writes all bytes in `contents` to the current position of `stream`.

The exact placement behavior is determined by the stream's current position and open mode.

For example, if the stream was opened in append mode, the write follows the stream's append behavior.

### Text `stream_put_contents`

```cpp
auto stream_put_contents(
    text_stream& stream,
    std::u8string_view contents)
    -> xer::result<void>;
```

This overload writes the UTF-8 text in `contents` to the current position of `stream`.

The stream's encoding controls how the UTF-8 text is encoded externally.

### Relationship to File Convenience Functions

`file_get_contents` and `file_put_contents` are file-opening convenience wrappers around these stream-level helpers.

Conceptually:

```cpp
auto stream = xer::fopen(filename, "r");
return xer::stream_get_contents(*stream);
```

or:

```cpp
auto stream = xer::fopen(filename, "w");
return xer::stream_put_contents(*stream, contents);
```

The stream-level functions contain the reusable read/write logic, while the file-level functions handle opening the file and applying file-specific options such as the binary `offset` argument.

### Error Handling

These functions follow XER's ordinary failure model.

On success:

* `stream_get_contents` returns the read data
* `stream_put_contents` returns an empty success value

On failure, they return an error through `xer::result`.

Typical failure conditions include:

* the stream is not readable or writable for the requested operation
* reading or writing fails
* text decoding or encoding fails
* memory allocation fails while collecting the result

### Example

```cpp
std::u8string buffer;

auto stream = xer::stropen(buffer, "w+");
if (!stream.has_value()) {
    return 1;
}

const auto written = xer::stream_put_contents(
    *stream,
    std::u8string_view(u8"hello XER"));

if (!written.has_value()) {
    return 1;
}

const auto rewound = xer::rewind(*stream);
if (!rewound.has_value()) {
    return 1;
}

const auto text = xer::stream_get_contents(*stream);
if (!text.has_value()) {
    return 1;
}
```

---

## Closing and Flushing

This header also provides operations such as:

```cpp
fclose
fflush
tmpfile
```

and a text-oriented temporary-file overload or equivalent helper for explicitly encoded text streams.

### Role of This Group

These functions support:

* explicit closing
* explicit flush control
* temporary stream creation

### Design Direction

Even though stream destructors perform automatic cleanup, explicit close and flush operations still matter because:

* callers may want deterministic resource release
* callers may want explicit error observation before destruction
* explicit flushing is part of normal stream control

---

## File-Entry Operations

`<xer/stdio.h>` also provides file-entry operations such as:

```cpp
file_exists
is_file
is_dir
is_readable
is_writable

remove
rename
mkdir
rmdir
copy
touch

chdir
getcwd
realpath

file_get_contents
file_put_contents
```

### Role of This Group

These functions operate on filesystem entries rather than on open stream objects.

They are grouped here because they are operationally close to stream/file handling.

### Design Direction

These functions are intentionally separate from stream objects themselves.

They typically operate on `xer::path`, not on raw native path strings.

This aligns them with XER's own path model, where path values are represented internally as UTF-8 strings with `/` as the normalized separator.

Some functions in this group are simple predicates, while others perform actual filesystem operations.

Predicate functions such as `file_exists`, `is_file`, `is_dir`, `is_readable`, and `is_writable` return `bool`.

Operations that can fail normally return `xer::result`.

---

## Current Working Directory Operations

`<xer/stdio.h>` provides current-working-directory helpers:

```cpp
auto chdir(const path& target) -> xer::result<void>;
auto getcwd() -> xer::result<path>;
```

### `chdir`

`chdir` changes the process-wide current working directory.

```cpp
auto chdir(const path& target) -> xer::result<void>;
```

The argument is a `xer::path`.

On success, the function returns an empty success value.
On failure, it returns an error through `xer::result`.

Because the current working directory is process-wide state, callers should use this function carefully in programs where multiple components or threads may depend on the current directory.

### `getcwd`

`getcwd` returns the current working directory.

```cpp
auto getcwd() -> xer::result<path>;
```

The returned value is a `xer::path`.

The path is converted into XER's internal UTF-8 representation and uses `/` as the normalized separator.

The result is a snapshot of the process-wide current working directory at the time of the call.

---

## `realpath`

```cpp
auto realpath(const path& filename) -> xer::result<path>;
```

### Purpose

`realpath` returns the canonicalized absolute path of an existing filesystem entry.

It queries the actual filesystem through the platform path canonicalization mechanism.

### Behavior

The target path must exist.

Relative path components are resolved.
Symbolic links and other filesystem-level indirections are resolved according to the behavior of the underlying platform.

On POSIX-like environments, the behavior follows the platform `realpath` facility.
On Windows, the implementation uses Windows path canonicalization facilities and converts the result back into XER's path representation.

### Return Value

On success, `realpath` returns a `xer::path`.

The returned path:

* is absolute
* refers to an existing filesystem entry
* is converted to XER's UTF-8 path representation
* uses `/` as the internal separator

On failure, it returns an error through `xer::result`.

Typical failure conditions include:

* the target path does not exist
* the caller lacks permission to access the path
* native path conversion fails
* platform path canonicalization fails

### Difference from Lexical Path Operations

`realpath` is not a purely lexical path operation.

It depends on the actual filesystem and may observe filesystem state such as symbolic links, mounted volumes, permissions, and existing entries.

For purely lexical path manipulation, use path helpers such as `basename`, `parent_path`, `extension`, `stem`, `is_absolute`, and `is_relative`.

### Example

```cpp
const auto resolved = xer::realpath(xer::path(u8"."));
if (!resolved.has_value()) {
    return 1;
}
```

After success, `resolved` contains the canonicalized absolute path of the current directory.

---

## Whole-File Convenience Operations

`<xer/stdio.h>` provides PHP-inspired whole-file convenience operations:

```cpp
auto file_get_contents(
    const path& filename,
    std::uint64_t offset = 0,
    std::uint64_t length = std::numeric_limits<std::uint64_t>::max())
    -> xer::result<std::vector<std::byte>>;

auto file_get_contents(
    const path& filename,
    encoding_t encoding)
    -> xer::result<std::u8string>;

auto file_put_contents(
    const path& filename,
    std::span<const std::byte> contents)
    -> xer::result<void>;

auto file_put_contents(
    const path& filename,
    std::u8string_view contents,
    encoding_t encoding)
    -> xer::result<void>;
```

### Purpose

`file_get_contents` and `file_put_contents` provide compact helpers for reading and writing an entire file without manually opening a stream.

They are file-opening convenience wrappers around `stream_get_contents` and `stream_put_contents`. The reusable read/write behavior belongs to the stream-level helpers, while the file-level helpers additionally open the target file and apply file-specific options.

They are inspired by PHP functions of the same names, but their behavior follows XER's stream and encoding model.

### Binary and Text Selection

The overload set uses the presence or absence of an `encoding_t` argument to select binary or text behavior.

* when no encoding is specified, the file is handled through `binary_stream`
* when an encoding is specified, the file is handled through `text_stream`

This keeps the call site explicit without introducing a separate mode flag.

### Binary `file_get_contents`

```cpp
auto file_get_contents(
    const path& filename,
    std::uint64_t offset = 0,
    std::uint64_t length = std::numeric_limits<std::uint64_t>::max())
    -> xer::result<std::vector<std::byte>>;
```

This overload opens the file as binary and returns its contents as `std::vector<std::byte>`.

The optional `offset` and `length` arguments are byte-based.

If `offset` is greater than the file size, the function returns `error_t::invalid_argument`.

If `offset` is exactly equal to the file size, the function succeeds and returns an empty byte vector.

If `length` is zero, the function succeeds and returns an empty byte vector.

### Text `file_get_contents`

```cpp
auto file_get_contents(
    const path& filename,
    encoding_t encoding)
    -> xer::result<std::u8string>;
```

This overload opens the file as text and returns its contents as UTF-8 text.

The specified encoding controls how the external file bytes are decoded.

`encoding_t::auto_detect` is valid for this input-side operation.

Text-mode `file_get_contents` does not provide `offset` or `length` arguments because byte offsets, decoded characters, line ending behavior, and encoding state can otherwise become ambiguous.

### Binary `file_put_contents`

```cpp
auto file_put_contents(
    const path& filename,
    std::span<const std::byte> contents)
    -> xer::result<void>;
```

This overload opens the file as binary and writes all bytes in `contents`.

Existing file contents are replaced.

### Text `file_put_contents`

```cpp
auto file_put_contents(
    const path& filename,
    std::u8string_view contents,
    encoding_t encoding)
    -> xer::result<void>;
```

This overload opens the file as text and writes the UTF-8 text in `contents` using the specified output encoding.

`encoding_t::auto_detect` is invalid for writing and results in `error_t::invalid_argument`.

### Why PHP-Style Flags Are Not Provided

XER intentionally does not provide PHP-style `flags` arguments for these functions.

In particular, append behavior and locking behavior are not hidden inside `file_put_contents`.

If file locking is required, the caller should perform it explicitly with an outer operation such as `flock`.

If append-style output is required, the caller should use stream APIs directly, such as opening a stream with append mode and writing with `fwrite` or `fputs`.

If append-style output is required, the caller should use stream APIs directly, such as opening a stream with append mode and writing with `fwrite`, `fputs`, or `stream_put_contents`.

### Error Handling

These functions follow XER's ordinary failure model.

On success:

* `file_get_contents` returns the read data
* `file_put_contents` returns an empty success value

On failure, they return an error through `xer::result`.

Typical failure conditions include:

* the file cannot be opened
* seeking fails
* reading or writing fails
* `offset` is invalid
* `encoding_t::auto_detect` is used for text output
* text decoding or encoding fails

### Example

```cpp
const auto text = xer::file_get_contents(
    xer::path(u8"sample.txt"),
    xer::encoding_t::utf8);

if (!text.has_value()) {
    return 1;
}

const auto written = xer::file_put_contents(
    xer::path(u8"copy.txt"),
    *text,
    xer::encoding_t::utf8);

if (!written.has_value()) {
    return 1;
}
```

---

## Native Handle Access

The header may also expose support related to native-handle access.

### Role

This exists for cases where callers need to bridge XER stream abstractions to lower-level platform or runtime facilities.

### Design Direction

Such support is supplementary rather than central.
The normal user-facing abstraction remains `binary_stream` or `text_stream`, not the native handle.

---

## Internal Design Direction

Although `<xer/stdio.h>` is a public header, its conceptual design depends on the following ideas:

* stream objects are lightweight public handles
* internal state is hidden behind those handles
* binary and text streams may use function-pointer-based internal dispatch
* text-stream state may include buffering, encoding resolution, and multibyte intermediate state

These implementation ideas are important to understand the shape of the public API, even though the internal structures themselves are not the public abstraction.

---

## Relationship to XER's Text Model

`<xer/stdio.h>` is one of the headers most tightly coupled to XER's overall text model.

In particular:

* UTF-8 is the primary public string representation
* `char32_t` is used for individual text characters where appropriate
* text streams support UTF-8 and CP932 explicitly
* automatic text input detection is limited and explicit

This means that `<xer/stdio.h>` should always be read together with the broader encoding-related policies.

---

## Relationship to Other Headers and Policies

`<xer/stdio.h>` should be understood together with:

* `policy_project_outline.md`
* `policy_stdio.md`
* `policy_encoding.md`
* `header_path.md`
* `header_stdlib.md`

The rough boundary is:

* `<xer/path.h>` handles lexical path representation and native-path conversion
* `<xer/stdio.h>` handles stream I/O and file-entry operations
* `<xer/stdlib.h>` handles multibyte conversion and related utility facilities
* encoding policy explains the broader text-encoding model behind text streams

---

## Documentation Notes

When this header is used in generated documentation, it is usually enough to explain:

* that XER distinguishes `binary_stream` and `text_stream`
* that text encodings are explicit rather than locale-driven
* that stream objects are move-only RAII types
* that both low-level I/O and higher-level facilities such as formatted I/O and CSV are included
* that path-oriented file-entry operations are part of the header
* that `realpath` is filesystem-dependent and distinct from lexical path operations
* that `stream_get_contents` reads from the current stream position and intentionally does not provide an offset argument
* that `file_get_contents` and `file_put_contents` are convenience APIs whose binary/text behavior is selected by the presence of an encoding argument
* that `file_get_contents` and `file_put_contents` are file-opening wrappers around the stream-level content helpers

Detailed per-function semantics should be described in the reference manual or generated API sections.

---

## Example Topics Commonly Worth Showing

The following kinds of examples are especially suitable for this header:

* opening a binary file and reading bytes
* opening a UTF-8 text stream and reading text
* writing text with `puts` or `fputs`
* using `fgetpos` / `fsetpos`
* using `tmpfile`
* reading or writing CSV
* performing `rename`, `remove`, or `copy`
* changing and restoring the current working directory with `chdir` and `getcwd`
* canonicalizing an existing path with `realpath`
* reading and writing already-open streams with `stream_get_contents` and `stream_put_contents`
* reading and writing whole files with `file_get_contents` and `file_put_contents`

These are good candidates for executable examples under `examples/`.

---

## Example

```cpp
#include <xer/stdio.h>

auto main() -> int
{
    if (!xer::puts(u8"hello").has_value()) {
        return 1;
    }

    return 0;
}
```

This example shows the basic XER style:

* use XER text I/O directly
* work with UTF-8-oriented text
* check `xer::result` explicitly

---

## See Also

* `policy_project_outline.md`
* `policy_stdio.md`
* `policy_encoding.md`
* `header_path.md`
* `header_stdlib.md`


---
