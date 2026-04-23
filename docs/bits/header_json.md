# `<xer/json.h>`

## Purpose

`<xer/json.h>` provides JSON value handling and JSON encode/decode facilities in XER.

Its purpose is not merely to treat JSON as a string-processing convenience.
Instead, it provides a structured representation of JSON data together with parsing and serialization functions.

This header is therefore positioned as a data-format facility rather than as part of the ordinary string utility layer.

---

## Main Role

The main role of `<xer/json.h>` is to make it possible to:

- parse JSON text into a structured XER value model
- inspect and manipulate JSON values in memory
- serialize structured JSON values back into text

This makes the header useful for configuration data, simple structured interchange, and JSON-oriented utility processing.

---

## Main Entities

At minimum, `<xer/json.h>` provides the following entities:

```cpp
struct json_value;

using json_array = json_value::array_type;
using json_object = json_value::object_type;

auto json_decode(std::u8string_view text) -> xer::result<json_value>;
auto json_encode(const json_value& value) -> xer::result<std::u8string>;
```

The exact helper members and constructors of `json_value` may evolve, but this is the core public shape.

---

## `json_value`

`json_value` is the central value type for JSON in XER.

It stores one JSON value in structured form.

### Supported Value Kinds

At minimum, `json_value` can represent the following JSON kinds:

* `null`
* boolean
* number
* string
* array
* object

### Concrete Stored Forms

The value model currently corresponds to the following C++ forms:

* `std::nullptr_t`
* `bool`
* `double`
* `std::u8string`
* array of `json_value`
* object represented as `std::vector<std::pair<std::u8string, json_value>>`

This means that objects preserve insertion order rather than being normalized into an associative container.

### Why This Matters

This design keeps the JSON model lightweight and practical:

* strings fit naturally into XER's UTF-8-oriented text model
* numbers use `double`
* arrays remain straightforward recursive containers
* objects preserve source order naturally

---

## `json_array`

`json_array` is an alias for the array type used by `json_value`.

### Role

This alias exists to make array-oriented code more readable and to avoid exposing the implementation form only indirectly through `json_value`.

### Typical Use

A caller may use `json_array` when constructing or inspecting an array value explicitly.

---

## `json_object`

`json_object` is an alias for the object type used by `json_value`.

### Role

This alias exists to make object-oriented code easier to read.

### Current Representation

A JSON object is represented as an ordered sequence of key/value pairs:

```cpp
std::vector<std::pair<std::u8string, json_value>>
```

### Notes

This means:

* key order is preserved
* the representation is simple and predictable
* behavior should not be assumed to match that of a hash map or tree map automatically

---

## `json_decode`

```cpp id="tjh4ki"
auto json_decode(std::u8string_view text) -> xer::result<json_value>;
```

### Purpose

`json_decode` parses UTF-8 JSON text and returns a structured `json_value`.

### Input Model

The input text is provided as:

```cpp id="6ve0b2"
std::u8string_view
```

This matches XER's general UTF-8-oriented public text policy.

### Return Model

On success, `json_decode` returns a `json_value`.

On failure, it returns an error through `xer::result`.

### Role in the Header

This is the main entry point from text into the in-memory JSON model.

---

## `json_encode`

```cpp id="d05h93"
auto json_encode(const json_value& value) -> xer::result<std::u8string>;
```

### Purpose

`json_encode` serializes a structured `json_value` into UTF-8 JSON text.

### Return Model

On success, it returns:

```cpp id="7v7txm"
std::u8string
```

On failure, it returns an error through `xer::result`.

### Role in the Header

This is the main entry point from the in-memory JSON model back into text.

---

## Accessors and Inspection

`json_value` may provide inspection and accessor functions such as:

```cpp id="2z9586"
is_null
is_bool
is_number
is_string
is_array
is_object

as_bool
as_number
as_string
as_array
as_object
```

### Role of These Functions

These functions allow callers to:

* inspect which JSON kind is currently stored
* retrieve the underlying value in typed form

### Basic Direction

The design is intentionally explicit.

A caller should first know or check what kind of value is present, and then access it accordingly.

---

## Number Representation

Numbers in `json_value` are represented as:

```cpp id="enb0ih"
double
```

### Design Direction

This keeps the implementation simple and aligns with the current project policy.

### Implications

* the JSON number model is handled as floating-point data
* integers are represented through the same numeric storage
* the API should not be read as preserving arbitrary-precision numeric structure

---

## String Representation

JSON strings are represented as:

```cpp id="5bbuxu"
std::u8string
```

### Why This Matters

This matches XER's broader text model:

* public text APIs are UTF-8 oriented
* `char8_t`-based strings are the normal representation
* JSON processing remains consistent with the rest of the library

---

## Object Representation

Objects are represented as an ordered vector of key/value pairs rather than as a map-like container.

### Reasons This Is Reasonable

This design has practical advantages:

* preserves insertion order
* avoids committing the public model to hash-based semantics
* keeps the stored structure straightforward
* aligns well with many JSON-oriented use cases

### Important Note

Documentation should make clear that object behavior is based on ordered pairs, not on implicit map semantics.

---

## Error Handling

`<xer/json.h>` follows XER's ordinary failure model.

That means:

* parsing failure is reported through `xer::result`
* serialization failure is reported through `xer::result`
* normal failure is not expressed through exceptions by default

### Design Direction

This keeps JSON processing aligned with the rest of XER's public APIs.

---

## Relationship to Other Headers

`<xer/json.h>` should be understood together with the following documents and headers:

* `policy_project_outline.md`
* `policy_result_arguments.md`
* `header_string.md`

The rough boundary is:

* `<xer/string.h>` handles string and text utilities
* `<xer/json.h>` handles structured JSON data and JSON text conversion

This separation is intentional.
JSON is treated as an independent data-format facility rather than as a mere extension of string helpers. 

---

## Documentation Notes

When this header is used in generated documentation, it is usually enough to explain:

* that it provides a structured JSON value model
* that `json_decode` parses UTF-8 JSON text
* that `json_encode` serializes the structured model back to UTF-8 text
* that objects preserve insertion order

Detailed grammar and escaping rules belong in the detailed reference or generated API documentation.

---

## Example Topics Commonly Worth Showing

The following kinds of examples are especially suitable for this header:

* decoding a simple JSON object
* decoding a JSON array
* reading values from `json_value`
* encoding a constructed JSON value back into text

These are good candidates for executable examples in `examples/`.

---

## Example

```cpp id="q07rge"
#include <xer/json.h>

auto main() -> int
{
    const auto decoded = xer::json_decode(u8"{\"value\":123}");
    if (!decoded.has_value()) {
        return 1;
    }

    if (!decoded->is_object()) {
        return 1;
    }

    const auto encoded = xer::json_encode(*decoded);
    if (!encoded.has_value()) {
        return 1;
    }

    return 0;
}
```

This example shows the general style:

* parse UTF-8 JSON text with `json_decode`
* inspect the resulting `json_value`
* serialize it again with `json_encode`
* check `xer::result` explicitly at each fallible step

---

## See Also

* `policy_project_outline.md`
* `policy_result_arguments.md`
* `header_string.md`
