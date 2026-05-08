# Public Headers

## Basic Policy

- Public headers are placed directly under `xer/`.
- Headers under `xer/bits/` are for internal implementation and are not treated as public headers.
- The goal is not to mirror the names of C standard library headers exactly, but to reorganize them into units that are meaningful for XER.
- `wchar.h` and `wctype.h` are not provided as independent headers; their related functionality is absorbed into other headers.
- `math.h` is reserved for future consideration and is not included among the public headers at this time.

---

## Public Headers to Be Provided

### Foundation

- `xer/error.h`
- `xer/assert.h`
- `xer/typeinfo.h`
- `xer/diag.h`
- `xer/scope.h`

### Character and String Processing

- `xer/string.h`
- `xer/ctype.h`
- `xer/stdlib.h`

### Data Encoding and Format Processing

- `xer/bytes.h`
- `xer/base64.h`
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

At least at the current stage, XER does not reimplement the following headers:

- `complex.h`
- `fenv.h`
- `float.h`
- `limits.h`
- `locale.h`
- `signal.h`

---

## Headers Deferred for Later Consideration

- `math.h`

Notes:

- The mathematical function family may be considered in the future according to actual need.
- However, it is not included in the current priority set.

---

## Supplement

### Why `locale.h` Is Not Provided

XER minimizes locale dependence as much as possible and handles character classification, character conversion, and character encoding conversion according to its own library policy.
For that reason, `locale.h` is not placed at the core of the public API.

### Why `wchar.h` and `wctype.h` Are Not Separated

XER does not attempt to mimic the header structure of the C standard library as it is.
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

`xer/iostream.h` provides opt-in iostream insertion and extraction operators for selected XER value types.

XER's ordinary input/output model remains based on `xer/stdio.h`, `binary_stream`, and `text_stream`. However, iostream operators are useful as a bridge for diagnostics, tests, examples, and generic `%@` formatting and scanning support.

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

---

## Final List of Public Headers

```text
xer/error.h
xer/assert.h
xer/typeinfo.h
xer/diag.h
xer/scope.h
xer/string.h
xer/ctype.h
xer/stdlib.h
xer/bytes.h
xer/base64.h
xer/json.h
xer/ini.h
xer/toml.h
xer/stdio.h
xer/iostream.h
xer/path.h
xer/dirent.h
xer/socket.h
xer/stdint.h
xer/stdfloat.h
xer/arithmetic.h
xer/cyclic.h
xer/interval.h
xer/color.h
xer/quantity.h
xer/matrix.h
xer/image.h
xer/process.h
xer/cmdline.h
xer/time.h
xer/version.h
```
