# `<xer/ini.h>`

## Purpose

`<xer/ini.h>` provides INI decode and encode facilities in XER.

INI is treated as a small configuration-file format rather than as a general structured data language.
The purpose of this header is to support practical reading and writing of simple UTF-8 INI text while preserving the parts of the format that are important for ordinary configuration files.

The initial implementation deliberately supports a small and predictable subset.
It does not try to define every INI dialect.

---

## Main Role

The main role of `<xer/ini.h>` is to make it possible to:

- parse UTF-8 INI text into a simple ordered in-memory representation
- inspect global entries and section entries
- serialize that representation back into UTF-8 INI text
- preserve duplicate keys and duplicate sections instead of silently discarding information

This makes the header useful for simple configuration files and for data that does not require the stricter type system of formats such as TOML.

---

## Main Entities

At minimum, `<xer/ini.h>` provides the following entities:

```cpp
struct ini_entry;
struct ini_section;
struct ini_file;

auto ini_decode(std::u8string_view text) -> xer::result<ini_file, parse_error_detail>;
auto ini_encode(const ini_file& value) -> xer::result<std::u8string>;
````

The exact helper functions may expand later, but these are the core public entities of the initial INI facility.

---

## `ini_entry`

```cpp
struct ini_entry {
    std::u8string key;
    std::u8string value;
};
```

`ini_entry` represents one key-value pair.

INI does not have a single strong standard type system.
For that reason, both keys and values are represented as UTF-8 strings.

### Notes

* keys are stored after ASCII whitespace trimming during decoding
* values are stored after ASCII whitespace trimming during decoding
* duplicate keys are preserved
* the value is not interpreted as a number, boolean, string literal, or other typed value

---

## `ini_section`

```cpp
struct ini_section {
    std::u8string name;
    std::vector<ini_entry> entries;
};
```

`ini_section` represents one section and its entries.

Section names are stored as UTF-8 strings.
The section's entries are stored in source order.

### Notes

* duplicate section names are preserved
* sections are not automatically merged
* section entries preserve their order
* an empty section is valid

---

## `ini_file`

```cpp
struct ini_file {
    std::vector<ini_entry> entries;
    std::vector<ini_section> sections;
};
```

`ini_file` represents a decoded INI document.

Entries before the first section are stored in `entries`.
Sectioned entries are stored in `sections`.

### Why Global Entries Are Separate

Many INI files allow key-value entries before the first section.
XER represents these entries explicitly instead of inventing an artificial section name.

### Preservation Policy

The representation preserves:

* global entry order
* section order
* entry order within each section
* duplicate keys
* duplicate sections

This is intentional.
INI has many dialects, and silently overwriting or merging data can lose information.

---

## Supported INI Subset

The initial implementation supports the following INI elements:

```ini
; comment
# comment

key = value

[section]
name = xer
version = 0.2.0a3
```

### Supported Forms

* blank lines
* full-line comments beginning with `;`
* full-line comments beginning with `#`
* global key-value entries
* section headers
* section key-value entries

### Line Endings

The decoder accepts the following line endings:

* LF
* CRLF
* CR

---

## Whitespace Handling

During decoding, leading and trailing ASCII whitespace is removed from:

* each logical line
* section names
* keys
* values

Only ASCII whitespace is treated as trimming whitespace.

This keeps the rule simple and avoids locale dependence.

---

## Comments

Only full-line comments are recognized.

A line is treated as a comment when its first non-trimmed character is either:

```text
;
#
```

Inline comments are not recognized.

For example:

```ini
name = xer ; this remains part of the value
```

is decoded as the value:

```text
xer ; this remains part of the value
```

This rule avoids guessing dialect-specific inline-comment behavior.

---

## Sections

A section line has the following form:

```ini
[section]
```

The section name is trimmed as ASCII whitespace.

For example:

```ini
[ main ]
```

is decoded as the section name:

```text
main
```

### Invalid Section Lines

A section line is invalid if:

* the closing `]` is missing
* the section name is empty after trimming

Such cases are reported as ordinary parse failures.

---

## Entries

An entry line has the following form:

```ini
key=value
```

The first `=` separates the key and value.

For example:

```ini
path = a=b
```

is decoded as:

* key: `path`
* value: `a=b`

### Invalid Entry Lines

An entry line is invalid if:

* it does not contain `=`
* the key is empty after trimming

Such cases are reported as ordinary parse failures.

---

## Quoting and Escaping

The initial INI implementation does not interpret quotes or escape sequences.

For example:

```ini
name = "xer"
```

is decoded as the value:

```text
"xer"
```

including the quote characters.

This is intentional.
INI dialects differ widely in quoting and escaping behavior.
XER keeps the initial INI feature small and predictable, and leaves typed or strongly escaped configuration syntax to formats such as TOML.

---

## `ini_decode`

```cpp
auto ini_decode(std::u8string_view text) -> xer::result<ini_file, parse_error_detail>;
```

### Purpose

`ini_decode` parses UTF-8 INI text and returns an `ini_file`.

### Input Model

The input text is provided as:

```cpp
std::u8string_view
```

This follows XER's UTF-8-oriented public text model.

### Return Model

On success, `ini_decode` returns:

```cpp
ini_file
```

On failure, it returns an error through `xer::result`.

### Failure Conditions

At minimum, decoding fails when:

* the input contains invalid UTF-8
* a section line is malformed
* a section name is empty
* an entry line has no `=`
* an entry key is empty

Invalid UTF-8 is treated as an encoding error.
Malformed INI structure is treated as an invalid argument.

---

## `ini_encode`

```cpp
auto ini_encode(const ini_file& value) -> xer::result<std::u8string>;
```

### Purpose

`ini_encode` serializes an `ini_file` into UTF-8 INI text.

### Output Model

On success, it returns:

```cpp
std::u8string
```

The generated text uses `\n` line endings.

### Serialization Form

The encoder emits a compact deterministic form.

Global entries are written first:

```ini
key=value
```

Sections are then written as:

```ini
[section]
key=value
```

A blank line is inserted before each section when there is previous output.

### Representability

Because the initial INI subset has no quoting or escaping rules, the encoder rejects values that cannot be represented without changing their meaning when decoded again.

For example, the encoder rejects:

* empty keys
* keys containing `=`
* keys with leading or trailing ASCII whitespace
* values with leading or trailing ASCII whitespace
* keys, values, or section names containing line breaks
* section names containing `]`
* invalid UTF-8

This keeps `ini_encode` honest and avoids silently emitting ambiguous INI text.

---

## Error Handling

`<xer/ini.h>` follows XER's ordinary failure model.

That means:

* parsing failure is reported through `xer::result`
* serialization failure is reported through `xer::result`
* normal failure is not expressed through exceptions by default

The usual pattern is:

```cpp
const auto decoded = xer::ini_decode(text);
if (!decoded.has_value()) {
    return 1;
}
```

---

## Relationship to Other Headers

`<xer/ini.h>` should be understood together with the following documents and headers:

* `policy_project_outline.md`
* `policy_result_arguments.md`
* `policy_encoding.md`
* `header_string.md`
* `header_json.md`

The rough boundary is:

* `<xer/string.h>` handles general string and text utilities
* `<xer/json.h>` handles JSON as a structured data format
* `<xer/ini.h>` handles INI as a simple configuration data format
* `<xer/toml.h>` handles a stricter typed configuration format

This separation is intentional.
INI is treated as an independent data-format facility rather than as a string helper or stream helper.

---

## Documentation Notes

When this header is used in generated documentation, it is usually enough to explain:

* that INI processing uses UTF-8 text
* that INI values are stored as strings
* that global entries and section entries are separated
* that duplicate keys and duplicate sections are preserved
* that only full-line comments are recognized
* that quotes and escapes are not interpreted in the initial implementation

Detailed dialect-specific behavior should not be implied unless it is actually implemented.

---

## Example Topics Commonly Worth Showing

The following kinds of examples are especially suitable for this header:

* decoding simple INI text
* reading a value from a section
* encoding an `ini_file` back into text
* preserving duplicate keys or duplicate sections where relevant

These are good candidates for executable examples in `examples/`.

---

## Example

```cpp
#include <xer/ini.h>

auto main() -> int
{
    const auto decoded = xer::ini_decode(
        u8"title = sample\n"
        u8"\n"
        u8"[project]\n"
        u8"name = xer\n"
        u8"version = 0.2.0a3\n");

    if (!decoded.has_value()) {
        return 1;
    }

    if (decoded->entries.empty()) {
        return 1;
    }

    if (decoded->sections.empty()) {
        return 1;
    }

    const auto encoded = xer::ini_encode(*decoded);
    if (!encoded.has_value()) {
        return 1;
    }

    return 0;
}
```

This example shows the general style:

* parse UTF-8 INI text with `ini_decode`
* inspect the resulting `ini_file`
* serialize it again with `ini_encode`
* check `xer::result` explicitly at each fallible step

---

## See Also

* `policy_project_outline.md`
* `policy_result_arguments.md`
* `policy_encoding.md`
* `header_json.md`

---

## INI find and load/save helpers

This header also provides ini_find, ini_load, and ini_save.  The find helpers inspect already-decoded in-memory values and return pointers to existing entries or values.  They return `nullptr` when the requested item is not present or when the searched value has the wrong shape.

The load helpers combine UTF-8 file reading with decoding and return `xer::result<..., parse_error_detail>`.  If file I/O fails before parsing begins, the returned error uses `parse_error_reason::none` and leaves `offset`, `line`, and `column` at zero.

The save helpers combine encoding with UTF-8 file writing and return `xer::result<void>`.
