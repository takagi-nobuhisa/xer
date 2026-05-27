# Release Notes for xer v0.6.0b1

## Overview

v0.6.0b1 is the first beta of the v0.6.0 series.

This beta focuses on two new functional areas:

- ZIP archive utilities through `<xer/zip.h>`
- fixed-schema binary serialization through `<xer/serialize.h>` and the PHP `xfer` generator

The release also finalizes the common EOF handling policy introduced for sequential input APIs.

---

## ZIP Archive Utilities

`<xer/zip.h>` provides ordinary ZIP archive handling as a compression-and-archive facility.

The v0.6.0b1 scope includes:

- opening ZIP archives
- sequential entry metadata reading
- exact-name entry lookup
- whole-entry reads
- archive creation
- adding in-memory byte data as deflated entries
- adding source files as deflated entries
- explicit writer finalization with `zip_commit`
- extracting a single entry to a chosen file
- extracting a single entry below a destination directory
- extracting all entries below a destination directory

The initial ZIP implementation targets ordinary non-ZIP64 single-disk archives. ZIP64, encrypted entries, archive comments, multi-disk archives, and streaming large-entry I/O are deferred.

---

## Fixed-Schema Binary Serialization

`<xer/serialize.h>` provides low-level binary transfer archives:

- `xer::binary_output_archive`
- `xer::binary_input_archive`

The format is intentionally not self-describing. It stores data only, using fixed field order and little-endian scalar representation.

Supported low-level field types include:

- `bool`
- fixed-width signed and unsigned integers
- `float`
- `double`
- `std::u8string`
- `std::vector<std::byte>`
- `std::array<T, N>`
- `std::vector<T>`
- `std::map<K, V>`

User-defined structures are handled through generated `xfer` functions rather than reflection.

---

## PHP `xfer` Generator

`php/generate_xfer_struct.php` generates C++ structures and `xfer` functions from PHP schema arrays.

The generator supports:

- single-structure schemas
- multiple-structure schemas
- compact type tokens such as `u32`, `s`, `bin`, `v`, `m`, and `[a, N]`
- generated-at schema timestamp constants
- `--generated-at=<text>` for reproducible generated examples and tests
- include minimization based on used field types
- schema validation for identifiers, namespaces, and supported type forms

---

## Common EOF Handling

Sequential input APIs now use `error_t::end_of_file` when the next item cannot be read because the input is exhausted.

This keeps EOF distinct from `error_t::not_found`, which is reserved for named or keyed lookup failure.

Examples:

- `zip_read` returns `end_of_file` when no more entries exist.
- `zip_locate_name` returns `not_found` when a named entry does not exist.
- stream and directory sequential reads use `end_of_file` for exhausted input.

---

## Documentation and Examples

v0.6.0b1 includes reference-manual coverage and executable examples for:

- ZIP reading
- ZIP creation
- ZIP name lookup
- ZIP extraction
- hand-written `xfer`
- generated single-structure serialization
- generated multiple-structure serialization

The reference manual target version has been updated to v0.6.0b1.
