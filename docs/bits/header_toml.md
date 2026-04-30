# `<xer/toml.h>`

## Purpose

`<xer/toml.h>` provides TOML decode and encode facilities in XER.

TOML is treated as a typed configuration data format.
The purpose of this header is to support practical reading and writing of simple UTF-8 TOML text while keeping the implementation small enough to fit XER's incremental development policy.

The initial implementation supports a practical subset of TOML.
It does not claim complete TOML v1.0.0 compatibility.

---

## Main Role

The main role of `<xer/toml.h>` is to make it possible to:

- parse UTF-8 TOML text into a structured XER value model
- inspect booleans, integers, floating-point numbers, strings, arrays, and tables
- serialize the supported value model back into UTF-8 TOML text
- use TOML as a typed configuration format distinct from INI

This makes the header useful for configuration data that needs more structure than INI but does not require the full TOML feature set at the initial stage.

---

## Main Entities

At minimum, `<xer/toml.h>` provides the following entities:

```cpp
struct toml_value;

using toml_array = std::vector<toml_value>;
using toml_table = std::vector<std::pair<std::u8string, toml_value>>;

auto toml_decode(std::u8string_view text) -> xer::result<toml_value>;
auto toml_encode(const toml_value& value) -> xer::result<std::u8string>;
````

The exact helper functions and supported TOML syntax may expand later, but these are the core public entities of the initial TOML facility.

---

## `toml_value`

`toml_value` is the central value type for TOML in XER.

It stores one TOML value in structured form.

### Supported Value Kinds

The initial implementation supports the following value kinds:

* boolean
* integer
* floating-point number
* string
* array
* table

The internal representation corresponds to:

```cpp
std::variant<
    bool,
    std::int64_t,
    double,
    std::u8string,
    toml_array,
    toml_table
>
```

### Notes

* integer values are stored as `std::int64_t`
* floating-point values are stored as `double`
* strings are stored as UTF-8 `std::u8string`
* arrays are stored as `std::vector<toml_value>`
* tables are stored as ordered key-value pairs

---

## `toml_array`

```cpp
using toml_array = std::vector<toml_value>;
```

`toml_array` represents an array of TOML values.

The initial implementation stores arrays as ordinary ordered vectors.

### Notes

* array elements preserve order
* arrays may contain values of different supported kinds
* arrays containing tables are not supported by the initial encoder
* array-of-tables syntax is deferred

---

## `toml_table`

```cpp
using toml_table = std::vector<std::pair<std::u8string, toml_value>>;
```

`toml_table` represents a TOML table.

Tables are represented as ordered key-value pairs rather than as a map-like container.

### Why Ordered Pairs Are Used

This representation keeps the value model simple and preserves source order.

It also matches the general XER tendency to avoid prematurely committing public data-format models to hash-map or tree-map semantics.

### Duplicate Keys

TOML does not allow duplicate keys in the same table.

The decoder rejects duplicate keys as invalid input.

---

## Accessors and Inspection

`toml_value` provides inspection and accessor functions.

At minimum, the following inspection functions are provided:

```cpp
is_bool
is_integer
is_float
is_string
is_array
is_table
```

At minimum, the following accessor functions are provided:

```cpp
as_bool
as_integer
as_float
as_string
as_array
as_table
```

The accessor functions return pointers to the stored value when the value has the requested kind, and `nullptr` otherwise.

### Example

```cpp
const auto* table = value.as_table();
if (table == nullptr) {
    return 1;
}
```

This style keeps type inspection explicit and avoids throwing exceptions for ordinary kind checks.

---

## Supported TOML Subset

The initial implementation supports the following TOML-style input:

```toml
title = "xer"
enabled = true
count = 123
hex = 0xFF
mask = 0b1010_0101
ratio = 1.5
large = 1_000_000
ports = [8000, 8001, 8002]

[project]
name = "xer"
version = "0.2.0a3"
```

### Supported Forms

The initial decoder supports:

* blank lines
* comments beginning with `#`
* end-of-line comments outside strings
* bare keys
* key-value pairs
* ordinary tables
* basic double-quoted strings
* literal strings
* multiline basic and literal strings
* booleans
* signed decimal integers
* hexadecimal, octal, and binary integers
* numeric separators between digits
* finite and special floating-point numbers
* arrays

### Line Endings

The decoder accepts the following line endings:

* LF
* CRLF
* CR

---

## Deferred TOML Features

The following TOML features are intentionally deferred:

* quoted keys
* dotted keys
* nested table syntax such as `[a.b]`
* date and time values
* inline tables
* array-of-tables
* detailed error position reporting

These features may be added later one by one.

The initial implementation should therefore be described as a practical TOML subset, not as a complete TOML implementation.

---

## Keys

The initial implementation supports bare keys only.

A bare key may contain:

* ASCII letters
* ASCII digits
* `_`
* `-`

Examples:

```toml
name = "xer"
build-target = "ucrt64"
version_1 = "0.2.0a3"
```

Quoted keys and dotted keys are not supported in the initial implementation.

---

## Tables

A table line has the following form:

```toml
[project]
```

The table name must be a supported bare key.

The table becomes the destination for subsequent key-value entries until another table is declared.

### Notes

* duplicate table declarations are rejected
* nested table names such as `[a.b]` are deferred
* array-of-tables syntax such as `[[project]]` is deferred

---

## Strings

The implementation supports basic strings, literal strings, multiline basic strings, and multiline literal strings.

```toml
name = "xer"
path = 'C:\\Users\\xer'
description = """
first line
second line"""
literal_block = '''
C:\\Users\\xer
'''
```

Basic strings support the following escapes:

```text
\"
\\
\b
\t
\n
\f
\r
\uXXXX
\UXXXXXXXX
```

Literal strings do not process escape sequences. Multiline strings remove the first newline immediately following the opening delimiter, following TOML-style behavior.

---

## Booleans

The following boolean values are supported:

```toml
enabled = true
disabled = false
```

They are stored as `bool`.

---

## Integers

The initial implementation supports signed decimal integers and prefixed non-decimal integers. Numeric separators may be used between digits.

```toml
count = 123
offset = -5
positive = +123
large = 1_000_000
hex = 0xFF
octal = 0o755
binary = 0b1010_0101
```

They are stored as `std::int64_t`.

The supported integer prefixes are `0x` for hexadecimal, `0o` for octal, and `0b` for binary. A leading `+` or `-` may be used before the prefix. Separators are valid only between digits of the corresponding base.

---

## Floating-Point Numbers

The implementation supports finite decimal floating-point numbers and TOML special floating-point values. Numeric separators may be used between decimal digits of finite decimal values.

```toml
ratio = 1.5
scale = 1e-3
large = 1_000.25_5
positive = +1.5
negative = -1.5
positive_inf = inf
negative_inf = -inf
not_a_number = nan
```

They are stored as `double`.

Special values `inf`, `+inf`, `-inf`, `nan`, `+nan`, and `-nan` are accepted. Separators are valid only between digits of finite decimal forms; forms such as `1_.0`, `1._0`, and `1.0e_3` are rejected.

---

## Arrays

The initial implementation supports arrays.

```toml
ports = [8000, 8001, 8002]
mixed = ["xer", true, 3]
```

Arrays preserve element order.

The implementation stores arrays as `toml_array`.

### Notes

* arrays can contain supported scalar values and arrays
* arrays containing tables are not supported by the initial encoder
* array-of-tables syntax is deferred

---

## Comments

A `#` starts a comment when it appears outside a string.

```toml
name = "xer" # comment
```

A `#` inside a string is treated as part of the string.

```toml
name = "x#r"
```

---

## `toml_decode`

```cpp
auto toml_decode(std::u8string_view text) -> xer::result<toml_value>;
```

### Purpose

`toml_decode` parses UTF-8 TOML text and returns a `toml_value`.

### Input Model

The input text is provided as:

```cpp
std::u8string_view
```

This follows XER's UTF-8-oriented public text model.

### Return Model

On success, `toml_decode` returns a `toml_value`.

The returned value is a table value.

### Failure Conditions

At minimum, decoding fails when:

* the input contains invalid UTF-8
* a key-value line does not contain `=`
* a key is malformed
* a table declaration is malformed
* a key is duplicated
* a table is duplicated
* a value uses unsupported syntax
* a value is malformed

Invalid UTF-8 is treated as an encoding error.
Malformed TOML structure is treated as an invalid argument.

---

## `toml_encode`

```cpp
auto toml_encode(const toml_value& value) -> xer::result<std::u8string>;
```

### Purpose

`toml_encode` serializes a supported TOML value model into UTF-8 TOML text.

### Input Model

The value to encode must be a table value.

### Output Model

On success, it returns:

```cpp
std::u8string
```

The generated text uses `\n` line endings.

### Serialization Form

The encoder emits ordinary key-value entries first.

```toml
title = "sample"
enabled = true
count = 123
```

Then it emits table sections.

```toml
[project]
name = "xer"
version = "0.2.0a3"
```

A blank line is inserted before a table when there is preceding output.

### Representability

Because the initial TOML implementation supports only a subset, `toml_encode` rejects values that cannot be represented by that subset.

For example, it rejects:

* a top-level value that is not a table
* unsupported key forms
* nested tables beyond the initial section model
* arrays containing tables
* invalid UTF-8 strings
* non-finite floating-point values

---

## Error Handling

`<xer/toml.h>` follows XER's ordinary failure model.

That means:

* parsing failure is reported through `xer::result`
* serialization failure is reported through `xer::result`
* normal failure is not expressed through exceptions by default

The usual pattern is:

```cpp
const auto decoded = xer::toml_decode(text);
if (!decoded.has_value()) {
    return 1;
}
```

This follows XER's general policy that fallible public APIs return `xer::result`, while ordinary public APIs should accept ordinary values rather than `xer::result` arguments.

---

## Relationship to Other Headers

`<xer/toml.h>` should be understood together with the following documents and headers:

* `policy_project_outline.md`
* `policy_result_arguments.md`
* `policy_encoding.md`
* `header_string.md`
* `header_json.md`
* `header_ini.md`

The rough boundary is:

* `<xer/string.h>` handles general string and text utilities
* `<xer/json.h>` handles JSON as a structured data format
* `<xer/ini.h>` handles INI as a simple string-valued configuration data format
* `<xer/toml.h>` handles TOML as a typed configuration data format

This separation is intentional.
TOML is treated as an independent data-format facility rather than as a string helper or stream helper.

---

## Documentation Notes

When this header is used in generated documentation, it is usually enough to explain:

* that TOML processing uses UTF-8 text
* that the initial implementation is a practical TOML subset
* that top-level decode results are table values
* that TOML values are typed
* that duplicate keys and duplicate tables are rejected
* that hexadecimal, octal, and binary integers are supported
* that numeric separators are supported only between digits
* that deferred TOML features are not yet supported

Detailed feature coverage should be kept in sync with the implementation as support expands.

---

## Example Topics Commonly Worth Showing

The following kinds of examples are especially suitable for this header:

* decoding simple TOML text
* reading a value from the top-level table
* reading a value from a section table
* encoding a `toml_value` table back into TOML text

These are good candidates for executable examples in `examples/`.

---

## Example

```cpp
#include <xer/toml.h>

auto main() -> int
{
    const auto decoded = xer::toml_decode(
        u8"title = \"sample\"\n"
        u8"enabled = true\n"
        u8"\n"
        u8"[project]\n"
        u8"name = \"xer\"\n"
        u8"version = \"0.2.0a3\"\n");

    if (!decoded.has_value()) {
        return 1;
    }

    const auto* root = decoded->as_table();
    if (root == nullptr) {
        return 1;
    }

    const auto encoded = xer::toml_encode(*decoded);
    if (!encoded.has_value()) {
        return 1;
    }

    return 0;
}
```

This example shows the general style:

* parse UTF-8 TOML text with `toml_decode`
* inspect the resulting top-level table
* serialize it again with `toml_encode`
* check `xer::result` explicitly at each fallible step

---

## See Also

* `policy_project_outline.md`
* `policy_result_arguments.md`
* `policy_encoding.md`
* `header_json.md`
* `header_ini.md`
