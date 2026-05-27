# `<xer/serialize.h>`

## Purpose

`<xer/serialize.h>` provides low-level binary transfer archives for fixed-schema data.

The design is intentionally simple. It does not store type names, field names, schema information, object identifiers, or version metadata. Callers and generated code are expected to know the exact field order and field types.

This header is intended to support generated `xfer` functions. A schema generator can emit one field-transfer function and use it for both output and input by passing either `binary_output_archive` or `binary_input_archive`.

The recommended workflow is:

1. define a fixed structure schema in PHP
2. generate a C++ structure and its `xfer` function
3. pass that structure to a binary output archive or binary input archive
4. handle format versioning outside the low-level archive layer

---

## Design Model

xer serialization is not a reflection-based serializer.

C++23 has no standardized reflection facility that can enumerate the fields of an arbitrary user-defined structure. Instead of relying on macros, runtime registration, intrusive base classes, or heavy template metaprogramming, xer uses generated field-transfer functions.

The low-level archive layer only knows how to transfer scalar values and selected standard containers. User-defined structures are handled by generated code that calls the archive once for each field in fixed order.

This keeps the binary format compact and predictable.

---

## Binary Format Policy

The binary format is fixed as follows:

- scalar values are stored directly
- multi-byte scalar values are little-endian
- `float` is stored as IEEE 754 binary32 bytes
- `double` is stored as IEEE 754 binary64 bytes
- `bool` is stored as one byte, `0` or `1`
- `std::u8string` is stored as `uint64 byte_size` followed by UTF-8 bytes
- `std::vector<std::byte>` is stored as `uint64 byte_size` followed by raw bytes
- `std::array<T, N>` stores only its elements; the array size is not stored
- `std::vector<T>` stores `uint64 element_count` followed by elements
- `std::map<K, V>` stores `uint64 element_count` followed by key/value pairs in map iteration order

No byte-order marker is written. Little-endian is part of the format.

The format is suitable when both sides share the same schema or are generated from the same schema source. It is intentionally not a self-describing interchange format.

---

## Main Entities

```cpp
class binary_output_archive;
class binary_input_archive;
```

The archive objects expose `operator()` instead of separate `write` and `read` function names. This allows generated code to use a single transfer function for both directions.

```cpp
template<class Archive>
auto xfer(Archive& archive, sample& value) -> xer::result<void>
{
    if (auto r = archive(value.id); !r) {
        return r;
    }
    if (auto r = archive(value.name); !r) {
        return r;
    }
    return {};
}
```

When `archive` is a `binary_output_archive`, the values are written. When it is a `binary_input_archive`, the values are read into the same fields.

---

## Supported Types

The initial low-level implementation directly supports the following types:

```cpp
bool

std::uint8_t
std::uint16_t
std::uint32_t
std::uint64_t

std::int8_t
std::int16_t
std::int32_t
std::int64_t

float
double

std::u8string
std::vector<std::byte>

std::array<T, N>
std::vector<T>
std::map<K, V>
```

The container element types must themselves be supported by the same archive.

Environment-dependent integer types such as `int`, `long`, and `std::size_t` are intentionally not provided as direct serialization types. Fixed-width integer types should be used in serialized structures.

Non-owning types such as `std::u8string_view` and `std::span` are not intended to be generated as structure fields. They may appear as output-side convenience arguments where explicitly supported, but serialized structures should use owning field types.

---

## `binary_output_archive`

```cpp
class binary_output_archive;
```

`binary_output_archive` owns an internal byte buffer and appends serialized data to it.

```cpp
auto bytes() const noexcept -> std::span<const std::byte>;
auto release() noexcept -> std::vector<std::byte>;
```

`bytes` returns a non-owning view of the current buffer. The view is invalidated by later writes or by `release`.

`release` moves out the accumulated byte sequence and clears the archive.

The archive provides `operator()` overloads for supported output types.

```cpp
xer::binary_output_archive out;
std::uint32_t id = 42;
std::u8string name = u8"xer";

out(id);
out(name);
```

Output functions return `xer::result<void>` for symmetry and to report allocation or size-related failures.

---

## `binary_input_archive`

```cpp
class binary_input_archive;
```

`binary_input_archive` reads serialized data from a byte span.

```cpp
explicit binary_input_archive(std::span<const std::byte> data) noexcept;

auto remaining_size() const noexcept -> std::size_t;
auto empty() const noexcept -> bool;
```

The archive provides `operator()` overloads for supported input types.

```cpp
xer::binary_input_archive in(bytes);
std::uint32_t id{};
std::u8string name;

in(id);
in(name);
```

Input functions store the result into the supplied reference and return `xer::result<void>`.

When the input does not contain enough bytes for the requested value, the function returns:

```cpp
error_t::end_of_file
```

Invalid bool values and duplicate map keys in serialized input are reported as:

```cpp
error_t::invalid_argument
```

Excessively large length values are reported as:

```cpp
error_t::length_error
```

---

## Hand-Written `xfer` Functions

A hand-written transfer function should call the archive for each field in fixed order.

```cpp
struct sample {
    std::uint32_t id;
    std::u8string name;
    std::vector<std::uint16_t> flags;
};

template<class Archive>
auto xfer(Archive& archive, sample& value) -> xer::result<void>
{
    if (auto r = archive(value.id); !r) {
        return r;
    }
    if (auto r = archive(value.name); !r) {
        return r;
    }
    if (auto r = archive(value.flags); !r) {
        return r;
    }
    return {};
}
```

The same function is used for output and input:

```cpp
sample source{};
xer::binary_output_archive out;
xfer(out, source);

auto data = out.release();

sample restored{};
xer::binary_input_archive in(data);
xfer(in, restored);
```

This pattern is demonstrated by `examples/example_serialize_basic.cpp`.

---

## Generated Structures and `xfer` Functions

For ordinary use, xer recommends generating structures and `xfer` functions from a schema file instead of writing them by hand.

The script is:

```text
php/generate_xfer_struct.php
```

A schema file is a PHP file that returns an array. The short type tokens are defined by the generator before the schema file is loaded.

```php
<?php

declare(strict_types=1);

return [
    'namespace' => 'demo',
    'struct' => 'record',
    'fields' => [
        'id' => u32,
        'name' => s,
        'payload' => bin,
        'scores' => [[s, f64], m],
        'history' => [u16, v],
        'fixed' => [u32, [a, 4]],
    ],
];
```

The generator command is:

```text
php php/generate_xfer_struct.php schema.php record.hpp
```

For reproducible generated examples or tests, the timestamp can be supplied explicitly:

```text
php php/generate_xfer_struct.php schema.php record.hpp --generated-at=2026-05-27T00:00:00+00:00
```

If `--generated-at` is omitted, the current time is embedded in ISO 8601 format.

The generated header contains:

- `struct record`
- `template<class Archive> auto xfer(Archive& ar, record& value) -> xer::result<void>`
- a generated-at schema timestamp constant

The generated-at timestamp is embedded both in the file comment and in a C++ constant such as:

```cpp
inline constexpr char record_xfer_schema_generated_at[] = "2026-05-27T13:29:13+00:00";
```

This timestamp is not stored in the serialized binary payload. It identifies the generated schema source used to produce the header.

This generated workflow is demonstrated by:

```text
examples/example_serialize_generated_schema.php
examples/example_serialize_generated.hpp
examples/example_serialize_generated.cpp
```

A schema can also generate multiple structures at once:

```php
<?php

declare(strict_types=1);

return [
    'namespace' => 'demo',
    'structs' => [
        'packet_header' => [
            'version' => u16,
            'kind' => u16,
            'sequence' => u32,
        ],
        'sensor_sample' => [
            'id' => u32,
            'name' => s,
            'values' => [f32, v],
            'calibration' => [[s, f64], m],
            'raw' => [u8, [a, 8]],
        ],
    ],
];
```

This emits both structures and one `xfer` function for each structure. The generated header includes only the standard headers required by the selected field types. The multi-structure workflow is demonstrated by:

```text
examples/example_serialize_generated_multi_schema.php
examples/example_serialize_generated_multi.hpp
examples/example_serialize_generated_multi.cpp
```

---

## PHP Schema Type DSL

`php/generate_xfer_struct.php` supports the following scalar tokens:

```text
b

u8 u16 u32 u64

i8 i16 i32 i64

f32 f64

s
bin
```

They map to C++ types as follows:

```text
b   -> bool

u8  -> std::uint8_t
u16 -> std::uint16_t
u32 -> std::uint32_t
u64 -> std::uint64_t

i8  -> std::int8_t
i16 -> std::int16_t
i32 -> std::int32_t
i64 -> std::int64_t

f32 -> float
f64 -> double

s   -> std::u8string
bin -> std::vector<std::byte>
```

Container forms are written as type modifiers:

```php
[T, v]          // std::vector<T>
[T, [a, N]]     // std::array<T, N>
[[K, V], m]     // std::map<K, V>
```

For example:

```php
[u32, v]          // std::vector<std::uint32_t>
[u32, [a, 16]]    // std::array<std::uint32_t, 16>
[[s, f64], m]     // std::map<std::u8string, double>
```

`std::array<T, N>` does not store `N` in the binary payload. `std::vector<T>`, `std::map<K, V>`, `std::u8string`, and `std::vector<std::byte>` store a `uint64` length before their contents.

---

## Versioning Model

This format is not self-describing. If a structure changes across versions, callers should handle compatibility outside the low-level archive layer.

Common strategies include:

- placing a version field at the start of the structure
- wrapping payloads in an outer versioned structure
- generating separate `xfer` functions for old and new layouts
- restricting additions to trailing fields when suitable
- exchanging the schema-generated timestamp out-of-band when that is useful for diagnostics

`<xer/serialize.h>` deliberately keeps this policy outside the low-level archive implementation.

---

## Relationship with ZIP

`<xer/serialize.h>` produces and consumes byte sequences. Those byte sequences can be stored directly, sent over a stream, encoded with Base64, or placed into a ZIP archive.

The serialization layer does not compress data by itself. Compression and archive handling belong to `<xer/zip.h>` or future compression utilities.
