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

### Data Format Processing

- `xer/json.h`
- `xer/ini.h`
- `xer/toml.h`

### Input/Output and File-Related Facilities

- `xer/stdio.h`
- `xer/path.h`
- `xer/dirent.h`
- `xer/socket.h`

### Numeric and Arithmetic Facilities

- `xer/stdint.h`
- `xer/stdfloat.h`
- `xer/arithmetic.h`
- `xer/cyclic.h`
- `xer/quantity.h`
- `xer/matrix.h`

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

### Why `quantity.h` Is Independent

Physical quantity and unit facilities are a kind of type-system-oriented feature distinct from string handling, input/output, and arithmetic helpers.

Also, concepts such as `dimension`, `unit`, `quantity`, and `xer::units` are easier for users to understand when treated as one coherent group.

For that reason, physical quantity and unit functionality is not absorbed into `xer/arithmetic.h`, but is provided as the independent public header `xer/quantity.h`.

### Why `matrix.h` Is Independent

Matrix and affine transform facilities form a small but coherent numeric feature group.
They are related to arithmetic helpers, but their main concepts are fixed-size matrices, column vectors, and transform construction rather than scalar arithmetic.

For that reason, matrix functionality is not absorbed into `xer/arithmetic.h`, but is provided as the independent public header `xer/matrix.h`.

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
xer/json.h
xer/ini.h
xer/toml.h
xer/stdio.h
xer/path.h
xer/dirent.h
xer/socket.h
xer/stdint.h
xer/stdfloat.h
xer/arithmetic.h
xer/cyclic.h
xer/quantity.h
xer/matrix.h
xer/process.h
xer/cmdline.h
xer/time.h
xer/version.h
```
