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

### Character and String Processing

- `xer/string.h`
- `xer/ctype.h`
- `xer/stdlib.h`

### Data Format Processing

- `xer/json.h`

### Input/Output and File-Related Facilities

- `xer/stdio.h`
- `xer/path.h`
- `xer/socket.h`

### Numeric and Arithmetic Facilities

- `xer/stdint.h`
- `xer/stdfloat.h`
- `xer/arithmetic.h`
- `xer/cyclic.h`
- `xer/quantity.h`

### Process

- `xer/process.h`

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

### Why `quantity.h` Is Independent

Physical quantity and unit facilities are a kind of type-system-oriented feature distinct from string handling, input/output, and arithmetic helpers.

Also, concepts such as `dimension`, `unit`, `quantity`, and `xer::units` are easier for users to understand when treated as one coherent group.

For that reason, physical quantity and unit functionality is not absorbed into `xer/arithmetic.h`, but is provided as the independent public header `xer/quantity.h`.

---

## Final List of Public Headers

```text
xer/error.h
xer/assert.h
xer/typeinfo.h
xer/diag.h
xer/string.h
xer/ctype.h
xer/stdlib.h
xer/json.h
xer/stdio.h
xer/path.h
xer/socket.h
xer/stdint.h
xer/stdfloat.h
xer/arithmetic.h
xer/cyclic.h
xer/quantity.h
xer/process.h
xer/time.h
xer/version.h
