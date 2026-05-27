# `<xer/zip.h>`

## Purpose

`<xer/zip.h>` provides ZIP archive reading and writing facilities in XER.

ZIP is technically an archive format, but in practical use it is also a familiar compression and expansion format. XER treats the initial ZIP API as a small compression-and-archive utility that can later support serialized data packages, bundled resources, and ordinary file exchange.

The initial API is intentionally small. It supports sequential reading and simple archive creation. Extraction helpers, name lookup, comments, and ZIP64 support are deferred.

---

## External Dependency

`<xer/zip.h>` requires zlib development headers and the zlib library.

The public header checks for `<zlib.h>` with `__has_include` when available and emits a compile-time diagnostic if the header is missing. Programs using this header must also link with zlib, for example `-lz` on typical Unix-like environments.

The project test runner detects `xer/zip.h` as the `zip` feature and links matching tests and examples with zlib when it is available.

---

## Main Role

The main role of `<xer/zip.h>` is to make it possible to:

- open a ZIP archive
- read entry metadata sequentially
- obtain entry names, sizes, and compression method names
- read and expand entry data
- create a ZIP archive
- add in-memory bytes or a source file as deflated entries
- explicitly commit the writer so finalization errors can be reported
- report archive end through XER's ordinary error model

This design avoids returning `std::optional` for archive end. Reaching the end of the entry sequence is reported as:

```cpp
error_t::end_of_file
```

This keeps `zip_read` consistent with other sequential input operations in XER.

---

## Main Entities

At minimum, `<xer/zip.h>` provides the following types and functions:

```cpp
class zip_archive;
class zip_entry;

auto zip_open(std::u8string_view filename) -> xer::result<zip_archive>;

auto zip_create(std::u8string_view filename) -> xer::result<zip_archive>;

auto zip_read(zip_archive& archive) -> xer::result<zip_entry>;

auto zip_entry_name(const zip_entry& entry) -> xer::result<std::u8string>;

auto zip_entry_filesize(const zip_entry& entry) -> xer::result<std::uint64_t>;

auto zip_entry_compressed_size(const zip_entry& entry)
    -> xer::result<std::uint64_t>;

auto zip_entry_compression_method(const zip_entry& entry)
    -> xer::result<std::u8string>;

auto zip_entry_read(zip_archive& archive, const zip_entry& entry)
    -> xer::result<std::vector<std::byte>>;

auto zip_add_from_bytes(
    zip_archive& archive,
    std::u8string_view entry_name,
    std::span<const std::byte> data) -> xer::result<void>;

auto zip_add_file(
    zip_archive& archive,
    std::u8string_view source_path,
    std::u8string_view entry_name) -> xer::result<void>;

auto zip_commit(zip_archive& archive) -> xer::result<void>;
```

All public operations return `xer::result`, including metadata accessors that are not expected to fail in ordinary cases. This preserves API symmetry and leaves room for future validation, conversion, or backend changes.

---

## `zip_archive`

```cpp
class zip_archive;
```

`zip_archive` is a move-only archive handle.

For reading, it owns the underlying binary stream and the central-directory read position. For writing, it owns the output stream and the pending central-directory records. Destruction closes the underlying stream automatically. Copying is disabled.

Callers normally obtain this object through `zip_open` for reading or `zip_create` for writing.

When using a writer, callers should explicitly call `zip_commit`. Destruction can close the stream, but it cannot report finalization errors through `xer::result`.

---

## `zip_entry`

```cpp
class zip_entry;
```

`zip_entry` stores metadata for one archive entry read from the central directory.

It is a lightweight value object containing at least:

- entry name
- uncompressed size
- compressed size
- compression method identifier
- flags
- local header offset

Callers should use the public `zip_entry_*` functions rather than depending on internal representation details.

---

## `zip_open`

```cpp
auto zip_open(std::u8string_view filename) -> xer::result<zip_archive>;
```

### Purpose

`zip_open` opens a ZIP archive for reading.

### Input Model

The filename is a UTF-8 path string. Internally, it is converted through XER's path handling before the file is opened.

### Supported Archives

The initial implementation supports ordinary non-ZIP64 archives with a single-disk central directory.

The following are rejected as `error_t::invalid_argument`:

- invalid ZIP files
- missing end-of-central-directory record
- multi-disk archives
- ZIP64 archives
- inconsistent central-directory ranges

### Return Model

On success, the function returns an open `zip_archive`.

On failure, it returns `xer::result` error information. File-opening failures are reported using the ordinary file error model. Format-level failures are generally reported as `error_t::invalid_argument`.

---

## `zip_create`

```cpp
auto zip_create(std::u8string_view filename) -> xer::result<zip_archive>;
```

### Purpose

`zip_create` opens a ZIP archive for writing.

The returned archive is a writer. It is not a complete ZIP file until `zip_commit` writes the central directory and closes the stream.

### Output Model

The initial writer creates ordinary non-ZIP64 single-disk archives. Entry names are stored as UTF-8 names with the ZIP UTF-8 flag set.

---

## `zip_read`

```cpp
auto zip_read(zip_archive& archive) -> xer::result<zip_entry>;
```

### Purpose

`zip_read` reads the next entry metadata from the archive's central directory.

### Sequential Model

The archive stores a current central-directory position. Each successful call advances that position to the next entry.

This avoids building a full entry list in memory. That is important for large archives with many entries.

### End of Archive

When there are no more entries, the function returns:

```cpp
error_t::end_of_file
```

This is an error result, not an empty optional value.

### Unsupported Entry Metadata

The initial implementation rejects encrypted entries, multi-disk entry references, and ZIP64-sized entries as `error_t::invalid_argument`.

---

## `zip_entry_name`

```cpp
auto zip_entry_name(const zip_entry& entry) -> xer::result<std::u8string>;
```

`zip_entry_name` returns the entry name as a UTF-8 string.

The initial implementation accepts entry names that are valid UTF-8. When the ZIP UTF-8 name flag is set and the stored name is not valid UTF-8, the operation fails with `error_t::encoding_error` during `zip_read`.

CP437 name conversion is not implemented yet. Non-UTF-8 names are therefore rejected rather than guessed.

---

## `zip_entry_filesize`

```cpp
auto zip_entry_filesize(const zip_entry& entry) -> xer::result<std::uint64_t>;
```

`zip_entry_filesize` returns the uncompressed entry size in bytes.

The name follows PHP's `zip_entry_filesize` vocabulary while still returning a C++ integer type through `xer::result`.

---

## `zip_entry_compressed_size`

```cpp
auto zip_entry_compressed_size(const zip_entry& entry)
    -> xer::result<std::uint64_t>;
```

`zip_entry_compressed_size` returns the compressed entry size in bytes.

The function name uses snake_case rather than PHP's `zip_entry_compressedsize` spelling because this is a C++ API and readability is preferred where compatibility is not exact.

---

## `zip_entry_compression_method`

```cpp
auto zip_entry_compression_method(const zip_entry& entry)
    -> xer::result<std::u8string>;
```

`zip_entry_compression_method` returns a textual compression method name.

The initial implementation returns:

```text
store
```

for stored entries and:

```text
deflate
```

for deflated entries.

Other method identifiers are returned as:

```text
unknown
```

Reading data for an unsupported method fails with `error_t::invalid_argument`.

---

## `zip_entry_read`

```cpp
auto zip_entry_read(zip_archive& archive, const zip_entry& entry)
    -> xer::result<std::vector<std::byte>>;

auto zip_add_from_bytes(
    zip_archive& archive,
    std::u8string_view entry_name,
    std::span<const std::byte> data) -> xer::result<void>;

auto zip_add_file(
    zip_archive& archive,
    std::u8string_view source_path,
    std::u8string_view entry_name) -> xer::result<void>;

auto zip_commit(zip_archive& archive) -> xer::result<void>;
```

### Purpose

`zip_entry_read` reads and expands one entry body.

The entry must have been obtained from the same archive. The initial API does not attempt to validate cross-archive use.

### Supported Compression Methods

The initial implementation supports:

- stored entries
- deflated entries

Stored entries are returned as-is. Deflated entries are expanded with raw deflate through zlib.

### Output Model

On success, the function returns a `std::vector<std::byte>` containing the uncompressed entry bytes.

This is intentionally an owning byte vector. Streaming entry reads can be added later if large-entry use cases require them.

---

## `zip_add_from_bytes`

```cpp
auto zip_add_from_bytes(
    zip_archive& archive,
    std::u8string_view entry_name,
    std::span<const std::byte> data) -> xer::result<void>;
```

### Purpose

`zip_add_from_bytes` adds one in-memory byte sequence to a ZIP archive writer.

The entry is compressed with raw deflate and written with a local file header. The central-directory record is retained in memory until `zip_commit` is called.

### Limits

The initial writer does not support ZIP64. Therefore the entry name, compressed size, uncompressed size, local header offset, and entry count must fit ordinary ZIP fields.

Entry names must be non-empty valid UTF-8 strings. Non-UTF-8 names fail with `error_t::encoding_error`.

---

## `zip_add_file`

```cpp
auto zip_add_file(
    zip_archive& archive,
    std::u8string_view source_path,
    std::u8string_view entry_name) -> xer::result<void>;
```

### Purpose

`zip_add_file` reads a source file and adds its contents as one ZIP entry.

This is a convenience wrapper around `file_get_contents` and `zip_add_from_bytes`. The initial implementation reads the whole source file into memory before compression. Streaming file-to-ZIP output can be added later if large-file use cases require it.

---

## `zip_commit`

```cpp
auto zip_commit(zip_archive& archive) -> xer::result<void>;
```

### Purpose

`zip_commit` finalizes a ZIP archive writer.

It writes the central directory and the end-of-central-directory record, flushes the stream, and closes it. Because errors can occur at finalization time, callers should treat this as a required step for writer archives.

Calling write operations after `zip_commit` fails with `error_t::invalid_argument`.

---

## Error Handling

`<xer/zip.h>` follows XER's ordinary failure model.

That means:

- normal failure is reported through `xer::result`
- archive end is reported as `error_t::end_of_file`
- malformed archives and invalid operation order are reported as `error_t::invalid_argument`
- invalid entry-name encoding is reported as `error_t::encoding_error`
- stream failures are reported through ordinary file or I/O errors

The initial implementation does not provide detailed ZIP parse positions. If position-aware diagnostics become useful later, an error-detail type can be added separately.

---

## Deferred Items and Limitations

The following items are intentionally deferred:

- file extraction helpers
- name lookup
- entry comments and archive comments
- ZIP64
- multi-disk archives
- encrypted entries
- CP437 filename conversion
- streaming entry-body reads
- stored-entry write option
- data-descriptor-oriented write support
- CRC verification as a public option
- streaming file-to-ZIP writes

The first goal is a small, predictable, PHP-inspired ZIP reader and writer that fits XER's `xer::result` and sequential EOF model.
